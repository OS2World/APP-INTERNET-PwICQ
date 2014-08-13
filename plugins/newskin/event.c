/*
 * MAIN.C - pwICQ's Skin manager entry points
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>
 #include <pwicqgui.h>

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static void setModeIcon(HICQ, ULONG);
 static void setSysButtonIcon(HICQ);

/*---[ Event processors ]---------------------------------------------------------------------------------*/

 icqDeclareSystemEventProcessor(ICQEVENT_SECONDARY);
 icqDeclareSystemEventProcessor(ICQEVENT_ONLINE);
 icqDeclareSystemEventProcessor(ICQEVENT_OFFLINE);
 icqDeclareSystemEventProcessor(ICQEVENT_HIDE);
 icqDeclareSystemEventProcessor(ICQEVENT_SHOW);
 icqDeclareSystemEventProcessor(ICQEVENT_TIMER);
 icqDeclareSystemEventProcessor(ICQEVENT_TERMINATE);
 icqDeclareSystemEventProcessor(ICQEVENT_CHANGED);
 icqDeclareSystemEventProcessor(ICQEVENT_CONNECTING);
 icqDeclareSystemEventProcessor(ICQEVENT_FINDINGSERVER);
 icqDeclareSystemEventProcessor(ICQEVENT_MESSAGECHANGED);
 
 icqDeclareUserEventProcessor(ICQEVENT_MESSAGECHANGED);

#ifdef EXTENDED_LOG
 icqDeclareUserEventProcessor(ICQEVENT_MESSAGEADDED);
#endif

 icqDeclareGuiEventProcessor(ICQEVENT_TIMER);
 icqDeclareGuiEventProcessor(ICQEVENT_CALLBACK);
 icqDeclareGuiEventProcessor(ICQEVENT_STARTUP);
 icqDeclareGuiEventProcessor(ICQEVENT_POPUP);

/*---[ Event processor table ]----------------------------------------------------------------------------*/

 static const struct icqEventProcessors proctable[] =
 {
    icqGuiEventProcessor(ICQEVENT_TIMER),
    icqSystemEventProcessor(ICQEVENT_ONLINE),
    icqSystemEventProcessor(ICQEVENT_OFFLINE),
    icqSystemEventProcessor(ICQEVENT_SHOW),
    icqSystemEventProcessor(ICQEVENT_HIDE),
    icqSystemEventProcessor(ICQEVENT_SECONDARY),
    icqSystemEventProcessor(ICQEVENT_TIMER),
    icqSystemEventProcessor(ICQEVENT_CHANGED),
    icqSystemEventProcessor(ICQEVENT_CONNECTING),
    icqSystemEventProcessor(ICQEVENT_FINDINGSERVER),
    icqSystemEventProcessor(ICQEVENT_MESSAGECHANGED),
    icqUserEventProcessor(ICQEVENT_MESSAGECHANGED),
    icqGuiEventProcessor(ICQEVENT_CALLBACK),
    icqGuiEventProcessor(ICQEVENT_STARTUP),
    icqGuiEventProcessor(ICQEVENT_POPUP),
#ifdef EXTENDED_LOG
    icqUserEventProcessor(ICQEVENT_MESSAGEADDED),
#endif
    icqSystemEventProcessor(ICQEVENT_TERMINATE)

 };

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 int icqskin_event(HICQ icq, ULONG uin, UCHAR type, USHORT event, ULONG parm)
 {
    int f;
    const struct icqEventProcessors *ptr = proctable;

    icqExecuteGuiListeners(icq, uin, type, event, parm);

    for(f=0;f < ( sizeof(proctable) / sizeof(struct icqEventProcessors) ); f++)
    {
       if(ptr->type == type && ptr->event == event)
          return ptr->exec(icq,uin,parm);
       ptr++;
    }

    return 0;
 }

 icqBeginSystemEventProcessor(ICQEVENT_SECONDARY,icq,uin,parm)
 {
    SKINDATA *cfg              = icqskin_getDataBlock(icq);

	CHKPoint();
	
    icqskin_loadUsersList(icqskin_getDlgItemHandle(cfg->mainWindow,ID_USERLIST), cfg->icq);
	cfg->flags |= SKINDATA_FLAGS_LOADED;

	cfg->eventIcon =
	cfg->readIcon  = ICQICON_INACTIVE;
	
    setSysButtonIcon(icq);
	
    /* Show main window */
    icqskin_setVisible(cfg, TRUE);

// #ifdef DEBUG
//    icqskin_postEvent(icq, 0, 'g', ICQEVENT_POPUP, 0);
//#endif

    return 0;
 }

 icqBeginSystemEventProcessor(ICQEVENT_TIMER,icq,uin,parm)
 {
    SKINDATA *cfg = icqskin_getDataBlock(icq);

#ifndef __OS2__
	if(!(cfg->flags & SKINDATA_FLAGS_LOADED))
       icqSysEvent_ICQEVENT_SECONDARY(icq,0,0);
#endif       	

    return 0;
 }

 icqBeginSystemEventProcessor(ICQEVENT_ONLINE,icq,uin,parm)
 {
    SKINDATA *cfg = icqskin_getDataBlock(icq);
    cfg->flags &= ~SKINDATA_FLAGS_CONNECTING;

    CHKPoint();
    setModeIcon(icq, icqQueryOnlineMode(icq));

    return 0;
 }

 icqBeginSystemEventProcessor(ICQEVENT_CHANGED,icq,uin,parm)
 {
    SKINDATA *cfg = icqskin_getDataBlock(icq);
    cfg->flags &= ~SKINDATA_FLAGS_CONNECTING;
    setModeIcon(icq, icqQueryOnlineMode(icq));
    return 0;
 }

 icqBeginSystemEventProcessor(ICQEVENT_FINDINGSERVER,icq,uin,parm)
 {
    SKINDATA *skn = icqskin_getDataBlock(icq);
    icqskin_setFrameIcon(skn->frame, skn, ICQICON_CONNECTING);
    icqskin_setButtonText(icqskin_getDlgItemHandle(skn->mainWindow,ID_MODE),"Finding");
    return 0;
 }

 icqBeginSystemEventProcessor(ICQEVENT_CONNECTING,icq,uin,parm)
 {
    SKINDATA *skn = icqskin_getDataBlock(icq);
    skn->flags |= SKINDATA_FLAGS_CONNECTING;

    CHKPoint();
    icqskin_setFrameIcon(skn->frame, skn, ICQICON_CONNECTING);
    icqskin_setButtonText(icqskin_getDlgItemHandle(skn->mainWindow,ID_MODE),"Trying");

    return 0;
 }

 icqBeginSystemEventProcessor(ICQEVENT_OFFLINE,icq,uin,parm)
 {
    SKINDATA *cfg = icqskin_getDataBlock(icq);
    cfg->flags &= ~SKINDATA_FLAGS_CONNECTING;
	
    setModeIcon(icq, icqQueryOnlineMode(icq));
    CHKPoint();
    return 0;
 }

 icqBeginSystemEventProcessor(ICQEVENT_SHOW,icq,uin,parm)
 {
    CHKPoint();
    icqskin_setVisible(icqskin_getDataBlock(icq), TRUE);
    CHKPoint();
    return 0;
 }

 icqBeginSystemEventProcessor(ICQEVENT_HIDE,icq,uin,parm)
 {
    CHKPoint();
    icqskin_setVisible(icqskin_getDataBlock(icq), FALSE);
    CHKPoint();
    return 0;
 }

 icqBeginSystemEventProcessor(ICQEVENT_TERMINATE,icq,uin,parm)
 {
	icqskin_terminate(icq);
    return 0;
 }

 icqBeginGuiEventProcessor(ICQEVENT_TIMER,icq,uin,parm)
 {
    SKINDATA  *skn = icqskin_getDataBlock(icq);

	skn->flags ^= SKINDATA_FLAGS_BLINK;
	
    if(skn->flags & SKINDATA_FLAGS_CONNECTING)
       icqskin_setButtonIcon(icqskin_getDlgItemHandle(skn->mainWindow,ID_MODE),ICQICON_ANIMATE+skn->animate);

	if(skn->eventIcon != skn->readIcon)
       icqskin_setButtonIcon(icqskin_getDlgItemHandle(skn->mainWindow,ID_SYSMSG),  (skn->flags & SKINDATA_FLAGS_BLINK) ? skn->eventIcon : skn->readIcon);

    return 0;
 }

 icqBeginSystemEventProcessor(ICQEVENT_MESSAGECHANGED,icq,uin,parm)
 {
    setSysButtonIcon(icq);
    setModeIcon(icq, icqQueryOnlineMode(icq));
	return 0;
 }

 icqBeginUserEventProcessor(ICQEVENT_MESSAGECHANGED,icq,uin,parm)
 {
    setModeIcon(icq, icqQueryOnlineMode(icq));
	return 0;
 }

 static void setModeIcon(HICQ icq, ULONG sel)
 {
    const ICQMODETABLE  *mode;
    char			    buffer[0x0100];
	
    SKINDATA            *skn      = icqskin_getDataBlock(icq);
    HMSG                msg       = NULL;
    ULONG               uin       = 0;
	USHORT              modeIcon  = 0xFFFF;
	const char          *modeName = "Offline";
	
#ifdef __OS2__	
    HWND                sysTray = icqQuerySystray(icq);
#endif	

    CHKPoint();
    icqQueryFirstMessage(icq, &uin, &msg);

    DBGTracex(icqQueryModeTable(icq));
	
	if(icqIsOnline(icq))
	{
       for(mode = icqQueryModeTable(icq);mode->descr && modeIcon == 0xFFFF;mode++)
       {
          if(mode->mode == sel)
          {
		     modeIcon = mode->icon;
			 modeName = mode->descr;
	      }
       }	
    }	
	   
    if(modeIcon == 0xFFFF)
	{
	   modeIcon = ICQICON_OFFLINE;
	   modeName = "Offline";
	}
	
    icqskin_setButtonText(icqskin_getDlgItemHandle(skn->mainWindow,ID_MODE),modeName);
    icqskin_setButtonIcon(icqskin_getDlgItemHandle(skn->mainWindow,ID_MODE),modeIcon);

    switch(icqskin_queryTaskICON(icq))
    {
    case 1:		// Change icon on mode change
       CHKPoint();
       icqskin_setFrameIcon(skn->frame, skn, modeIcon);
       break;

    case 2:		// Change icon in message
       DBGTracex(msg);
       if(msg && msg->mgr)
	   {
          icqskin_setFrameIcon(skn->frame, skn, msg->mgr->icon[0]);
	   }
       else
	   {
		  icqWriteSysLog(icq,PROJECT,"Message queue is empty");
          icqskin_setFrameIcon(skn->frame, skn, modeIcon);
	   }
       break;
	}

    switch(icqskin_queryTaskTITLE(icq))
    {
    case 1:		// Change title on mode change
       sprintf(buffer,"%lu - %s",icqQueryUIN(icq),modeName);
       icqskin_setFrameTitle(skn->frame,buffer);
       break;

    case 2:		// Change title in message change
       if(msg)
		  icqQueryMessageTitle(icq, uin, FALSE, msg, buffer, 0xFF);
	   else
          sprintf(buffer,"%lu - %s",icqQueryUIN(icq),modeName);
       icqskin_setFrameTitle(skn->frame,buffer);
       break;
    }
		
#ifdef __OS2__	
    if(sysTray)
       WinPostMsg(sysTray,WM_TRAYICON,(MPARAM) skn->mainWindow,0);
#endif	

 }

 icqBeginGuiEventProcessor(ICQEVENT_CALLBACK,icq,uin,parm)
 {
    #pragma pack(1)

    struct _parm
    {
   	   void (* _System callback)(ULONG);
    } *p = (struct _parm *) parm;

    #pragma pack()

    CHKPoint();

    if(p->callback)
       p->callback(parm);

    CHKPoint();

    return 0;	
 }

 icqBeginGuiEventProcessor(ICQEVENT_STARTUP,icq,uin,parm)
 {
    CHKPoint();
    return 0;
 }

 static void setSysButtonIcon(HICQ icq)
 {
    SKINDATA  *skn = icqskin_getDataBlock(icq);
	HMSG      msg  = icqQueryMessage(icq,0);

    if(!msg)
	{
	   skn->eventIcon = skn->readIcon;
	}
	else if(msg->mgr)
	{
       skn->eventIcon = msg->mgr->icon[0];
	   skn->readIcon  = msg->mgr->icon[1];
	}
	else
	{
       skn->eventIcon = ICQICON_UNREADMESSAGE;
	   skn->readIcon  = ICQICON_READMESSAGE;
	}
	
	icqskin_setButtonIcon(icqskin_getDlgItemHandle(skn->mainWindow,ID_SYSMSG),skn->eventIcon);
 }

#ifdef EXTENDED_LOG
 icqBeginUserEventProcessor(ICQEVENT_MESSAGEADDED,icq,uin,parm)
 {
    char buffer[0x0100];
    sprintf(buffer,"Message added in queue. UIN: %ld  ID: %08lx",uin,(ULONG) parm);
    DBGMessage(buffer);
    icqWriteSysLog(icq,PROJECT,buffer);
    return 0;
 }
#endif

 icqBeginGuiEventProcessor(ICQEVENT_POPUP,icq,uin,parm)
 {
    icqskin_popupUserList(icq);
    return 0;
 }

