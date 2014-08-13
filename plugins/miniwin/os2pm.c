/*
 * os2pm.c - Create a small drag&drop icon on desktop for pwICQ Status information
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_WIN
 #define INCL_GPILCIDS
 #define INCL_GPIPRIMITIVES
#else
 #error Only for OS2 Version
#endif

#ifndef WS_TOPMOST
 #define WS_TOPMOST 0x00200000L
#endif

 #include <malloc.h>
 #include <stdio.h>
 #include <string.h>
 #include "icqminiw.h"


/*---[ Definitions ]-------------------------------------------------------------------------------------*/

 #define BITMAP_ROWS    16
 #define BITMAP_COUNTER 53

 #pragma pack(1)
 typedef struct wdgdata
 {
    USHORT      sz;
    HICQ        icq;
    HWND        frame;
    ULONG       dragPos;
    PDRAGINFO   dragInfo;
    HBITMAP     icons;
    USHORT      selected;
    BOOL        blink;
    UCHAR       tick;
 } WDGDATA;

/*---[ Statics ]-----------------------------------------------------------------------------------------*/

 static const char      *className      = "pwICQMINIWINDOW";

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/

 static void            create(HWND,PCREATESTRUCT);
 static void            destroy(HWND);
 static void            paint(HWND);
 static int _System     callback(HICQ, HWND, const char *, void *);
 static void            initialize(HWND,HICQ,HWND);
 static void            beginDrag(HWND,ULONG);
 static void            endDrag(HWND);
 static void            mouseMouse(HWND,SHORT,SHORT);
 static MRESULT         dragOver(HWND,PDRAGINFO);
 static void            dragLeave(HWND);
 static MRESULT         drop(HWND);
 static void            event(HWND, WDGDATA *, UCHAR, USHORT, ULONG);
 static void            setIcon(HWND,WDGDATA *,ULONG);
 static void            setBlink(HWND,WDGDATA *,ULONG);
 static void            doTimer(HWND,WDGDATA *);

 MRESULT EXPENTRY icqMiniStatusWindow(HWND, ULONG, MPARAM, MPARAM);

/*---[ Implementation ]----------------------------------------------------------------------------------*/

 void icqminiw_initializeGui(HICQ icq)
 {
    CHKPoint();
    WinRegisterClass(icqQueryAnchorBlock(icq), (PSZ) className, icqMiniStatusWindow, 0, sizeof(WDGDATA *));
 }

 void EXPENTRY icqminiw_Event(HICQ icq, HWND *hwnd, ULONG uin, char id, USHORT eventCode, ULONG parm)
 {
    DBGTrace(id == 'S');

    if(*hwnd == 4)
    {
       return;
    }
    else if(id == 'S' && eventCode == ICQEVENT_SECONDARY)
    {
       CHKPoint();
       if(icqLoadValue(icq,PROJECT ":Enable", 1))
          icqOpenWindow(icq, 0, className, callback, hwnd);
    }
    else if(*hwnd)
    {
       WinPostMsg(*hwnd,WM_USER+1001,MPFROMSH2CH(eventCode, id, 0),MPFROMLONG(uin));
    }

 }

 static int _System callback(HICQ icq, HWND owner, const char *wndClass, void *wndSave)
 {
    ULONG  ulFrameFlags  = FCF_TASKLIST|FCF_NOMOVEWITHOWNER|FCF_NOBYTEALIGN;
    HWND   frame;
    HWND   mainWin;

    DBGTracex(wndSave);

    frame = WinCreateStdWindow(  HWND_DESKTOP,
                                 WS_VISIBLE|WS_TOPMOST,
                                 &ulFrameFlags,
                                 (PSZ) wndClass,
                                 "pwICQ Mini Window",
                                 0,
                                 module,
                                 100,
                                 &mainWin
                             );

    if(!mainWin)
    {
       icqWriteSysLog(icq,PROJECT,"Failure creating child window");
       return -1;
    }

    WinSendMsg(mainWin,WM_USER+1000,(MPARAM) icq, (MPARAM) frame);

    *( (HWND *) wndSave) = mainWin;

    return 0;
 }

 MRESULT EXPENTRY icqMiniStatusWindow(HWND hwnd, ULONG ulMessage, MPARAM mp1, MPARAM mp2)
 {

    switch(ulMessage)
    {
    case WM_CREATE:
       create(hwnd,PVOIDFROMMP(mp2));
       return WinDefWindowProc(hwnd, ulMessage, mp1, mp2);

    case WM_PAINT:
       paint(hwnd);
       break;

    case WM_DESTROY:
       destroy(hwnd);
       return WinDefWindowProc(hwnd, ulMessage, mp1, mp2);

    case WM_BEGINDRAG:
       beginDrag(hwnd,(ULONG) mp1);
       return (MRESULT) TRUE;

    case WM_MOUSEMOVE:
       mouseMouse(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1));
       return WinDefWindowProc(hwnd, ulMessage, mp1, mp2);

    case WM_ENDDRAG:
       endDrag(hwnd);
       break;

    case WM_USER+1000:  // mp1 = HICQ, mp2 = Frame Window
       initialize(hwnd, (HICQ) mp1, (HWND) mp2);
       break;

    case WM_USER+1001:
       event(hwnd,WinQueryWindowPtr(hwnd,0),CHAR3FROMMP(mp1),SHORT1FROMMP(mp1),LONGFROMMP(mp2));
       break;

    case WM_USER+1002:  // Set Icon = MP1 = code
       setIcon(hwnd,WinQueryWindowPtr(hwnd,0),LONGFROMMP(mp1));
       break;

    case WM_USER+1003:  // Set Icon = MP1 = code
       setBlink(hwnd,WinQueryWindowPtr(hwnd,0),LONGFROMMP(mp1));
       break;

    case DM_DRAGOVER:
       return dragOver(hwnd, (PDRAGINFO)mp1);

    case DM_DRAGLEAVE:
       return (MRESULT)FALSE;
       break;

    case WM_TIMER:
       doTimer(hwnd,WinQueryWindowPtr(hwnd,0));
       break;

    case DM_DROP:
       return drop(hwnd);

    case WM_BUTTON1CLICK:
       icqminiw_openButton(hwnd,((WDGDATA *) WinQueryWindowPtr(hwnd,0))->icq);
       return (MRESULT) TRUE;

    case WM_BUTTON2CLICK:
       icqminiw_modeButton(hwnd,((WDGDATA *) WinQueryWindowPtr(hwnd,0))->icq);
       return (MRESULT) TRUE;

    case WM_BUTTON3CLICK:
       icqminiw_sysButton(hwnd,((WDGDATA *) WinQueryWindowPtr(hwnd,0))->icq);
       return (MRESULT) TRUE;

    default:
       return WinDefWindowProc(hwnd, ulMessage, mp1, mp2);

    }

    return 0;

 }

 static void create(HWND hwnd, PCREATESTRUCT ctl)
 {
    WDGDATA *wnd = malloc(sizeof(WDGDATA));

    if(!wnd)
    {
       WinPostMsg(hwnd,WM_CLOSE,0,0);
       return;
    }

    memset(wnd,0,sizeof(WDGDATA));
    wnd->sz       = sizeof(WDGDATA);
    wnd->dragPos  = 0xFFFFFFFF;
    wnd->selected = 10;

    WinSetWindowPtr(hwnd,0,wnd);

 }

 static void destroy(HWND hwnd)
 {
    WDGDATA *wnd = WinQueryWindowPtr(hwnd,0);

    if(wnd)
    {
       WinSetWindowPtr(hwnd,0,0);
       free(wnd);
    }

 }

 static void paint(HWND hwnd)
 {
    WDGDATA     *wnd = WinQueryWindowPtr(hwnd,0);
    POINTL       p;
    RECTL        rcl;
    HPS          hps;
    ULONG        bg;

    hps = WinBeginPaint(hwnd,NULLHANDLE,&rcl);
    WinFillRect(hps, &rcl, CLR_BLACK);

    /* Draw current bitmap */

    if(!wnd->icons)
    {
       wnd->icons = GpiLoadBitmap(hps, module, 100, BITMAP_ROWS*BITMAP_COUNTER, BITMAP_ROWS);
       DBGTracex(wnd->icons);
    }

    p.x = p.y = 3;

    rcl.yTop    = BITMAP_ROWS;
    rcl.xLeft   = BITMAP_ROWS*wnd->selected;

    rcl.yBottom = 0;
    rcl.xRight  = rcl.xLeft+BITMAP_ROWS;

    if(!wnd->blink || (wnd->tick&1))
       WinDrawBitmap(hps, wnd->icons, &rcl, &p, CLR_WHITE, CLR_BLACK, DBM_NORMAL);

    /* Draw window border */

    WinQueryWindowRect(hwnd, &rcl);

    rcl.yTop--;
    rcl.xRight--;

    GpiSetColor(hps,CLR_DARKGRAY);

    p.y = rcl.yTop;
    p.x = rcl.xRight;
    GpiMove(hps,&p);
    p.x = rcl.xLeft;
    GpiLine(hps,&p);
    p.y = rcl.yBottom;
    GpiLine(hps,&p);
    p.x = rcl.xRight;
    GpiLine(hps,&p);
    p.y = rcl.yTop;
    GpiLine(hps,&p);


    WinEndPaint(hps);

 }

 static void initialize(HWND hwnd, HICQ icq, HWND frame)
 {
    WDGDATA     *wnd = WinQueryWindowPtr(hwnd,0);
    ULONG       pos  = icqLoadValue(icq,PROJECT":pos",0xFFFFFFFF);
    SWP         p;

    wnd->icq   = icq;
    wnd->frame = frame;

    WinQueryWindowPos(HWND_DESKTOP, &p);

    if(SHORT1FROMMR(pos) < p.cx && SHORT2FROMMR(pos) < p.cy)
       WinSetWindowPos(frame, 0, SHORT1FROMMR(pos), SHORT2FROMMR(pos), 22, 22, SWP_MOVE|SWP_SIZE);
    else
       WinSetWindowPos(frame, 0, p.cx-22, 25, 22, 22, SWP_MOVE|SWP_SIZE);

    WinSetWindowPos(frame, 0, 100, 2, 22, 22, SWP_ACTIVATE|SWP_SHOW);

    icqminiw_setIcon(hwnd,icqQueryModeIcon(icq,icqQueryOnlineMode(icq)));

 }

 static void beginDrag(HWND hwnd,ULONG vlr)
 {
    WDGDATA *wnd = WinQueryWindowPtr(hwnd,0);
    WinSetCapture(HWND_DESKTOP,hwnd);
    wnd->dragPos = vlr;
 }

 static void endDrag(HWND hwnd)
 {
    WDGDATA *wnd = WinQueryWindowPtr(hwnd,0);
    SWP     swp;
    ULONG   pos;

    WinSetCapture(HWND_DESKTOP,NULLHANDLE);
    wnd->dragPos = 0xFFFFFFFF;

    if(!WinQueryWindowPos(wnd->frame, &swp))
       return;

    pos = (ULONG) MRFROM2SHORT(swp.x,swp.y);
    icqSaveValue(wnd->icq,PROJECT":pos",pos);

 }

 static void mouseMouse(HWND hwnd, SHORT x, SHORT y)
 {
    WDGDATA *wnd = WinQueryWindowPtr(hwnd,0);
    POINTS  p;
    SWP     swp;

    if(wnd->dragPos == 0xFFFFFFFF)
       return;

   *( (ULONG *) &p ) = wnd->dragPos;

   x -= p.x;
   y -= p.y;

   if(!WinQueryWindowPos(wnd->frame, &swp))
      return;

   swp.x += x;
   swp.y += y;

   WinSetWindowPos(wnd->frame, 0, swp.x, swp.y, 0,0, SWP_MOVE);

 }

 static MRESULT dragOver(HWND hwnd, PDRAGINFO dragInfo)
 {
    WDGDATA     *wnd = WinQueryWindowPtr(hwnd,0);
    int         i;
    USHORT      cItems;
    PDRAGITEM   dragItem;
    USHORT      rc              = DOR_NODROPOP;
    char buffer[0x0100];

    wnd->dragInfo = dragInfo;

    DrgAccessDraginfo(dragInfo);

    cItems = DrgQueryDragitemCount(dragInfo);

    for (i = 0; i < cItems && rc == DOR_NODROPOP; i++)
    {
       dragItem = DrgQueryDragitemPtr(dragInfo, i);

       DrgQueryStrName(dragItem->hstrType, 0xFF, buffer);
       DBGMessage(buffer);

    }

    DrgFreeDraginfo(dragInfo);
    return (MRFROM2SHORT(rc, DO_COPY));

 }

 static void dragLeave(HWND hwnd)
 {
    WDGDATA     *wnd = WinQueryWindowPtr(hwnd,0);
    wnd->dragInfo = NULL;
 }

 static MRESULT drop(HWND hwnd)
 {
    WDGDATA     *wnd = WinQueryWindowPtr(hwnd,0);
    char        buffer[0x0100];
    int         i;
    USHORT      cItems;
    PDRAGITEM   dragItem;
    int         sz;

    DBGTracex(wnd->dragInfo);

    if(!wnd->dragInfo)
       return (MRFROM2SHORT (DOR_NODROPOP, 0));

    DrgAccessDraginfo(wnd->dragInfo);

    cItems = DrgQueryDragitemCount(wnd->dragInfo);
    DBGTrace(cItems);
    for(i = 0; i < cItems; i++)
    {
       dragItem = DrgQueryDragitemPtr(wnd->dragInfo, i);

       DrgQueryStrName(dragItem->hstrType, 0xFF, buffer);
       if(!strcmp(buffer,"UniformResourceLocator"))
       {
          sz = DrgQueryStrName(dragItem->hstrSourceName, 0xFF, buffer);
          DBGTrace(sz);

          if(sz > 0)
          {
             *(buffer+sz) = 0;
             DBGMessage(buffer);
          }
       }
    }

    CHKPoint();

    DrgFreeDraginfo(wnd->dragInfo);
    wnd->dragInfo = NULL;

    CHKPoint();
    return (MRFROM2SHORT(DOR_DROP, DO_COPY));
 }

 static void event(HWND hwnd, WDGDATA *wnd, UCHAR type, USHORT code, ULONG uin)
 {
    switch(type)
    {
    case 'S':
       icqminiw_systemEvent(hwnd,wnd->icq,code);
       break;

    case 'U':
       icqminiw_userEvent(hwnd,wnd->icq,code,uin);
       break;
    }

 }

 static void setIcon(HWND hwnd,WDGDATA *wnd,ULONG icon)
 {
    wnd->selected = icon;

    if(wnd->blink)
    {
       wnd->blink    = FALSE;
       WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, 1);
    }
    WinInvalidateRect(hwnd,0,0);
 }

 static void setBlink(HWND hwnd,WDGDATA *wnd,ULONG icon)
 {
    wnd->selected = icon;

    CHKPoint();

    if(!wnd->blink)
    {
       wnd->blink = TRUE;
       WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, 1, 500);
    }
 }

 static void doTimer(HWND hwnd,WDGDATA *wnd)
 {
    wnd->tick++;
    if(wnd->blink)
       WinInvalidateRect(hwnd,0,0);
 }


