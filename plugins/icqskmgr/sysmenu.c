/*
 * SYSMENU.C - Processa selecoes pelo menu do sistema
 */

 #include <stdio.h>

 #include "icqskmgr.h"

/*---[ Prototipos ]---------------------------------------------------------------------------------*/

 static void updateAll(HICQ);

/*---[ Implementacao ]------------------------------------------------------------------------------*/

 void procSysMenu(HWND hwnd, ICQWNDINFO *ctl, ULONG id)
 {
    DBGTrace(id);

    switch(id)
    {
    case ICQICON_DISCONNECT:      /* Disconnect */
       icqWriteSysLog(ctl->icq,PROJECT,"Disconnect request received");
       icqDisconnect(ctl->icq);
       break;

    case 26:                           /* About pwICQ */
       break;

    case 28:                           /* Current user info */
       break;

    case ICQICON_CONFIGURE:            /* Configure */
       icqOpenConfigDialog(ctl->icq, 0, 1);
       break;

    case ICQICON_SEARCH:               /* Search/Add users */
       icqOpenSearchWindow(ctl->icq,0);
       break;

    case ICQICON_UPDATE:               /* Update all users */
       icqWriteSysLog(ctl->icq,PROJECT,"Updating all users in the contact list");
       updateAll(ctl->icq);
       break;

    case ICQICON_ABOUT: // About user
       aboutUser(ctl->icq,icqQueryUIN(ctl->icq),ctl->buffer);
       break;

    case ICQICON_SHUTDOWN:            /* Shutdown */
       icqWriteSysLog(ctl->icq,PROJECT,"Stopping by user request");
       WinPostMsg(hwnd,WM_CLOSE,0,0);
       break;

    }
 }

 static void updateAll(HICQ icq)
 {
    HUSER usr;
    for(usr = icqQueryFirstUser(icq); usr; usr = icqQueryNextUser(icq,usr) )
       usr->flags |= USRF_REFRESH;
 }


