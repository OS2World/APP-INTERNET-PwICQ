/*
 * TIMER.C - Funcao de timer
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

#ifdef linux
   #include <unistd.h>
#endif

 #include <stdio.h>
 #include <time.h>
 #include <string.h>
 #include <stdlib.h>
 #include <sys/types.h>
 #include <sys/stat.h>

 #include <icqkernel.h>

 #include <icqplugins.h>

 #include "icqcore.h"

/*---[ Prototipos ]---------------------------------------------------------------*/

 static void _System autoSave(ICQTHREAD *);

/*---[ Statics ]------------------------------------------------------------------*/

 static int thread = -1;

/*---[ Implementacao ]------------------------------------------------------------*/

 int oneMinute(HICQ icq)
 {
    HUSER               usr;
    HUSER               next;
    time_t              tm;
    char                buffer[0x0100];
//    char                *ptr;
    struct search       *Search;
    struct search       *NextSearch;
    struct stat         fileInfo;

    if( !((icq->srvcStatus & ICQSRVC_C2S) || icqIsOnline(icq)) && icq->connctTimer > 0)
    {
       if(--icq->connctTimer < 1)
          icqReconnect(icq);
    }

    /* Precisa verificar arquivos */
    icq->minutes++;

    if( !(icq->minutes % 5) )
    {
#ifdef EXTENDED_LOG
       sysLog(icq,"Checking list files");
#endif
       icqQueryPath(icq,"badword.dat",buffer,0xFF);

       memset(&fileInfo,0,sizeof(fileInfo));
       stat(buffer, &fileInfo);

       if(icq->badWordTime && icq->badWordTime != fileInfo.st_mtime)
       {
          sysLog(icq,"Badword list was changed");
          free(icq->badWords);
          icq->badWords = NULL;
          icq->badWords = loadTextFile(icq,"badword.dat",&icq->badWordTime);
       }
    }

    /* Valida lista de pesquisas pendentes */
    Search = icqQueryFirstElement(icq->pendingSearchs);
    while(Search)
    {
       NextSearch = icqQueryNextElement(icq->pendingSearchs, Search);

       if(Search->idle++ > 5)
       {
          icqEvent(icq, Search->uin, 'S', ICQEVENT_SEARCHTIMEOUT, (ULONG) Search);
#ifdef EXTENDED_LOG
          sprintf(buffer,"Timeout on search %ld (ICQ#%lu)",Search->sequence,Search->uin);
          sysLog(icq,buffer);
#endif

          if(icq->searchs)
             icq->searchs--;

          icqRemoveElement(icq->pendingSearchs, Search);

       }
       Search = NextSearch;
    }

    time(&tm);

    /* Valida a lista de usuarios */
    usr = icqQueryFirstElement(icq->users);
    while(usr)
    {
       next = icqQueryNextElement(icq->users,usr);

       usr->u.idle++;

       if(usr->u.idle > 10 && usr->u.flags & USRF_DELETED && thread < 1)
       {
#ifdef EXTENDED_LOG
          sprintf(buffer,"Cleaning User %s (ICQ#%lu)",icqQueryUserNick(usr),usr->u.uin);
          sysLog(icq,buffer);
#endif
          icqDestroyList(usr->msg);
          icqRemoveElement(icq->users,usr);
       }
       else if(usr->u.idle == 5)
       {
          /* IDLE a 5 minutos, faco um refresh do icone */
          icqAjustUserIcon(icq, usr, TRUE);
       }
       else if( !(usr->u.flags & USRF_HIDEONLIST) && usr->u.idle == 60
                                                  && (usr->u.flags & USRF_TEMPORARY)
                                                  && !icqQueryFirstElement(usr->msg) )
       {
          /* Temporario, IDLE a uma hora e sem mensagem, oculto na contact-list */
#ifdef EXTENDED_LOG
         sprintf(buffer,"Hiding %s (ICQ#%lu) on contact-list",icqQueryUserNick(usr),usr->u.uin);
         sysLog(icq,buffer);
#endif
          icqUserEvent(icq, usr, ICQEVENT_HIDE);
          icqAjustUserIcon(icq, usr, TRUE);
       }
       else if(usr->u.lastAction && (usr->u.flags & USRF_REJECTED) && difftime(tm,usr->u.lastAction) > 345600)
       {
          // Usuarios rejeitados sao removidos em 4 dias
          sprintf(buffer,"Discarding rejected user %s (ICQ#%ld)",icqQueryUserNick(usr),usr->u.uin);
          icqWriteSysLog(icq,PROJECT,buffer);
          icqRemoveUser(icq, usr);
       }
       else if(usr->u.lastAction && (usr->u.flags & USRF_HIDEONLIST) && (usr->u.flags & USRF_TEMPORARY) && !(usr->u.flags & USRF_IGNORE) && difftime(tm,usr->u.lastAction) > 604800)
       {
          // Usuarios temporarios sao removidos em 7 dias
          sprintf(buffer,"Discarding temporary user %s (ICQ#%ld)",icqQueryUserNick(usr),usr->u.uin);
          icqWriteSysLog(icq,PROJECT,buffer);
          icqRemoveUser(icq, usr);
       }

       usr = next;
    }

    DBGTrace(icq->autoSaveTimer);

    if(icq->autoSaveTimer++ > 2 && (icq->cntlFlags & ICQFC_CLEAR) && !icq->searchs )
    {
       icq->autoSaveTimer = 0;

       if(thread < 1)
       {
          thread = icqCreateThread(icq, autoSave, 8192, 0, NULL, "SAV");
       }
       else
       {
#ifdef EXTENDED_LOG
          sprintf(buffer,"Timeout in autosave thread (TID=%d)",thread);
          sysLog(icq,buffer);
#else
          sysLog(icq,"Timeout in autosave thread");
#endif
       }

    }

    return 0;
 }

 static void _System autoSave(ICQTHREAD *thd)
 {
    int f;

#ifdef __OS2__
    DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,PRTYD_MINIMUM,0);
#endif

    DBGMessage("Auto-save thread iniciada");

    for(f=0;f<100 && thread < 1;f++)
    {
#ifdef __OS2__
       DosSleep(100); /* Evita que a thread termine rapido demais, antes que o startup marque a variavel */
#endif
#ifdef linux
       usleep(1000);
#endif
    }

    if( (thd->icq->cntlFlags & (ICQFC_STARTUPOK|ICQFC_SAVEUSER)) == (ICQFC_STARTUPOK|ICQFC_SAVEUSER))
    {
       icqSystemEvent(thd->icq,ICQEVENT_SAVE);
       thd->icq->cntlFlags &= ~ICQFC_SAVEUSER;
    }

    saveMessageList(thd->icq);

    thread = -1;

    DBGMessage("Auto-save thread terminada");
 }


 int oneSecond(HICQ icq)
 {
    ICQTHREAD           *t;
    HPLUGIN             p;
    HUSER               usr;
    BOOL                refresh;
    EVENTLISTENER       *listener;
    EVENTLISTENER       *next;

    icqSystemEvent(icq, ICQEVENT_TIMER);

    for(t = icqQueryFirstElement(icq->threads);t;t = icqQueryNextElement(icq->threads,t))
       t->timer++;

    for(p=icqQueryFirstElement(icq->plugins);p;p=icqQueryNextElement(icq->plugins,p))
    {
       if(isProcValid(p,p->Timer))
          p->Timer(icq,p->dataBlock);
    }

    listener = icqQueryFirstElement(icq->listeners);

    while(listener)
    {
       next = icqQueryNextElement(icq->listeners,listener);

       if(listener->listener)
       {
          listener->timer = 0;
       }
       else if(listener->timer++ > 5)
       {
          DBGMessage("Removendo listener");
          icqRemoveElement(icq->listeners,listener);
          DBGTracex(icqQueryFirstElement(icq->listeners));
       }
       listener = next;
    }

    refresh = (icq->onlineFlags & ICQF_ONLINE) && !(icq->searchs)
                                               && (icq->cntlFlags & ICQFC_LOGONCOMPLETE)
                                               && (icq->cntlFlags & ICQFC_CLEAR);

    for(usr=icqQueryFirstElement(icq->users);usr;usr=icqQueryNextElement(icq->users,usr))
    {
       if(refresh && (usr->u.flags & USRF_REFRESH) && !((usr->u.flags&USRF_SEARCHING) || (usr->u.flags&USRF_DELETED)) )
       {
          refresh = FALSE;
          usr->u.flags |= USRF_SEARCHING;
          DBGMessage("Usuario com requisicao pendente");
          DBGTrace(usr->u.uin);
          icqRequestUserUpdate(icq, usr->u.uin);
       }

       if(usr->blinkTimer)
       {
          usr->blinkTimer--;

          if(usr->blinkTimer)
             usr->u.flags |= USRF_BLINK;
          else
             usr->u.flags &= ~USRF_BLINK;
       }
    }

    return 0;
 }



