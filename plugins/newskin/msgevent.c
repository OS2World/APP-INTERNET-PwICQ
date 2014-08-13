/*
 * msgEvent.c - Message editor event processor
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>
 #include <time.h>
 #include <ctype.h>

 #include <pwicqgui.h>

/*---[ Defines ]------------------------------------------------------------------------------------------*/

 #define icqDeclareMsgSystemEvent(e)           void icqMsgEditSysEvent_##e(HICQ,HWND,ICQMSGDIALOG *)
 #define icqMsgSystemEvent(e)                  { 'S', e, (void(*)(void *,HWND,ICQMSGDIALOG *)) icqMsgEditSysEvent_##e }
 #define icqBeginMsgSystemEvent(m,i,h,p)       void icqMsgEditSysEvent_##m(HICQ i,HWND h,ICQMSGDIALOG *p)

 #define icqDeclareMsgUserEvent(e)             void icqMsgEditUserEvent_##e(HICQ,HWND,ICQMSGDIALOG *)
 #define icqMsgUserEvent(e)                    { 'U', e, (void(*)(void *,HWND,ICQMSGDIALOG *)) icqMsgEditUserEvent_##e }
 #define icqBeginMsgUserEvent(m,i,h,p)         void icqMsgEditUserEvent_##m(HICQ i,HWND h, ICQMSGDIALOG *p)

/*---[ Event Processors ]---------------------------------------------------------------------------------*/

 icqDeclareMsgUserEvent(ICQEVENT_ONLINE);
 icqDeclareMsgUserEvent(ICQEVENT_OFFLINE);
 icqDeclareMsgUserEvent(ICQEVENT_CHANGED);
 icqDeclareMsgUserEvent(ICQEVENT_UPDATED);
 icqDeclareMsgUserEvent(ICQEVENT_MESSAGEADDED);
 icqDeclareMsgSystemEvent(ICQEVENT_TIMER);

/*---[ Event Processor table ]----------------------------------------------------------------------------*/

 #pragma pack(1)

 static const struct icqMessageEditorEventProcessor
 {
    UCHAR  type;
    USHORT event;
    void   (*exec)(HICQ,HWND,ICQMSGDIALOG *);
 } procTable[] =
 {
   icqMsgUserEvent(ICQEVENT_ONLINE),
   icqMsgUserEvent(ICQEVENT_OFFLINE),
   icqMsgUserEvent(ICQEVENT_UPDATED),
   icqMsgUserEvent(ICQEVENT_MESSAGEADDED),
   icqMsgSystemEvent(ICQEVENT_TIMER),
   icqMsgUserEvent(ICQEVENT_CHANGED)

 };

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static void _System callBack(HICQ,ULONG,UCHAR,USHORT,ULONG,hWindow);

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 void icqskin_initializeMessageEditor(HICQ icq, hWindow hwnd)
 {
	ICQMSGDIALOG *cfg = icqskin_getWindowDataBlock(hwnd);

    DBGTracex(cfg->flags & ICQMSGDIALOG_CHAT);

    if(cfg->flags & ICQMSGDIALOG_CHAT)
 	   cfg->flags |= ICQMSGDIALOG_LOAD;

    icqAddGuiEventListener(icq, (void *) callBack, (HWND) hwnd);

 }

 void icqskin_terminateMessageEditor(HICQ icq, hWindow hwnd)
 {
    icqRemoveGuiEventListener(icq, (void *) callBack, (HWND) hwnd);
 }

 static void _System callBack(HICQ icq, ULONG uin, UCHAR id, USHORT event, ULONG parm, hWindow hwnd)
 {
    int                                         f;
    const struct icqMessageEditorEventProcessor *ptr = procTable;
	ICQMSGDIALOG                                *cfg = icqskin_getWindowDataBlock(hwnd);

#ifdef DEBUG
    if(id == 'U')
    {
       DBGTrace(event);
       DBGTrace(cfg->uin);
       DBGTrace(uin);
    }
#endif

    if(id == 'U' && cfg->uin != uin)
       return;

    for(f=0;f < (sizeof(procTable) / sizeof(struct icqMessageEditorEventProcessor)); f++)
    {
       if(ptr->type == id && ptr->event == event)
       {
          ptr->exec(icq,hwnd,cfg);
          return;
       }
       ptr++;
    }

    if(cfg->mgr && cfg->mgr->event)
       cfg->mgr->event(&icqskin_dialogCalls,hwnd,cfg->fr.icq,cfg->uin,id,event,parm);

 }

 static void setUserIcon(hWindow hwnd, HICQ icq, ULONG uin)
 {
    HUSER usr = icqQueryUserHandle(icq,uin);

    if(usr)
       icqskin_setDlgItemIcon(hwnd,MSGID_USERMODE,usr->modeIcon);
    else
       icqskin_setDlgItemIcon(hwnd,MSGID_USERMODE,ICQICON_OFFLINE);
 }

 icqBeginMsgUserEvent(ICQEVENT_ONLINE,icq,hwnd,cfg)
 {
    CHKPoint();
    setUserIcon(hwnd,cfg->fr.icq,cfg->uin);
 }

 icqBeginMsgUserEvent(ICQEVENT_OFFLINE,icq,hwnd,cfg)
 {
    CHKPoint();
    setUserIcon(hwnd,cfg->fr.icq,ICQICON_OFFLINE);
 }

 icqBeginMsgUserEvent(ICQEVENT_CHANGED,icq,hwnd,cfg)
 {
    CHKPoint();
    setUserIcon(hwnd,cfg->fr.icq,cfg->uin);
 }

 icqBeginMsgUserEvent(ICQEVENT_UPDATED,icq,hwnd,cfg)
 {
    icqskin_setMessageUser(hwnd, cfg->uin);
 }

 icqBeginMsgUserEvent(ICQEVENT_MESSAGEADDED,icq,hwnd,cfg)
 {
    DBGTracex(icqQueryNextMessage(cfg->fr.icq, cfg->uin, NULL));

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"Reloading user queue");
#endif

    cfg->flags       |= ICQMSGDIALOG_LOAD;
 }

 static void addMessage(hWindow hwnd, ICQMSGDIALOG *cfg, HUSER usr, HMSG msg)
 {
    time_t  ltime;
    char    buffer[100];
    char    *ptr            =  (char *) (msg+1);

    cfg->idleSeconds  = 0;
    cfg->flags       |= ICQMSGDIALOG_SHOW;

    if(msg->msgTime)
       ltime = msg->msgTime;
    else
       time(&ltime);

    strftime(buffer, 69, "%m/%d/%Y %H:%M:%S ", localtime(&ltime));
    DBGMessage(buffer);

    if(usr)
       strncat(buffer,icqQueryUserNick(usr),99);
    *(buffer+99) = 0;

    CHKPoint();
    DBGMessage( (char *) (msg+1) );
    CHKPoint();

    while(*ptr && isspace(*ptr))
       ptr++;

    if(*ptr)
       icqskin_insertMessageHistory(hwnd, 0, buffer, ptr);
#ifdef EXTENDED_LOG
    else
       icqWriteSysLog(cfg->fr.icq,PROJECT,"No text in message, ignoring add request");
#endif

    CHKPoint();

    msg->msgFlags |= MSGF_READ;
    icqRemoveMessage(cfg->fr.icq, cfg->uin, msg);

    CHKPoint();

 }

 icqBeginMsgSystemEvent(ICQEVENT_TIMER,icq,hwnd,cfg)
 {
    HMSG         msg;
    HMSG         next;
    HUSER        usr;
    USHORT       autoHide = icqLoadValue(icq,"msgAutoHide",120);


#if defined(GTK1) || defined(GTK2)
	if(!GTK_WIDGET_DRAWABLE(hwnd))
	   return;
#endif

#ifdef __OS2__
    if(!WinIsWindowVisible(hwnd))
       return;
#endif	
	
    cfg->idleSeconds++;

    if(cfg->uin && (cfg->flags & ICQMSGDIALOG_LOAD))
    {
       cfg->flags &= ~ICQMSGDIALOG_LOAD;

       if(cfg->flags & ICQMSGDIALOG_CHAT)
	   {
#ifdef EXTENDED_LOG
          icqWriteSysLog(icq,PROJECT,"Reading user message queue");
#endif
          usr = icqQueryUserHandle(icq,cfg->uin);
          for(next = msg = icqQueryNextMessage(cfg->fr.icq, cfg->uin, NULL); next; msg = next)
          {
             next = icqQueryNextMessage(cfg->fr.icq, cfg->uin, msg);

#ifdef EXTENDED_LOG
             sprintf(cfg->buffer,"MSG %08lx Flag: %08lx Type: %04x Mgr.type: %04x",
             				(ULONG) msg,
             				(msg->msgFlags&MSGF_USECHATWINDOW),
             				(int) msg->type,
             				(int) cfg->mgr->type );
             icqWriteSysLog(icq,PROJECT,cfg->buffer);             				
#endif

             if( (msg->msgFlags&MSGF_USECHATWINDOW) || (msg->type == cfg->mgr->type) )
             {
                addMessage(hwnd,cfg,usr,msg);
             }
             else if(msg->mgr)
             {
                if(msg->mgr->flags & ICQMSGMF_USECHATWINDOW)
                   addMessage(hwnd,cfg,usr,msg);
			 }
          }
       }
	   else
	   {
	      icqskin_setChildEnabled(hwnd,MSGID_NEXTBUTTON,TRUE);
	   }
    }

    if(autoHide > 0)
    {
       if( !(cfg->flags & ICQMSGDIALOG_HIDE) && (cfg->idleSeconds > autoHide))
       {
          cfg->flags |= ICQMSGDIALOG_HIDE;
          icqskin_hideFrame(hwnd);
       }

    }

    if(cfg->flags & ICQMSGDIALOG_SHOW)
    {
       cfg->flags &= ~(ICQMSGDIALOG_HIDE|ICQMSGDIALOG_SHOW);
       icqskin_showFrame(hwnd);
    }

 }


