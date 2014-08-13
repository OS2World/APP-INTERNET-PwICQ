/*
 * CONTROL.C - Funcoes de apoio a controles
 */

 #define INCL_WIN
 #define INCL_GPI

 #include "skin.h"

 #include <string.h>

/*---[ Prototipos ]---------------------------------------------------------------*/

 void createControlBackground(HWND hwnd, PSWP swp, HBITMAP *bg, XBITMAP *bs)
 {
    /* Cria o bitmap com o fundo do botao (copia da janela mÆe) */
    HSKIN                skn     = (HSKIN) WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),WMSKN_QUERYSKIN,0,0);
    HPS                  hps;
    HDC                  hdc;
    SIZEL                sizl    = { 0, 0 };  /* use same page size as device */
    DEVOPENSTRUC         dop     = {0L, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};
    HAB                  hab     = WinQueryAnchorBlock(hwnd);
    RECTL                rcl;
    BITMAPINFOHEADER2    bmih;
    BITMAPINFOHEADER     bmpData; /* bit-map information header     */
    HBITMAP              hbmOld;
    POINTL               p;

    if(!skn || !bg)
      return;

    if(*bg)
       GpiDeleteBitmap(*bg);

    hdc = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);
    hps = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);

    memset(&bmih,0, sizeof(BITMAPINFOHEADER2));
    bmih.cbFix        = sizeof(bmih);
    bmih.cx           = swp->cx;
    bmih.cy           = swp->cy;
    bmih.cPlanes      = 1;
    bmih.cBitCount    = 24;

    *bg    = GpiCreateBitmap(hps,&bmih,0L,NULL,NULL);
    hbmOld = GpiSetBitmap(hps, *bg);

    GpiSetColor(hps,CLR_WHITE);
    GpiSetBackColor(hps,CLR_BLACK);

    p.x = p.y   = 0;

    rcl.xLeft   = swp->x;
    rcl.yBottom = swp->y;
    rcl.xRight  = swp->x+swp->cx;
    rcl.yTop    = swp->y+swp->cy;

    GpiErase(hps);

    WinDrawBitmap(    hps,
                      skn->BITMAP_BACKGROUND,
                      &rcl,
                      &p,
                      CLR_WHITE,
                      CLR_BLACK,
                      DBM_NORMAL);

   if(bs && validXBitmap(bs))
   {
      /* Pinta a imagem base */
//      CHKPoint();
      bmpData.cbFix = sizeof(BITMAPINFOHEADER);
      GpiQueryBitmapParameters(bs->masc, &bmpData);
      p.x = 0;
      for(p.y = 0; p.y < swp->cy;p.y += bmpData.cy)
         drawTransparent(hps, &p, bs);
   }

    GpiSetBitmap(hps, hbmOld);
    GpiDestroyPS(hps);
    DevCloseDC(hdc);

 }


