/*
 * os2pm.c - Create a small drag&drop icon on desktop with respects the paused status for new jobs
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_WIN
 #define INCL_GPILCIDS
 #define INCL_GPIPRIMITIVES
 #include <os2.h>
#else
 #error Only for OS2 Version
#endif

#ifndef WS_TOPMOST
 #define WS_TOPMOST 0x00200000L
#endif

 #include <malloc.h>
 #include <stdio.h>
 #include <string.h>
 #include "icqpmdc.h"

 #include <icqtlkt.h>

/*---[ Definitions ]-------------------------------------------------------------------------------------*/

 #pragma pack(1)
 typedef struct wdgdata
 {
    USHORT      sz;
    HICQ        icq;
    HWND        frame;
    HBITMAP     bitmap;
    ULONG       dragPos;
    PDRAGINFO   dragInfo;
 } WDGDATA;

/*---[ Statics ]-----------------------------------------------------------------------------------------*/

 static const char      *className      = "pwICQPMDC";

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

 MRESULT EXPENTRY icqPmdcWindow(HWND, ULONG, MPARAM, MPARAM);

/*---[ Implementation ]----------------------------------------------------------------------------------*/

 void icqpmdc_initializeGui(HICQ icq)
 {
    WinRegisterClass(icqQueryAnchorBlock(icq), (PSZ) className, icqPmdcWindow, 0, sizeof(WDGDATA *));
 }

 void icqpmdc_createWindow(HICQ icq)
 {
    if(icqLoadValue(icq,PROJECT ":DropBox", 0))
       icqOpenWindow(icq, 0, className, callback, NULL);
 }

 static int _System callback(HICQ icq, HWND owner, const char *wndClass, void *parm)
 {
    ULONG  ulFrameFlags  = FCF_TASKLIST|FCF_NOMOVEWITHOWNER|FCF_NOBYTEALIGN;
    HWND   frame;
    HWND   mainWin;

    CHKPoint();

    frame = WinCreateStdWindow(  HWND_DESKTOP,
                                 WS_VISIBLE|WS_TOPMOST,
                                 &ulFrameFlags,
                                 (PSZ) wndClass,
                                 "PMDC Drop Down",
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
    WinSendMsg(mainWin,WM_USER+1001,(MPARAM) parm, 0);

    CHKPoint();

    return 0;
 }

 MRESULT EXPENTRY icqPmdcWindow(HWND hwnd, ULONG ulMessage, MPARAM mp1, MPARAM mp2)
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

    case WM_USER+1001:  // mp1 = parameters
       break;

    case DM_DRAGOVER:
       return dragOver(hwnd, (PDRAGINFO)mp1);

    case DM_DRAGLEAVE:
       return (MRESULT)FALSE;
       break;

    case DM_DROP:
       return drop(hwnd);

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
    wnd->sz      = sizeof(WDGDATA);
    wnd->dragPos = 0xFFFFFFFF;

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

    /* Draw window border */

    WinQueryWindowRect(hwnd, &rcl);

//   bg = border(hps, style, &rcl,                 CLR_DARKGRAY, CLR_BLACK,    CLR_WHITE, CLR_BLACK);
//        border(HPS hps, ULONG style, PRECTL rcl, ULONG bg,     ULONG border, ULONG c1,  ULONG c2)

    if(!wnd->bitmap)
       wnd->bitmap = GpiLoadBitmap(hps, module, 10, 20, 20);

    WinFillRect(hps, &rcl, CLR_BLACK);
    rcl.yTop--;
    rcl.xRight--;

    p.x = p.y = 1;
    WinDrawBitmap(hps,wnd->bitmap,NULL,&p,0,0,DBM_NORMAL);

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

/*
    WinFillRect(hps, &rcl, CLR_BLACK);
    rcl.yBottom += 1;
    rcl.yTop    -= 2;
    rcl.xLeft   += 1;
    rcl.xRight  -= 1;
    WinFillRect(hps, &rcl, CLR_DARKGRAY);

    p.x = p.y = 1;
    WinDrawBitmap(hps,wnd->bitmap,NULL,&p,0,0,DBM_NORMAL);

    p.y = rcl.yTop;
    p.x = rcl.xRight-1;
    GpiMove(hps,&p);
    GpiSetColor(hps,CLR_WHITE);
    p.x = rcl.xLeft;
    GpiLine(hps,&p);
    p.y = rcl.yBottom;
    GpiLine(hps,&p);

    GpiSetColor(hps,CLR_BLACK);
    p.x = rcl.xRight-1;
    GpiLine(hps,&p);
    p.y = rcl.yTop;
    GpiLine(hps,&p);

    rcl.xLeft++;
    rcl.xRight--;
    rcl.yBottom++;
*/




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

       if(!strcmp(buffer,"UniformResourceLocator"))
       {
#ifdef DEBUG
          rc = DOR_DROP;
#else
          if(DrgQueryStrName(dragItem->hstrSourceName, 0xFF, buffer))
             rc = DOR_DROP;
#endif
       }
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
             icqpmdc_downloadFile(wnd->icq, buffer);
          }
       }
    }

    CHKPoint();

    DrgFreeDraginfo(wnd->dragInfo);
    wnd->dragInfo = NULL;

    CHKPoint();
    return (MRFROM2SHORT(DOR_DROP, DO_COPY));
 }

