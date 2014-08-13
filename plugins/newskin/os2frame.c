/*
 * os2frame.c - OS2 Frame Window message processing
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

/*---[ Constants ]----------------------------------------------------------------------------------------*/

 const char *icqFrameWindowClass = "pwICQFrameWindow";

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static void    erase(HWND, HPS, PRECTL);
 static void    configure(HWND,int);
 static void    destroy(HWND);
 static void    setbuttons(HWND,const struct icqButtonTable *, int);
 static void    setpallete(HPS, const ULONG *);
 static void    ppchanged(HWND,ULONG);
 static void    paint(HWND);
 static void    broadcast(HWND, ULONG, MPARAM, MPARAM);
 static int     event(HWND, UCHAR, UCHAR, USHORT, ULONG);
 static void    loadSkin(HWND,const char *,SKINFILESECTION *);
 static void    resize(HWND, short, short);
 static int     paintbg(HWND, HPS, PRECTL);
 static MRESULT beginDrag(HWND,POINTS *);
 static MRESULT endDrag(HWND);
 static void    saveMouse(HWND, short, short);
 static void    timer(HWND);
 static void    clearSkin(ICQFRAME *);
 static void    restoreWindow(HWND, const char *, USHORT, USHORT);
 static void    storeWindow(HWND, const char *);
 static void    close(HWND);
 static MRESULT setparm(HWND, PWNDPARAMS, MPARAM);
 static void    autoSize(HWND);
 static void    createchild(HWND, const MSGCHILD *, USHORT);
 static USHORT  sizeButtonBar(HWND, short, USHORT, const USHORT *);
 static int     action(HWND, USHORT, USHORT);
 static void    loadSkin4Childs(HWND,const char *,SKINFILESECTION *);
 static void    skinElement(HWND,HWND,const char **);
 static void    skinChild(HWND, ICQFRAME *, const char *, const char *, const char *, SKINFILESECTION *);
 static void    openMessage(HICQ);

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 MRESULT EXPENTRY icqFrameWindow(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    switch(msg)
    {
    case WM_DDE_INITIATEACK:
       return (MRESULT) icqDDEInitiateAck(hwnd, (HWND)mp1, (PDDEINIT)mp2);

    case WM_DDE_DATA:
       DBGMessage("WM_DDE_DATA");
       icqDDEData(hwnd, (PDDESTRUCT) mp2);
       break;

    case WM_CLOSE:
       DBGMessage("WM_CLOSE");
       close(hwnd);
       break;

    case WM_CREATE:
       icqskin_cfgWindow(hwnd,ICQFRAME,0);
       break;

    case WM_DESTROY:
       destroy(hwnd);
       break;

    case WM_ERASEBACKGROUND:
       erase(hwnd,(HPS) mp1, (PRECTL) mp2);
       break;

    case WM_SIZE:
       resize(hwnd,SHORT1FROMMP(mp2),SHORT2FROMMP(mp2));
       broadcast(hwnd,WMICQ_AUTOSIZE,0,0);
       break;

    case WM_PAINT:
       paint(hwnd);
       break;

    case WM_SETICON:
       WinSendMsg(WinQueryWindow(hwnd,QW_PARENT),msg,mp1,mp2);
       break;

    case WM_COMMAND:
       action(hwnd,SHORT1FROMMP(mp1), SHORT1FROMMP(mp2));
       break;

    case WM_PRESPARAMCHANGED:
       ppchanged(hwnd,LONGFROMMP(mp1));
       break;

    case WM_SETWINDOWPARAMS:
       return setparm(hwnd, PVOIDFROMMP(mp1), mp2);

    case WMICQ_SETNAME:
//       strncpy( ((ICQFRAME *) WinQueryWindowPtr(hwnd,0))->name, (const char *) mp1, 19);
       ((ICQFRAME *) WinQueryWindowPtr(hwnd,0))->name = (const char *) mp1;
       break;

    case WMICQ_SETICQHANDLE:
       DBGTracex(mp1);
       ((ICQFRAME *) WinQueryWindowPtr(hwnd,0))->icq = (HICQ) mp1;
       break;

    case WMICQ_GETICQHANDLE:
       return (MRESULT) ((ICQFRAME *) WinQueryWindowPtr(hwnd,0))->icq;

    case WMICQ_SETBUTTONTBL:
       setbuttons(hwnd,(const struct icqButtonTable *) mp1, (int) mp2);
       break;

    case WMICQ_QUERYPALLETE:
       return (MRESULT) ((ICQFRAME *) WinQueryWindowPtr(hwnd,0))->pal;

    case WMICQ_EVENT:  // MP1 = type,code,user  MP2 = parm
       event(hwnd,CHAR3FROMMP(mp1),CHAR4FROMMP(mp1),SHORT1FROMMP(mp1),(ULONG) mp2);
       break;

    case WMICQ_SELECTPAL:
       icqskin_loadPallete( (HPS) mp1, 0, ((ICQFRAME *) WinQueryWindowPtr(hwnd,0))->pal);
       break;

    case 0x041E:
       return WinDefWindowProc(hwnd, msg, mp1, mp2);

    case 0x041F:
       return WinDefWindowProc(hwnd, msg, mp1, mp2);

    case WM_BEGINDRAG:
       return beginDrag(hwnd, (POINTS *) &mp1);

    case WM_ENDDRAG:
       return endDrag(hwnd);

    case WM_TIMER:
       timer(hwnd);
       break;

#ifdef SKINNED_GUI
    case WMICQ_QUERYBGIMAGE:
       return (MRESULT) ((ICQFRAME *) WinQueryWindowPtr(hwnd,0))->bg;
#else
    case WMICQ_QUERYBGIMAGE:
       return (MRESULT) NO_IMAGE;
#endif

    case WM_OWNERPOSCHANGE:
       DBGMessage("WM_OWNERPOSCHANGE");
       return WinDefWindowProc(hwnd,msg,mp1,mp2);

    case WM_MOUSEMOVE:
       if(WinQueryCapture(HWND_DESKTOP) == hwnd);
          saveMouse(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1));
       return WinDefWindowProc(hwnd,msg,mp1,mp2);

    case WMICQ_CLEAR:
       clearSkin( (ICQFRAME *) WinQueryWindowPtr(hwnd,0));
       break;

    case WMICQ_LOADSKIN:
       loadSkin(hwnd,(const char *) mp1, (SKINFILESECTION *) mp2);
       break;

    case WMICQ_SKINCHILDS:
       loadSkin4Childs(hwnd,(const char *) mp1, (SKINFILESECTION *) mp2);
       break;

    case WMICQ_RESTORE:
       restoreWindow(hwnd,(const char *) mp1,SHORT1FROMMP(mp2),SHORT2FROMMP(mp2));
       break;

    case WMICQ_STORE:
       storeWindow(hwnd,(const char *) mp1);
       break;

    case WMICQ_CONFIGURE:
       configure(hwnd,(int) mp1);
       break;

    case WMICQ_SETMSGCHILD:
       createchild(hwnd, (const MSGCHILD *) mp1, SHORT1FROMMP(mp2));
       break;

    case WMICQ_SKINELEMENT:
       skinElement(hwnd,(HWND) mp1, (const char **) mp2);
       break;

    case WMICQ_DRAWCONTENTS:
       break;

    case WMICQ_AUTOSIZE:
       autoSize(hwnd);
       break;

    case WMICQ_SIZEBUTTONS:
       return (MRESULT) sizeButtonBar(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1),(const USHORT *) mp2);

    case WMICQ_INVALIDATE:
       DBGMessage("WMICQ_INVALIDATE");
       WinInvalidateRect(hwnd,NULL,TRUE);
       break;

    /*---[ SysTray ]--------------------------------------------*/

    case WM_BUTTON2CLICK | 0x2000:
       icqShowPopupMenu(((ICQFRAME *) WinQueryWindowPtr(hwnd,0))->icq, 0, ICQMNU_MODES, 0, 0);
       return (MRESULT) TRUE;

    case WM_BUTTON1CLICK | 0x2000:
       openMessage(((ICQFRAME *) WinQueryWindowPtr(hwnd,0))->icq);
       return (MRESULT) TRUE;

    case WM_TRAYEXIT:	// When tray exits/crashes, it posts WM_TRAYEXIT (0xCD20) to all tray windows.
       icqDisableSysTray(((ICQFRAME *) WinQueryWindowPtr(hwnd,0))->icq);
       return (MRESULT) TRUE;

    /*---[ Default window processing ]--------------------------*/

    default:
       return WinDefWindowProc(hwnd,msg,mp1,mp2);
    }
    return 0;
 }

 static void configure(HWND hwnd, int sz)
 {
    ICQFRAME *cfg = WinQueryWindowPtr(hwnd,0);
    int       f;

    DBGTracex(cfg);

    if(!cfg)
    {
       DBGMessage("*********** CONFIGURATION FAILURE IN FRAME WINDOW");
       icqskin_closeWindow(hwnd);
       return;
    }

    if(!sz)
       sz = sizeof(ICQFRAME);

    memset(cfg,0,sz);

    cfg->sz     = sz; // sizeof(ICQFRAME);

#ifdef SKINNED_GUI
    cfg->bg     = NO_IMAGE;
    for(f=0;f<ICQFRAME_SKINBITMAPS;f++)
       cfg->img[f] = cfg->msk[f] = NO_IMAGE;
#endif

    cfg->xPos   =
    cfg->yPos   =
    cfg->xMouse =
    cfg->yMouse =
    cfg->maxCx  =
    cfg->maxCy  = 0xFFFF;

    cfg->iconID = 0xFFFF;

    cfg->minCx  =
    cfg->minCy  = 0;

    cfg->pal[ICQCLR_FOREGROUND] = 0x00000000;
    cfg->pal[ICQCLR_BACKGROUND] = 0x00D1D1D1;

    CHKPoint();

    CHKPoint();
 }

 static void destroy(HWND hwnd)
 {
    ICQFRAME *cfg = WinQueryWindowPtr(hwnd,0);

    DBGPrint("Window \"%s\" destroyed",cfg->name);

    if(WinQueryCapture(HWND_DESKTOP) == hwnd)
       WinSetCapture(HWND_DESKTOP,NULLHANDLE);

    if(!cfg || cfg->sz != sizeof(ICQFRAME))
       return;

    clearSkin(cfg);

    if(cfg->icon)
       WinDestroyPointer(cfg->icon);

    cfg->sz = 0;
    free(cfg);
 }

 static void clearSkin(ICQFRAME *cfg)
 {
#ifdef SKINNED_GUI
    int      f;

    for(f=0;f<ICQFRAME_SKINBITMAPS;f++)
    {
       icqskin_deleteImage(cfg->img[f]);
       icqskin_deleteImage(cfg->msk[f]);
    }

    icqskin_deleteImage(cfg->bg);

#endif

 }

 static void close(HWND hwnd)
 {
    HWND             frame = WinQueryWindow(hwnd,QW_PARENT);
    ICQFRAME         *cfg  = WinQueryWindowPtr(hwnd,0);
    SKINDATA         *skn;
    ICQMSGDIALOG     *msg  = NULL;
    ICQCONFIGDIALOG  *cgw  = NULL;

//    if(!cfg || cfg->sz != sizeof(ICQFRAME) || !cfg->icq)

    if(!(cfg && cfg->sz >= sizeof(ICQFRAME) && cfg->icq) )
    {
       DBGTracex(cfg);
       DBGTrace(cfg->sz);
       DBGTracex(cfg->icq);

       DBGMessage("***** ERROR CLOSING FRAME WINDOW *****");

       WinPostMsg(hwnd,WM_QUIT,0,0);
       return;
    }

    DBGMessage(cfg->name);
    if(*cfg->name)
       icqStoreWindow(frame, cfg->icq, 0, cfg->name);

    skn = icqskin_getDataBlock(cfg->icq);

    if(hwnd == skn->mainWindow)
    {
       if(skn->TrayServer)
          WinPostMsg(skn->TrayServer,WM_TRAYDELME,(MPARAM) skn->mainWindow,0);

       DBGMessage("Closing configuration windows");

       while(skn->firstCfgWin && skn->firstCfgWin != cgw)
       {
          cgw = skn->firstCfgWin;
          WinSendMsg(cgw->hwnd,WM_CLOSE,0,0);
       }
       DBGTracex(skn->firstCfgWin);

       DBGMessage("Closing message Windows");

       while(skn->firstMsgWin && skn->firstMsgWin != msg)
       {
          msg = skn->firstMsgWin;
          WinSendMsg(msg->hwnd,WM_CLOSE,0,0);
       }
       DBGTracex(skn->firstMsgWin);

       DBGMessage("Closing main Window");

       WinPostMsg(hwnd,WM_QUIT,0,0);
       return;
    }

    DBGMessage("Closing child window");

    WinDestroyWindow(frame);

 }

 static void erase(HWND hwnd, HPS hps, PRECTL rcl)
 {
    POINTL    p;
    ICQFRAME  *cfg = WinQueryWindowPtr(hwnd,0);

    hps = WinBeginPaint(hwnd,NULLHANDLE,rcl);

    icqskin_loadPallete(hps, 0, cfg->pal);

#ifdef SKINNED_GUI
    if(cfg->bg == NO_IMAGE)
    {
       WinFillRect(hps, rcl, ICQCLR_BACKGROUND);
    }
    else
    {
       p.x = rcl->xLeft;
       p.y = rcl->yBottom;
       WinDrawBitmap(hps, cfg->bg, rcl, &p, CLR_WHITE, CLR_BLACK, DBM_NORMAL);
    }
#else
    WinFillRect(hps, rcl, ICQCLR_BACKGROUND);
#endif

    icqskin_drawContents(hwnd,hps);

    WinEndPaint(hps);
 }

 static void paint(HWND hwnd)
 {
    HPS   hps;
    RECTL rcl;
    erase(hwnd,hps,&rcl);
 }

 static void broadcast(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    HWND   hwndNext;
    HENUM  henum = WinBeginEnumWindows(hwnd);
    while ((hwndNext = WinGetNextWindow(henum)) != NULLHANDLE)
       WinSendMsg(hwndNext,msg,mp1,mp2);
    WinEndEnumWindows (henum);
 }

 static void setbuttons(HWND hwnd, const struct icqButtonTable *btn, int sz)
 {
    ICQFRAME *cfg = WinQueryWindowPtr(hwnd,0);

    cfg->btn      = btn;
    cfg->btnCount = sz;
    icqskin_createButtons(hwnd,icqskin_getDataBlock(cfg->icq),btn,sz);
 }

 static void ppchanged(HWND hwnd, ULONG id)
 {
    ICQFRAME *cfg = WinQueryWindowPtr(hwnd,0);

    DBGPrint("Frame presentation parameter %d changed",id);

    if(cfg)
       icqskin_updatePresParam(hwnd,id,cfg->pal);
 }

 static int event(HWND hwnd, UCHAR id, UCHAR usr, USHORT event, ULONG parm)
 {
    ICQFRAME *cfg = WinQueryWindowPtr(hwnd,0);

    if(cfg && cfg->sz == sizeof(ICQFRAME))
    {
//       DBGTrace(event);

       if(usr)
          icqskin_event(cfg->icq, parm, id, event, 0);
       else
          icqskin_event(cfg->icq, 0,    id, event, parm);
    }
    return 0;
 }

 static void loadSkin(HWND hwnd, const char *name, SKINFILESECTION *fl)
 {
    static const char *pallete[ICQCLR_COUNTER]     = { STANDARD_SKIN_PALLETE_NAMES };
#ifdef SKINNED_GUI
    static const char *names[ICQFRAME_SKINBITMAPS] = { STANDARD_FRAME_ELEMENTS     };
#endif

    ICQFRAME          *cfg = WinQueryWindowPtr(hwnd,0);
    RECTL             rcl;

#ifdef SKINNED_GUI
    const char        *ptr;
    int               f;

    for(f=0;f<ICQFRAME_SKINBITMAPS;f++)
    {
       ptr = icqskin_getSkinFileEntry(fl, name, names[f]);
       icqskin_loadImage(cfg->icq, ptr, &cfg->img[f], &cfg->msk[f]);
    }
#endif

    icqskin_loadSkinPallete(hwnd, name, fl, pallete, ICQCLR_COUNTER, cfg->pal);

    WinQueryWindowRect( hwnd, &rcl);
    resize(hwnd,rcl.xRight-rcl.xLeft,rcl.yTop-rcl.yBottom);

 }

 static void autoSize(HWND hwnd)
 {
    ICQFRAME *cfg = WinQueryWindowPtr(hwnd,0);
    SWP      swp;

    WinQueryWindowPos(hwnd, &swp);
    WinSetWindowPos(hwnd, 0, swp.x, swp.y, swp.cx, swp.cy, SWP_SIZE);
 }

 static void resize(HWND hwnd, short cx, short cy)
 {
#ifdef SKINNED_GUI
    ICQFRAME *cfg = WinQueryWindowPtr(hwnd,0);
    if(cfg)
       cfg->bg = icqskin_createFrameBackground(hwnd,cfg->bg, cx, cy, cfg->img, cfg->msk);
#endif
    WinInvalidateRect(hwnd,NULL,TRUE);

 }

 #define LEFT_MARGIN    15
 #define RIGHT_MARGIN   15
 #define TOP_MARGIN     15
 #define BOTTOM_MARGIN  15

 static MRESULT beginDrag(HWND hwnd, POINTS *p)
 {
    ICQFRAME *cfg = WinQueryWindowPtr(hwnd,0);
    RECTL    rcl;
    POINTL   ptl  = { p->x, p->y };

    CHKPoint();
    DBGTrace(p->x);
    DBGTrace(p->y);

    cfg->xPos =
    cfg->yPos = 0xFFFF;

    WinMapWindowPoints(hwnd, HWND_DESKTOP, &ptl, 1);

    WinQueryWindowRect(hwnd, &rcl);
    rcl.xRight -= rcl.xLeft;
    rcl.yTop   -= rcl.yBottom;

    DBGTrace(rcl.xRight);
    DBGTrace(rcl.yTop);

    if(p->y > (rcl.yTop - TOP_MARGIN) && (p->x < (rcl.xRight - RIGHT_MARGIN)) && (p->x > LEFT_MARGIN) )
    {
       DBGMessage("Drag window");
       cfg->yPos   = ptl.y;
       cfg->xPos   = ptl.x;
       cfg->flags |= ICQFRAME_FLAG_MOVEX|ICQFRAME_FLAG_MOVEY;
    }
    else
    {
       if(p->x < LEFT_MARGIN)
       {
          cfg->xPos   = ptl.x;
          cfg->flags |= ICQFRAME_FLAG_SETCX|ICQFRAME_FLAG_MOVEX;
          CHKPoint();
       }

       if(p->x > (rcl.xRight - RIGHT_MARGIN))
       {
          CHKPoint();
          cfg->xPos   = ptl.x;
          cfg->flags |= ICQFRAME_FLAG_SETCX;
       }

       if(p->y < BOTTOM_MARGIN)
       {
          CHKPoint();
          cfg->yPos   = ptl.y;
          cfg->flags |= ICQFRAME_FLAG_SETCY|ICQFRAME_FLAG_MOVEY;
       }

       if(p->y > (rcl.yTop - TOP_MARGIN))
       {
          CHKPoint();
          cfg->yPos   = ptl.y;
          cfg->flags |= ICQFRAME_FLAG_SETCY;
       }
    }

    DBGTracex(cfg->xPos);
    DBGTracex(cfg->yPos);

    if(cfg->xPos != 0xFFFF || cfg->yPos != 0xFFFF)
    {
       DBGTrace(cfg->xPos)
       DBGTrace(cfg->yPos);
       WinSetCapture(HWND_DESKTOP, hwnd);
       cfg->timer = WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, 1, 50);

       DBGTracex(cfg->timer);

       if(!cfg->timer)
       {
          DBGMessage("Error starting timer");
          icqWriteSysLog(cfg->icq,PROJECT,"Failure starting size timer");
       }
    }

    return (MRESULT) TRUE;
 }

 static MRESULT endDrag(HWND hwnd)
 {
    ICQFRAME *cfg = WinQueryWindowPtr(hwnd,0);

    DBGTracex(cfg->timer);

    if(cfg->timer)
    {
       if(!WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, cfg->timer))
       {
          DBGMessage("Error stoping timer");
          icqWriteSysLog(cfg->icq,PROJECT,"Failure stopping size timer");
       }
    }

    cfg->timer = 0;

    timer(hwnd);

    cfg->xPos   =
    cfg->yPos   =
    cfg->xMouse =
    cfg->yMouse = 0xFFFF;

    cfg->flags &= ~(ICQFRAME_FLAG_SETCY|ICQFRAME_FLAG_SETCX|ICQFRAME_FLAG_MOVEX|ICQFRAME_FLAG_MOVEY);

    if(WinQueryCapture(HWND_DESKTOP) == hwnd)
       WinSetCapture(HWND_DESKTOP,NULLHANDLE);

    return (MRESULT) TRUE;
 }

 static void saveMouse(HWND hwnd, short x, short y)
 {
    ICQFRAME *cfg  = WinQueryWindowPtr(hwnd,0);
    cfg->xMouse = x;
    cfg->yMouse = y;
 }

 static void restoreWindow(HWND hwnd, const char *name, USHORT cx, USHORT cy)
 {
    ICQFRAME *cfg  = WinQueryWindowPtr(hwnd,0);

    if(!name)
       name = cfg->name;

    icqRestoreWindow(WinQueryWindow(hwnd,QW_PARENT), cfg->icq, 0, name, cx, cy);
 }

 static void storeWindow(HWND hwnd, const char *name)
 {
    ICQFRAME *cfg  = WinQueryWindowPtr(hwnd,0);

    if(!name)
       name = cfg->name;

    icqStoreWindow(WinQueryWindow(hwnd,QW_PARENT), cfg->icq, 0, name);
 }

 static void timer(HWND hwnd)
 {
    ICQFRAME *cfg  = WinQueryWindowPtr(hwnd,0);
    int      xMove = 0;
    int      yMove = 0;
    SWP      swp;
    ULONG    flag  = 0;
    POINTL   p     = { cfg->xMouse, cfg->yMouse };

    if( ((USHORT) cfg->xMouse) == 0xFFFF || ((USHORT)cfg->yMouse) == 0xFFFF)
       return;

    WinMapWindowPoints(hwnd, HWND_DESKTOP, &p, 1);

    hwnd = WinQueryWindow(hwnd,QW_PARENT);

    WinQueryWindowPos(hwnd, &swp);

    if(cfg->xPos != 0xFFFF)
    {
       if(cfg->flags & ICQFRAME_FLAG_MOVEX)
       {
          xMove  = p.x-cfg->xPos;
          swp.x += xMove;
          flag  |= SWP_MOVE;
       }
       else
       {
          xMove  = cfg->xPos - p.x;
       }

       if(cfg->flags & ICQFRAME_FLAG_SETCX)
       {
          swp.cx -= xMove;
          flag   |= SWP_SIZE;
       }

       cfg->xPos  = p.x;

    }

    if(cfg->yPos != 0xFFFF)
    {
       if(cfg->flags & ICQFRAME_FLAG_MOVEY)
       {
          yMove  = p.y - cfg->yPos;
          swp.y += yMove;
          flag  |= SWP_MOVE;
       }
       else
       {
          yMove  = cfg->yPos - p.y;
       }

       if(cfg->flags & ICQFRAME_FLAG_SETCY)
       {
          swp.cy -= yMove;
          flag   |= SWP_SIZE;
       }

       cfg->yPos  = p.y;

    }

    if(flag)
       WinSetWindowPos(hwnd, 0, swp.x, swp.y, swp.cx, swp.cy, flag);

    cfg->xMouse =
    cfg->yMouse = 0xFFFF;

 }

 static MRESULT setparm(HWND hwnd, PWNDPARAMS p, MPARAM mp2)
 {
   if(p->fsStatus == WPM_TEXT)
      return WinSendMsg(WinQueryWindow(hwnd,QW_PARENT),WM_SETWINDOWPARAMS,(MPARAM) p,mp2);
   return (MRESULT) FALSE;
 }

 static void createchild(HWND hwnd, const MSGCHILD *def, USHORT qtd)
 {
    int      f;
    HWND     h;
    ICQFRAME *cfg  = WinQueryWindowPtr(hwnd,0);
    ULONG    bg    = 0x00D1D1D1;

    DBGMessage("Creating child windows");

    cfg->childCount = qtd;
    cfg->childList  = def;

    for(f=0;f < qtd;f++)
    {
       h = WinCreateWindow(   hwnd,                     /* Parent window             */
                              (PSZ) def->wClass,        /* Class name                */
                              (PSZ) def->wText,         /* Window text               */
                              def->wStyle|WS_VISIBLE,   /* Window style              */
                               0, 0,                    /* Position (x,y)            */
                              10,10,                    /* Size (width,height)       */
                              hwnd,                     /* Owner window              */
                              HWND_BOTTOM,              /* Sibling window            */
                              DLG_ELEMENT_ID+def->id,   /* Window id                 */
                              NULL,                     /* Control data              */
                              NULL);                    /* Pres parameters           */

       icqskin_setICQHandle(h,cfg->icq);
       icqskin_setFont(h,def->font);
       icqskin_setButtonName(h,def->name);

#ifdef STANDARD_GUI
       WinSetPresParam(h,PP_BACKGROUNDCOLOR,sizeof(bg),&bg);
       WinSetPresParam(h,PP_DISABLEDBACKGROUNDCOLOR,sizeof(bg),&bg);
#endif

       def++;
    }




 }

 static USHORT sizeButtonBar(HWND hwnd, short cx, USHORT btnCount, const USHORT *btnlist)
 {
    USHORT              pos;
    int                 ySize = 0;
    USHORT			    xSize;
    int                 f;
    USHORT				y     = 0;
    HWND                h;

    for(f=0;f < btnCount; f++)
    {
       icqskin_querySizes(CHILD_WINDOW(hwnd,btnlist[f]),0,&y);
       if(y > ySize)
          ySize = y;
    }

    pos = cx;

    for(f=0;f < btnCount; f++)
    {
       h = CHILD_WINDOW(hwnd,btnlist[f]);
       if(WinIsWindowVisible(h))
       {
          xSize  = (USHORT) icqskin_querySizes(h,0,0);
          pos   -= xSize;
          WinSetWindowPos(h, 0, pos, 0, xSize, ySize, SWP_SIZE|SWP_MOVE);
          pos   -= 4;
       }
    }

    return ySize;
 }

 static int action(HWND hwnd, USHORT id, USHORT type)
 {
    if(type != CMDSRC_ACCELERATOR)
       return -1;

    DBGMessage("Keyboard accelerator");
    DBGTrace(id);

    hwnd = WinWindowFromID(hwnd,id);
    DBGTracex(hwnd);

    if(hwnd)
       WinPostMsg(hwnd,WMICQ_CLICKED,0,0);

    return 0;
 }

 static const char *getSkinInfo(ICQFRAME *cfg, const char *tag, const char *name, SKINFILESECTION *fl)
 {
    return icqskin_getSkinFileEntry(fl, name, tag);
 }

 static void skinElement(HWND hwnd, HWND h, const char **vlr)
 {
    ICQFRAME *cfg    = WinQueryWindowPtr(hwnd,0);
    skinChild(h, cfg, vlr[0], vlr[1], vlr[2], (SKINFILESECTION *) vlr[3]);
 }

 static void skinChild(HWND h, ICQFRAME *cfg, const char *name, const char *defName, const char *defFont, SKINFILESECTION *fl)
 {
    ULONG		bg      = 0x00D1D1D1;
    ULONG		fg      = 0x00000040;
    ULONG		wrk;
    char        buffer[0x0100];
    const char 	*ptr;

    if(cfg)
    {
       bg = cfg->pal[ICQCLR_BACKGROUND];
       fg = cfg->pal[ICQCLR_FOREGROUND];
    }

    // Element font

    sprintf(buffer,"%s.Font",defName);
    DBGMessage(buffer);

    ptr = getSkinInfo(cfg, buffer, name, fl);

    if(ptr)
    {
       DBGMessage(ptr);
       icqskin_setButtonFont(h,ptr);
    }
    else
    {
       icqskin_setButtonFont(h,defFont);
    }

    // Foreground color

    sprintf(buffer,"%s.Foreground",defName);
    ptr = getSkinInfo(cfg, buffer, name, fl);

    if(!ptr)
       ptr = getSkinInfo(cfg, "Foreground", name, fl);

    if(ptr)
    {
       wrk = icqskin_loadPalleteFromString(ptr);
       WinSetPresParam(h,PP_FOREGROUNDCOLOR,sizeof(ULONG),&wrk);
    }
    else
    {
       WinSetPresParam(h,PP_FOREGROUNDCOLOR,sizeof(ULONG),&bg);
    }

    // Background color

    sprintf(buffer,"%s.Background",defName);
    ptr = getSkinInfo(cfg, buffer, name, fl);

    if(!ptr)
       ptr = getSkinInfo(cfg, "Background", name, fl);

    if(ptr)
    {
       wrk = icqskin_loadPalleteFromString(ptr);
       WinSetPresParam(h,PP_BACKGROUNDCOLOR,sizeof(ULONG),&wrk);
    }
    else
    {
       WinSetPresParam(h,PP_BACKGROUNDCOLOR,sizeof(ULONG),&bg);
    }

 }

 static void loadSkin4Childs(HWND hwnd, const char *name, SKINFILESECTION *fl)
 {
    ICQFRAME            *cfg    = WinQueryWindowPtr(hwnd,0);
    int                 f;
    HWND                h;
    const MSGCHILD      *def    = cfg->childList;

    DBGMessage(name);
    DBGMessage(cfg->name);
    DBGTracex(icqskin_getSkinFileSection(fl, cfg->name));

    if(icqskin_getSkinFileSection(fl, cfg->name))
       name = cfg->name;

    DBGMessage(name);

    for(f=0;f < cfg->childCount;f++)
    {
       h = CHILD_WINDOW(hwnd,def->id);

       icqskin_setICQHandle(h,cfg->icq);

       if(!icqskin_loadSkin(h,name,fl))
          skinChild(h, cfg, name, def->name, def->font, fl);

       def++;
    }

 }

 static void openMessage(HICQ icq)
 {
    HMSG        msg     = 0;
    ULONG       uin;

    if(icqQueryFirstMessage(icq,&uin,&msg))
    {
       CHKPoint();
       icqPopupUserList(icq);
       return;
    }
    CHKPoint();
    icqOpenMessage(icq, uin, msg);
 }


