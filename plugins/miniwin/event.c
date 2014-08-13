/*
 * Event Processor
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdio.h>

 #include "icqminiw.h"


/*---[ Definitions ]-------------------------------------------------------------------------------------*/


/*---[ Statics ]-----------------------------------------------------------------------------------------*/

 static void statusChanged(HWND,HICQ);

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/

/*---[ Implementation ]----------------------------------------------------------------------------------*/


 void icqminiw_systemEvent(HWND hwnd, HICQ icq, USHORT code)
 {
    switch(code)
    {
    case ICQEVENT_CONNECTING:
       icqminiw_setIcon(hwnd,39);
       break;

    case ICQEVENT_ONLINE:
    case ICQEVENT_OFFLINE:
    case ICQEVENT_CHANGED:
       statusChanged(hwnd,icq);
       break;

    case ICQEVENT_MESSAGECHANGED:
       DBGMessage("System message has changed");
       statusChanged(hwnd,icq);
       break;
    }
 }

 void icqminiw_userEvent(HWND hwnd, HICQ icq, USHORT code, ULONG uin)
 {
    switch(code)
    {
    case ICQEVENT_MESSAGECHANGED:
       DBGMessage("User message has changed");
       statusChanged(hwnd,icq);
       break;
    }


 }

 static void statusChanged(HWND hwnd, HICQ icq)
 {
    HMSG        msg	= 0;

#ifdef LOG_ALL
    icqWriteSysLog(icq,PROJECT,"Status changed");
#endif

    if(icqQueryFirstMessage(icq,NULL,&msg))
    {
       icqminiw_setIcon(hwnd,icqQueryModeIcon(icq,icqQueryOnlineMode(icq)));
       return;
    }

    if(msg && msg->mgr)
    {
       icqminiw_setBlink(hwnd,msg->mgr->icon[0]);
       return;
    }

    icqminiw_setBlink(hwnd,0);

 }


