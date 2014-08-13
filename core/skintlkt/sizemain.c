 #define INCL_WIN
 #define INCL_GPI
 #include "skin.h"

 #include <string.h>

/*---[ Prototipos ]---------------------------------------------------------------*/

static void  draw(HPS, HBITMAP, USHORT, USHORT);
static void  btmSize(HBITMAP, PPOINTL);
static ULONG ajustPos(USHORT, SHORT);
static void ajustPoint(PPOINTL, PPOINTL, XBITMAP *, USHORT, USHORT);

/*---[ Constantes ]---------------------------------------------------------------*/

/*---[ Implementacao ]------------------------------------------------------------*/

void EXPENTRY sknResize(HWND hwnd, USHORT cx, USHORT cy, HSKIN cfg, const SKINELEMENT *elm)
{
   HPS                  hps;
   HDC                  hdc;
   SIZEL                sizl    = { 0, 0 };  /* use same page size as device */
   DEVOPENSTRUC         dop     = {0L, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};
   HAB                  hab     = WinQueryAnchorBlock(hwnd);
   BITMAPINFOHEADER2    bmih;
   HBITMAP              hbmOld;
   RECTL                rcl;
   ULONG                temp;
   const SKINELEMENT    *cmd;

   if( (cx > cfg->maxSize.x) || (cx < cfg->minSize.x) ||
       (cy > cfg->maxSize.y) || (cy < cfg->minSize.y) )
   {

      if(cx > cfg->maxSize.x)
         cx = cfg->maxSize.x;

      if(cx < cfg->minSize.x)
         cx = cfg->minSize.x;

      if(cy > cfg->maxSize.y)
         cy = cfg->maxSize.y;

      if(cy < cfg->maxSize.x)
         cy = cfg->maxSize.y;

      WinPostMsg(hwnd,WMSKN_SETSIZE,MPFROM2SHORT(cx,cy),0);

      return;
   }

   if(cfg->BITMAP_BACKGROUND)
      GpiDeleteBitmap(cfg->BITMAP_BACKGROUND);

   /* Constroi um novo bitmap ajustando o tamanho da janela */
   hdc = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);
   hps = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);

   memset(&bmih,0, sizeof(BITMAPINFOHEADER2));
   bmih.cbFix        = sizeof(bmih);
   bmih.cx           = cx;
   bmih.cy           = cy;
   bmih.cPlanes      = 1;
   bmih.cBitCount    = 24;

   cfg->BITMAP_BACKGROUND = GpiCreateBitmap(    hps,
                                                &bmih,
                                                0L,
                                                NULL,
                                                NULL);


//   DBGTracex(cfg->BITMAP_BACKGROUND);

   hbmOld = GpiSetBitmap(hps, cfg->BITMAP_BACKGROUND);

   paintMainBackGround(cfg,hps,cx,cy);

   // Finaliza o processo
   GpiSetBitmap(hps, hbmOld);

   GpiDestroyPS(hps);
   DevCloseDC(hdc);

   // Salva o novo tamanho
   cfg->cx = cx;
   cfg->cy = cy;

   // Ajusta posicao dos botoes
   for(cmd = elm;cmd->type;cmd++)
   {
      switch(cmd->type)
      {
      case CMD_BUTTON:
         WinSendDlgItemMsg(hwnd,cmd->pos,WMSKN_AJUSTPOS,MPFROM2SHORT(cfg->cx,cfg->cy),0);
         break;

      case CMD_SLIDER:
      case CMD_LISTBOX:
         WinSendDlgItemMsg(hwnd,cmd->pos,WMSKN_AJUSTPOS,MPFROM2SHORT(cfg->cx,cfg->cy),0);
         break;
      }
   }

   // Redesenha todo mundo
   WinInvalidateRect(hwnd, 0, TRUE);

}

void paintMainBackGround(HSKIN cfg, HPS hps, USHORT cx, USHORT cy)
{
   // Pinta o novo bitmap de fundo
   RECTL                rcl;
   POINTL               p;
   USHORT               left;
   USHORT               right;
   USHORT               top;
   USHORT               bottom;

   rcl.xLeft   = 0;
   rcl.yBottom = 0;
   rcl.xRight  = cx;
   rcl.yTop    = cy;

   // Centro da janela
   btmSize(cfg->BITMAP_MIDCENTER, &p);
   for(bottom = 0; bottom < cy;bottom += p.y)
   {
      for(left=0;left < cx;left += p.x)
         draw(hps, cfg->BITMAP_MIDCENTER, left, bottom);
   }

   // Lado esquerdo
   btmSize(cfg->BITMAP_MIDLEFT, &p);
   for(bottom = 0; bottom < cy;bottom += p.y)
      draw(hps, cfg->BITMAP_MIDLEFT, 0, bottom);

   // Lado direito
   btmSize(cfg->BITMAP_MIDRIGHT, &p);
   right = cx - p.x;
   for(bottom = 0; bottom < cy;bottom += p.y)
      draw(hps, cfg->BITMAP_MIDRIGHT, right, bottom);

   //--[ Barra de titulo ]----------------------------------------------

   if(cfg->BITMAP_TITLECENTER)
   {
      btmSize(cfg->BITMAP_TITLECENTER, &p);
      top = cy - p.y;
      for(left=0;left < cx;left += p.x)
         draw(hps, cfg->BITMAP_TITLECENTER, left, top);
   }

   if(cfg->BITMAP_TITLELEFT)
   {
      btmSize(cfg->BITMAP_TITLELEFT, &p);
      draw(hps, cfg->BITMAP_TITLELEFT, 0, cy-p.y);
   }

   if(cfg->BITMAP_TITLERIGHT)
   {
      btmSize(cfg->BITMAP_TITLERIGHT, &p);
      draw(hps, cfg->BITMAP_TITLERIGHT, cx-p.x, cy-p.y);
   }

   //--[ Parte de cima ]------------------------------------------------

   if(cfg->BITMAP_TOPCENTER)
   {
      top = cy;
      if(cfg->BITMAP_TITLECENTER)
      {
        btmSize(cfg->BITMAP_TITLECENTER, &p);
        top -= p.y;
      }
      btmSize(cfg->BITMAP_TOPCENTER, &p);
      top -= p.y;

      for(left=0;left < cx;left += p.x)
         draw(hps, cfg->BITMAP_TOPCENTER, left, top);
   }

   if(cfg->BITMAP_TOPLEFT)
   {
      top = cy;
      if(cfg->BITMAP_TITLELEFT)
      {
        btmSize(cfg->BITMAP_TITLELEFT, &p);
        top -= p.y;
      }
      btmSize(cfg->BITMAP_TOPLEFT, &p);
      top -= p.y;
      draw(hps, cfg->BITMAP_TOPLEFT, 0, top);
   }

   if(cfg->BITMAP_TOPRIGHT)
   {
      top = cy;
      if(cfg->BITMAP_TITLERIGHT)
      {
        btmSize(cfg->BITMAP_TITLERIGHT, &p);
        top -= p.y;
      }
      btmSize(cfg->BITMAP_TOPRIGHT, &p);
      top -= p.y;
      draw(hps, cfg->BITMAP_TOPRIGHT, cx-p.x, top);
   }

   //--[ Parte de baixo ]-----------------------------------------------

   if(cfg->BITMAP_BOTTOMCENTER)
   {
      btmSize(cfg->BITMAP_BOTTOMCENTER, &p);
      for(left=0;left < cx;left += p.x)
         draw(hps, cfg->BITMAP_BOTTOMCENTER, left, 0);
   }

   if(cfg->BITMAP_BOTTOMLEFT)
      draw(hps, cfg->BITMAP_BOTTOMLEFT, 0, 0);

   if(cfg->BITMAP_BOTTOMRIGHT)
   {
      btmSize(cfg->BITMAP_BOTTOMRIGHT, &p);
      draw(hps, cfg->BITMAP_BOTTOMRIGHT, cx-p.x, 0);
   }

   if(cfg->BITMAP_BOTTOMMIDLEFT)
   {
      btmSize(cfg->BITMAP_BOTTOMLEFT, &p);
      draw(hps, cfg->BITMAP_BOTTOMMIDLEFT, p.x, 0);
   }

   if(cfg->BITMAP_BOTTOMMIDRIGHT)
   {
      btmSize(cfg->BITMAP_BOTTOMRIGHT, &p);
      right = cx-p.x;
      btmSize(cfg->BITMAP_BOTTOMMIDRIGHT, &p);
      draw(hps, cfg->BITMAP_BOTTOMMIDRIGHT, right-p.x, 0);
   }

   // Desenha extras

   ajustPoint(&p,cfg->pxbm,cfg->xbm,cx,cy);
   drawTransparent(hps,&p,cfg->xbm);
}

static void draw(HPS hps, HBITMAP hbm, USHORT x, USHORT y)
{
   POINTL p;

   p.x = x;
   p.y = y;

   WinDrawBitmap(    hps,
                     hbm,
                     0,
                     &p,
                     SKC_BACKGROUND,
                     SKC_BACKGROUND,
                     DBM_NORMAL);

}

static void btmSize(HBITMAP hbm, PPOINTL p)
{
   BITMAPINFOHEADER  pbmpData; /* bit-map information header     */
   pbmpData.cbFix = sizeof(BITMAPINFOHEADER);

   GpiQueryBitmapParameters(hbm, &pbmpData);
   p->x = pbmpData.cx;
   p->y = pbmpData.cy;
}

static ULONG ajustPos(USHORT sz, SHORT fator)
{
   if(fator > 0)
      return (ULONG) fator;
   return (ULONG) (sz+fator);
}

static void ajustPoint(PPOINTL p, PPOINTL src, XBITMAP *b, USHORT cx, USHORT cy)
{
   BITMAPINFOHEADER     bmpData;

   bmpData.cbFix = sizeof(bmpData);
   GpiQueryBitmapParameters(b->masc, &bmpData);

   bmpData.cy >>= 1;
   bmpData.cx >>= 1;

//   DBGTracex(src->x);
//   DBGTracex(src->y);

   if(src->x < 0)
      p->x = cx+src->x;
   else if(src->x == POS_CENTRALIZE)
      p->x = (cx >> 1) - bmpData.cx;
   else
      p->x = src->x;

   if(src->y < 0)
      p->y = cy+src->y;
   else if(src->y == POS_CENTRALIZE)
      p->y = (cy >> 1) - bmpData.cy;
   else
      p->y = src->y;
}



