/*
 * os2menu.c - OS2 Menu Window message processing
 */

 #pragma strings(readonly)

 #define INCL_WIN
 #define INCL_GPI
 #include <string.h>
 #include <malloc.h>
 #include <stdlib.h>
 #include <pwicqgui.h>

/*---[ Control data structures ]--------------------------------------------------------------------------*/

 #pragma pack(1)

 #define DEFAULT_TIMER_STEP 5

 struct creation_data
 {
    USHORT  cb;
    HICQ    icq;
 };

 #define ICQCLR_BORDER      ICQCLR_COUNTER

 #define ICQCLR_3DSELECTED  ICQCLR_COUNTER+1
 #define ICQCLR_3DTEXT      ICQCLR_COUNTER+2
 #define ICQCLR_CTLCOLORS   ICQCLR_COUNTER+3

 typedef struct _icqmenuelement
 {
    USHORT                 sz;
    struct _icqmenuelement *up;
    struct _icqmenuelement *down;

    MENUCALLBACK           *cbk;
    ULONG                  parm;
    USHORT                 id;
    USHORT				   yTop;
    USHORT				   yBottom;

    char                   text[1];

 } ICQMENUELEMENT;


 typedef struct _icqmenu
 {
    USHORT         sz;
    HICQ           icq;

    const char     *name;

    /* Window information */
    ULONG          flags;
    #define ICQMENU_FLG_RESIZE	0x00000001
    #define ICQMENU_FLG_SKINNED 0x00000002
    #define ICQMENU_FLG_ANIMATE 0x00000004

    ICQMENUELEMENT *firstOption;
    ICQMENUELEMENT *lastOption;
    ICQMENUELEMENT *select;

    ULONG          cbkParm;
    HWND           frame;
    ULONG          timer;

    /* Window size */
    short          xSize;
    short          ySize;

    short		   xTimer;
    USHORT		   timerStep;

    /* Skin system */
    ULONG          pal[ICQCLR_CTLCOLORS];
    USHORT         topBorder;
    USHORT         bottomBorder;
    USHORT         leftBorder;
    USHORT         rightBorder;
    USHORT         iconLeft;
    USHORT         iconTop;
    USHORT         iconBottom;
    USHORT         textLeft;
    USHORT         lineSpacing;

#ifdef SKINNED_GUI
    /* Menu background */
    HBITMAP        bg;
    HBITMAP        img[ICQFRAME_SKINBITMAPS];
    HBITMAP        msk[ICQFRAME_SKINBITMAPS];

    /* Selection bar */
    HBITMAP        imgSelected;
    HBITMAP        mskSelected;
#endif

    /* Menu title */
    short      szTitle;
    char       title[1];

 } ICQMENU;

 #define NO_ICON_LEFT_MARGIN 0

/*---[ Constants ]----------------------------------------------------------------------------------------*/

 const char *icqMenuWindowClass = "pwICQMenuWindow";

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static void create(HWND,struct creation_data *);
 static void destroy(HWND);
 static void erase(HWND, HPS, PRECTL);
 static void paint(HWND);
 static void resize(HWND,short,short);
 static int  chkKeyboard(HWND, USHORT, USHORT);
 static void mouseMove(HWND, SHORT, SHORT);
 static void click(HWND);
 static void show(HWND, ULONG, USHORT, USHORT);
 static void autosize(HWND, ICQMENU *, BOOL);
 static void deactivate(HWND, ICQMENU *);
 static void drawContents(HWND, HPS, ICQMENU *, PRECTL);
 static void selectOption(HWND, ICQMENU *, ICQMENUELEMENT *);
 static void invalidateOption(HWND, ICQMENU *, ICQMENUELEMENT *);
 static void setselectbar(HWND,const char *);
 static void loadSkin(HWND,const char *,SKINFILESECTION *);
 static void loadPallete(HPS, ICQMENU *);
 static void drawBackground(HPS, ICQMENU *, PRECTL);
 static void timer(HWND);

#ifdef SKINNED_GUI
 static void createDefaultBackground(HWND, ICQMENU *);
#endif

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 hWindow icqskin_CreateMenu(HICQ icq, USHORT id, SKINDATA *cfg)
 {
    ULONG   ulFrameFlags  = FCF_NOMOVEWITHOWNER; // |FCF_SIZEBORDER;
    HWND    frame;
    HWND    ret;
    ICQMENU *mnu;

    frame = WinCreateStdWindow(  HWND_DESKTOP,
                                 0,
                                 &ulFrameFlags,
                                 (PSZ) icqMenuWindowClass,
                                 "sknMenu",
                                 WS_TOPMOST,
                                 (HMODULE) 0,
                                 2000+id,
                                 &ret
                             );
    DBGTracex(frame);
    DBGTracex(ret);

    if(!ret)
       return ret;

    mnu = WinQueryWindowPtr(ret,0);

    if(mnu && mnu->sz == sizeof(ICQMENU))
    {
       mnu->icq   = icq;
       mnu->frame = frame;
    }

	return ret;
 }

 int icqskin_insertMenuOption(HWND hwnd, const char *text, USHORT id, MENUCALLBACK *cbk, ULONG cbkParm)
 {
    ICQMENU        *cfg = WinQueryWindowPtr(hwnd,0);
    ICQMENUELEMENT *el;

    if(!cfg || cfg->sz != sizeof(ICQMENU))
       return 3;

    el = malloc(sizeof(ICQMENUELEMENT) + strlen(text)+1);

    if(!el)
       return 4;

    memset(el,0,sizeof(ICQMENUELEMENT));

    el->sz   = sizeof(ICQMENUELEMENT);
    el->id   = id;
    el->parm = cbkParm;
    el->cbk  = cbk;

    strcpy(el->text,text);

    /* Add the new element in the list */
    el->up = cfg->lastOption;

    /* Correct the list chain */
    if(el->up)
       el->up->down     = el;
    else
       cfg->firstOption = el;

    if(el->down)
       el->down->up     = el;
    else
       cfg->lastOption  = el;

//    DBGPrint("Incluindo: %s",el->text);

    cfg->flags |= ICQMENU_FLG_RESIZE;

    return 0;
 }

 void icqskin_setMenuTitle(HICQ icq, USHORT id, const char *title)
 {
	SKINDATA *skn = icqskin_getDataBlock(icq);
	DBGTrace(id);
	if(id >= ICQMNU_COUNTER || !skn->menu[id])
	   return;
	CHKPoint();
 }

 MRESULT EXPENTRY icqMenuListWindow(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
  {
    switch(msg)
    {
    case WM_CREATE:
       create(hwnd,(struct creation_data *) mp1);
       break;

    case WM_DESTROY:
       destroy(hwnd);
       break;

    case WM_ERASEBACKGROUND:
       erase(hwnd,(HPS) mp1, (PRECTL) mp2);
       break;

    case WM_COMMAND:
       CHKPoint();
       return WinDefWindowProc(hwnd,msg,mp1,mp2);

    case WM_CHAR:
       if(chkKeyboard(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp2)))
          return WinDefWindowProc(hwnd,msg,mp1,mp2);
       break;

    case WM_PAINT:
       paint(hwnd);
       break;

    case WM_SIZE:
       resize(hwnd,SHORT1FROMMP(mp2),SHORT2FROMMP(mp2));
       break;

   case WM_BUTTON1CLICK:
       DBGMessage("WM_BUTTON1CLICK");
       click(hwnd);
       break;

    case WM_MOUSEMOVE:
       mouseMove(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1));
       return WinDefWindowProc(hwnd,msg,mp1,mp2);

    case WM_FOCUSCHANGE:
//       DBGMessage("****** WM_FOCUSCHANGE ******");
//       DBGTracex(hwnd);
//       DBGTracex(mp1);
//       DBGTracex(SHORT1FROMMP(mp2));
//       DBGMessage("****** WM_FOCUSCHANGE ******");
       if(!SHORT1FROMMP(mp2))
          deactivate(hwnd, (ICQMENU *) WinQueryWindowPtr(hwnd,0));
       return WinDefWindowProc(hwnd,msg,mp1,mp2);

    case WMICQ_SETICQHANDLE:
       DBGTracex(mp1);
       ((ICQMENU *) WinQueryWindowPtr(hwnd,0))->icq = (HICQ) mp1;
       break;

    case WMICQ_MEASURE:
       DBGMessage("WMICQ_MEASURE");
       break;

    case WMICQ_AUTOSIZE:
       DBGMessage("WMICQ_AUTOSIZE");
       autosize(hwnd,WinQueryWindowPtr(hwnd,0),FALSE);
       break;

    case WMICQ_SETSELECTBAR:
       setselectbar(hwnd,(const char *) mp1);
       break;

    case WMICQ_SETNAME:
       ((ICQMENU *) WinQueryWindowPtr(hwnd,0))->name = (const char *) mp1;
       break;

    case WMICQ_SHOWMENU:	/* Popup Window  MP1=parm, MP2=x,y */
       DBGMessage("Show popup menu");
       show(hwnd,(ULONG) mp1, SHORT1FROMMP(mp2), SHORT2FROMMP(mp2));
       break;

    case WM_USER+501:   /* Invalidate element MP1=Element */
       invalidateOption(hwnd, (ICQMENU *) WinQueryWindowPtr(hwnd,0), (ICQMENUELEMENT *) mp1);
       invalidateOption(hwnd, (ICQMENU *) WinQueryWindowPtr(hwnd,0), (ICQMENUELEMENT *) mp2);
       break;

    case WMICQ_LOADSKIN:
       loadSkin(hwnd,(const char *) mp1, (SKINFILESECTION *) mp2);
       break;

    case WMICQ_PAINTBG:
       drawBackground((HPS) mp1, (ICQMENU *) WinQueryWindowPtr(hwnd,0), (PRECTL) mp2);
       break;

    case WM_TIMER:
       timer(hwnd);
       break;

    default:
       return WinDefWindowProc(hwnd,msg,mp1,mp2);
    }

    return 0;
 }

 static void create(HWND hwnd,struct creation_data *cd)
 {
    ICQMENU *cfg  = malloc(sizeof(ICQMENU));
    ULONG   *pal;
    int     f;

    WinSetWindowPtr(hwnd,0,cfg);

    if(!cfg)
    {
       icqskin_closeWindow(hwnd);
       return;
    }
    memset(cfg,0,sizeof(ICQMENU));
    cfg->sz      = sizeof(ICQMENU);

    if(cd && cd->cb == sizeof(struct creation_data))
       cfg->icq = cd->icq;

    pal = (ULONG *) WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),WMICQ_QUERYPALLETE,0,0);

    if(pal)
    {
       for(f=0;f<ICQCLR_COUNTER;f++)
          cfg->pal[f] = pal[f];
    }
    else
    {
       cfg->pal[ICQCLR_FOREGROUND] = 0x00000000;
       cfg->pal[ICQCLR_BACKGROUND] = 0x00D1D1D1;
    }

    cfg->pal[ICQCLR_SELECTEDBG]    = 0x00000080;
    cfg->pal[ICQCLR_SELECTEDFG]    = 0x00FFFFFF;
    cfg->pal[ICQCLR_BORDER]        = 0x00000000;
    cfg->pal[ICQCLR_3DTEXT]        = 0xFFFFFFFF;


    cfg->lineSpacing               = 1;

    cfg->iconTop                   =
    cfg->iconBottom                = 3;

    cfg->iconLeft                  =
    cfg->textLeft                  =
    cfg->topBorder                 =
    cfg->bottomBorder              =
    cfg->leftBorder                =
    cfg->rightBorder               = 4;

#ifdef SKINNED_GUI
    cfg->bg                        =
    cfg->imgSelected               =
    cfg->mskSelected               = NO_IMAGE;

    for(f=0;f<ICQFRAME_SKINBITMAPS;f++)
       cfg->img[f] = cfg->msk[f] = NO_IMAGE;
#endif

 }

 static void destroy(HWND hwnd)
 {
    ICQMENU        *cfg = WinQueryWindowPtr(hwnd,0);
    ICQMENUELEMENT *el;
    int            f;

    if(!cfg || cfg->sz != sizeof(ICQMENU))
       return;

    if(WinQueryCapture(HWND_DESKTOP) == hwnd)
       WinSetCapture(HWND_DESKTOP,NULLHANDLE);

    while(cfg->firstOption)
    {
       el               = cfg->firstOption;
       cfg->firstOption = el->down;
       el->sz           = 0;
       free(el);
    }

#ifdef SKINNED_GUI
    icqskin_deleteImage(cfg->bg);
    icqskin_deleteImage(cfg->imgSelected);
    icqskin_deleteImage(cfg->mskSelected);

    for(f=0;f<ICQFRAME_SKINBITMAPS;f++)
    {
       icqskin_deleteImage(cfg->img[f]);
       icqskin_deleteImage(cfg->msk[f]);
    }
#endif

    cfg->sz = 0;
    free(cfg);
 }

 static void resize(HWND hwnd, short cx, short cy)
 {
    ICQMENU *cfg = WinQueryWindowPtr(hwnd,0);

    if(!cfg || cfg->sz != sizeof(ICQMENU) || (cfg->flags & ICQMENU_FLG_ANIMATE) )
       return;

    cfg->xSize  = cx;
    cfg->ySize  = cy;
    cfg->flags &= ~ICQMENU_FLG_RESIZE;

#ifdef SKINNED_GUI
    if(cfg->flags & ICQMENU_FLG_SKINNED)
       cfg->bg = icqskin_createFrameBackground(hwnd, cfg->bg, cx, cy, cfg->img, cfg->msk);
    else
       createDefaultBackground(hwnd, cfg);
#endif

    WinInvalidateRect(hwnd,NULL,FALSE);
 }

 static void paint(HWND hwnd)
 {
    HPS   hps;
    RECTL rcl;
    erase(hwnd,hps,&rcl);
 }

 static void erase(HWND hwnd, HPS hps, PRECTL rcl)
 {
    POINTL   p;
    ICQMENU  *cfg = WinQueryWindowPtr(hwnd,0);

    hps = WinBeginPaint(hwnd,NULLHANDLE,rcl);

#ifdef SKINNED_GUI
    p.x = rcl->xLeft;
    p.y = rcl->yBottom;
    WinDrawBitmap(hps, cfg->bg, rcl, &p, CLR_WHITE, CLR_BLACK, DBM_NORMAL);
#else
    rcl->xLeft   =
    rcl->yBottom = 0;
    rcl->xRight  = cfg->xSize;
    rcl->yTop    = cfg->ySize;
    drawBackground(hps, cfg, rcl);
#endif

    drawContents(hwnd, hps, cfg, rcl);

    WinEndPaint(hps);
 }

 static void loadPallete(HPS hps, ICQMENU *cfg)
 {
    icqskin_loadPallete(hps, ICQCLR_CTLCOLORS, cfg->pal);
 }

 static void drawContents(HWND hwnd, HPS hps, ICQMENU *cfg, PRECTL rcl)
 {
    ICQMENUELEMENT *el;
    SKINDATA       *skn = icqskin_getDataBlock(cfg->icq);
    ULONG          fg;
    int 		   sd;

    loadPallete(hps, cfg);

    /* Draw elements, from the last to the first */

    rcl->xRight  = cfg->xSize - cfg->rightBorder;
    rcl->yBottom = cfg->bottomBorder;

    for(el = cfg->lastOption;el;el = el->up)
    {
       rcl->xLeft   = cfg->leftBorder;
       el->yTop     =
       rcl->yTop    = rcl->yBottom + skn->iconSize + cfg->iconBottom + cfg->iconTop;
       el->yBottom  = rcl->yBottom;

       if(el == cfg->select)
       {
#ifdef SKINNED_GUI
          icqskin_drawSelected(hwnd,hps,rcl,cfg->imgSelected,cfg->mskSelected);
#else
          WinFillRect(hps, rcl, ICQCLR_SELECTEDBG);
#endif
          fg = ICQCLR_SELECTEDFG;
          sd = ICQCLR_3DSELECTED;
       }
       else
       {
          fg = ICQCLR_FOREGROUND;
          sd = ICQCLR_3DTEXT;
       }

       if(el->id <= PWICQICONBARSIZE)
       {
          icqskin_drawIcon(skn, hps, el->id, cfg->leftBorder+cfg->iconLeft, rcl->yBottom+cfg->iconBottom);
          rcl->xLeft   += skn->iconSize + cfg->textLeft + cfg->iconLeft;
       }
       else
       {
          rcl->xLeft   += NO_ICON_LEFT_MARGIN + cfg->textLeft;
       }

       if(cfg->pal[sd] == 0xFFFFFFFF)
       {
          WinDrawText(hps, -1, el->text, rcl, fg, ICQCLR_BACKGROUND, DT_VCENTER);
       }
       else
       {
          rcl->yTop--;
          rcl->xLeft++;
          WinDrawText(hps, -1, el->text, rcl, sd, ICQCLR_BACKGROUND, DT_VCENTER);

          rcl->xLeft--;
          rcl->xRight--;
          rcl->yTop++;
          rcl->yBottom++;
          WinDrawText(hps, -1, el->text, rcl, fg, ICQCLR_BACKGROUND, DT_VCENTER);
       }

       rcl->yBottom = rcl->yTop+cfg->lineSpacing;
    }
 }

 static int chkKeyboard(HWND hwnd, USHORT fsflags, USHORT code)
 {
    ICQMENU *cfg;

    if( !(fsflags&KC_VIRTUALKEY) )
       return 1;

    if(fsflags&KC_KEYUP)
       return 2;

    cfg = WinQueryWindowPtr(hwnd,0);

    switch(code)
    {
    case 0x08:	// ENTER!
       click(hwnd);
       break;

    case 0x18:
       if(cfg->select)
          selectOption(hwnd, cfg, cfg->select->down);
       if(!cfg->select)
          selectOption(hwnd, cfg, cfg->firstOption);
       break;

    case 0x16:
       if(cfg->select)
          selectOption(hwnd, cfg, cfg->select->up);
       if(!cfg->select)
          selectOption(hwnd, cfg, cfg->lastOption);
       break;

    case 0x14:
       selectOption(hwnd, cfg, cfg->firstOption);
       break;

    case 0x13:
       selectOption(hwnd, cfg, cfg->lastOption);
       break;

    case 0x0f:  // Esc
       deactivate(hwnd,cfg);
       break;

    default:
       DBGTracex(code);
       return 3;

    }
    WinInvalidateRect(hwnd,NULL,FALSE);

    return 0;
 }

 static void click(HWND hwnd)
 {
    ICQMENU *cfg = WinQueryWindowPtr(hwnd,0);

    if(!cfg->select)
       return;

    DBGPrint("Selected: %s",cfg->select->text);

    if(cfg->select->cbk)
       cfg->select->cbk(cfg->icq, cfg->cbkParm,(USHORT) cfg->select->id, cfg->select->parm);

    deactivate(hwnd,cfg);
 }

 static void mouseMove(HWND hwnd, SHORT x, SHORT y)
 {
    ICQMENU        *cfg = WinQueryWindowPtr(hwnd,0);
    ICQMENUELEMENT *el;

    for(el = cfg->lastOption;el;el = el->up)
    {
       if(el->yTop > y && el->yBottom < y)
       {
          if(cfg->select != el)
             selectOption(hwnd, cfg, el);
          return;
       }
    }

    if( x > cfg->xSize || y > cfg->ySize || x < 0 || y < 0)
       deactivate(hwnd, cfg);

 }

 static void autosize(HWND hwnd, ICQMENU *cfg, BOOL flg)
 {
    ICQMENUELEMENT *el;
    SKINDATA       *skn     = icqskin_getDataBlock(cfg->icq);
    USHORT         cx       = 0;
    USHORT         cy       = cfg->topBorder + cfg->bottomBorder;
    HPS            hps;
    RECTL          rcl;
    USHORT         iconSize = NO_ICON_LEFT_MARGIN;


    hps = WinGetPS(hwnd);

    for(el = cfg->lastOption;el;el = el->up)
    {
       rcl.xLeft   = 0;
       rcl.xRight  = 9999;

       rcl.yBottom = 0;
       rcl.yTop    = 9999;

       DBGMessage(el->text);

       if(el->id <= PWICQICONBARSIZE)
          iconSize = skn->iconSize + (cfg->iconLeft*2);

       WinDrawText(hps, -1, el->text, &rcl, ICQCLR_FOREGROUND, ICQCLR_BACKGROUND, DT_VCENTER|DT_QUERYEXTENT);
       if( (rcl.xRight - rcl.xLeft) > cx)
          cx = (rcl.xRight - rcl.xLeft);
       cy += (skn->iconSize + cfg->iconTop + cfg->iconBottom + cfg->lineSpacing);
    }

    cx += (cfg->leftBorder + cfg->rightBorder + iconSize + cfg->textLeft + 4);

    DBGTrace(cx);
    DBGTrace(cy);
    DBGTracex(flg);

    WinReleasePS(hps);

    if(flg)
       WinSetWindowPos(cfg->frame, 0, 0, 0, cx, cy, SWP_SIZE);
    else
       resize(hwnd, cx, cy);

 }

 static void show(HWND hwnd, ULONG cbkParm, USHORT x, USHORT y)
 {
    ICQMENU *cfg     = WinQueryWindowPtr(hwnd,0);
    ULONG   swpFlags = SWP_SHOW|SWP_ACTIVATE|SWP_MOVE;
    POINTL  p;
    SWP     desktop;
    SWP     current;
    USHORT  timerStep  =  icqLoadValue(cfg->icq,"AnimateMenus",DEFAULT_TIMER_STEP);

    cfg->cbkParm = cbkParm;

    if(cfg->flags & ICQMENU_FLG_RESIZE)
    {
       DBGMessage("The Menu needs resizing");
       DBGTrace(timerStep);
       autosize(hwnd,cfg, (timerStep == 0) );
    }

    if(!(x|y))
    {
       DBGMessage("Ajust menu based on mouse");
       WinQueryPointerPos(HWND_DESKTOP, &p);
       x = p.x-5;
       y = p.y-5;
       WinSetCapture(HWND_DESKTOP, hwnd);
    }

    WinQueryWindowPos(HWND_DESKTOP,&desktop);
    WinQueryWindowPos(cfg->frame,  &current);

    DBGTrace(current.cx);
    DBGTrace(current.cy);

    if( (x+current.cx) > desktop.cx)
       x = (desktop.cx - current.cx) - 5;

    if( (y+current.cy) > desktop.cy)
       y = (desktop.cy - current.cy) - 5;

    if(timerStep)
    {
       cfg->flags     |= ICQMENU_FLG_ANIMATE;
       cfg->timerStep  = timerStep;
       cfg->xTimer     = current.cx/timerStep;
       WinSetWindowPos(cfg->frame, 0, x, y, cfg->xTimer, current.cy, swpFlags|SWP_SIZE);
       cfg->timer = WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, 1, 0);
       DBGTrace(cfg->timer);
    }
    else
    {
       WinSetWindowPos(cfg->frame, 0, x, y, cfg->xTimer, current.cy, swpFlags);
    }

 }

 static void timer(HWND hwnd)
 {
    ICQMENU *cfg     = WinQueryWindowPtr(hwnd,0);

    cfg->xTimer += (cfg->xSize/cfg->timerStep);

    if(cfg->xTimer >= cfg->xSize)
    {
       WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, cfg->timer);
       cfg->flags  &= ICQMENU_FLG_ANIMATE;
       autosize(hwnd,cfg,TRUE);
    }

    WinSetWindowPos(cfg->frame, 0, 0, 0, cfg->xTimer, cfg->ySize, SWP_SIZE);

 }


 static void drawBackground(HPS hps, ICQMENU *cfg, PRECTL rcl)
 {
    loadPallete(hps, cfg);
    GpiSetColor(hps,ICQCLR_FOREGROUND);
    GpiSetBackColor(hps,ICQCLR_BACKGROUND);
    WinFillRect(hps, rcl, ICQCLR_BACKGROUND);
    icqskin_drawBorder(hps, rcl, ICQCLR_BORDER, ICQCLR_BORDER);

 }

#ifdef SKINNED_GUI
 static void createDefaultBackground(HWND hwnd, ICQMENU *cfg)
 {

    HPS                  hps;
    HDC                  hdc;
    SIZEL                sizl    = { 0, 0 };  /* use same page size as device */
    DEVOPENSTRUC         dop     = {0L, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};
    HAB                  hab     = WinQueryAnchorBlock(hwnd);
    BITMAPINFOHEADER2    bmih;
    BITMAPINFOHEADER     bmpData; /* bit-map information header     */
    RECTL                rcl;

    hdc = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);
    hps = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);

    memset(&bmih,0, sizeof(BITMAPINFOHEADER2));
    bmih.cbFix        = sizeof(bmih);
    bmih.cx           = cfg->xSize;
    bmih.cy           = cfg->ySize;
    bmih.cPlanes      = 1;
    bmih.cBitCount    = 24;

    cfg->bg = GpiCreateBitmap(hps,&bmih,0L,NULL,NULL);
    GpiSetBitmap(hps, cfg->bg);

    /* Draw inside the bitmap */
    rcl.xLeft   =
    rcl.yBottom = 0;
    rcl.xRight  = cfg->xSize;
    rcl.yTop    = cfg->ySize;
    drawBackground(hps, cfg, &rcl);

    /* Release resources */
    GpiSetBitmap(hps, NULLHANDLE);
    GpiDestroyPS(hps);
    DevCloseDC(hdc);

 }
#endif

 static void deactivate(HWND hwnd, ICQMENU *cfg)
 {
    DBGMessage("Deactivating popup menu");

    if(WinQueryCapture(HWND_DESKTOP) == hwnd)
    {
       DBGMessage("Mouse pointer released");
       WinSetCapture(HWND_DESKTOP,NULLHANDLE);
    }

    WinSetWindowPos(cfg->frame,0,0,0,0,0,SWP_DEACTIVATE|SWP_HIDE);

    //WinShowWindow(cfg->frame,FALSE);
 }

 static void invalidateOption(HWND hwnd, ICQMENU *cfg, ICQMENUELEMENT *el)
 {
    RECTL rcl;

    if(!el)
       return;

    if(el->yTop == el->yBottom)
       return;

    rcl.xLeft   = 0;
    rcl.xRight  = cfg->xSize;
    rcl.yBottom = el->yBottom;
    rcl.yTop    = el->yTop;

    WinInvalidateRect(hwnd, &rcl, FALSE);

 }

 static void selectOption(HWND hwnd, ICQMENU *cfg, ICQMENUELEMENT *el)
 {
    WinPostMsg(hwnd,WM_USER+501,(MPARAM) cfg->select, (MPARAM) el);
    cfg->select = el;
 }

 static void setselectbar(HWND hwnd, const char *filename)
 {
#ifdef SKINNED_GUI
    ICQMENU *cfg = WinQueryWindowPtr(hwnd,0);
    icqskin_loadImage(cfg->icq, filename, &cfg->imgSelected, &cfg->mskSelected);
    WinPostMsg(hwnd,WM_USER+501,(MPARAM) cfg->select, 0);
#endif
 }

 static void loadSkin(HWND hwnd, const char *name, SKINFILESECTION *fl)
 {
    static const char *pallete[ICQCLR_CTLCOLORS]   = { STANDARD_SKIN_PALLETE_NAMES, "Border", "SelectedShadow", "Shadow" };
#ifdef SKINNED_GUI
    static const char *names[ICQFRAME_SKINBITMAPS] = { STANDARD_FRAME_ELEMENTS };
#endif

    ICQMENU           *cfg                         = WinQueryWindowPtr(hwnd,0);
    int               f;

    char              buffer[0x0100];
    const char        *ptr;

   // Bottom,Left,Top,Right

    USHORT            *margin[4] = { &cfg->bottomBorder, &cfg->leftBorder, &cfg->topBorder, &cfg->rightBorder };

    DBGMessage(cfg->name);

    if(icqskin_getSkinFileSection(fl, cfg->name))
    {
       name = cfg->name;
    }
    if(!icqskin_getSkinFileSection(fl, name))
    {
       name = "Menu";
    }

    DBGMessage(name);

    icqskin_loadSkinPallete(hwnd, name, fl, pallete, ICQCLR_CTLCOLORS, cfg->pal);
    icqskin_setSelectBar(hwnd,icqskin_getSkinFileEntry(fl, name, "SelectionBar"));

#ifdef SKINNED_GUI
    for(f=0;f<ICQFRAME_SKINBITMAPS;f++)
    {
       ptr = icqskin_getSkinFileEntry(fl, name, names[f]);
       if(ptr)
          cfg->flags |= ICQMENU_FLG_SKINNED;
       icqskin_loadImage(cfg->icq, ptr, &cfg->img[f], &cfg->msk[f]);
    }
#endif

    ptr = icqskin_getSkinFileEntry(fl, name, "Margin");

    if(ptr)
    {
       DBGMessage(ptr);

       strncpy(buffer,ptr,0xFF);
       ptr = strtok(buffer, ",");

       for(f=0;f<4&&ptr;f++)
       {
          DBGTrace(f);
          DBGMessage(ptr);
          *margin[f] = atoi(ptr);
          ptr = strtok(NULL,",");
       }
       DBGTrace(cfg->bottomBorder);
       DBGTrace(cfg->leftBorder);
       DBGTrace(cfg->topBorder);
       DBGTrace(cfg->rightBorder);
    }

    ptr = icqskin_getSkinFileEntry(fl,name,"Font");

    if(ptr)
       icqskin_setButtonFont(hwnd,ptr);

 }

