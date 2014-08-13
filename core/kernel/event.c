/*
 * EVENT.C - Lanca eventos
 */


 #include <icqkernel.h>
 #include "icqcore.h"

/*---[ Processadores de evento ]---------------------------------------------------------------------*/

 icqDeclareSystemEventProcessor(ICQEVENT_CONTACT);
 icqDeclareSystemEventProcessor(ICQEVENT_ONLINE);
 icqDeclareSystemEventProcessor(ICQEVENT_CHANGED);
 icqDeclareSystemEventProcessor(ICQEVENT_OFFLINE);
 icqDeclareSystemEventProcessor(ICQEVENT_PLISTCLEAR);
 icqDeclareSystemEventProcessor(ICQEVENT_LOGONCOMPLETE);
 icqDeclareSystemEventProcessor(ICQEVENT_UPDATED);

 static const struct icqEventProcessors proctable[] =
 {
    icqSystemEventProcessor(ICQEVENT_CONTACT),
    icqSystemEventProcessor(ICQEVENT_ONLINE),
    icqSystemEventProcessor(ICQEVENT_CHANGED),
    icqSystemEventProcessor(ICQEVENT_OFFLINE),
    icqSystemEventProcessor(ICQEVENT_PLISTCLEAR),
    icqSystemEventProcessor(ICQEVENT_LOGONCOMPLETE),
    icqSystemEventProcessor(ICQEVENT_UPDATED)
 };

/*---[ Prototipos ]----------------------------------------------------------------------------------*/

 static int procEventTable(HICQ, ULONG, UCHAR, USHORT, ULONG, int, const struct icqEventProcessors *);

/*---[ Implementacao ]-------------------------------------------------------------------------------*/

 static void ajustModeIcon(HICQ icq)
 {
    if(icq->info)
    {
       icq->info->mode     = icq->currentMode;
       icq->info->modeIcon = icqQueryModeIcon(icq, icq->currentMode);
       icq->info->eventCounter++;
    }
 }

 static void blinkUser(HICQ icq, HUSER user)
 {
    user->blinkTimer  = icqLoadValue(icq, "UserBlinkTime", 10);
    if(user->blinkTimer)
       user->u.flags |= USRF_BLINK;
 }

 icqBeginSystemEventProcessor(ICQEVENT_CONTACT,icq,uin,parm)
 {
    icq->onlineFlags |= ICQF_CONTACTED;
    return 0;
 }

 icqBeginSystemEventProcessor(ICQEVENT_ONLINE,icq,uin,parm)
 {
    icq->onlineFlags &= ~ICQF_CONNECTING;
    icq->onlineFlags |= ICQF_ONLINE;
    icq->loginRetry   = 0;
    icq->connctTimer  = 0;

    ajustModeIcon(icq);
    return 0;
 }

 icqBeginSystemEventProcessor(ICQEVENT_CHANGED,icq,uin,parm)
 {
    ajustModeIcon(icq);
    return 0;
 }

 icqBeginSystemEventProcessor(ICQEVENT_OFFLINE,icq,uin,parm)
 {
    HUSER usr;

    icqWriteSysLog(icq,PROJECT,"System offline");
    icq->onlineFlags    = 0;
    icq->searchs        = 0;
    icq->cntlFlags     &= ~(ICQFC_LOGONCOMPLETE);
    icq->currentMode    = icqQueryOfflineMode(icq);

    if(!icq->connctTimer)
       icqSetConnectTimer(icq,0);

    icqClearList(icq->pendingSearchs);

    for(usr = icqQueryFirstUser(icq);usr;usr = icqQueryNextUser(icq,usr))
    {
       if(usr->u.mode != icq->currentMode)
          icqUserEvent(icq, usr, ICQEVENT_SETOFFLINE);
       usr->u.mode = icq->currentMode;
       if(usr->u.flags & USRF_TEMPORARY)
          usr->u.flags |= USRF_HIDEONLIST;
       icqAjustUserIcon(icq,usr,FALSE);
    }
    ajustModeIcon(icq);
    return 0;
 }

 icqBeginSystemEventProcessor(ICQEVENT_PLISTCLEAR,icq,uin,parm)
 {
    icq->cntlFlags |= ICQFC_CLEAR;
    return 0;
 }

 icqBeginSystemEventProcessor(ICQEVENT_LOGONCOMPLETE,icq,uin,parm)
 {
    icqWriteSysLog(icq,PROJECT,"Logon process complete");
    icq->cntlFlags |= ICQFC_LOGONCOMPLETE;
    return 0;
 }

 icqBeginSystemEventProcessor(ICQEVENT_UPDATED,icq,uin,parm)
 {
    icq->cntlFlags |= ICQFC_SAVEUSER;
    return 0;
 }


 void EXPENTRY icqSystemEvent(HICQ icq, USHORT event)
 {
    if(icq->sz != sizeof(ICQ))
    {
       DBGMessage("******** CODIGO DE CONEXAO INVALIDO!!!");
       icqWriteSysLog(NULL,PROJECT,"Invalid connection handler received on event processor (Invalid plugin?)");
       return;
    }
    icqEvent(icq,0,'S',event,0);
 }

 void EXPENTRY icqUserEvent(HICQ icq, HUSER user, USHORT event)
 {
    HPLUGIN     p;

    if(!user)
       return;

    switch(event)
    {
    case ICQEVENT_UPDATED:
       icq->cntlFlags |= ICQFC_SAVEUSER;
       break;

    case ICQEVENT_MODIFIED:
       icqAjustUserIcon(icq, user, FALSE);
       icq->cntlFlags |= ICQFC_SAVEUSER;
       break;

    case ICQEVENT_REJECTED:
       user->u.flags  |= USRF_REJECTED;
       user->u.flags  &= ~USRF_ONLIST;
       icq->cntlFlags |= ICQFC_SAVEUSER;
       break;

    case ICQEVENT_ONLINE:
       user->u.flags   |= USRF_ONLINE;
       blinkUser(icq,user);
       break;

    case ICQEVENT_OFFLINE:
       user->u.flags   &= ~USRF_ONLINE;
       blinkUser(icq,user);
       break;

    case ICQEVENT_SETOFFLINE:
       user->u.flags   &= ~USRF_ONLINE;
       break;

    case ICQEVENT_HIDE:
       user->u.flags  |= USRF_HIDEONLIST;
       icq->cntlFlags |= ICQFC_SAVEUSER;
#ifdef EXTENDED_LOG
       icqWriteUserLog(icq,PROJECT,user,"Hide");
#endif
       break;

    case ICQEVENT_SHOW:
       user->u.flags  &= ~USRF_HIDEONLIST;
#ifdef EXTENDED_LOG
       icqWriteUserLog(icq,PROJECT,user,"Unhide");
#endif
       icq->cntlFlags |= ICQFC_SAVEUSER;
       break;
    }

    for(p=icqQueryFirstElement(icq->plugins);p;p=icqQueryNextElement(icq->plugins,p))
    {
       if(isProcValid(p,p->UserEvent))
          p->UserEvent(icq,p->dataBlock,user,event);
    }

    icqEvent(icq,user->u.uin,'U',event,(ULONG)user);
 }

 static int procEventTable(HICQ icq, ULONG uin, UCHAR type, USHORT event, ULONG parm, int sz, const struct icqEventProcessors *ptr)
 {
    int f;

    for(f=0;f < sz; f++)
    {
       if(ptr->type == type && ptr->event == event)
          return ptr->exec(icq,uin,parm);
       ptr++;
    }
    return 0;
 }

 void EXPENTRY icqEvent(HICQ icq, ULONG uin, char id, USHORT event, ULONG parm)
 {
    HPLUGIN             p;
    HTHREAD             thd;
    EVENTLISTENER       *listener;

    if(icq->sz != sizeof(ICQ))
    {
       DBGMessage("******** CODIGO DE CONEXAO INVALIDO!!!");
       icqWriteSysLog(NULL,PROJECT,"Invalid connection handler received on event processor (Invalid plugin?)");
       return;
    }
	
    if(icq->info)
       icq->info->eventCounter++;

    procEventTable(  	icq,
    					uin,
    					id,
    					event,
    					parm,
    					(sizeof(proctable) / sizeof(struct icqEventProcessors)),
    					proctable );

    if(icq->skin && icq->skin->event)
       icq->skin->event(icq,uin,id,event,parm,icq->skinData);

    for(p=icqQueryFirstElement(icq->plugins);p;p=icqQueryNextElement(icq->plugins,p))
    {
       if(id == 'S' && isProcValid(p,p->SystemEvent))
          p->SystemEvent(icq,p->dataBlock,event);

       if(isProcValid(p,p->ProcessEvent))
          p->ProcessEvent(icq,p->dataBlock,uin,id,event,parm);
    }

    for(thd = icqQueryFirstElement(icq->threads);thd;thd = icqQueryNextElement(icq->threads,thd))
    {
       if(thd->event)
          thd->event(thd,uin,id,event,parm);
    }

    for(listener = icqQueryFirstElement(icq->listeners);listener;listener = icqQueryNextElement(icq->listeners,listener))
    {
       if(listener->listener)
          listener->listener(icq,listener->parm,uin,id,event,parm);
    }

 }



