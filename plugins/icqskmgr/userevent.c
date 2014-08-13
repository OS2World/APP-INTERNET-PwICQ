/*
 * USEREVENT.C - User event processing
 */


 #define INCL_WIN
 #include <os2.h>
 #include <stdio.h>

 #include "icqskmgr.h"

/*---[ Prototipos ]---------------------------------------------------------------------------------*/

 static void showUser(ICQWNDINFO *, HWND, ULONG);
 static void hideUser(ICQWNDINFO *, HWND, ULONG);

/*---[ Implementacao ]------------------------------------------------------------------------------*/

 void userEvent(HWND hwnd, USHORT code, ULONG user)
 {
    ICQWNDINFO *ctl = WinQueryWindowPtr(hwnd,0);

    switch(code)
    {
    case ICQEVENT_ICONCHANGED:          /* User's icon has changed */
       sknRedrawControl(hwnd,1003);
       break;

    case ICQEVENT_MESSAGECHANGED:       /* Current active message has changed */
       sknRedrawControl(hwnd,1003);
       WinPostMsg(hwnd,WM_USER+17,0,0);
       break;

    case ICQEVENT_DELETED:
       DBGMessage("User deleted, redrawing from list");
       WinPostMsg(hwnd,WM_USER+11,0,0); /* Force an imediate redraw */
       break;

    case ICQEVENT_POSCHANGED:           /* User's positioning has changed */
    case ICQEVENT_CHANGED:              /* User's status has changed */
       ctl->flags |= WFLG_SORT;
       break;

    case ICQEVENT_UPDATED:              /* User information was updated */
    case ICQEVENT_HIDE:                 /* Hide user */
        ctl->flags |= WFLG_REDRAW;
        break;

    case ICQEVENT_SHOW:
       showUser(ctl,hwnd,user);
       break;

#ifdef EXTENDED_LOG
    case ICQEVENT_MESSAGEADDED:
       icqWriteSysLog(ctl->icq,PROJECT,"Message added in queue");
       break;
#endif
    }

    icqExecuteGuiListeners(ctl->icq, user, 'U', code, 0);

 }

 static void showUser(ICQWNDINFO *ctl, HWND hwnd, ULONG uin)
 {
    int pos = sknQueryHandler(hwnd,1003,icqQueryUserHandle(ctl->icq,uin));
    ctl->flags |= ( pos == -1 ? WFLG_REDRAW : WFLG_SORT);
 }


