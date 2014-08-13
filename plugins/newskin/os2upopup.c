/*
 * os2upopup.c - OS2 User popup window
 */

 #pragma strings(readonly)

 #define INCL_WIN
 #define INCL_GPI
 #include <string.h>
 #include <stdlib.h>
 #include <malloc.h>
 #include <pwicqgui.h>

/*---[ Defines ]------------------------------------------------------------------------------------------*/

 #define CHILD_WINDOW(h,i) WinWindowFromID(h,DLG_ELEMENT_ID+i)

/*---[ Structures ]---------------------------------------------------------------------------------------*/

 #pragma pack(1)

 typedef struct _userpopup
 {
    ICQFRAME fr;

 } USERPOPUP;

 #pragma pack()

/*---[ Constants ]----------------------------------------------------------------------------------------*/

 const char *icqUPopupWindowClass = "pwICQUserPopupWindow";

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/


/*---[ Implementation ]-----------------------------------------------------------------------------------*/


 void icqskin_popupUserList(HICQ icq)
 {
    ULONG      ulFrameFlags = FCF_TITLEBAR|FCF_SIZEBORDER|FCF_MINMAX|FCF_TASKLIST|FCF_SYSMENU|FCF_NOMOVEWITHOWNER;
    HWND       frame;
    HWND       hwnd    = 0;
    USERPOPUP  *cfg;
    POINTL     p;

    frame = WinCreateStdWindow(   HWND_DESKTOP,
                                  0,
                                  &ulFrameFlags,
                                  (PSZ) icqUPopupWindowClass,
                                  (PSZ) "Contact-List",
                                  WS_VISIBLE,
                                  (HMODULE) module,
                                  1000,
                                  &hwnd
                              );

    DBGTracex(frame);

    if(!hwnd)
       return;

    DBGTracex(icq);

    cfg = WinQueryWindowPtr(hwnd,0);
    WinSetWindowPtr(frame,QWL_USER,cfg);

    /* Configure window */
    icqskin_setICQHandle(hwnd,icq);
    icqskin_setFrameName(hwnd,"UserPopup");
//    icqskin_createChilds(hwnd);




    /* Show popup */
    WinQueryPointerPos(HWND_DESKTOP, &p);
    p.x -= 6;
    p.y -= 6;

    if(p.x < 0)
       p.y = 0;

    if(p.y < 0)
       p.y = 0;

    CHKPoint();

    WinSetWindowPos(frame,HWND_TOP,p.x,p.y,150,300,SWP_SIZE|SWP_MOVE|SWP_ZORDER|SWP_SHOW|SWP_ACTIVATE);

 }

 MRESULT EXPENTRY icqUserPopupWindow( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    switch(msg)
    {
    case WM_CREATE:
       icqskin_cfgWindow(hwnd,USERPOPUP,0);
       break;

    case WMICQ_CREATECHILDS:
       break;

    default:
       return icqFrameWindow(hwnd, msg, mp1, mp2);

    }

    return 0;

 }


