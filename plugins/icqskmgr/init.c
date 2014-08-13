/*
 * INIT.C - Create/Destroy the main window
 */

 #define INCL_WIN
 #include <os2.h>

 #include <malloc.h>
 #include <string.h>

 #include "icqskmgr.h"

/*---[ Implementacao ]-----------------------------------------------------------------------*/

 void destroy(HWND hwnd)
 {
    ICQWNDINFO                  *wnd = WinQueryWindowPtr(hwnd,0);
    int                         f;

    WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, 1);

    if(!wnd)
       return;

    if(wnd->icon)
       WinDestroyPointer(wnd->icon);

    sknDestroy(wnd->skn);
 }

 void create(HWND hwnd, PCREATESTRUCT info)
 {
    ICQWNDINFO                  *wnd    = malloc(sizeof(ICQWNDINFO));
    HAB                         hab     = WinQueryAnchorBlock(hwnd);

    if(!wnd)
    {
       WinPostMsg(hwnd,WM_CLOSE,0,0);
       return;
    }

    memset(wnd,0,sizeof(ICQWNDINFO));

    WinSetWindowPtr(hwnd,0,wnd);

    wnd->skn       = sknCreate(hwnd,winElements,info->cx,info->cy,info->pszText);
    wnd->divisor   = 20;
    wnd->modeDiv   = 10;

    wnd->modeIcon  =
    wnd->eventIcon =
    wnd->readIcon  = 10;

    sknConnectListBox(hwnd,1003,1000,1001,1002);

    sknCreatePopupMenu(hwnd, 1003, 2000, WMSKS_SENDACTION);
    sknCreatePopupMenu(hwnd, 101,  2001, WMSKS_SENDACTION);
    sknCreatePopupMenu(hwnd, 102,  2002, WMSKS_SENDACTION);

    *wnd->buffer = 0;
    WinLoadString(hab, module, 1201, 0xFF, (PSZ) wnd->buffer);
    WinSetDlgItemText(hwnd, 102, wnd->buffer);

    *wnd->buffer = 0;
    WinLoadString(hab, module, 1202, 0xFF, (PSZ) wnd->buffer);
    WinSetDlgItemText(hwnd, 103, wnd->buffer);

    WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, 1, TIMER_TICK);

 }


