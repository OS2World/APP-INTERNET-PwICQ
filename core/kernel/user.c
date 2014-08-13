/*
 * user.c - User management
 */

 #define INCL_DOSPROCESS
 #define INCL_WINSHELLDATA
 #include <stdio.h>
 #include <stdlib.h>
 #include <ctype.h>
 #include <malloc.h>
 #include <string.h>
 #include <icqkernel.h>
 #include <time.h>

 #ifndef linux
    #include <io.h>
 #endif

 #include "icqcore.h"

 #define MAX_LINE_SIZE  1024

/*---[ Funcoes estaticas ]---------------------------------------------------------*/

 static char  *insertString(char *, const char *);
 static HUSER insertUser(HICQ, const char *, ICQITEM *);
 static HUSER updateUser(HICQ, HUSER, const char *, const char *, const char *, const char *, const char *, BOOL, BOOL);
 static HUSER insertTemporary(HICQ,ULONG,ULONG,BOOL);
 static int   procAdding(HICQ, HUSER, ULONG);
 static int   deleteUser(HICQ, HUSER);

/*---[ Implementation ]------------------------------------------------------------*/

 HUSER EXPENTRY icqInsertTemporaryUser(HICQ icq, ULONG uin)
 {
    return insertTemporary(icq,uin,0,TRUE);
 }

 HUSER EXPENTRY icqAddUserInCache(HICQ icq, ULONG uin)
 {
    return insertTemporary(icq,uin,USRF_HIDEONLIST,FALSE);
 }

 static HUSER insertTemporary(HICQ icq, ULONG uin, ULONG flags, BOOL event)
 {
#ifdef EXTENDED_LOG
    char        logBuffer[0x0100];
#endif

    char        nickname[40];
    HUSER       ret             = icqQueryUserHandle(icq,uin);

    if(ret)
       return ret;

    sprintf(nickname,"%lu",uin);

    DBGPrint("Usuario temporario %lu anexado ***********************",uin);

    ret = icqInsertUser(icq, uin, nickname, "", "", "", "", USRF_TEMPORARY|flags);

    if(!ret)
       return ret;
	
    if(icq->c2s && icq->c2s->addUser)
       icq->c2s->addUser(icq,ret);

    if(event)
       icqUserEvent(icq, ret, ICQEVENT_USERADDED);

    icq->cntlFlags |= ICQFC_SAVEUSER;
    ret->u.flags   |= USRF_REFRESH;
	ret->u.c2sGroup = 0x0F00;

#ifdef EXTENDED_LOG
    sprintf(logBuffer,"Temp User added. RFlags: %08lx (%08lx) OFlags: %08lx Searchs: %u",
                                (ULONG) icq->cntlFlags & (ICQFC_LOGONCOMPLETE|ICQFC_CLEAR),
                                (ULONG) ICQFC_LOGONCOMPLETE|ICQFC_CLEAR,
                                (ULONG) icq->onlineFlags & ICQF_ONLINE,
                                icq->searchs );
    DBGMessage(logBuffer);
    icqWriteSysLog(icq,PROJECT,logBuffer);
#endif

    return ret;

 }

 static HUSER updateUser(HICQ icq, HUSER usr, const char *nickName, const char *firstName, const char *lastName, const char *email, const char *sound, BOOL auth, BOOL event)
 {
    ICQITEM     *itn;
    HUSER       ret;
    int         sz      = sizeof(ICQITEM) + sizeof(USERINFO);
    char        *ptr,*src;
    char        buffer[80];
    int         f;

    if(!usr)
    {
       icqSysLog(icq,"Invalid user update requested");
       return NULL;
    }

    icq->cntlFlags |= ICQFC_SAVEUSER;
    usr->u.flags   &= ~USRF_REFRESH;

#ifdef EXTENDED_LOG
    for(ptr = (char *) nickName;*ptr && isspace(*ptr);ptr++);

    if(isspace(*ptr))
    {
       icqSysLog(icq,"Invalid update user request - No nickName supplied");
       nickName = icqQueryUserNick(usr);
    }
#endif

    if(isEmpty(nickName))
       nickName = icqQueryUserNick(usr);

    if(!firstName || !*firstName)
       firstName = icqQueryUserFirstName(usr);

    if(!lastName || !*lastName)
       lastName = icqQueryUserLastName(usr);

    if(!email || !*email)
       email = icqQueryUserEmail(usr);

    if(!sound)
       sound = icqQueryUserSound(usr);

    if( !(      strcmp(nickName,icqQueryUserNick(usr))
                | strcmp(firstName,icqQueryUserFirstName(usr))
                | strcmp(lastName,icqQueryUserLastName(usr))
                | strcmp(email,icqQueryUserEmail(usr))
                | strcmp(sound,icqQueryUserSound(usr)) ) )
    {
       DBGMessage("Nao foi necessario atualizar o usuario");
       icqUpdateUserFlag(icq, usr, auth, USRF_AUTHORIZE);
       if(event)
          icqUserEvent(icq, usr, ICQEVENT_UPDATED);
       return usr;
    }

    DBGMessage("Atualizar dados de usuario");
    sprintf(buffer,"ICQ#%lu information was updated",usr->u.uin);
    icqSysLog(icq,buffer);

    usr->u.idle    = 0;

    if(!*nickName)
    {
       /* Nao tem nickname */
       if(*firstName)
       {
          nickName = firstName;
       }
       else
       {
          sprintf(buffer,"%lu",usr->u.uin);
          nickName = (const char *) buffer;
       }
    }


    sz += strlen(nickName);
    sz += strlen(firstName);
    sz += strlen(lastName);
    sz += strlen(email);
    sz += strlen(sound);
    sz += 7;

    itn = malloc(sz);

    if(!itn)
    {
       icqWriteSysLog(icq,PROJECT,"Unable to allocate memory for user update");
       return NULL;
    }

    memset(itn,0,sz);

    src = (char *) usr;
    ptr = (char *) (itn+1);
    ret = (HUSER) ptr;

    for(f=0;f<sizeof(USERINFO);f++)
       *(ptr++) = *(src++);

    ptr = insertString((char *)(ret+1), nickName);
    ptr = insertString(ptr,firstName);
    ptr = insertString(ptr,lastName);
    ptr = insertString(ptr,email);
    ptr = insertString(ptr,sound);

    icqUserEvent(icq, usr, ICQEVENT_WILLCHANGE);

    insertUser(icq, nickName, itn);

    usr->u.flags |= USRF_DELETED;
    usr->msg      = NULL;

    if(auth)
       ret->u.flags |= USRF_AUTHORIZE;
    else
       ret->u.flags &= ~USRF_AUTHORIZE;

    icqAjustUserIcon(icq,ret,TRUE);

    icqUserEvent(icq, ret, ICQEVENT_UPDATED);

    icq->cntlFlags |= ICQFC_SAVEUSER;

    return ret;

 }

 int EXPENTRY icqAddUserInContactList(HICQ icq, ULONG uin)
 {
    return procAdding(icq,icqQueryUserHandle(icq, uin),uin);
 }

 static int procAdding(HICQ icq, HUSER usr, ULONG uin)
 {
    int     ret        = 0;
    ULONG   oldFlags;

    if(!usr)
    {
       usr = icqInsertTemporaryUser(icq, uin);
       if(!usr)
          return -1;
       usr->u.flags |= USRF_REFRESH;
    }

    if(!usr)
       return -1;

    oldFlags      = usr->u.flags;
    usr->u.flags &= ~(USRF_TEMPORARY|USRF_CACHED);

    if(icq->c2s && icq->c2s->addUser)
    {
       ret = icq->c2s->addUser(icq,usr);
       if(ret)
       {
          usr->u.flags = usr->u.flags;
          return ret;
       }
    }

    usr->u.flags |= USRF_ONLIST;

    if(usr->u.flags & USRF_HIDEONLIST)
       icqUserEvent(icq, usr, ICQEVENT_SHOW);

    icq->cntlFlags |= ICQFC_SAVEUSER;
    return ret;
 }

 HUSER EXPENTRY icqAddNewUser(HICQ icq, ULONG uin, const char *nickName, const char *firstName, const char *lastName, const char *email, BOOL authorize)
 {
    HUSER               usr     =  icqQueryUserHandle(icq, uin);

    DBGTracex(usr);

    if(!usr)
    {
       usr = icqInsertUser(icq, uin, nickName, firstName, lastName, email, NULL, authorize ? USRF_AUTHORIZE : 0 );
    }
    else if(usr->u.flags & USRF_TEMPORARY)
    {
       CHKPoint();

       usr->u.flags &= ~USRF_TEMPORARY;
       usr = updateUser(icq, usr, nickName, firstName, lastName, email, NULL, authorize, TRUE);
    }
    else
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected add user request");
       return NULL;
    }

    procAdding(icq, usr, uin);

    icqSendConfirmation(icq,uin,MSG_ADD,FALSE,NULL);

    return usr;

 }

 HUSER EXPENTRY icqUpdateUserInfo(HICQ icq, ULONG uin, BOOL add, const char *nickName, const char *firstName, const char *lastName, const char *email, BOOL authorize)
 {
    HUSER usr;

    if(uin == icq->uin)
    {
       if(isEmpty(nickName))
          return NULL;

       DBGMessage("Atualizar usuario corrente");
       icqSaveString(icq, "NickName",   nickName);
       icqSaveString(icq, "FirstName",  firstName);
       icqSaveString(icq, "LastName",   lastName);
       icqSaveString(icq, "EMail",      email);
       icqSaveValue( icq, "Authorize",  authorize ? 1 : 0);
       icqSaveValue( icq, "UpdateUser", 0);
       return NULL;
    }

    usr =  icqQueryUserHandle(icq, uin);

    if(usr)
    {
       usr->u.flags &= ~USRF_REFRESH;
       if(isEmpty(nickName))
          nickName = firstName;
       return updateUser(icq, usr, nickName, firstName, lastName, email, NULL, authorize, FALSE);
    }

    if(add)
       return icqInsertUser(icq, uin, nickName, firstName, lastName, email, "", authorize ? USRF_AUTHORIZE : 0);

    usr = icqInsertUser(icq, uin, nickName, firstName, lastName, email, "", (authorize ? USRF_AUTHORIZE : 0)|USRF_HIDEONLIST|USRF_TEMPORARY);

    icq->cntlFlags |= ICQFC_SAVEUSER;

    return usr;
 }

 HUSER EXPENTRY icqInsertUser(HICQ icq, ULONG uin, const char *nickName, const char *firstName, const char *lastName, const char *email, const char *sound, ULONG flags)
 {
    ICQITEM     *itn;
    int         sz      = sizeof(ICQITEM) + sizeof(USERINFO);
    HUSER       usr;
    char        *ptr;

    DBGMessage(nickName);

    if(!uin || uin == icq->uin || icq->sz != sizeof(ICQ))
    {
       icqWriteSysLog(icq, PROJECT, "Invalid request in icqInsertUser()");
       return NULL;
    }

    sz += strlen(nickName);
    sz += strlen(firstName);
    sz += strlen(lastName);
    sz += strlen(email);
    sz += strlen(sound);
    sz += 7;

    itn = malloc(sz);
    DBGTracex(itn);

    if(!itn)
       return NULL;

    memset(itn,0,sz);
    usr = (HUSER) (itn+1);

    /* Inicializa campos para default */
    usr->u.sz           = sizeof(ICQUSER);
    usr->u.uin          = uin;
    usr->u.flags        = flags;
    usr->u.mode         = icqQueryOfflineMode(icq);
    usr->u.offlineIcon  = 10;
    usr->msg            = icqCreateList();

    DBGTracex(usr);
    DBGTracex(usr->msg);

    icqAjustUserIcon(icq,usr,TRUE);

    /* Copia informacoes do usuario */
    ptr = insertString((char *)(usr+1), nickName);
    ptr = insertString(ptr,firstName);
    ptr = insertString(ptr,lastName);
    ptr = insertString(ptr,email);
    ptr = insertString(ptr,sound);

    insertUser(icq, nickName, itn);

    icqUserEvent(icq, usr, ICQEVENT_UPDATED);

    return usr;
 }

 static HUSER insertUser(HICQ icq, const char *nickName, ICQITEM *itn)
 {
    ICQITEM     *i;
    ICQITEM     *pos    = NULL;

    icqLockList(icq->users);

    for(i = icq->users->first; i && !pos; i = i->down)
    {
       if(stricmp((((char *) i) + sizeof(ICQITEM) + sizeof(USERINFO)),nickName) > 0)
          pos = i;
    }

    if(pos)
    {
       /* Inserir antes do usuario em POS */
       itn->up          = pos->up;
       itn->down        = pos;

       if(itn->down)
          itn->down->up         = itn;
       else
          icq->users->last      = itn;

       if(itn->up)
          itn->up->down         = itn;
       else
          icq->users->first     = itn;

    }
    else
    {
       /* Inserir no final da lista */

       if(icq->users->first == NULL)
       {
          /* A lista est  vazia */
          icq->users->first = icq->users->last = itn;
          itn->up                              = itn->down = NULL;
       }
       else
       {
          /* Tem elemento na lista, anexa no final */
          icq->users->last->down = itn;
          itn->up                = icq->users->last;
          itn->down              = NULL;
          icq->users->last       = itn;
       }
    }

    icqUnlockList(icq->users);

    return (HUSER) (itn+1);

 }

 static char *insertString(char *dst, const char *src)
 {
    strcpy(dst,src);
    dst += strlen(src)+1;
    return dst;
 }


 HUSER EXPENTRY icqQueryUserHandle(HICQ icq, ULONG uin)
 {
    HUSER ret;

    for(ret = icqQueryFirstElement(icq->users);ret && ((ret->u.uin != uin) || (ret->u.flags&USRF_DELETED));ret = icqQueryNextElement(icq->users,ret));

#ifdef DEBUG
    if(ret && ret->u.uin != uin)
    {
       icqWriteSysLog(icq,PROJECT,"FATAL ERROR!! Failure in userlist management");
       DBGMessage("*************** ERRO GRAVE!!!! Achou o usuario errado");
       icq->ready = FALSE;
    }
#endif
    return ret;
 }

 HUSER EXPENTRY icqQueryUserByNick(HICQ icq, const char *nick)
 {
    ULONG uin;
    HUSER ret;

    if(icq->sz != sizeof(ICQ))
    {
       icqWriteSysLog(icq,PROJECT,"** Warning: Unexpected call to icqQueryUserByNick" );
       return NULL;
    }

    if(!icq->users)
       return NULL;

    for(ret = icqQueryFirstElement(icq->users);ret && (stricmp((char *)(ret+1),nick) || (ret->u.flags|USRF_DELETED));ret = icqQueryNextElement(icq->users,ret));

    if(ret)
       return ret;

    uin = atoi(nick);

    if(uin)
       return icqQueryUserHandle(icq, uin);

    return ret;
 }

 HUSER EXPENTRY icqQueryNextUser(HICQ icq, HUSER usr)
 {
    /* Obtem o proximo usuario da lista */

    if(!icq->users)
       return NULL;

    if(usr)
       usr = icqQueryNextElement(icq->users,usr);
    else
       usr = icqQueryFirstElement(icq->users);

    while(usr && usr->u.flags & (USRF_DELETED|USRF_CACHED) )
       usr = icqQueryNextElement(icq->users,usr);

    return usr;

 }

 HUSER EXPENTRY icqSetUserOnlineMode(HICQ icq, ULONG uin, ULONG mode)
 {
    char        buffer[0x0100];
    HUSER       usr             = icqQueryUserHandle(icq,uin);
    ULONG       offline         = icqQueryOfflineMode(icq);
    ULONG       lastMode;
    USHORT      event           = ICQEVENT_CHANGED;

    if(!usr)
       return usr;

    usr->u.idle = 0;

    if(usr->u.mode == mode)
       return usr;

    usr->u.lastAction = time(NULL);
    lastMode          = usr->u.mode;
    usr->u.mode       = mode;

    if(mode == offline)
    {
       sprintf(buffer,"User %s (ICQ#%ld) is offline",(char *)(usr+1),usr->u.uin);
       sysLog(icq,buffer);


       usr->u.flags     &= ~USRF_ONLINE;

       usr->ip[0]        =
       usr->ip[1]        = 0;
       usr->port         = 0;
       event             = ICQEVENT_OFFLINE;

    }
    else if(!(usr->u.flags&USRF_ONLINE) )
    {
       usr->ip[0]         =
       usr->ip[1]         = 0;

       usr->port          = 0;
       usr->u.lastOnline  = usr->u.lastAction;

       usr->u.flags      |= USRF_ONLINE;

       sprintf(buffer,"User %s (ICQ#%ld) changed to online (%08lx)",(char *)(usr+1),usr->u.uin,mode);
       sysLog(icq,buffer);

       event = ICQEVENT_ONLINE;
    }

    icqAjustUserIcon(icq,usr,TRUE);

    if(!(usr->u.flags & USRF_HIDEONLIST))
       icqUserEvent(icq, usr, event);

    return usr;
 }

 HUSER EXPENTRY icqUserOnline(HICQ icq, ULONG uin, ULONG status, long ip, long gateway, short port, long tcpVersion, ULONG version)
 {
    char        buffer[0x0100];
    HUSER       usr             =  icqSetUserIPandPort(icq, uin, gateway, ip, port);
    USHORT      event           = ICQEVENT_ONLINE;

    if(!usr)
       return usr;

    usr->u.clientVersion  = version;
    usr->u.peerVersion    = tcpVersion;

    usr->u.lastAction     = time(NULL);

    if(usr->u.flags & USRF_ONLINE)
       event = ICQEVENT_CHANGED;
    else
       usr->u.lastOnline = usr->u.lastAction;

    usr->u.flags |= USRF_ONLINE;

    if(usr->u.mode == status)
       return usr;

    usr->u.mode   = status;

#ifdef EXTENDED_LOG
    sprintf(buffer,"User %s (ICQ#%ld) online (Mode:%08lx TCP:%02x ID:%08lx Port:%d)",(char *)(usr+1),usr->u.uin,status,(int) tcpVersion, version, (int) port);
#else
    sprintf(buffer,"User %s (ICQ#%ld) is online (%08lx)",(char *)(usr+1),usr->u.uin,status);
#endif
    sysLog(icq,buffer);

    icqAjustUserIcon(icq,usr,TRUE);
    icqUserEvent(icq, usr, event);

    return usr;
 }

 USHORT EXPENTRY icqQueryUserPosLimit(HICQ icq)
 {
    /*
     * Posicoes para a lista
     *
     * 0 - Online e com evento
     * 1 - Online, sem evento, com atividade
     * 2 - Online, sem evento e sem atividade
     * 3 - Offline e com evento
     * 4 - Offline, sem evento, com atividade
     * 5 - Offline, sem evento e sem atividade
     * 6 - Aguardando autorizacao
     *
     */
    return 7;
 }

 void EXPENTRY icqAjustUserIcon(HICQ icq, HUSER usr, BOOL event)
 {
    ICQMSG                       *msg           = NULL;
    USHORT                       modeIcon       = usr->u.modeIcon;
    USHORT                       eventIcon      = usr->u.eventIcon;
    USHORT                       index          = usr->u.index;

#ifdef NDEBUG
    if(!usr)
       return;
#else
    if(!usr)
    {
       DBGMessage("**************** ERRO GRAVE! Tentou ajustar icone de usuario inexistente");
       return;
    }
#endif

    /* Verifica o primeiro evento do usuario */
    if(usr->msg)
       msg = icqQueryFirstElement(usr->msg);

    /* Ajusta posicionamento do usuario */
    if(msg)
    {
       /* Tem evento no usuario */
       usr->u.index     = 0;
       usr->u.idle      = 0;    /* Usuario com evento pendente nao conta como IDLE */

       if(usr->u.flags & USRF_HIDEONLIST)
          icqUserEvent(icq, usr, ICQEVENT_SHOW);

    }
    else if(usr->u.idle < 5)
    {
       /* Teve atividade nos £ltimos 5 minutos */
       usr->u.index     = 1;
    }
    else
    {
       /* Usuario sem atividade */
       usr->u.index = 2;
    }

    /* Faz a correcao da posicao de acordo com o modo */

//    if( usr->u.mode  != icqQueryOfflineMode(icq))
    if(usr->u.flags & USRF_ONLINE)
    {
       /* Usuario online, ajustar ¡cone */
       usr->u.modeIcon = icqQueryModeIcon(icq,usr->u.mode);
    }
    else
    {
       /* Usuario offline, ajustar de acordo */
       usr->u.modeIcon  = usr->u.offlineIcon;
       usr->u.index    += 3;

       if(usr->u.flags & USRF_REJECTED)
       {
          usr->u.index    = 6;
          usr->u.modeIcon = 32;
       }
       else if(!msg && (usr->u.flags & USRF_WAITING))
       {
          usr->u.index = 6;
       }

    }

    if(msg)
    {
       if(msg->mgr)
          usr->u.eventIcon = msg->mgr->icon[0];
       else
          usr->u.eventIcon = 0;
    }
    else
    {
       usr->u.eventIcon = usr->u.modeIcon;

       if(usr->u.flags & USRF_HIDEONLIST)
          usr->u.index = 0xFFFF;           /* Manda bem p'ro final de forma a nao aparecer na lista */
    }

/*
    if(msg && msg->mgr)
       usr->u.eventIcon = msg->mgr->icon[0];
    else
       usr->u.eventIcon = usr->u.modeIcon;
*/

    if(event)
    {
       if( (usr->u.eventIcon != eventIcon) || (usr->u.modeIcon != modeIcon) )
          icqUserEvent(icq, usr, ICQEVENT_ICONCHANGED);
       if(usr->u.index != index)
          icqUserEvent(icq, usr, ICQEVENT_POSCHANGED);
    }

 }

 void EXPENTRY icqOpenUserMessageWindow(HICQ icq, HUSER usr)
 {
    openUserMessageWindow(icq, usr, TRUE);
 }

 void openUserMessageWindow(HICQ icq, HUSER usr, BOOL edit)
 {
    HMSG        m   = NULL;

    CHKPoint();

    if(!(icq && usr))
       return;

    if(ALWAYS_HIDE_USER(icq,usr))
       return;

    DBGMessage("Abrir caixa de mensagems para o usuario");

    m = icqQueryFirstElement(usr->msg);

    DBGTracex(m);

    if(m)
    {
       DBGMessage("Usuario tem mensagem, abri-la");
       icqOpenMessage(icq, usr->u.uin, m);
       return;
    }

    if(edit)
       icqNewUserMessage(icq, usr->u.uin, 0, 0, 0);

/*
    if(icq->skin && icq->skin->newMessage)
       icq->skin->newMessage(icq, usr->u.uin, MSG_NORMAL, 0, 0);
    else
       icqWriteSysLog(icq,PROJECT,"Can't determine a valid message editor");
*/

 }

 void EXPENTRY icqSetUserAutoOpen(HICQ icq, HUSER usr, BOOL flag)
 {
    if(flag)
       usr->u.flags |= USRF_AUTOOPEN;
    else
       usr->u.flags &= ~USRF_AUTOOPEN;

    icqUserEvent(icq, usr, ICQEVENT_UPDATED);

 }

 void EXPENTRY icqNewUserMessage(HICQ icq, ULONG uin, USHORT type, const char *p1, const char *p2)
 {
    const MSGMGR      *mgr = icqQueryMessageManager(icq, type);
    HUSER             usr  = icqQueryUserHandle(icq,uin);

#ifdef DEBUG

    DBGTracex(mgr);

    if(mgr)
    {
       DBGTrace(mgr->sz);
       DBGTracex(mgr->edit);
    }

    if(icq->skin)
    {
       DBGTracex(icq->skin);
       DBGTracex(icq->skin->newMessage);
    }

#endif

    icqBeginPeerSession(icq,usr);

    if(mgr && mgr->edit)
    {
       CHKPoint();
       mgr->edit(icq,uin,TRUE,NULL,p1,p2);
    }
    else if(icq->skin && icq->skin->newMessage)
    {
       CHKPoint();
       icq->skin->newMessage(icq,uin,type,p1,p2);
    }
    else
    {
       CHKPoint();
       sysLog(icq,"Can't determine a valid message editor");
    }

 }

 ULONG EXPENTRY icqUpdateUserFlag(HICQ icq, HUSER usr, BOOL modo, ULONG flag)
 {
    ULONG lastStatus = usr->u.flags;

    if(modo)
       usr->u.flags |= flag;
    else
       usr->u.flags &= ~flag;

    if(lastStatus == usr->u.flags)
       return usr->u.flags;

    /* Tabela de flags foi mudada */

    icq->cntlFlags |= ICQFC_SAVEUSER;
    icqUserEvent(icq, usr, ICQEVENT_UPDATED);

    return usr->u.flags;
 }

 static int RejectUser(HICQ icq, HUSER usr)
 {
#ifdef EXTENDED_LOG
    char        logBuffer[0x0100];
    int         qtdRemoved      = 0;
#endif
    HMSG        msg;

    if(!usr)
       return -1;

    msg = icqQueryFirstElement(usr->msg);

    if(msg)
    {
#ifdef EXTENDED_LOG
       sprintf(logBuffer,"Removing messages from ICQ#%lu",usr->u.uin);
       icqWriteSysLog(icq,PROJECT,logBuffer);
#endif
       while(msg)
       {
          icqRemoveElement(usr->msg,msg);
#ifdef EXTENDED_LOG
          qtdRemoved++;
#endif
          msg = icqQueryFirstElement(usr->msg);
       }

#ifdef EXTENDED_LOG
       sprintf(logBuffer,"Removed messages: %d",qtdRemoved);
       icqWriteSysLog(icq,PROJECT,logBuffer);
#endif

       icqUserEvent(icq,usr,ICQEVENT_MESSAGECHANGED);
       icq->cntlFlags |= ICQFC_SAVEQUEUES;
    }

    icqUserEvent(icq, usr, ICQEVENT_HIDE);

    if(icq->userDB && icq->userDB->deleteUserDB)
       icq->userDB->deleteUserDB(icq,usr);

#ifdef EXTENDED_LOG
       icqWriteSysLog(icq,PROJECT,"User Rejected");
#endif

    return 0;
 }


 ULONG EXPENTRY icqRejectUser(HICQ icq, HUSER usr, BOOL spammer)
 {
    ULONG       uin     = usr->u.uin;
    int         ret     = RejectUser(icq,usr);

    if(ret)
    {
#ifdef EXTENDED_LOG
       icqWriteUserLog(icq,PROJECT, usr, "Failure rejecting user");
#endif
       return ret;
    }


    if(usr->u.flags & USRF_REJECTED)
    {
#ifdef EXTENDED_LOG
       icqWriteUserLog(icq,PROJECT, usr, "Already rejected");
#endif
       return 0;
    }

    usr->u.flags |= USRF_IGNORE|USRF_REJECTED;

    if(spammer)
    {
#ifdef EXTENDED_LOG
       icqWriteUserLog(icq,PROJECT, usr, "Adding in ICQ# Blacklist (temporary)");
#endif
       icqBlacklistUser(icq,uin);
       deleteUser(icq, usr);
    }
#ifdef EXTENDED_LOG
    else
    {
       icqWriteUserLog(icq,PROJECT, usr, "Not added in temporary blacklist");
    }
#endif

    return ret;

 }

 static int removeUser(HICQ icq, HUSER usr)
 {
    int ret   = 0;

    if( (usr->u.flags & USRF_ONLIST) && icq->c2s && icq->c2s->delUser)
       ret = icq->c2s->delUser(icq,usr);

    if(ret)
       return ret;

    return 0;

 }

 static int deleteUser(HICQ icq, HUSER usr)
 {
    icqWriteUserLog(icq,PROJECT,usr, "User removed");
    icqUserEvent(icq,usr,ICQEVENT_DELETED);
    icq->cntlFlags |= ICQFC_SAVEUSER;
    usr->u.flags   |= USRF_DELETED;
    return 0;
 }

 ULONG EXPENTRY icqRemoveUser(HICQ icq, HUSER usr)
 {
    int ret = removeUser(icq,usr);

    if(ret)
    {
       icqWriteUserLog(icq,PROJECT,usr, "Can't remove user");
       icqUserEvent(icq,usr,ICQEVENT_CANTREMOVE);
       return ret;
    }

    ret = RejectUser(icq,usr);

    if(ret)
    {
       icqWriteUserLog(icq,PROJECT,usr, "Can't reject user");
       icqUserEvent(icq,usr,ICQEVENT_CANTREMOVE);
       return ret;
    }

    return deleteUser(icq,usr);

 }

 void EXPENTRY icqRegisterUserAction(HUSER usr)
 {
    usr->u.lastAction = time(NULL);
 }

 int ICQAPI icqConnectUser(HICQ icq, ULONG uin, int sock)
 {
#ifdef EXTENDED_LOG
    char  logBuffer[0x0100];
#endif

    HUSER usr = icqQueryUserHandle(icq,uin);
    long  ip;

    if(!(usr && usr->port))
       return -1;

    ip = icqQueryUserIP(icq,usr);

    if(!ip)
       return -1;

#ifdef EXTENDED_LOG
    sprintf(logBuffer,"Contacting %s (ICQ#%ld) at ",icqQueryUserNick(usr),uin);
    icqIP2String(ip,logBuffer+strlen(logBuffer));
    sprintf(logBuffer+strlen(logBuffer),":%d",usr->port);
    icqWriteSysLog(icq,PROJECT,logBuffer);
#endif

    if(sock)
       return icqConnectSock(sock,ip,usr->port);

//    return icqConnect(ip, usr->port);
    return -1;

 }


 
