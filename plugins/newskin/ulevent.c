/*
 * ulEvent.c - User's list event processor
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>
 #include <pwicqgui.h>

/*---[ Defines ]------------------------------------------------------------------------------------------*/

/*---[ Event Processors ]---------------------------------------------------------------------------------*/

 icqDeclareUserlistEventProcessor(ICQEVENT_TIMER);
 icqDeclareUserlistEventProcessor(ICQEVENT_POSCHANGED);
 icqDeclareUserlistEventProcessor(ICQEVENT_OFFLINE);
 icqDeclareUserlistEventProcessor(ICQEVENT_ICONCHANGED);
 icqDeclareUserlistEventProcessor(ICQEVENT_CHANGED);
 icqDeclareUserlistEventProcessor(ICQEVENT_SHOW);
 icqDeclareUserlistEventProcessor(ICQEVENT_HIDE);
 icqDeclareUserlistEventProcessor(ICQEVENT_UPDATED);
 icqDeclareUserlistEventProcessor(ICQEVENT_MESSAGECHANGED);

 static void BlinkTimer(HICQ,HWND,ULONG);

/*---[ Event Processor table ]----------------------------------------------------------------------------*/

 #pragma pack(1)

 static const struct icqUserlistEventProcessor
 {
    UCHAR  type;
    USHORT event;
    void   (*exec)(HICQ,HWND,ULONG);
 } procTable[] =
 {
    { 'g', ICQEVENT_TIMER, BlinkTimer },

    icqUserlistEventProcessor('U',ICQEVENT_POSCHANGED),
    icqUserlistEventProcessor('U',ICQEVENT_ICONCHANGED),
    icqUserlistEventProcessor('U',ICQEVENT_SHOW),
    icqUserlistEventProcessor('U',ICQEVENT_HIDE),
    icqUserlistEventProcessor('U',ICQEVENT_MESSAGECHANGED),
    icqUserlistEventProcessor('U',ICQEVENT_CHANGED),
    icqUserlistEventProcessor('U',ICQEVENT_UPDATED),
    icqUserlistEventProcessor('S',ICQEVENT_OFFLINE),
    icqUserlistEventProcessor('S',ICQEVENT_TIMER)
 };

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static void _System callBack(HICQ,ULONG,UCHAR,USHORT,ULONG,hWindow);

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 void icqskin_initializeUserList(HICQ icq, hWindow hwnd)
 {
    icqAddGuiEventListener(icq, (void *) callBack, (HWND) hwnd);
 }

 void icqskin_terminateUserList(HICQ icq, hWindow hwnd)
 {
    icqRemoveGuiEventListener(icq, (void *) callBack, (HWND) hwnd);
 }

 void _System callBack(HICQ icq, ULONG uin, UCHAR id, USHORT event, ULONG parm, hWindow hwnd)
 {
    int                                    f;
    const struct icqUserlistEventProcessor *ptr = procTable;

    for(f=0;f < (sizeof(procTable) / sizeof(struct icqUserlistEventProcessor)); f++)
    {
       if(ptr->type == id && ptr->event == event)
       {
          ptr->exec(icq,hwnd,uin);
       }
       ptr++;
    }
 }

 icqBeginUserlistEventProcessor(ICQEVENT_TIMER,hwnd,icq,uin)
 {
    ULONG *flags = icqskin_getUserListFlagPointer(hwnd);

    if(*flags & ICQULIST_SORT)
    {
	   icqWriteSysLog(icq,PROJECT,"Reordering user list");
       icqskin_sortUserList(hwnd);
       *flags &= ~ICQULIST_SORT;
    }

 }

 icqBeginUserlistEventProcessor(ICQEVENT_POSCHANGED,hwnd,icq,uin)
 {
    icqskin_setUserListFlag(hwnd,ICQULIST_SORT);
 }

 icqBeginUserlistEventProcessor(ICQEVENT_OFFLINE,hwnd,icq,uin)
 {
	/* System is offline */
	HUSER usr;
	
	for(usr = icqQueryFirstUser(icq);usr;usr = icqQueryNextUser(icq,usr))
	   icqskin_updateUserIcon(hwnd,usr->uin);
	
    icqskin_setUserListFlag(hwnd,ICQULIST_SORT);
 }

 icqBeginUserlistEventProcessor(ICQEVENT_ICONCHANGED,hwnd,icq,uin)
 {
    icqskin_updateUserIcon(hwnd,uin);
 }

 icqBeginUserlistEventProcessor(ICQEVENT_CHANGED,hwnd,icq,uin)
 {
    icqskin_updateUserIcon(hwnd,uin);
 }

 static void BlinkTimer(HICQ icq, HWND hwnd, ULONG uin)
 {
    BOOL  event;
    BOOL  mode;
    HUSER usr;

    icqskin_switchBlinkStatus(hwnd);

    icqWalkUsers(icq,usr)
    {
       event = (usr->eventIcon != usr->modeIcon);
       mode  = (usr->flags & USRF_BLINK);
       if(event || mode)
          icqskin_blinkUser(hwnd,usr->uin,mode,event);
    }
 }

 static void updateUser(HWND hwnd, HICQ icq, ULONG uin)
 {
    SKINDATA *cfg = icqskin_getDataBlock(icq);
    HUSER    usr;

	CHKPoint();
	
	if( !(cfg->flags & SKINDATA_FLAGS_LOADED) )
	   return;

    usr = icqskin_queryUserListEntry(hwnd,uin);

    DBGTrace(uin);
    DBGTracex(usr);
    DBGTracex(icqQueryUserHandle(icq,uin));

    if(!usr)
    {
       DBGMessage("Adding new user in the list");
       usr = icqQueryUserHandle(icq,uin);

       if(usr)
          icqskin_addUser(hwnd,usr);
    }
 }

 icqBeginUserlistEventProcessor(ICQEVENT_UPDATED,hwnd,icq,uin)
 {
    icqskin_updateUserHandle(hwnd,uin);
 }

 icqBeginUserlistEventProcessor(ICQEVENT_MESSAGECHANGED,hwnd,icq,uin)
 {
    DBGPrint("User message queue was changed, msg=%08lx",(ULONG) icqQueryNextMessage(icq, uin, NULL));
    updateUser(hwnd,icq,uin);
 }

 icqBeginUserlistEventProcessor(ICQEVENT_SHOW,hwnd,icq,uin)
 {
    updateUser(hwnd,icq,uin);
 }

 icqBeginUserlistEventProcessor(ICQEVENT_HIDE,hwnd,icq,uin)
 {
    CHKPoint();
    icqskin_delUser(hwnd,uin);
 }


