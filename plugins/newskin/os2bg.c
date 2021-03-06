/*
 * os2bg.c - Standard background creation
 */

 #pragma strings(readonly)

 #define INCL_DOSMEMMGR
 #define INCL_GPI
 #define INCL_GPILOGCOLORTABLE
 #define INCL_WIN

 #include <string.h>
 #include <io.h>
 #include <os2.h>

 #include <mmioos2.h>
 #include <pwicqgui.h>

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static void    drawTransparent(HPS, HBITMAP, HBITMAP, USHORT, USHORT, USHORT, USHORT, USHORT, USHORT);

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 HBITMAP icqskin_createBackground(HWND hwnd, HBITMAP base)
 {
    SIZEL                sizl    = { 0, 0 };
    DEVOPENSTRUC         dop     = {0L, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};
    HAB                  hab     = WinQueryAnchorBlock(hwnd);
    HBITMAP              btm     = NO_IMAGE;
    HWND                 owner   = WinQueryWindow(hwnd, QW_OWNER);
    HPS                  hps;
    HDC                  hdc;
    SWP                  swp;
    BITMAPINFOHEADER2    bmih;
    BITMAPINFOHEADER     bmpData;
    RECTL                rcl;
    HBITMAP              img;
    HBITMAP              msk;
    HBITMAP				 bg;
    POINTL               aptlPoints[4];
    POINTL               p;
    int                  f;
    int                  sz;

    WinQueryWindowPos(hwnd, &swp);

    hdc = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);
    hps = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);

    /* Make base images */
    GpiQueryBitmapParameters(base, &bmpData);

    memset(&bmih,0, sizeof(BITMAPINFOHEADER2));
    bmih.cbFix        = sizeof(bmih);
    bmih.cx           = bmpData.cx;
    bmih.cy           = bmpData.cy;
    bmih.cPlanes      = 1;
    bmih.cBitCount    = 1;
    msk               = GpiCreateBitmap(hps, &bmih, 0L, NULL, NULL);

    GpiSetBitmap(hps,msk);

    aptlPoints[0].x=0;
    aptlPoints[0].y=0;
    aptlPoints[1].x=aptlPoints[0].x+bmpData.cx-1;
    aptlPoints[1].y=aptlPoints[0].y+bmpData.cy-1;
    aptlPoints[2].x=0;
    aptlPoints[2].y=0;
    aptlPoints[3].x=aptlPoints[2].x+bmpData.cx;
    aptlPoints[3].y=aptlPoints[2].y+bmpData.cy;

    GpiSetColor(hps,CLR_WHITE);
    GpiSetBackColor(hps,CLR_PINK);
    GpiWCBitBlt(hps,base,4,aptlPoints,ROP_NOTSRCCOPY,BBO_IGNORE);

    /* Create base image */

    bmih.cbFix        = sizeof(bmih);
    bmih.cx           = bmpData.cx;
    bmih.cy           = bmpData.cy;
    bmih.cPlanes      = 1;
    bmih.cBitCount    = 24;
    img               = GpiCreateBitmap(  hps, &bmih, 0L, NULL, NULL);

    GpiSetBitmap(hps,img);
    GpiSetColor(hps,CLR_WHITE);
    GpiSetBackColor(hps,CLR_BLACK);

    GpiWCBitBlt( hps,msk,4,aptlPoints,ROP_NOTSRCCOPY,BBO_IGNORE);
    GpiWCBitBlt( hps,base,4,aptlPoints,ROP_SRCAND,BBO_IGNORE);

    /* Draw background */

    memset(&bmih,0, sizeof(BITMAPINFOHEADER2));
    bmih.cbFix        = sizeof(bmih);
    bmih.cx           = swp.cx;
    bmih.cy           = swp.cy;
    bmih.cPlanes      = 1;
    bmih.cBitCount    = 24;

    btm = GpiCreateBitmap(hps,&bmih,0L,NULL,NULL);
    GpiSetBitmap(hps, btm);

    // Ask owner window to draw the background
    icqskin_loadPallete(hps, 0, (const ULONG *) WinSendMsg(hwnd,WMICQ_QUERYPALLETE,0,0));

    rcl.xLeft   =
    rcl.yBottom = 0;
    rcl.xRight  = swp.cx;
    rcl.yTop    = swp.cy;

    bg = icqskin_queryBackground(owner);

    if(!bg || bg == NO_IMAGE)
    {
       WinFillRect(hps, &rcl, ICQCLR_BACKGROUND);
    }
    else
    {
       WinMapWindowPoints(hwnd, owner, (POINTL *) &rcl, 2);
       p.x = p.y = 0;
       WinDrawBitmap(hps, bg, &rcl, &p, CLR_WHITE, CLR_BLACK, DBM_NORMAL);
    }

    GpiSetColor(hps,CLR_WHITE);
    GpiSetBackColor(hps,CLR_BLACK);

    if(swp.cx > bmpData.cx)
    {
       sz = bmpData.cx / 3;
       for(f=sz;f<(swp.cx-(sz*2));f += sz)
          drawTransparent(hps, img, msk, f, 0, sz, bmpData.cy, sz, 0);
       drawTransparent(hps, img, msk, swp.cx-(sz*2), 0, sz, bmpData.cy, sz, 0);
       drawTransparent(hps, img, msk, 0, 0, sz, bmpData.cy, 0, 0);
       drawTransparent(hps, img, msk, swp.cx-sz, 0, sz, bmpData.cy, bmpData.cx-sz, 0);
    }
    else
    {
       sz = swp.cx / 2;
       drawTransparent(hps, img, msk, 0, 0, sz, bmpData.cy, 0, 0);
       drawTransparent(hps, img, msk, sz, 0, sz, bmpData.cy, bmpData.cx-sz, 0);
    }

    // Release resources
    GpiSetBitmap(hps, NULLHANDLE);
    GpiDeleteBitmap(img);
    GpiDeleteBitmap(msk);
    GpiDestroyPS(hps);
    DevCloseDC(hdc);

    return btm;
 }

 void icqskin_drawSelected(HWND hwnd, HPS hps, PRECTL rcl, HBITMAP img, HBITMAP msk)
 {
    BITMAPINFOHEADER bmpData;
    USHORT           cx         = rcl->xRight - rcl->xLeft;
    int              sz;
    int              f;

    if(img == NO_IMAGE)
    {
       WinFillRect(hps, rcl, ICQCLR_SELECTEDBG);
       return;
    }

    GpiQueryBitmapParameters(img, &bmpData);

    GpiSetColor(hps,CLR_WHITE);
    GpiSetBackColor(hps,CLR_BLACK);

    if(cx > bmpData.cx)
    {
       sz = bmpData.cx / 3;

       for(f=rcl->xLeft+sz;f<(rcl->xRight-(sz*2));f+=sz)
          drawTransparent(hps, img, msk, f, rcl->yBottom, sz, bmpData.cy, sz, 0);

       drawTransparent(hps, img, msk, rcl->xRight-(sz*2), rcl->yBottom, sz, bmpData.cy, sz, 0);
       drawTransparent(hps, img, msk, rcl->xLeft, rcl->yBottom, sz, bmpData.cy, 0, 0);
       drawTransparent(hps, img, msk, rcl->xRight-sz, rcl->yBottom, sz, bmpData.cy, bmpData.cx-sz, 0);

    }
    else
    {
       sz = cx / 2;
       drawTransparent(hps, img, msk, rcl->xRight-sz, rcl->yBottom, sz, bmpData.cy, bmpData.cx-sz, 0);
       drawTransparent(hps, img, msk, rcl->xLeft,     rcl->yBottom, sz, bmpData.cy, 0, 0);
    }

 }


 static void drawTransparent(HPS hps, HBITMAP img, HBITMAP msk, USHORT x, USHORT y, USHORT cx, USHORT cy, USHORT baseX, USHORT baseY)
 {
    POINTL aptlPoints[4];

    // Image coordinates
    aptlPoints[0].x = x;
    aptlPoints[0].y = y;
    aptlPoints[1].x = x+cx;
    aptlPoints[1].y = y+cy;

    // Source - Non inclusive
    aptlPoints[2].x = baseX;
    aptlPoints[2].y = baseY;
    aptlPoints[3].x = baseX+cx+1;
    aptlPoints[3].y = baseY+cy+1;

    GpiWCBitBlt( hps, msk, 4, aptlPoints, ROP_SRCAND,   BBO_IGNORE);
    GpiWCBitBlt( hps, img, 4, aptlPoints, ROP_SRCPAINT, BBO_IGNORE);
 }


 static void drawFrameBG(HBITMAP *img, HBITMAP *msk, USHORT id, HPS hps, int x, int y, int cx, int cy)
 {
    POINTL aptlPoints[4];

    // Image coordinates
    aptlPoints[0].y = y;
    aptlPoints[1].y = y+cy;
    aptlPoints[0].x = x;
    aptlPoints[1].x = x+cx;

    // Source - Non inclusive
    aptlPoints[2].y = 0;
    aptlPoints[3].y = cy+1;
    aptlPoints[2].x = 0;
    aptlPoints[3].x = cx+1;

    GpiWCBitBlt( hps, msk[id], 4, aptlPoints, ROP_SRCAND,   BBO_IGNORE);
    GpiWCBitBlt( hps, img[id], 4, aptlPoints, ROP_SRCPAINT, BBO_IGNORE);
 }

 HBITMAP icqskin_createFrameBackground(HWND hwnd, HBITMAP bg, short cx, short cy, HBITMAP *img, HBITMAP *msk)
 {
#ifndef SKINNED_GUI
    return NO_IMAGE;
#else
    SIZEL                sizl    = { 0, 0 };
    DEVOPENSTRUC         dop     = {0L, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};
    HAB                  hab     = WinQueryAnchorBlock(hwnd);
    HPS                  hps;
    HDC                  hdc;
    BITMAPINFOHEADER     bmpData;
    BITMAPINFOHEADER2    bmih;
    RECTL                rcl;
    int					 x;
    int                  y;
    int                  f;

    icqskin_deleteImage(bg);


    for(f=0;f<ICQFRAME_SKINBITMAPS && img[f] == NO_IMAGE;f++);

    if(f >= ICQFRAME_SKINBITMAPS)
       return NO_IMAGE;

    hdc = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);
    hps = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);

    bmih.cbFix        = sizeof(bmih);
    bmih.cx           = cx;
    bmih.cy           = cy;
    bmih.cPlanes      = 1;
    bmih.cBitCount    = 24;
    bg                = GpiCreateBitmap(hps, &bmih, 0L, NULL, NULL);

    GpiSetBitmap(hps,bg);

    /* Clear background */
    rcl.xLeft   =
    rcl.yBottom = 0;
    rcl.xRight  = cx;

    rcl.yTop    = cy;

    DBGTrace(cx);
    DBGTrace(cy);

    icqskin_paintBackground(hwnd,hps,&rcl);

    GpiSelectPalette(hps,NULLHANDLE);
    GpiSetColor(hps,CLR_WHITE);
    GpiSetBackColor(hps,CLR_BLACK);

    /* Stretch images */
    y = cy;

    if(img[ICQFRAME_MIDSTRETCH] != NO_IMAGE)
    {
       if(img[ICQFRAME_TITLESTRETCH] != NO_IMAGE)
       {
          GpiQueryBitmapParameters(img[ICQFRAME_TITLESTRETCH], &bmpData);
          y -= bmpData.cy;
       }

       if(img[ICQFRAME_MIDLEFT] == NO_IMAGE)
       {
          x = 0;
       }
       else
       {
          GpiQueryBitmapParameters(img[ICQFRAME_MIDLEFT], &bmpData);
          x = bmpData.cx;
       }

       GpiQueryBitmapParameters(img[ICQFRAME_MIDSTRETCH], &bmpData);
       y -= bmpData.cy;

       do
       {
          for(f=x;f<cx;f+=bmpData.cx)
             drawFrameBG(img,msk, ICQFRAME_MIDSTRETCH, hps, f, y, bmpData.cx, bmpData.cy);
          y -= bmpData.cy;
       } while(y > 0);

    }

    if(img[ICQFRAME_TITLESTRETCH] != NO_IMAGE)
    {
       GpiQueryBitmapParameters(img[ICQFRAME_TITLESTRETCH], &bmpData);
       y = cy-bmpData.cy;
       for(x=0; x<cx; x += bmpData.cx)
          drawFrameBG(img,msk, ICQFRAME_TITLESTRETCH, hps, x, y, bmpData.cx, bmpData.cy);
    }

    if(img[ICQFRAME_MIDLEFT] != NO_IMAGE)
    {
       GpiQueryBitmapParameters(img[ICQFRAME_MIDLEFT], &bmpData);
       drawFrameBG(img,msk, ICQFRAME_MIDLEFT, hps, 0, 0, bmpData.cx, bmpData.cy);
    }

    if(img[ICQFRAME_MIDRIGHT] != NO_IMAGE)
    {
       GpiQueryBitmapParameters(img[ICQFRAME_MIDRIGHT], &bmpData);
       drawFrameBG(img,msk, ICQFRAME_MIDRIGHT, hps, cx-bmpData.cx, 0, bmpData.cx, bmpData.cy);
    }

    y = cy;
    if(img[ICQFRAME_TITLELEFT] != NO_IMAGE)
    {
       GpiQueryBitmapParameters(img[ICQFRAME_TITLELEFT], &bmpData);
       y -= bmpData.cy;
       drawFrameBG(img,msk, ICQFRAME_TITLELEFT, hps, 0, y, bmpData.cx, bmpData.cy);
    }

    if(img[ICQFRAME_TOPLEFT] != NO_IMAGE)
    {
       GpiQueryBitmapParameters(img[ICQFRAME_TOPLEFT], &bmpData);
       y -= bmpData.cy;
       drawFrameBG(img,msk, ICQFRAME_TOPLEFT, hps, 0, y, bmpData.cx, bmpData.cy);
    }


    if(img[ICQFRAME_MIDLEFT] != NO_IMAGE)
    {
       GpiQueryBitmapParameters(img[ICQFRAME_MIDLEFT], &bmpData);
       while(y > 0)
       {
          y -= bmpData.cy;
          drawFrameBG(img,msk, ICQFRAME_MIDLEFT, hps, 0, y, bmpData.cx, bmpData.cy);
       }
    }

    // Right images
    y = cy;

    if(img[ICQFRAME_TITLERIGHT] != NO_IMAGE)
    {
       GpiQueryBitmapParameters(img[ICQFRAME_TITLERIGHT], &bmpData);
       y -= bmpData.cy;
       drawFrameBG(img,msk, ICQFRAME_TITLERIGHT, hps, cx-bmpData.cx, y, bmpData.cx, bmpData.cy);
    }

    if(img[ICQFRAME_TOPRIGHT] != NO_IMAGE)
    {
       GpiQueryBitmapParameters(img[ICQFRAME_TOPRIGHT], &bmpData);
       y -= bmpData.cy;
       drawFrameBG(img,msk, ICQFRAME_TOPRIGHT, hps, cx-bmpData.cx, y, bmpData.cx, bmpData.cy);
    }

    if(img[ICQFRAME_MIDRIGHT] != NO_IMAGE)
    {
       GpiQueryBitmapParameters(img[ICQFRAME_MIDRIGHT], &bmpData);
       x = cx-bmpData.cx;
       while(y > 0)
       {
          y -= bmpData.cy;
          drawFrameBG(img,msk, ICQFRAME_MIDRIGHT, hps, x, y, bmpData.cx, bmpData.cy);
       }
    }

    // Bottom images
    if(img[ICQFRAME_BOTTOMSTRETCH] != NO_IMAGE)
    {
       GpiQueryBitmapParameters(img[ICQFRAME_BOTTOMSTRETCH], &bmpData);
       for(x=0; x<cx; x += bmpData.cx)
          drawFrameBG(img,msk, ICQFRAME_BOTTOMSTRETCH, hps, x, 0, bmpData.cx, bmpData.cy);

    }

    x = 0;
    if(img[ICQFRAME_BOTTOMLEFT] != NO_IMAGE)
    {
       GpiQueryBitmapParameters(img[ICQFRAME_BOTTOMLEFT], &bmpData);
       x += bmpData.cx;
       drawFrameBG(img,msk, ICQFRAME_BOTTOMLEFT, hps, 0, 0, bmpData.cx, bmpData.cy);
    }

    if(img[ICQFRAME_BOTTOMMIDLEFT] != NO_IMAGE)
    {
       GpiQueryBitmapParameters(img[ICQFRAME_BOTTOMMIDLEFT], &bmpData);
       drawFrameBG(img,msk, ICQFRAME_BOTTOMMIDLEFT, hps, x, 0, bmpData.cx, bmpData.cy);
    }

    x = cx;
    if(img[ICQFRAME_BOTTOMRIGHT] != NO_IMAGE)
    {
       GpiQueryBitmapParameters(img[ICQFRAME_BOTTOMRIGHT], &bmpData);
       x -= bmpData.cx;
       drawFrameBG(img,msk, ICQFRAME_BOTTOMRIGHT, hps, x, 0, bmpData.cx, bmpData.cy);
    }

    if(img[ICQFRAME_BOTTOMMIDRIGHT] != NO_IMAGE)
    {
       GpiQueryBitmapParameters(img[ICQFRAME_BOTTOMMIDRIGHT], &bmpData);
       x -= bmpData.cx;
       drawFrameBG(img,msk, ICQFRAME_BOTTOMMIDRIGHT, hps, x, 0, bmpData.cx, bmpData.cy);
    }

    /* Release image */
    GpiSetBitmap(hps, NULLHANDLE);
    GpiDestroyPS(hps);
    DevCloseDC(hdc);

    return bg;
#endif
 }


