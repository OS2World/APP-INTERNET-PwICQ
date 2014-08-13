/*
 * MSG.C - Administracao da lista de mensagems
 */

#ifdef __IBMC__
 #pragma strings(readonly)
#endif

 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>
 #include <time.h>
 #include <ctype.h>
 #include <icqkernel.h>

 #include "icqcore.h"

/*---[ Definicoes ]------------------------------------------------------------------------------------------*/

 #define FILTER_NONE            0
 #define FILTER_TYPE            1
 #define FILTER_URL             2
 #define FILTER_BADWORD         3
 #define FILTER_BLACKLIST       4


/*---[ Prototipos ]------------------------------------------------------------------------------------------*/

 static void  logMessage(HICQ, HUSER, ULONG, BOOL, ICQMSG *);
 static void  messageTitle(HICQ, HUSER, ULONG, BOOL, ICQMSG *, char *, int);
 static int   dispatchMsg(HICQ, HUSER, MQUEUE *, unsigned char *);
 static void  icqSendAutoRefuse(HICQ, ULONG);
 static HUSER checkForTemporaryUser(HICQ,ULONG,const MSGMGR *);
 static void  ajustSharedBlock(HICQ);


/*---[ Constantes ]------------------------------------------------------------------------------------------*/


/*---[ Implementacao ]---------------------------------------------------------------------------------------*/

 BOOL EXPENTRY icqInsertMessage(HICQ icq, ULONG uin, ULONG sequenceNumber, USHORT type, time_t tm, ULONG flags, int sz, const char *txt)
 {
    /* Anexa uma mensagem na fila correta */
#ifdef EXTENDED_LOG
    char                extLog[80];
#endif

    HMSG                m        = NULL;
    const char          *ignore  = NULL;
    HUSER               usr      = NULL;
    ULONG               filter   = FILTER_NONE;
    BOOL                tempUser = TRUE;
    const MSGMGR        *mgr     = icqQueryMessageManager(icq, type);

    DBGTrace(uin);

    if(sz < 0)
       sz = strlen(txt)+1;

    DBGTrace(sz);
    DBGTrace(strlen(txt));
    DBGTrace(uin);
    DBGMessage(txt);

    if(!mgr && icqLoadValue(icq,"IgnoreUnknown",0))
    {
       icqWriteSysLog(icq,PROJECT,"Unknown messages ignored by configuration");
       return FALSE;
    }

    if(uin > SYSTEM_UIN)
    {
       usr = icqQueryUserHandle(icq,uin);
       DBGTracex(usr);

       if(icqCheckIgnoreList(icq, uin))
       {
          filter = FILTER_BLACKLIST;
       }
       else if(usr)
       {
          /* Usuario ja consta na contact-list */
#ifdef EXTENDED_LOG
          sprintf(extLog,"User Flags: %08lx (Hide: %08lx Temporary: %08lx)",usr->u.flags,usr->u.flags & USRF_HIDEONLIST, usr->u.flags & USRF_TEMPORARY);
          icqWriteSysLog(icq,PROJECT,extLog);
#endif

          if(usr->u.flags & USRF_ONLIST)
             tempUser = FALSE;

          filter   = icqLoadValue(icq,"Filter.K",0);
//          if(icqLoadValue(icq,"badWord.K",0) && searchTextFile( txt, icq->badWords))
          if(icqLoadValue(icq,"badWord.K",0) && icqCheckBadWord(icq, txt))
          {
             /* Badword detectada */
             filter = FILTER_BADWORD;
          }
       }
       else
       {
          /* Usuario nao existe, verifica Filtros especiais */
#ifdef EXTENDED_LOG
          icqWriteSysLog(icq,PROJECT,"Not cached user");
#endif

          filter = icqLoadValue(icq,"Filter.U",0);

          if(type == MSG_URL && icqLoadValue(icq,"ignoreURL",0))
          {
             /* URL de usuario desconhecido, ignora */
             filter = FILTER_URL;
          }
//          else if(icqLoadValue(icq,"badWord.U",0) && searchTextFile( txt, icq->badWords))
          else if(icqLoadValue(icq,"badWord.U",0) && icqCheckBadWord(icq, txt))
          {
             /* Badword detectada */
             filter = FILTER_BADWORD;
          }
          else if(filter == FILTER_NONE || (filter == FILTER_TYPE && type == MSG_NORMAL))
          {
             /* Anexa um usuario temporario */
             usr = checkForTemporaryUser(icq,uin,mgr);
          }
       }
    }
    else
    {
       uin = 0;
    }

    m = addInputMessage(icq,uin,&usr,type,tm,sz,txt,filter == FILTER_NONE);

    DBGTracex(m);

    if(!m)
       return FALSE;

    m->msgFlags |= flags;
    DBGTracex(m->msgFlags);

    m->mgr            = mgr;
    m->sequenceNumber = sequenceNumber;

#ifdef EXTENDED_LOG
    icqDumpPacket(icq, usr, "Received message", strlen( (char *)(m+1) ), (unsigned char *) (m+1));
#endif

    if(mgr && mgr->preProcess)
       mgr->preProcess(icq,(UCHAR *)(m+1),m);

//    DBGMessage((m+1));

    /* Se tenho um conversor instalado, transfiro o texto para ele */
    if(icq->convertCpg )
       icqConvertCodePage(icq, TRUE, (UCHAR *) (m+1), sz);

    if( !ignore )
    {
       /*
        * Verifica se a mensagem e para ser ignorada
        */
       if(isEmpty((const char *) (m+1)))
          m->msgFlags |= MSGF_EMPTY;

       if( (m->msgFlags & MSGF_EMPTY) && icqLoadValue(icq,"ignoreEmpty",0))
          ignore = "Message without text ignored by SPAM protection configuration";

       if(filter == FILTER_BADWORD)
          ignore = "Badword detected";

       if(filter == FILTER_BLACKLIST)
          ignore = "Blacklisted UIN detected";

       if(filter == FILTER_URL)
          ignore = "Message ignored by SPAM protection configuration";

       if(filter == FILTER_TYPE && type != MSG_NORMAL)
          ignore = "Message ignored by SPAM protection configuration (only normal messages accepted)";

       if(m->msgFlags & MSGF_BLACKLIST)
          ignore = "Message Blacklisted by plugin request";

       if(type == MSG_PAGER && icqLoadValue(icq,"IgnorePager",0))
          ignore = "ICQPager Ignored by configuration";

       if(type == MSG_MAIL && icqLoadValue(icq,"IgnoreMail",0))
          ignore = "ICQMail Ignored by configuration";

/*
       if(icqCheckIgnoreList(icq, uin))
          ignore = "Blacklisted UIN detected";
*/
       if(usr && (usr->u.flags & USRF_IGNORE))
          ignore = "Ignored user";

       if(usr && (usr->u.flags & USRF_REJECTED))
          ignore = "Rejected user";

    }

    if(ignore || (m->msgFlags & MSGF_IGNORE) )
    {
       logMessage(icq,usr,uin,FALSE,m);
       icqEvent(icq, uin, 'U', ICQEVENT_IGNORED, (ULONG) (m+1));

       icqRemoveMessage(icq, uin, m);

       if(ignore)
          icqWriteSysLog(icq,PROJECT,ignore);

       if(type == MSG_REQUEST)
       {
          icqSendAutoRefuse(icq, uin);
          return TRUE;
       }

       return FALSE;
    }

    /* Mensagem foi aceita */

    if(type == MSG_REQUEST)
    {
       if(!tempUser && icqLoadValue(icq,"AutoAccept",0) && uin > SYSTEM_UIN)
       {
          m->msgFlags |= MSGF_ACCEPTED;
          icqWriteSysLog(icq,PROJECT,"Sending Auto-accept");
          icqSendMessage(icq, uin, MSG_AUTHORIZED, "");
       }

       if(tempUser && icqLoadValue(icq,"AutoRefuse",0))
       {
          icqWriteSysLog(icq,PROJECT,"Sending Auto-refuse");
          icqSendAutoRefuse(icq, uin);
       }

    }

    /* Atualiza dados do usuario */
    if(usr)
       icqAjustUserIcon(icq,usr,TRUE);

    /* Emite eventos da mensagem */
    if( (m->msgFlags & MSGF_UNHIDE) && usr && !ALWAYS_HIDE_USER(icq,usr) )
       icqUserEvent(icq, usr, ICQEVENT_SHOW);

    CHKPoint();

    if( !(usr && ALWAYS_HIDE_USER(icq,usr)) )
       icqEvent(icq, uin, 'M', m->type, (ULONG) m );

    /* Emite um evento avisando que a fila de mensagems foi mudada */
    if(usr)
    {
       icqUserEvent(icq,usr,ICQEVENT_MESSAGECHANGED);
       icqEvent(icq,uin,'U',ICQEVENT_MESSAGEADDED,(ULONG) m);
    }
    else
    {
       m->msgFlags |= MSGF_SYSTEM;
       icqEvent(icq,uin,'U',ICQEVENT_MESSAGEADDED,(ULONG) m);
    }

    if(m->msgFlags & MSGF_SYSTEM)
    {
       icqSystemEvent(icq, ICQEVENT_MESSAGECHANGED);
    }

    /* Grava o log da mensagem */

    logMessage(icq,usr,uin,FALSE,m);

    /* Verifica se pode abrir direto */

#ifdef EXTENDED_LOG
    if(usr)
    {
       sprintf(extLog,
                "ModeFlag: %08lx  User Flag: %08lx  Open Windows: %d (%lu/%lu)",
                icqQueryModeFlags(icq),
                usr->u.flags,
                usr->u.wndCounter,
                icqQueryModeFlags(icq) & ICQMF_AUTOOPEN,
                usr->u.flags & USRF_AUTOOPEN
              );
       icqWriteSysLog(icq,PROJECT,extLog);
    }
#endif

    if(m->msgFlags & MSGF_OPEN)
    {
#ifdef EXTENDED_LOG
       icqWriteSysLog(icq,PROJECT,"Open message by request");
#endif
       icqOpenMessage(icq, uin, m);

    }
    else if(    usr     && (icqQueryModeFlags(icq) & ICQMF_AUTOOPEN)
                        && (usr->u.flags & USRF_AUTOOPEN)
                        && !usr->u.wndCounter)
    {
#ifdef EXTENDED_LOG
       icqWriteSysLog(icq,PROJECT,"Auto-open message");
#endif
       openUserMessageWindow(icq, usr, FALSE);
    }

    icq->cntlFlags |= ICQFC_SAVEQUEUES;

    ajustSharedBlock(icq);

//    icqQueryFirstMessage(icq, &uin, &msg);

    return TRUE;

 }

 BOOL isEmpty(const char *txt)
 {
    if(!txt)
       return TRUE;

    while(*txt)
    {
       if(!isspace(*txt))
          return FALSE;
       txt++;
    }
/*
    while(*txt && isspace(*txt))
       txt++;
    return !*txt;
*/
    return TRUE;
 }

 HMSG EXPENTRY icqQueryUserMessage(HICQ icq, ULONG uin)
 {
    HUSER usr = icqQueryUserHandle(icq,uin);

    if(usr)
       return icqQueryFirstElement(usr->msg);

    return NULL;
 }


 BOOL EXPENTRY icqRemoveMessage(HICQ icq, ULONG uin, HMSG msg)
 {
    /* Pesquisa pela mensagem nas filas do usuario (Se for valido) e do sistema, se achar remove */
    HUSER       usr     = NULL;
    HMSG        m       = NULL;
    SYSMSG      *s      = NULL;

    if(uin)
       usr = icqQueryUserHandle(icq,uin);

    DBGTracex(usr);

    if(usr)
    {
       /* Pesquisar mensagem na fila do usuario */
       for(m = icqQueryFirstElement(usr->msg); m && m != msg; m = icqQueryNextElement(usr->msg,m));

       if(m)
       {
          if(m->mgr && m->mgr->icon[1])
             usr->u.offlineIcon = m->mgr->icon[1];

          usr->u.idle = 0;
          icqRemoveElement(usr->msg,m);
          icqAjustUserIcon(icq, usr, TRUE);
          icqUserEvent(icq,usr,ICQEVENT_MESSAGECHANGED);
          icq->cntlFlags |= ICQFC_SAVEQUEUES;
          ajustSharedBlock(icq);
          return TRUE;
       }
    }

    /* Pesquisar mensagem na fila do sistema */

    for(s = icqQueryFirstElement(icq->msg); s && &s->m != msg; s = icqQueryNextElement(icq->msg,s));

    DBGTracex(s);

    if(s)
    {
       icqRemoveElement(icq->msg,s);
       icqSystemEvent(icq, ICQEVENT_MESSAGECHANGED);
       icq->cntlFlags |= ICQFC_SAVEQUEUES;
       ajustSharedBlock(icq);
       return TRUE;
    }

    return FALSE;
 }

 char * EXPENTRY icqQueryMessageTitle(HICQ icq, ULONG uin, BOOL out, ICQMSG *m, char *buffer, int sz)
 {
    CHKPoint();
    messageTitle(icq, icqQueryUserHandle(icq,uin),uin,out,m,buffer,sz);
    CHKPoint();
    return buffer;
 }

 static void messageTitle(HICQ icq, HUSER usr, ULONG uin, BOOL out, ICQMSG *m, char *buffer, int sz)
 {
    *buffer = 0;
    if(m)
    {
       DBGTracex(m->mgr);

       if(m->mgr && m->mgr->title)
       {
          CHKPoint();
          strncpy(buffer,m->mgr->title(icq,usr,uin,out,m,buffer,sz),sz);
       }
       else
       {
          sprintf(buffer,"Message %04x ",m->type);
          strncat(buffer,out ? "to " : "from ",0x80);

          if(usr)
             strncat(buffer,(char *)(usr+1),0x80);
          else
             strncat(buffer,"User",0x80);
          CHKPoint();
       }
    }
    DBGMessage(buffer);
 }

 static void logMessage(HICQ icq, HUSER usr, ULONG uin, BOOL out, ICQMSG *m)
 {
    char *buffer = malloc(MAX_MSG_SIZE+2);

    if(!buffer)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when writing message log");
       return;
    }

    CHKPoint();

    *buffer = 0;
    messageTitle(icq,usr,uin,out,m,buffer,0xFF);

    CHKPoint();

    icqWriteSysLog(icq,PROJECT,buffer);

    *buffer = 0;

    if(m->mgr && m->mgr->formatLog)
       m->mgr->formatLog(icq,out,m,buffer,MAX_MSG_SIZE);
    else
       strncpy(buffer,(char *)(m+1),MAX_MSG_SIZE);

    printf("%s\n",buffer);
    fflush(stdout);

    free(buffer);

 }

 int EXPENTRY icqSendMessage(HICQ icq, ULONG uin, USHORT type, unsigned char *txt)
 {
    if(uin > SYSTEM_UIN)
       return sendMessage(icq, uin, type, txt, TRUE);

    icqWriteSysLog(icq,PROJECT,"Invalid UIN when sending message");
    return -1;
 }

 static void logSentMessage(HICQ icq, ULONG uin, USHORT type, const char *txt, int sz)
 {
    ICQMSG      *m   = malloc(sizeof(ICQMSG)+sz+5);
    const char  *src;
    char        *dst;
    int         f;

    if(!m)
    {
       icqWriteSysLog(icq,PROJECT,"Unable to allocate memory for message buffer");
       return;
    }

    m->sz       = sizeof(ICQMSG);
    m->type     = type;
    m->msgTime  = time(NULL);
    m->msgSize  = sz;
    m->mgr      = icqQueryMessageManager(icq, type);

    src         = txt;
    dst         = (char *) (m+1);

    for(f=0;f<sz;f++)
       *(dst++) = *(src++);
    *dst = 0;

    logMessage(icq, icqQueryUserHandle(icq,uin), uin, TRUE, m);

    icqEvent(icq, uin, 'm', m->type, (ULONG) m );

    free(m);
 }


 int sendMessage(HICQ icq, ULONG uin, USHORT type, unsigned char *txt, BOOL log)
 {
    /* Registro a mensagem na fila de sa¡da */
    int         sz      = strlen(txt);
    int         rc      = 0;
    MQUEUE      *msg;
    HUSER       usr     = icqQueryUserHandle(icq,uin);

    if(!txt)
       txt = "";

    if(log)
       logSentMessage(icq,uin,type,txt,sz);

    msg         = icqAddElement(icq->msgQueue,sz + sizeof(MQUEUE) +5 );
    msg->uin    = uin;
    msg->type   = type;
    msg->id     = 0;

    strcpy((char *)(msg+1),txt);

    if(msg->type == MSG_REQUEST)
    {
       if(usr)
       {
          usr->u.flags &= ~USRF_TEMPORARY;
          usr->u.flags |= USRF_WAITING;
          icqAjustUserIcon(icq,usr,TRUE);
       }
    }

    if(usr)
    {
       usr->u.lastAction  =
       usr->u.lastMessage = time(NULL);
    }

    if(!(icq->onlineFlags & ICQF_ONLINE))
    {
       icq->cntlFlags |= ICQFC_SAVEQUEUES;

       if(usr && usr->peer && ((PEERPROTMGR *) usr->peer)->sendMessage)
       {
#ifdef EXTENDED_LOG
          icqWriteSysLog(icq,PROJECT,"Offline, trying peer protocol manager");
#endif
          msg->id = 0;
          rc      = ((PEERPROTMGR *) usr->peer)->sendMessage(icq, usr, msg->type, txt, &msg->id, usr->peerData );
          if(!rc)
          {
              icqRemoveElement(icq->msgQueue,msg);
              return rc;
          }

       }

       if(log)
          icqWriteSysLog(icq,PROJECT,"Can't send when offline, message queued");

       return 98;
    }

    return dispatchMsg(icq, usr, msg, txt);
 }

 static int dispatchMsg(HICQ icq, HUSER usr, MQUEUE *msg, unsigned char *txt)
 {
    int      rc    = 99;

    if(icq->convertCpg)
       icqConvertCodePage(icq, FALSE, txt, -1);

    DBGTracex(usr);
    DBGTracex(usr->c2s);

    CHKPoint();

    if(usr && usr->peer && ((PEERPROTMGR *) usr->peer)->sendMessage)
    {
#ifdef EXTENDED_LOG
       icqWriteSysLog(icq,PROJECT,"Trying peer protocol manager");
#endif
       msg->id = 0;
       rc      = ((PEERPROTMGR *) usr->peer)->sendMessage(icq, usr, msg->type, txt, &msg->id, usr->peerData );
    }

    if(!rc)
    {
       msg->mode = 2;
       if(!msg->id)
          icqRemoveElement(icq->msgQueue,msg);
       icq->cntlFlags |= ICQFC_SAVEQUEUES;
       return rc;
    }

    msg->mode = 1;
    icq->cntlFlags &= ~ICQFC_CLEAR;

    if(usr && usr->c2s && usr->c2s->sendMessage)
    {
#ifdef EXTENDED_LOG
       icqWriteSysLog(icq,PROJECT,"Using user protocol manager");
#endif
       msg->id = 0;
       rc      = usr->c2s->sendMessage(icq, msg->uin, msg->type, txt, &msg->id );
    }
    else if(icq->c2s && icq->c2s->sendMessage)
    {
#ifdef EXTENDED_LOG
       icqWriteSysLog(icq,PROJECT,"Using default protocol manager");
#endif
       msg->id = 0;
       rc      = icq->c2s->sendMessage(icq, msg->uin, msg->type, txt, &msg->id );
    }
    else
    {
       icqWriteSysLog(icq,PROJECT,"No protocol manager when sending message");
       rc = -1;
    }

    if(rc)
       icqWriteSysLog(icq,PROJECT,"Failure sending message");

    if(!(rc||msg->id))
    {
       icq->cntlFlags |= ICQFC_SAVEQUEUES;
       icqRemoveElement(icq->msgQueue,msg);
    }

    return rc;
 }

 int EXPENTRY icqRemoveMessageByID(HICQ icq, ULONG id)
 {
#ifdef EXTENDED_LOG
    char        logBuffer[0x0100];
#endif
    MQUEUE      *msg;

    for(msg = icqQueryFirstElement(icq->msgQueue);msg; msg = icqQueryNextElement(icq->msgQueue,msg))
    {
       if(msg->id == id)
       {
#ifdef EXTENDED_LOG
          sprintf(logBuffer,"Removing message %08lx from queue (sendtype=%d)",id,(int) msg->mode);
          icqWriteSysLog(icq,PROJECT,logBuffer);
#endif
          icqRemoveElement(icq->msgQueue,msg);
          return 0;
       }
    }
    return -1;
 }

 void EXPENTRY icqProcessServerConfirmation(HICQ icq, ULONG uin, ULONG id)
 {
    MQUEUE      *msg;

    for(msg = icqQueryFirstElement(icq->msgQueue);msg && !(msg->id == id && msg->mode == 1); msg = icqQueryNextElement(icq->msgQueue,msg));

    if(msg)
       icqRemoveElement(icq->msgQueue,msg);

    icqSendQueued(icq);
 }

 int EXPENTRY icqSendQueued(HICQ icq)
 {
    /* Verifico se tem outra mensagem para enviar */
    char        *txt;
    MQUEUE      *msg;

    if(!icqIsOnline(icq))
       return 2;

    for(msg = icqQueryFirstElement(icq->msgQueue);msg && !msg->id; msg = icqQueryNextElement(icq->msgQueue,msg));

    if(!msg)
       return 0; // Sem mensagem para enviar

    DBGMessage("Enviando a proxima mensagem da fila");

    txt = strdup((char *)(msg+1));
    if(!txt)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation when sending queued message");
       return 3;
    }

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"Sending queued messages");
#endif

    dispatchMsg(icq, icqQueryUserHandle(icq,msg->uin), msg, txt);

    free(txt);

    return 0;
 }

 BOOL EXPENTRY icqOpenUserMessage(HICQ icq, ULONG uin)
 {
    HUSER       usr = icqQueryUserHandle(icq, uin);
    HMSG        m   = NULL;

    if(usr)
       m = icqQueryFirstElement(usr->msg);

    DBGTracex(m);

    if(m)
       icqOpenMessage(icq, uin, m);

    return m != NULL;

 }

 BOOL EXPENTRY icqOpenSystemMessage(HICQ icq)
 {
    SYSMSG *s = icqQueryFirstElement(icq->msg);
    if(s)
       icqOpenMessage(icq, s->uin, &s->m);

    return s != NULL;
 }

 void EXPENTRY icqOpenMessage(HICQ icq, ULONG uin, HMSG m)
 {
    if(m->mgr && m->mgr->edit )
       m->mgr->edit(icq, uin, FALSE, m, 0, 0);
    else if(icq->skin && icq->skin->view)
       icq->skin->view(icq, uin, m);
    else
       icqWriteSysLog(icq,PROJECT,"Can't determine a valid message viewer");

 }

 const MSGMGR * EXPENTRY icqQueryMessageManager(HICQ icq, USHORT type)
 {
    const MSGMGR **mgr;

    if(icq->msgMgr)
    {
       for(mgr = icq->msgMgr;*mgr;mgr++)
       {
          if( (*mgr)->type == type)
             return *mgr;
       }
    }
	
	if(type & 0x8000)
	   return icqQueryMessageManager(icq,type & 0x7FFF);
	
    return NULL;
 }

 HMSG EXPENTRY icqQueryMessage(HICQ icq, ULONG uin)
 {
    HUSER       usr = uin ? icqQueryUserHandle(icq, uin) : NULL;
    SYSMSG      *s;

    if(usr)
       return icqQueryFirstElement(usr->msg);

    s = icqQueryFirstElement(icq->msg);

    if(s)
       return &s->m;

    return NULL;
 }

 static void ajustSharedBlock(HICQ icq)
 {
    HMSG msg;

    if(!icq->info)
       return;

    icq->info->eventCounter++;

    if(icqQueryFirstMessage(icq,NULL,&msg))
    {
       icq->info->eventIcon = 0xFF;
       return;
    }

    if(msg->mgr)
       icq->info->eventIcon = msg->mgr->icon[0];

 }

 HMSG EXPENTRY icqQueryNextMessage(HICQ icq, ULONG uin, HMSG msg)
 {
    HUSER usr = icqQueryUserHandle(icq,uin);

    if(!usr)
    {
       icqWriteSysLog(icq,PROJECT,"Invalid User-ID when requesting message list");
       return NULL;
    }

    if(!msg)
       return icqQueryFirstElement(usr->msg);

    return icqQueryNextElement(usr->msg, msg);
 }

 int EXPENTRY icqQueryFirstMessage(HICQ icq, ULONG *uin, HMSG *msg)
 {
    HUSER       usr;
    HMSG        uMsg    = NULL;
    HMSG        ufMsg   = NULL;
    SYSMSG      *sMsg;

    if(uin)
       *uin = 0;

    if(msg)
       *msg = NULL;

    CHKPoint();

    for(usr = icqQueryFirstElement(icq->users);usr;usr = icqQueryNextElement(icq->users,usr))
    {
       uMsg = icqQueryFirstElement(usr->msg);
       if(uMsg)
       {
         if(!ufMsg)
         {
            ufMsg = uMsg;
            if(uin)
               *uin = usr->u.uin;
         }
         else if(uMsg->msgTime < ufMsg->msgTime)
         {
            ufMsg = uMsg;
            if(uin)
               *uin = usr->u.uin;
         }
       }
    }

    DBGTracex(ufMsg);

    if(ufMsg)
    {
       if(msg)
          *msg = ufMsg;
       return 0;
    }

    sMsg = icqQueryFirstElement(icq->msg);

    DBGTracex(sMsg);

    if(sMsg)
    {
       if(uin)
          *uin = sMsg->uin;
       if(sMsg)
          *msg = &sMsg->m;
       return 0;
    }

    return -1;
 }

 UCHAR * EXPENTRY icqConvertCodePage(HICQ icq, BOOL in, UCHAR *ptr, int sz)
 {
#ifdef __OS2__
    ULONG  cpIn;
    ULONG  cpOut;

    if(in)      /* Mensagem entrando */
    {
       cpIn  = icq->cpgOut;
       cpOut = icq->cpgIn;
    }
    else        /* Mensagem saindo */
    {
       cpIn  = icq->cpgIn;
       cpOut = icq->cpgOut;
    }

    if(ptr && icq->convertCpg)
       icq->convertCpg(icq,cpIn,cpOut,ptr,sz);
#endif
    return ptr;
 }

 int EXPENTRY icqSendConfirmation(HICQ icq, ULONG uin, USHORT type, BOOL log, char *text)
 {
#ifdef EXTENDED_LOG
    char  logBuffer[0x0100];
#endif
    HUSER usr = icqQueryUserHandle(icq,uin);
    int   rc;

    if(!text)
       text = "";

    if(usr && usr->c2s && usr->c2s->sendConfirmation)
       return usr->c2s->sendConfirmation(icq,uin,type,log,text);

    if(icq->c2s && icq->c2s->sendConfirmation)
       return icq->c2s->sendConfirmation(icq,uin,type,log,text);

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"No protocol manager for the confirmation packet, sending as regular message");
#endif

    rc = sendMessage(icq, uin, type, text, log);

#ifdef EXTENDED_LOG
    sprintf(logBuffer,"Confirmation message %04x send to ICQ#%ld, rc=%d",type,uin,rc);
#endif

    return rc;
 }


 HMSG addInputMessage(HICQ icq, ULONG uin, HUSER *usr, USHORT type, time_t tm, int sz, const char *txt, BOOL addUser)
 {
    SYSMSG              *s      = NULL;
    HMSG                m;
    const MSGMGR        *mgr;
    const char          *src;
    char                *dst;
    BOOL                sysMSG  = FALSE;
    int                 f;

    CHKPoint();

    if(uin > SYSTEM_UIN)
    {
       CHKPoint();

       mgr = icqQueryMessageManager(icq,type);

       DBGTracex(mgr);

       if(mgr && (mgr->flags & ICQMSGMF_SYSTEMMESSAGE) )
          sysMSG = TRUE;
    }
    else
    {
       *usr   = NULL;
       sysMSG = TRUE;
    }

    DBGTracex(*usr);

    if(*usr)
    {
       /* Mesmo que anexe na fila do sistema revalida o usuario */
       (*usr)->u.idle       = 0;
       (*usr)->u.lastAction = time(NULL);

       /* Ajusta flags do usuario de acordo com o tipo da mensagem */
       switch(type)
       {
       case MSG_REFUSED:
          icqUpdateUserFlag(icq, *usr, TRUE,  USRF_TEMPORARY);
          break;

       case MSG_AUTHORIZED:
          icqUpdateUserFlag(icq, *usr, FALSE, USRF_WAITING);
          break;
       }
    }
    else if(!sysMSG && addUser)
    {
#ifdef EXTENDED_LOG
       icqWriteSysLog(icq,PROJECT,"User message - Adding temporary");
#endif
       *usr = icqInsertTemporaryUser(icq, uin);
    }


    /*
     * Anexa a mensagem na fila correta
     */

    if(sysMSG || !*usr)
    {
       /* Anexa na fila do sistema */
       DBGMessage("Anexar mensagem na fila do sistema");
       s = icqAddElement(icq->msg,sz + sizeof(SYSMSG) +5 );
       DBGTracex(s);

       if(!s)
          return NULL;
       s->uin = uin;
       m     = &s->m;

       m->msgFlags |= MSGF_SYSTEM;

    }
    else
    {
       /* Anexa na fila do usuario */

       DBGMessage("Anexar mensagem na fila do usuario");
       DBGTracex(usr);

       (*usr)->u.lastMessage = time(NULL);

       DBGTracex( (*usr)->msg);
       m = icqAddElement( (*usr)->msg, sz + sizeof(ICQMSG) +5 );

       DBGTracex(m);

       if(!m)
          return m;

       if( (*usr)->u.flags & (USRF_HIDEONLIST|USRF_CACHED) )
       {
          (*usr)->u.flags &= ~USRF_CACHED; /* Usuario que tem mensagem na lista deixa de ser cache only */
          m->msgFlags = MSGF_UNHIDE;
       }

//       (*usr)->u.flags            |= USRF_ONLIST;


    }
    m->sz   = sizeof(ICQMSG);
    m->type = type;

    if(tm)
       m->msgTime = tm;
    else
       m->msgTime = time(NULL);

    m->msgSize  = sz;
    src         = txt;
    dst         = (char *) (m+1);

    for(f=0;f<sz;f++)
    {
#ifdef linux
           if(*src == 0x0d)
                  src++;
           else
          *(dst++) = *(src++);
#else
       *(dst++) = *(src++);
#endif
        }
    *dst = 0;

    DBGTracex(m);

    return m;
 }

 static void icqSendAutoRefuse(HICQ icq, ULONG uin)
 {
    char buffer[0x0100];

    icqLoadString(icq, "AutoRefuseMsg", "", buffer, 0xFF);
    icqWriteSysLog(icq,PROJECT,"Sending Auto-refuse");
    icqSendMessage(icq, uin, MSG_REFUSED, buffer);

 }

 static HUSER checkForTemporaryUser(HICQ icq, ULONG uin, const MSGMGR *mgr)
 {

    if(!(uin <= SYSTEM_UIN && mgr))
       return NULL;

    if(mgr->flags & ICQMSGMF_SYSTEMMESSAGE)
    {
#ifdef EXTENDED_LOG
       icqWriteSysLog(icq,PROJECT,"System message - Don't cache user");
#endif
       return NULL;
    }

#ifdef EXTENDED_LOG
	icqPrintLog(icq,PROJECT,"Caching ICQ# %ld",uin);
#endif

    return icqInsertTemporaryUser(icq, uin);
 }

 int EXPENTRY icqNewMultiUserMessage(HICQ icq, USHORT type, const char *text, const char *url)
 {
	if(icq->skin && icq->skin->msgMultDest)
	   return icq->skin->msgMultDest(icq,type,text,url);
	return 1;
 }

 int EXPENTRY icqNewMessageWithManager(HICQ icq, ULONG uin, const MSGMGR *mgr)
 {
    DBGTracex(mgr);

    if(!mgr || mgr->sz != sizeof(MSGMGR))
       return -1;

    CHKPoint();
	if(uin)
       checkForTemporaryUser(icq, uin, mgr);

    DBGTracex(mgr->edit);
    if(mgr->edit)
       return mgr->edit(icq,uin,FALSE,0,0,0);

    CHKPoint();
    if(icq->skin && icq->skin->msgWithMgr)
       return icq->skin->msgWithMgr(icq,uin,mgr,NULL);

#ifdef EXTENDED_LOG
	icqPrintLog(icq,PROJECT,"Failure creating message edit window for ICQ#%ld",uin);
#endif
	
    return 1;

 }


