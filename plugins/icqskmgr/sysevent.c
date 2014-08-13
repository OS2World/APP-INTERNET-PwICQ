/*
 * sysEvent.c - Processa eventos de sistema
 */

 #define INCL_WIN
 #include <os2.h>
 #include <stdio.h>

 #include "icqskmgr.h"

/*---[ Prototipos ]--------------------------------------------------------------------------*/

 static void ajustSysMsg(HWND, ICQWNDINFO *);

/*---[ Implementacao ]-----------------------------------------------------------------------*/

 void systemEvent(HWND hwnd, ICQWNDINFO *info, USHORT event)
 {
    switch(event)
    {
    case ICQEVENT_STARTUP:
       WinPostMsg(hwnd,WM_USER+11,0,0);
       break;

    case ICQEVENT_ONLINE:
    case ICQEVENT_OFFLINE:
       setModeButton(hwnd,info,icqQueryOnlineMode(info->icq));
       info->flags |= WFLG_SORT;
       break;

    case ICQEVENT_CHANGED:
       setModeButton(hwnd,info,icqQueryOnlineMode(info->icq));
       icqUpdateSysTray(info->icq);
       break;

    case ICQEVENT_MESSAGECHANGED:
       DBGMessage("Fila do sistema foi mudada");
       ajustSysMsg(hwnd,info);
       break;

    case ICQEVENT_CONNECTING:
    case ICQEVENT_FINDINGSERVER:
       setFrameIcon(hwnd,ICQICON_CONNECTING);
       setConnecting(hwnd,info);
       icqUpdateSysTray(info->icq);
       break;

    case ICQEVENT_ABEND:
       WinPostMsg(hwnd,WM_CLOSE,0,0);
       break;

    case ICQEVENT_HIDE:
       DBGMessage("HIDE Window");
       WinShowWindow(info->frame,FALSE);
       break;

    case ICQEVENT_SHOW:
       DBGMessage("Show Window");
       WinShowWindow(info->frame,TRUE);
       break;

    case ICQEVENT_SECONDARY:
       icqReconnect(info->icq);
       WinPostMsg(hwnd,WM_USER+11,0,0);
       break;

    }

    icqExecuteGuiListeners(info->icq, 0, 'S', event, 0);

 }

 void setModeButton(HWND hwnd, ICQWNDINFO *info, ULONG sel)
 {
    const ICQMODETABLE  *mode;

    DBGTracex(icqQueryModeTable(info->icq));

    if(!icqQueryModeTable(info->icq))
    {
       WinEnableWindow(WinWindowFromID(hwnd,101),FALSE);
       return;
    }

    for(mode = icqQueryModeTable(info->icq);mode->descr;mode++)
    {
       if(mode->mode == sel)
       {
          info->flags &= ~WFLG_CONNECTING;
          DBGTracex(mode);
          WinSendDlgItemMsg(hwnd,101,WMSKN_INSERTHANDLE,MPFROMP(mode),0);
          WinPostMsg(hwnd,WM_USER+17,0,0);
          return;
       }
    }
 }

 void setConnecting(HWND hwnd, ICQWNDINFO *info)
 {
    static ICQMODETABLE mode = { 0xFFFFFFFF, 18, "Connecting" };

    mode.mode = icqQueryOfflineMode(info->icq);

    info->flags |= WFLG_CONNECTING;
    WinSendDlgItemMsg(hwnd,101,WMSKN_INSERTHANDLE,MPFROMP(&mode),0);
    WinSendMsg(hwnd,WM_USER+17,MPFROMP(mode.descr),0);

 }

 static void ajustSysMsg(HWND hwnd, ICQWNDINFO *ctl)
 {
    HMSG msg = icqQueryMessage(ctl->icq,0);

    DBGTracex(msg);

    if(!msg)
    {
       ctl->eventIcon = ctl->modeIcon = ctl->readIcon;
    }
    else if(msg->mgr)
    {
       ctl->readIcon  = msg->mgr->icon[1];
       ctl->eventIcon = msg->mgr->icon[0];
    }
    else
    {
       CHKPoint();
       ctl->readIcon  = 1;
       ctl->eventIcon = 0;
    }

    CHKPoint();
    WinPostMsg(hwnd,WM_USER+17,0,0);
    WinInvalidateRect(WinWindowFromID(hwnd,103),0,FALSE);
    CHKPoint();

 }
