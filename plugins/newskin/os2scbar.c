/*
 * os2scbar.c - OS2 Scroll Bar Message Processing
 */

 #pragma strings(readonly)

 #define INCL_WIN
 #define INCL_GPI

 #include <string.h>
 #include <stdlib.h>
 #include <ctype.h>
 #include <malloc.h>
 #include <pwicqgui.h>

/*---[ Defines ]------------------------------------------------------------------------------------------*/


/*---[ Button control data structure ]--------------------------------------------------------------------*/

 #pragma pack(1)

 #pragma pack(1)

 enum SCBAR_BITMAPS
 {
    BITMAP_NORMAL,
    BITMAP_PRESSED,
    BITMAP_MOUSEOVER,

    BITMAP_COUNTER
 };


 typedef struct _icqscbar
 {
    USHORT     sz;
    HICQ       icq;
    const char *name;

    /* Window Positioning */
    short      x;
    short      y;
    short      cx;
    short      cy;
    USHORT	   xSize;
    USHORT     ySize;

    /* Slider */
    int 	   selTop;
    int		   selBottom;
    short      selSize;

    /* Scroll Bar information */
    short      sslider;
    short      sfirst;
    short      slast;
    short      svisible;
    short      stotal;

    /* Skin system */
    short      sCx;
    short	   sCy;
    HBITMAP    img;
    HBITMAP    msk;

    USHORT     btmSelected;
    HBITMAP    baseImage[BITMAP_COUNTER];	// Button base bitmaps
    ULONG      pal[ICQCLR_COUNTER];

 } ICQSCBAR;

 struct creation_data
 {
    USHORT  cb;
    HICQ    icq;
 };

/*---[ Constants ]----------------------------------------------------------------------------------------*/

 const char *icqSCBarWindowClass = "pwICQSCBar";

 static const char *cntrl_name   = "ScrollBar";

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static void create(HWND,struct creation_data *);
 static void destroy(HWND);
 static void setsize(HWND,short,short,short,short);
 static void autosize(HWND);
 static void erase(HWND, HPS, PRECTL);
 static void clear(ICQSCBAR *);
 static void paint(HWND);
 static void loadSkin(HWND,const char *,SKINFILESECTION *);
 static void setbackground(HWND, USHORT, const char *);
 static void resize(HWND, short, short);
 static void drawTransparent(HPS, HBITMAP, HBITMAP, USHORT, USHORT, USHORT, USHORT, USHORT, USHORT);
 static void setScBar(HWND, short, short, short);
 static void calcSizes(HWND, ICQSCBAR *, short);
 static void setThumbSize(HWND,SHORT,SHORT);
 static void setImage(HWND, ICQSCBAR *, USHORT);

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 hWindow icqSkin_createStandardScrollBar(HICQ icq, hWindow hwnd, USHORT id)
 {
    HWND 					ret;
    struct creation_data    parm;

    DBGTracex(hwnd);

    memset(&parm,0,sizeof(parm));
    parm.cb    = sizeof(parm);
    parm.icq   = icq;

    ret = WinCreateWindow(hwnd, (PSZ) icqSCBarWindowClass, "", WS_VISIBLE, 0,0,10,10,hwnd, HWND_TOP, id, &parm, NULL);

    DBGTracex(ret);

    return ret;
 }


 MRESULT EXPENTRY icqSCBarWindow(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    switch(msg)
    {
    case WM_CREATE:
       create(hwnd,(struct creation_data *) mp1);
       break;

    case WM_DESTROY:
       destroy(hwnd);
       break;

    case WM_SIZE:
       resize(hwnd,SHORT1FROMMP(mp2),SHORT2FROMMP(mp2));
       break;

    case WMICQ_AUTOSIZE:
       autosize(hwnd);
       break;

    case WM_ERASEBACKGROUND:
       erase(hwnd,(HPS) mp1, (PRECTL) mp2);
       break;

    case WM_PAINT:
       paint(hwnd);
       break;

    case SBM_SETSCROLLBAR:
       setScBar(hwnd, SHORT1FROMMP(mp1), SHORT1FROMMP(mp2), SHORT2FROMMP(mp2));
       break;

    case SBM_SETPOS:
       calcSizes(hwnd, WinQueryWindowPtr(hwnd,0), SHORT1FROMMP(mp1));
       break;

    case SBM_SETTHUMBSIZE:
       setThumbSize(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1));
       break;

    case WMICQ_SETSIZE:
       setsize(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1),SHORT1FROMMP(mp2),SHORT2FROMMP(mp2));
       break;

    case WMICQ_LOADSKIN:
       loadSkin(hwnd,(const char *) mp1, (SKINFILESECTION *) mp2);
       break;

    case WMICQ_CLEAR:
       clear( (ICQSCBAR *) WinQueryWindowPtr(hwnd,0));
       break;

    case WMICQ_SETBGIMAGE:
       setbackground(hwnd,SHORT1FROMMP(mp1),(const char *) mp2);
       break;

    default:
       return WinDefWindowProc(hwnd,msg,mp1,mp2);

    }

    return 0;
 }

 static void create(HWND hwnd,struct creation_data *cd)
 {
    ICQSCBAR  *cfg = malloc(sizeof(ICQSCBAR));
    int       f;
    ULONG     *pal;

    DBGTracex(cfg);

    WinSetWindowPtr(hwnd,0,cfg);

    if(!cfg)
    {
       icqskin_closeWindow(hwnd);
       return;
    }

    memset(cfg,0,sizeof(ICQSCBAR));
    cfg->sz   = sizeof(ICQSCBAR);
    cfg->name = cntrl_name;

    cfg->img  =
    cfg->msk  = NO_IMAGE;

    for(f=0;f<BITMAP_COUNTER;f++)
       cfg->baseImage[f] = NO_IMAGE;

    if(cd && cd->cb == sizeof(struct creation_data))
       cfg->icq = cd->icq;

    pal = (ULONG *) WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),WMICQ_QUERYPALLETE,0,0);

    if(pal)
    {
       for(f=0;f<ICQCLR_COUNTER;f++)
          cfg->pal[f] = pal[f];
    }

 }

 static void destroy(HWND hwnd)
 {
    ICQSCBAR *cfg = WinQueryWindowPtr(hwnd,0);

    DBGTracex(cfg);

    if(!cfg || cfg->sz != sizeof(ICQSCBAR))
       return;

    clear(cfg);

    cfg->sz = 0;
    free(cfg);
 }

 static void clear(ICQSCBAR *cfg)
 {
    int f;

    for(f=0;f<BITMAP_COUNTER;f++)
    {
       if(cfg->baseImage[f] != NO_IMAGE)
       {
          GpiDeleteBitmap(cfg->baseImage[f]);
          cfg->baseImage[f] = NO_IMAGE;
       }
    }

    icqskin_deleteImage(cfg->img);
    icqskin_deleteImage(cfg->msk);

 }

 static void eraseRect(HWND hwnd, HWND owner, HPS hps, HBITMAP bg, PRECTL rcl)
 {
    POINTL   p;
    RECTL	 rMap;

    if(bg == NO_IMAGE)
    {
       WinFillRect(hps, rcl, ICQCLR_BACKGROUND);
    }
    else
    {
       rMap.xLeft   = rcl->xLeft;
       rMap.xRight  = rcl->xRight;
       rMap.yTop    = rcl->yTop;
       rMap.yBottom = rcl->yBottom;
       WinMapWindowPoints(hwnd, owner, (POINTL *) &rMap, 2);

       p.x = rcl->xLeft;
       p.y = rcl->yBottom;
       WinDrawBitmap(hps, bg, &rMap, &p, CLR_WHITE, CLR_BLACK, DBM_NORMAL);
    }
 }

 static void erase(HWND hwnd, HPS hps, PRECTL rcl)
 {
    ICQSCBAR *cfg  = WinQueryWindowPtr(hwnd,0);
    HWND     owner = WinQueryWindow(hwnd, QW_OWNER);
    HBITMAP  bg    = icqskin_queryBackground(owner);
    int      y;
    int      yStep;
    int      baseY;

    hps = WinBeginPaint(hwnd,NULLHANDLE,rcl);

    icqskin_loadPallete(hps, 0, cfg->pal);

    eraseRect(hwnd, owner, hps, bg, rcl);

    if(cfg->sCx)
    {
       y = (cfg->ySize - cfg->sCx);
       while(y >= cfg->selTop)
       {
          y -= cfg->sCx;
          drawTransparent(hps, cfg->img, cfg->msk, 0, y, cfg->sCx, cfg->sCx, 0, (cfg->sCx*4));
       }

       y = cfg->sCx;
       while(y <= cfg->selBottom)
       {
          drawTransparent(hps, cfg->img, cfg->msk, 0, y, cfg->sCx, cfg->sCx, 0, cfg->sCx);
          y += cfg->sCx;
       }

       rcl->xLeft   = 0;
       rcl->yBottom = 0;
       rcl->xRight  =
       rcl->yTop    = cfg->sCx;
       eraseRect(hwnd, owner, hps, bg, rcl);
       drawTransparent(hps, cfg->img, cfg->msk, 0, 0, cfg->sCx, cfg->sCx, 0, 0);

       rcl->xLeft   = 0;
       rcl->xRight  = cfg->xSize;
       rcl->yBottom = cfg->ySize-cfg->sCx;
       rcl->yTop    = cfg->ySize;
       eraseRect(hwnd, owner, hps, bg, rcl);
       drawTransparent(hps, cfg->img, cfg->msk, 0, cfg->ySize-cfg->sCx, cfg->sCx, cfg->sCx, 0, cfg->sCy - cfg->sCx);

       yStep = (cfg->sCx * 2) / 3;

       if(cfg->selTop && cfg->selTop > cfg->selBottom)
       {
          y     = cfg->selTop-yStep;
          baseY = (cfg->sCx * 2)+yStep;

          rcl->xLeft   = 0;
          rcl->xRight  = cfg->xSize;

          while(y > cfg->selBottom)
          {
             rcl->yBottom = y;
             rcl->yTop    = y+yStep;
             eraseRect(hwnd, owner, hps, bg, rcl);
             drawTransparent(hps, cfg->img, cfg->msk, 0, y, cfg->sCx, yStep, 0, baseY);
             y -= yStep;
          }

       }

       if(cfg->selBottom)
       {
          rcl->xLeft   = 0;
          rcl->xRight  = cfg->xSize;
          rcl->yBottom = cfg->selBottom;
          rcl->yTop    = cfg->selBottom+cfg->sCx;
          eraseRect(hwnd, owner, hps, bg, rcl);
          drawTransparent(hps, cfg->img, cfg->msk, 0, cfg->selBottom, cfg->sCx, cfg->sCx, 0, cfg->sCx * 2);
       }

       if(cfg->selTop)
       {
          rcl->xLeft   = 0;
          rcl->xRight  = cfg->xSize;
          rcl->yTop    = cfg->selTop;
          rcl->yBottom = cfg->selTop-cfg->sCx;
          eraseRect(hwnd, owner, hps, bg, rcl);
          drawTransparent(hps, cfg->img, cfg->msk, 0, cfg->selTop-cfg->sCx, cfg->sCx, cfg->sCx, 0, cfg->sCx*3);
       }

    }

    WinEndPaint(hps);
 }

 static void paint(HWND hwnd)
 {
    HPS   hps;
    RECTL rcl;
    erase(hwnd,hps,&rcl);
 }

 static void setsize(HWND hwnd, short x, short y, short cx, short cy)
 {
    ICQSCBAR *cfg = WinQueryWindowPtr(hwnd,0);

    cfg->x     = x;
    cfg->y     = y;
    cfg->cx    = cx;
    cfg->cy    = cy;

    WinPostMsg(hwnd,WMICQ_AUTOSIZE,0,0);
 }

 static void autosize(HWND hwnd)
 {
    ICQSCBAR *cfg = WinQueryWindowPtr(hwnd,0);

    if(icqskin_autoSize(hwnd,cfg->x,cfg->y,cfg->cx,cfg->cy,0))
       WinPostMsg(WinQueryWindow(hwnd, QW_OWNER),WMICQ_SIZECHANGED,(MPARAM) WinQueryWindowUShort(hwnd,QWS_ID),(MPARAM) hwnd);
 }

 static void setbackground(HWND hwnd, USHORT id, const char *filename)
 {
    ICQSCBAR *cfg = WinQueryWindowPtr(hwnd,0);

    DBGTrace(id);

    if(cfg->baseImage[id] != NO_IMAGE)
    {
       GpiDeleteBitmap(cfg->baseImage[id]);
       cfg->baseImage[id] = NO_IMAGE;
    }

    cfg->baseImage[id] = icqskin_loadBitmap(cfg->icq, filename);

    WinInvalidateRect(hwnd,NULL,FALSE);
 }

 static void loadSkin(HWND hwnd, const char *name, SKINFILESECTION *fl)
 {
    static const char *pallete[ICQCLR_COUNTER] = { STANDARD_SKIN_PALLETE_NAMES };

    ICQSCBAR   *cfg        = WinQueryWindowPtr(hwnd,0);
    int        pos[]       = { 0,0,0,0 };
    char       buffer[0x0100];
    char       *tok;
    const char *ptr;
    ULONG      pal;
    int        f, c;

    ptr = icqskin_getSkinFileEntry(fl, name, cfg->name);

    if(ptr)
    {
       strncpy(buffer,ptr,0xFF);
       DBGMessage(buffer);

       tok = strtok(buffer, ",");
       for(f=0;f<4 && tok;f++)
       {
          pos[f] = atoi(tok);
          tok    = strtok(NULL, ",");
       }

       icqskin_setButtonSize(hwnd,pos[0],pos[1],pos[2],pos[3]);

       for(f=0;f<BITMAP_COUNTER;f++)
       {
          if(tok)
          {
             DBGMessage(tok);
             icqskin_setButtonBackground(hwnd,f,tok);
             tok = strtok(NULL,",");
          }
       }

       DBGTracex(cfg->baseImage[BITMAP_NORMAL] == NO_IMAGE);
       DBGTracex(cfg->baseImage[BITMAP_PRESSED] == NO_IMAGE);
       DBGTracex(cfg->baseImage[BITMAP_MOUSEOVER] == NO_IMAGE);

    }
    else
    {
       clear(cfg);
    }

    for(f=0;f<ICQCLR_COUNTER;f++)
    {
       sprintf(buffer,"%s.%s",cfg->name,pallete[f]);

       DBGMessage(buffer);
       ptr = icqskin_getSkinFileEntry(fl,name,buffer);

       if(ptr)
       {
          strncpy(buffer,ptr,0xFF);
          DBGMessage(buffer);

          pal = 0;
          ptr = strtok(buffer, ",");
          for(c=0;c<3&& ptr;c++)
          {
             pal <<= 8;
             pal  |=  (atoi(ptr) & 0x000000ff);
             ptr   = strtok(NULL,",");
          }
          cfg->pal[f] = pal;
       }
    }

    setImage(hwnd, cfg, cfg->btmSelected);
    WinPostMsg(hwnd,WMICQ_AUTOSIZE,0,0);

 }

 static void drawTransparent(HPS hps, HBITMAP img, HBITMAP msk, USHORT x, USHORT y, USHORT cx, USHORT cy, USHORT baseX, USHORT baseY)
 {
    POINTL aptlPoints[4];

    GpiSetColor(hps,CLR_WHITE);
    GpiSetBackColor(hps,CLR_BLACK);

    // Image coordinates
    aptlPoints[0].x = x;
    aptlPoints[0].y = y;
    aptlPoints[1].x = x+cx-1;
    aptlPoints[1].y = y+cy-1;

    // Source - Non inclusive
    aptlPoints[2].x = baseX;
    aptlPoints[2].y = baseY;
    aptlPoints[3].x = baseX+cx;
    aptlPoints[3].y = baseY+cy;

    GpiWCBitBlt( hps, msk, 4, aptlPoints, ROP_SRCAND,   BBO_IGNORE);
    GpiWCBitBlt( hps, img, 4, aptlPoints, ROP_SRCPAINT, BBO_IGNORE);
 }

 static void resize(HWND hwnd, short cx, short cy)
 {
    ICQSCBAR *cfg    = WinQueryWindowPtr(hwnd,0);
    cfg->xSize     = cx;
    cfg->ySize     = cy;
    calcSizes(hwnd, cfg, cfg->sslider);
 }

 static void setImage(HWND hwnd, ICQSCBAR *cfg, USHORT img)
 {
    SIZEL                sizl    = { 0, 0 };
    DEVOPENSTRUC         dop     = {0L, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};

    HPS                  hps;
    HDC                  hdc;

    HAB                  hab     = WinQueryAnchorBlock(hwnd);
    HWND                 owner   = WinQueryWindow(hwnd, QW_OWNER);

    HBITMAP				 bg;
    BITMAPINFOHEADER     bmpData;

    cfg->btmSelected = img;

    hdc = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);
    hps = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);

    icqskin_MakeTransparent(hps, cfg->baseImage[cfg->btmSelected], &cfg->img, &cfg->msk);

    bg = cfg->baseImage[cfg->btmSelected];

    if(bg != NO_IMAGE)
    {
       GpiQueryBitmapParameters(bg, &bmpData);
       cfg->sCx = bmpData.cx;
       cfg->sCy = bmpData.cy;
    }
    else
    {
       cfg->sCx =
       cfg->sCy = 0;
    }

    GpiDestroyPS(hps);
    DevCloseDC(hdc);

    calcSizes(hwnd, cfg, cfg->sslider);

 }

 static void setScBar(HWND hwnd, short sslider, short sfirst, short slast)
 {
    ICQSCBAR *cfg = WinQueryWindowPtr(hwnd,0);
    cfg->sfirst  = sfirst;
    cfg->slast   = slast;

    calcSizes(hwnd,cfg,sslider);
 }


 static void calcSizes(HWND hwnd, ICQSCBAR *cfg, short sslider)
 {
    int slSize = (cfg->slast - cfg->sfirst);
    int wnSize = (cfg->ySize - (cfg->sCx*2));

    if(!slSize)
       return;

    cfg->sslider = sslider;

    /*
     * Calculate button size
     *
     *   selSize  -------- wnSize
     *   sVisible -------- stotal;
     */

    cfg->selSize = (cfg->svisible * wnSize) / cfg->stotal;

    if(cfg->selSize < cfg->sCx)
       cfg->selSize = cfg->sCx;

    /*
     * Calculate button position
     *
     * selBottom ------------- sslider
     * wnSize    ------------- slSize
     *
     */
    wnSize -= cfg->selSize;

    cfg->selTop    = cfg->ySize - (((wnSize * sslider) / slSize) + cfg->sCx);
    cfg->selBottom = cfg->selTop - cfg->selSize;

    WinInvalidateRect(hwnd,NULL,FALSE);
 }

 static void setThumbSize(HWND hwnd, SHORT svisible, SHORT stotal)
 {
    ICQSCBAR *cfg = WinQueryWindowPtr(hwnd,0);
    cfg->svisible = svisible;
    cfg->stotal   = stotal;
    calcSizes(hwnd, cfg, cfg->sslider);
 }


