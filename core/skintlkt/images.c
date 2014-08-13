/*
 * IMAGES.C - Administracao de imagens
 */

 #define INCL_WIN
 #define INCL_GPI
 #include <os2.h>

 #include <string.h>

 #include "skin.h"

/*---[ Implementacao ]------------------------------------------------------------*/

 HBITMAP createBaseImage(HWND hwnd, USHORT cx, USHORT cy)
 {
    HDC                  hdc;
    SIZEL                sizl    = { 0, 0 };  /* use same page size as device */
    DEVOPENSTRUC         dop     = {0L, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};
    HAB                  hab     = WinQueryAnchorBlock(hwnd);
    BITMAPINFOHEADER2    bmih;
    HPS                  hps;
    RECTL                rcl;
    HBITMAP              ret;

    hdc = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);
    hps = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);

    memset(&bmih,0, sizeof(BITMAPINFOHEADER2));
    bmih.cbFix        = sizeof(bmih);
    bmih.cx           = cx;
    bmih.cy           = cy;
    bmih.cPlanes      = 1;
    bmih.cBitCount    = 24;
    ret               = GpiCreateBitmap(hps, &bmih, 0L, NULL, NULL);

    GpiSetBitmap(hps, ret);

    memset(&rcl,0,sizeof(RECTL));
    rcl.yTop   = cy;
    rcl.xRight = cx;
    WinFillRect(hps, &rcl, CLR_WHITE);

    GpiSetBitmap(hps, NULLHANDLE);

    GpiDestroyPS(hps);
    DevCloseDC(hdc);

    return ret;

 }

 void paintWindow(HWND hwnd, HBITMAP work, HBITMAP bg)
 {
    HDC                 hdc;
    HPS                 hpsBitmap;
    SIZEL               sizl    = { 0, 0 };  /* use same page size as device */
    DEVOPENSTRUC        dop     = {0L, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};
    HAB                 hab     = WinQueryAnchorBlock(hwnd);
    RECTL               rcl;
    POINTL              p       = { 0, 0 };
    int                 f;
    HSKIN               skn     = (HSKIN) WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),WMSKN_QUERYSKIN,0,0);
    LONG                alTable[SKC_MAXCOLORS];
    POINTL              aptl[4];


    WinQueryWindowRect(hwnd, &rcl);

    /*---[ Pinto a imagem dentro do BITMAP ]--------------------------------------------------------*/

    hdc       = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);
    hpsBitmap = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);

    GpiSetBitmap(hpsBitmap, work);

    WinDrawBitmap(hpsBitmap, bg, NULL, &p, CLR_WHITE, CLR_BLACK, DBM_NORMAL);

    /* Ajusta a tabela de cores */
    GpiQueryLogColorTable(hpsBitmap, 0L, 0L, SKC_MAXCOLORS, alTable);
    for(f=0;f<SKC_MAXCOLORS;f++)
       alTable[f] = skn->color[f];
    GpiCreateLogColorTable(hpsBitmap,0L,LCOLF_CONSECRGB,0L,SKC_MAXCOLORS,alTable);

    WinSendMsg( hwnd,
                WM_USER+1,
                MPFROMLONG(hpsBitmap),
                MPFROMP(&rcl) );



    /*---[ Libero recursos ]------------------------------------------------------------------------*/

    GpiSetBitmap(hpsBitmap, NULLHANDLE);
    GpiDestroyPS(hpsBitmap);
    DevCloseDC(hdc);


 }



