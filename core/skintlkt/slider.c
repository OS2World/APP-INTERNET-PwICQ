/*
 * SCBAR.C - Implementa a classe de scrollbar
 *
 *   BSCBAR_UP
 *
 *   BSCURSOR_UP
 *   BSCURSOR_STRETCH
 *   BSCURSOR_DOWN
 *
 *   BSCBAR_DOWN
 *
 */

#define INCL_WIN
#define INCL_GPI
#include <os2.h>

#include <malloc.h>
#include <string.h>

#include "skin.h"

/*---[ Definicoes ]---------------------------------------------------------------*/

#define BITMAP_NORMAL           0
#define BITMAP_MOUSEOVER        1
#define BITMAP_PRESS            2

/*---[ Prototipos ]---------------------------------------------------------------*/

static void     create(HWND, PCREATESTRUCT);
static void     destroy(HWND);
static void     sizePos(HWND,SHORT, SHORT, SHORT, SHORT);
static MRESULT  getpos(HWND);
static MRESULT  getsize(HWND);
static void     reposic(HWND,USHORT,USHORT);
static void     ajustBackground(HWND, PSWP);
static void     paint(HWND);
static void     loadImage(HWND,USHORT,XBITMAP *);
static void     selectBitmap(HWND,USHORT);
static void     setLimits(HWND, ULONG, ULONG);
static void     setValue(HWND, ULONG);
static void     ajustLimits(SLIDERDATA *);
static void     moveCursor(HWND,LONG);
static void     doConnect(HWND, HWND, ULONG);
static void     mouseMove(HWND,short,short);

/*---[ Constantes ]---------------------------------------------------------------*/

extern const char *scbarWindowName = "pwSKNSCBar.wnd";

/*---[ Implementacao ]------------------------------------------------------------*/

MRESULT EXPENTRY scbProcedure(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch(msg)
   {
   case WM_CREATE:
      create(hwnd,PVOIDFROMMP(mp2));
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   case 0x041E:
      selectBitmap(hwnd, BITMAP_MOUSEOVER);
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   case 0x041F:
      selectBitmap(hwnd, BITMAP_NORMAL);
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   case WM_BUTTON1DOWN:
      selectBitmap(hwnd, BITMAP_PRESS);
      break;

   case WM_BUTTON1UP:
      selectBitmap(hwnd, BITMAP_MOUSEOVER);
      break;

   case WM_DESTROY:
      destroy(hwnd); /* Limpa bitmaps */
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   case WM_PAINT:
      paint(hwnd);
      break;

   case WM_WINDOWPOSCHANGED:
      ajustBackground(hwnd,PVOIDFROMMP(mp1));
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   case WMSKN_AJUSTSIZEPOS:
      sizePos(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1),SHORT1FROMMP(mp2),SHORT2FROMMP(mp2));
      break;

   case WMSKN_QUERYPOS:
      return getpos(hwnd);

   case WMSKN_QUERYSIZE:
      return getsize(hwnd);

   case WMSKN_AJUSTPOS:
      reposic(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1));
      break;

   case WMSKN_SETIMAGE:                 /* MP1=XBITMAP, MP2=ID */
      loadImage(hwnd,SHORT1FROMMP(mp2),PVOIDFROMMP(mp1));
      break;

   case WMSKN_SETLIMITS:
      setLimits(hwnd,LONGFROMMP(mp1),LONGFROMMP(mp2));
      break;

   case WMSKN_SETVALUE:
      setValue(hwnd,LONGFROMMP(mp1));
      break;

   case WMSKN_UP:
      moveCursor(hwnd, -1);
      break;

   case WMSKN_DOWN:
      moveCursor(hwnd, 1);
      break;

   case WMSKN_CONNECT:
      doConnect(hwnd,LONGFROMMP(mp1),LONGFROMMP(mp2));
      break;

   case WM_BEGINDRAG:
      DBGMessage("Iniciou drag&drop");
      ((SLIDERDATA *) WinQueryWindowPtr(hwnd,0))->drag = TRUE;
      WinSetCapture(HWND_DESKTOP,hwnd);
      break;

   case WM_ENDDRAG:
      DBGMessage("Terminou drag&drop");
      ((SLIDERDATA *) WinQueryWindowPtr(hwnd,0))->drag = FALSE;
      WinSetCapture(HWND_DESKTOP,NULLHANDLE);
      break;

   case WM_MOUSEMOVE:
      if(((SLIDERDATA *) WinQueryWindowPtr(hwnd,0))->drag)
         mouseMove(hwnd, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1));
      break;

   default:
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   }
   return 0;
}

static void create(HWND hwnd, PCREATESTRUCT info)
{
   SLIDERDATA           *cfg            = malloc(sizeof(SLIDERDATA));

   WinSetWindowPtr(hwnd,0,cfg);

   if(!cfg)
   {
      WinPostMsg(hwnd,WM_CLOSE,0,0);
      return;
   }

   memset(cfg,0,sizeof(SLIDERDATA));
}

static void destroy(HWND hwnd)
{
   SLIDERDATA    *cfg = WinQueryWindowPtr(hwnd,0);
   int          f;

   if(!cfg)
      return;

   if(cfg->bg)
      GpiDeleteBitmap(cfg->bg);

   for(f=0;f<BITMAPS_SLIDER;f++)
   {
      if(cfg->btm[f].image)
         GpiDeleteBitmap(cfg->btm[f].image);
      if(cfg->btm[f].masc)
         GpiDeleteBitmap(cfg->btm[f].masc);
   }

   free(cfg);
}

static void sizePos(HWND hwnd,SHORT x, SHORT y, SHORT cx, SHORT cy)
{
   SLIDERDATA *cfg = WinQueryWindowPtr(hwnd,0);
   cfg->x  = x;
   cfg->y  = y;
   cfg->cx = cx;
   cfg->cy = cy;
}

static MRESULT getpos(HWND hwnd)
{
   SLIDERDATA *cfg = WinQueryWindowPtr(hwnd,0);
   return MRFROM2SHORT(cfg->x,cfg->y);
}

static MRESULT getsize(HWND hwnd)
{
   SLIDERDATA *cfg = WinQueryWindowPtr(hwnd,0);
   return MRFROM2SHORT(cfg->cx,cfg->cy);
}

static void ajustBackground(HWND hwnd, PSWP swp)
{
   SLIDERDATA *cfg    = WinQueryWindowPtr(hwnd,0);
   createControlBackground(hwnd, swp, &cfg->bg, cfg->btm+3);
}

static void reposic(HWND hwnd,USHORT mCx,USHORT mCy)
{
   SLIDERDATA    *cfg = WinQueryWindowPtr(hwnd,0);
   SHORT        x,y,cx,cy;

   if(!(cfg->cx || cfg->cy))
   {
      WinShowWindow(hwnd,FALSE);
      return;
   }

   x  = cfg->x  < 0 ? (mCx + cfg->x) - cfg->cx  : cfg->x;
   y  = cfg->y  < 0 ? (mCy + cfg->y) - cfg->cy  : cfg->y;

   cx = cfg->cx < 0 ? mCx + cfg->cx : cfg->cx;
   cy = cfg->cy < 0 ? mCy + cfg->cy : cfg->cy;

   WinSetWindowPos(     hwnd,
                        0,
                        x,y,
                        cx,cy,
                        SWP_MOVE|SWP_SIZE|SWP_SHOW );


}

static void paint(HWND hwnd)
{
   RECTL        rcl;
   SLIDERDATA   *cfg = WinQueryWindowPtr(hwnd,0);
   HPS          hps;
   POINTL       p;
   XBITMAP      *btm;
   ULONG        ySize;
   ULONG        rows;

   if(!(cfg->btm+cfg->current)->masc)
      selectBitmap(hwnd,BITMAP_NORMAL);

   btm = cfg->btm+cfg->current;

   hps = WinBeginPaint(hwnd,NULLHANDLE,&rcl);

   /* WinFillRect(hps, &rcl, CLR_RED); */

   if(cfg->bg)
   {
      p.x = p.y = 0;
      WinDrawBitmap(    hps,
                        cfg->bg,
                        0,
                        &p,
                        SKC_FOREGROUND,
                        SKC_BACKGROUND,
                        DBM_NORMAL);
   }

   if(cfg->selVal <= cfg->maxVal && cfg->maxVal > cfg->minVal && cfg->yCursor)
   {
      WinQueryWindowRect(hwnd, &rcl);
      ySize = (rcl.yTop - rcl.yBottom) - cfg->yCursor;
      rows  = cfg->maxVal - cfg->minVal;
      p.x = 0;
      p.y = ((rows-cfg->selVal) * ySize) / rows;
      drawTransparent(hps, &p, btm);
   }

   WinEndPaint(hps);
}

static void loadImage(HWND hwnd,USHORT id,XBITMAP *btm)
{
   SLIDERDATA *cfg = WinQueryWindowPtr(hwnd,0);

//   DBGTracex(btm->masc);
   //DBGTracex(btm->image);

   cfg->btm[id].masc  = btm->masc;
   cfg->btm[id].image = btm->image;

//   DBGTrace(id);
//   DBGTracex(cfg->btm[0].masc);
//   DBGTracex(cfg->btm[1].masc);
//   DBGTracex(cfg->btm[2].masc);

   if(id == cfg->current)
      selectBitmap(hwnd,id);
}

static void selectBitmap(HWND hwnd, USHORT btm)
{
   SLIDERDATA           *cfg = WinQueryWindowPtr(hwnd,0);
   BITMAPINFOHEADER     bmpData;

   cfg->current = btm;

   if(validXBitmap(cfg->btm + btm))
   {
      bmpData.cbFix = sizeof(bmpData);
      GpiQueryBitmapParameters((cfg->btm +btm)->masc, &bmpData);
      cfg->yCursor = bmpData.cy;
   }

   WinInvalidateRect(hwnd, 0, TRUE);
}

static void setLimits(HWND hwnd, ULONG min, ULONG max)
{
   SLIDERDATA *cfg = WinQueryWindowPtr(hwnd,0);

   cfg->minVal = min;
   cfg->maxVal = max;

   ajustLimits(cfg);

   WinInvalidateRect(hwnd, 0, TRUE);
}

static void setValue(HWND hwnd, ULONG vlr)
{
   SLIDERDATA *cfg = WinQueryWindowPtr(hwnd,0);
   cfg->selVal = vlr;
   ajustLimits(cfg);
   WinInvalidateRect(hwnd, 0, TRUE);
}

static void moveCursor(HWND hwnd, LONG step)
{
   SLIDERDATA *cfg = WinQueryWindowPtr(hwnd,0);
   cfg->selVal += step;
   ajustLimits(cfg);
   WinInvalidateRect(hwnd, 0, TRUE);

   if(cfg->hConnect)
      WinPostMsg(cfg->hConnect,cfg->mConnect,MPFROMSHORT(cfg->selVal),0);
}

static void ajustLimits(SLIDERDATA *cfg)
{
   if(cfg->selVal > cfg->maxVal)
      cfg->selVal = cfg->maxVal;

   if(cfg->selVal < cfg->minVal)
      cfg->selVal = cfg->minVal;

}

static void doConnect(HWND hwnd, HWND to, ULONG msg)
{
   SLIDERDATA *cfg = WinQueryWindowPtr(hwnd,0);
   cfg->hConnect = to;
   cfg->mConnect = msg;
}

static void mouseMove(HWND hwnd, short x, short y)
{
   SLIDERDATA   *cfg = WinQueryWindowPtr(hwnd,0);
   SWP          sz;
   int          pos;
   int          fator;

   WinQueryWindowPos(hwnd,&sz);

   if(y < 0)
      pos = sz.cy;
   else if(y > sz.cy)
      pos = 0;
   else
      pos   = sz.cy - y;

   fator = cfg->maxVal - cfg->minVal;

/*

   newVal ................ fator
   pos    ................ sz.cy

   newVal = (pos * fator) / sz.cy
*/

   cfg->selVal = (pos * fator) / sz.cy;
   ajustLimits(cfg);
   WinInvalidateRect(hwnd, 0, TRUE);

   if(cfg->hConnect)
      WinPostMsg(cfg->hConnect,cfg->mConnect,MPFROMSHORT(cfg->selVal),0);

}

