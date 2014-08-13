/*
 * BTNCLASS.C - Administracao da classe botao
 */


 #define INCL_WIN
 #define INCL_GPI

 #include "skin.h"

 #include <malloc.h>
 #include <string.h>

/*---[ Definicoes ]---------------------------------------------------------------*/

#define BITMAP_NORMAL           0
#define BITMAP_MOUSEOVER        1
#define BITMAP_PRESS            2

/*---[ Prototipos ]---------------------------------------------------------------*/

static void     create(HWND, PCREATESTRUCT);
static void     destroy(HWND);
static void     paint(HWND);
static void     size(HWND,USHORT,USHORT);
static void     loadImage(HWND,USHORT,XBITMAP *);
static void     ajustBackground(HWND,PSWP);
static void     setpos(HWND,SHORT,SHORT);
static MRESULT  getpos(HWND);
static void     selectBitmap(HWND,USHORT);
static void     setFlags(HWND, ULONG, SHORT);
static void     reposic(HWND,USHORT,USHORT);
static void     doPaint(HPS, XBITMAP *, int, int, int, int);
static void     doConnect(HWND, HWND, ULONG);
static void     doClick(HWND);
static void     buttonDown(HWND);
static void     doTimer(HWND);
static HWND     assignMenu(HWND, USHORT, ULONG);
static void     menuCommand(HWND, ULONG, MPARAM, MPARAM );
static MRESULT  SetParams(HWND, WNDPARAMS *);
static MRESULT  GetParams(HWND, WNDPARAMS *);
static void     removeBitMaps(SKINBUTTON *);
static void     createButtonBackground(HWND, USHORT, USHORT, USHORT, USHORT);


/*---[ Constantes ]---------------------------------------------------------------*/

extern const char *buttonWindowName = "pwSKNButton.wnd";

/*---[ Implementacao ]------------------------------------------------------------*/

MRESULT EXPENTRY btnProcedure(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch(msg)
   {
   case WM_PRESPARAMCHANGED:
      ajustPresParam(hwnd,LONGFROMMP(mp1));
      break;

   case WMSKN_SETIMAGE:                 /* MP1=XBITMAP, MP2=ID */
      loadImage(hwnd,SHORT1FROMMP(mp2),PVOIDFROMMP(mp1));
      break;

   case WMSKN_SAVEPOS:
      setpos(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1));
      break;

   case WMSKN_QUERYPOS:
      return getpos(hwnd);

   case WMSKN_AJUSTPOS:
      reposic(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1));
      break;

   case WMSKN_SETBUTTONFLAGS:
      setFlags(hwnd,LONGFROMMP(mp1),SHORT1FROMMP(mp2));
      break;

   case WMSKN_QUERYBUTTONFLAGS:
      return (MRESULT) ((SKINBUTTON *) WinQueryWindowPtr(hwnd,0))->flags;

   case WMSKN_QUERYMENU:
      return (MRESULT) ((SKINBUTTON *) WinQueryWindowPtr(hwnd,0))->hMenu;

   case WMSKN_CONNECT:
      doConnect(hwnd,LONGFROMMP(mp1),LONGFROMMP(mp2));
      break;

   case WM_CREATE:
      create(hwnd,PVOIDFROMMP(mp2));
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   case WM_PAINT:
      paint(hwnd);
      break;

   case WM_TIMER:
      doTimer(hwnd);
      break;

   case 0x041E:
      selectBitmap(hwnd, BITMAP_MOUSEOVER);
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   case 0x041F:
      selectBitmap(hwnd, BITMAP_NORMAL);
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   case WM_BUTTON1DOWN:
      buttonDown(hwnd);
      break;

   case WM_BUTTON1UP:   /* Click */
      WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, 1);
      doClick(hwnd);
      break;

   case WM_DESTROY:
      destroy(hwnd); /* Limpa bitmaps */
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   case WM_BEGINDRAG:
      if( (WinQueryWindowULong(hwnd,QWL_STYLE) & WMSKS_SIZEBUTTON) )
         WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),WMSKN_SETSIZING,MPFROMSHORT(1),mp1);
      return (MRESULT) TRUE;

   case WM_ENDDRAG:
      if( (WinQueryWindowULong(hwnd,QWL_STYLE) & WMSKS_SIZEBUTTON) )
         WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),WMSKN_SETSIZING,MPFROMSHORT(0),0);
      break;

   case WM_WINDOWPOSCHANGED:
      ajustBackground(hwnd,PVOIDFROMMP(mp1));
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   case MM_INSERTITEM:  /* Se o botao tem menu associado, anexa uma opcao a ele */
      break;

   case WMSKN_INSERTHANDLE:   /* Insere um handle no botao */
      ((SKINBUTTON *) WinQueryWindowPtr(hwnd,0))->hItem = LONGFROMMP(mp1);
      WinInvalidateRect(hwnd, 0, TRUE);
      break;

   case WMSKN_ASSIGNMENU: /* MP1=Menu ID, MP2=Style */
      return MRFROMLONG(assignMenu(hwnd,SHORT1FROMMP(mp1),LONGFROMMP(mp2)));

   case WM_SETWINDOWPARAMS:
      return SetParams(hwnd,(WNDPARAMS *) PVOIDFROMMP(mp1));

   case WM_QUERYWINDOWPARAMS:
      return GetParams(hwnd,(WNDPARAMS *) PVOIDFROMMP(mp1));

   case WMSKN_INSERTMENUITEM: /* MP1=Text, MP2=ID */
   case WMSKN_INSERTHANDLER:
   case WMSKN_SHOWMENU:
      menuCommand(hwnd,msg,mp1,mp2);
      break;

   case WMSKN_USEDEFAULTBITMAP: /* Remove bitmaps transparentes, carrega bitmap default */
      break;

   default:
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   }
   return 0;
}

static void create(HWND hwnd, PCREATESTRUCT info)
{
   SKINBUTTON           *cfg            = malloc(sizeof(SKINBUTTON));

   WinSetWindowPtr(hwnd,0,cfg);

   if(!cfg)
   {
      WinPostMsg(hwnd,WM_CLOSE,0,0);
      return;
   }

   memset(cfg,0,sizeof(SKINBUTTON));
   cfg->dragPos = 0xFFFFFFFF;
}

static void destroy(HWND hwnd)
{
   SKINBUTTON   *cfg = WinQueryWindowPtr(hwnd,0);

   if(!cfg)
      return;

   if(cfg->text)
      free(cfg->text);

   removeBitMaps(cfg);

   free(cfg);
}

static void removeBitMaps(SKINBUTTON *cfg)
{
   int          f;

   if(cfg->bg)
      GpiDeleteBitmap(cfg->bg);

   for(f=0;f<BITMAPS_BUTTON;f++)
   {
      if(cfg->btm[f].image)
         GpiDeleteBitmap(cfg->btm[f].image);
      if(cfg->btm[f].masc)
         GpiDeleteBitmap(cfg->btm[f].masc);
   }
}

static void paint(HWND hwnd)
{
   SKINBUTTON           *cfg    = WinQueryWindowPtr(hwnd,0);
//   BITMAPINFOHEADER     bmpData;
//   RECTL                rcl;
//   RECTL                r;
   HPS                  hps;
   POINTL               p;
//   XBITMAP              *btm;
//   int                  fator;
   int                  f;
   OWNERITEM            itn;
   HSKIN                skn     = (HSKIN) WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),WMSKN_QUERYSKIN,0,0);
   LONG                 alTable[SKC_MAXCOLORS];

   if(!(cfg->btm+cfg->current)->masc)
      cfg->current = BITMAP_NORMAL;

   // btm = cfg->btm+cfg->current;

   hps = WinBeginPaint(hwnd,NULLHANDLE,&itn.rclItem);

   GpiSetColor(hps,CLR_WHITE);
   GpiSetBackColor(hps,CLR_BLACK);

   if(cfg->bg)
   {
      /* Tem imagem b†sica, desenha-a */
      p.x = itn.rclItem.xLeft;
      p.y = itn.rclItem.yBottom;
      WinDrawBitmap(    hps,
                        cfg->bg,
                        &itn.rclItem,
                        &p,
                        CLR_WHITE,
                        CLR_BLACK,
                        DBM_NORMAL);
   }


   /* Avisa a janela m∆e para complementar */

   GpiQueryLogColorTable(hps, 0L, 0L, SKC_MAXCOLORS, alTable);

   for(f=0;f<SKC_MAXCOLORS;f++)
      alTable[f] = skn->color[f];
   alTable[SKC_FOREGROUND] = skn->color[SKC_BUTTONTEXT];

   GpiCreateLogColorTable(hps,0L,LCOLF_CONSECRGB,0L,SKC_MAXCOLORS,alTable);

   memset(&itn,0,sizeof(OWNERITEM));
   itn.hwnd                 = hwnd;
   itn.hps                  = hps;

   if(cfg->hItem)
   {
      itn.idItem = cfg->current;
      itn.hItem  = cfg->hItem;
   }
   else
   {
      itn.idItem = 0x8000 | cfg->current;
      itn.hItem  = (ULONG) cfg->text;
   }

   WinQueryWindowRect(hwnd, &itn.rclItem);
   WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),WM_DRAWITEM,MPFROMSHORT(WinQueryWindowUShort(hwnd,QWS_ID)),MPFROMP(&itn));

   WinEndPaint(hps);
}

static void loadImage(HWND hwnd,USHORT id,XBITMAP *btm)
{
   SKINBUTTON *cfg = WinQueryWindowPtr(hwnd,0);
   cfg->btm[id].masc  = btm->masc;
   cfg->btm[id].image = btm->image;
}

static void setpos(HWND hwnd,SHORT x,SHORT y)
{
   SKINBUTTON           *cfg = WinQueryWindowPtr(hwnd,0);
   cfg->x = x;
   cfg->y = y;
}

static MRESULT getpos(HWND hwnd)
{
   SKINBUTTON   *cfg = WinQueryWindowPtr(hwnd,0);
   return MRFROM2SHORT(cfg->x,cfg->y);
}

static void selectBitmap(HWND hwnd, USHORT btm)
{
   SKINBUTTON *cfg = WinQueryWindowPtr(hwnd,0);
   cfg->current = btm;
   WinInvalidateRect(hwnd, 0, TRUE);
}

static void ajustBackground(HWND hwnd, PSWP swp)
{
   createButtonBackground( hwnd, swp->x, swp->y, swp->cx, swp->cy);
}

static void createButtonBackground(HWND hwnd, USHORT x, USHORT y, USHORT cx, USHORT cy)
{
   /* Cria o bitmap com o fundo do botao (copia da janela m∆e) */
   HSKIN                skn     = (HSKIN) WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),WMSKN_QUERYSKIN,0,0);
   SKINBUTTON           *cfg    = WinQueryWindowPtr(hwnd,0);
   HPS                  hps;
   HDC                  hdc;
   SIZEL                sizl    = { 0, 0 };  /* use same page size as device */
   DEVOPENSTRUC         dop     = {0L, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};
   HAB                  hab     = WinQueryAnchorBlock(hwnd);
   RECTL                rcl;
   BITMAPINFOHEADER2    bmih;
   BITMAPINFOHEADER     bmpData; /* bit-map information header     */
   POINTL               p;
   int                  fator;
   int                  f;
   XBITMAP              *btm;

   if(!skn || !cfg)
     return;

   if(cfg->bg)
      GpiDeleteBitmap(cfg->bg);

   hdc = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);
   hps = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);

   memset(&bmih,0, sizeof(BITMAPINFOHEADER2));
   bmih.cbFix        = sizeof(bmih);
   bmih.cx           = cx;
   bmih.cy           = cy;
   bmih.cPlanes      = 1;
   bmih.cBitCount    = 24;

   cfg->bg = GpiCreateBitmap(hps,&bmih,0L,NULL,NULL);
   GpiSetBitmap(hps, cfg->bg);

   GpiSetColor(hps,CLR_WHITE);
   GpiSetBackColor(hps,CLR_BLACK);

   p.x = p.y = 0;

   rcl.xLeft   = x;
   rcl.yBottom = y;

   rcl.xRight  = x+cx;
   rcl.yTop    = y+cy;

   WinDrawBitmap(    hps,
                     skn->BITMAP_BACKGROUND,
                     &rcl,
                     &p,
                     CLR_WHITE,
                     CLR_BLACK,
                     DBM_NORMAL);



   if(cfg->flags & 1)
   {
      // Imprimir imagem de tamanho variavel
      btm = cfg->btm+cfg->current;

      bmpData.cbFix = sizeof(bmpData);
      GpiQueryBitmapParameters(btm->masc, &bmpData);
      fator = bmpData.cx / 3;

      // Pinta o meio
      for(f=0;f<cx;f+= fator)
         doPaint(hps,btm,f,bmpData.cy,fator,fator);

      // Pinta o lado esquerdo
      rcl.xLeft   = x;
      rcl.xRight  = x+fator;
      rcl.yBottom = y;
      rcl.yTop    = y+cy;

      p.x = p.y   = 0;

      WinDrawBitmap( hps,
                     skn->BITMAP_BACKGROUND,
                     &rcl,
                     &p,
                     CLR_WHITE,
                     CLR_BLACK,
                     DBM_NORMAL);
      doPaint(hps,btm,0,bmpData.cy,0,fator);

      // Pinta o lado direito
      rcl.xRight  = x+cx;
      rcl.xLeft   = rcl.xRight - fator;
      rcl.yBottom = y;
      rcl.yTop    = y+cy;
      p.x         = cx-fator;
      p.y         = 0;

      WinDrawBitmap( hps,
                     skn->BITMAP_BACKGROUND,
                     &rcl,
                     &p,
                     CLR_WHITE,
                     CLR_BLACK,
                     DBM_NORMAL);
      doPaint(hps,btm,p.x,bmpData.cy,bmpData.cx-fator,fator);

// static void doPaint(HPS hps, XBITMAP *b, int pos, int cy, int start, int fator)

   }
   else if(validXBitmap(cfg->btm))
   {
      drawTransparent(hps, &p, cfg->btm);
   }


   GpiSetBitmap(hps, NULLHANDLE);
   GpiDestroyPS(hps);
   DevCloseDC(hdc);

}

static void setFlags(HWND hwnd, ULONG flags, SHORT cx)
{
   SKINBUTTON *cfg = WinQueryWindowPtr(hwnd,0);
   cfg->flags = flags;
   cfg->cx    = cx;
}

static void reposic(HWND hwnd,USHORT mCx,USHORT mCy)
{
   SKINBUTTON           *cfg    = WinQueryWindowPtr(hwnd,0);
   SHORT                x,y;
   BITMAPINFOHEADER     bmpData;

   if(!validXBitmap(cfg->btm))
   {
      WinShowWindow(hwnd,FALSE);
      return;
   }

   bmpData.cbFix = sizeof(BITMAPINFOHEADER);
   GpiQueryBitmapParameters(cfg->btm->masc, &bmpData);
   if(cfg->cx < 0)
      bmpData.cx = mCx + cfg->cx;
   else if(cfg->cx > 0)
      bmpData.cx = cfg->cx;

   if(cfg->x < 0)
      x = (mCx + cfg->x) - bmpData.cx;
   else
      x = cfg->x;

   if(cfg->y < 0)
      y = (mCy + cfg->y) - bmpData.cy;
   else
      y = cfg->y;


   WinSetWindowPos(     hwnd,
                        0,
                        x,y,
                        bmpData.cx,bmpData.cy,
                        SWP_MOVE|SWP_SIZE|SWP_SHOW );
}


static void doPaint(HPS hps, XBITMAP *b, int pos, int cy, int start, int fator)
{
   POINTL       aptl[4];

   /* Target - Inclusive */
   aptl[0].x = pos;
   aptl[0].y = 0;
   aptl[1].x = pos+fator;
   aptl[1].y = cy;

   /* Source - Non inclusive */
   aptl[2].x = start;
   aptl[2].y = 0;
   aptl[3].x = start+fator+1;
   aptl[3].y = cy+1;

   GpiWCBitBlt( hps,
                b->masc,
                4,
                aptl,
                ROP_SRCAND,
                BBO_IGNORE);

   GpiWCBitBlt( hps,
                b->image,
                4,
                aptl,
                ROP_SRCPAINT,
                BBO_IGNORE);

/*
   GpiWCBitBlt( hps,
                b->masc,
                4,
                aptl,
                ROP_SRCCOPY,
                BBO_IGNORE);
*/

}

static void doConnect(HWND hwnd, HWND to, ULONG msg)
{
   SKINBUTTON *cfg = WinQueryWindowPtr(hwnd,0);
   cfg->hConnect = to;
   cfg->mConnect = msg;
}

static void doClick(HWND hwnd)
{
   SKINBUTTON *cfg = WinQueryWindowPtr(hwnd,0);
   selectBitmap(hwnd, BITMAP_MOUSEOVER);

   if(cfg->hMenu)
   {
      DBGMessage("Apresentar o menu");
      WinPostMsg(cfg->hMenu,WMSKN_SHOWMENU,0,0);
   }
   else
   {
      WinPostMsg(   WinQueryWindow(hwnd, QW_OWNER),
                    WM_COMMAND,
                    MPFROM2SHORT(WinQueryWindowUShort(hwnd,QWS_ID),0),
                    MPFROM2SHORT(CMDSRC_PUSHBUTTON,TRUE));
   }

   if(cfg->hConnect)
      WinPostMsg(cfg->hConnect,cfg->mConnect,MPFROMSHORT(WinQueryWindowUShort(hwnd,QWS_ID)),0);

}

static void buttonDown(HWND hwnd)
{
   SKINBUTTON *cfg = WinQueryWindowPtr(hwnd,0);
   selectBitmap(hwnd, BITMAP_PRESS);
   WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, 1,cfg->repeatTime ? cfg->repeatTime : 200);
}


static void doTimer(HWND hwnd)
{
   SKINBUTTON *cfg = WinQueryWindowPtr(hwnd,0);

   if(cfg->hConnect)
      WinPostMsg(cfg->hConnect,cfg->mConnect,MPFROMSHORT(WinQueryWindowUShort(hwnd,QWS_ID)),MPFROMSHORT(1));

}

static HWND assignMenu(HWND hwnd, USHORT id, ULONG style)
{
   SKINBUTTON *cfg  = WinQueryWindowPtr(hwnd,0);

   DBGTracex(cfg);

   if(cfg->hMenu)
      return NULLHANDLE;
   cfg->hMenu = CreatePopupMenu(WinQueryWindow(hwnd, QW_OWNER), id, style);

   DBGTracex(cfg->hMenu);

   return cfg->hMenu;
}

static void menuCommand(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   SKINBUTTON *cfg  = WinQueryWindowPtr(hwnd,0);
   if(cfg->hMenu)
      WinSendMsg(cfg->hMenu,msg,mp1,mp2);
}

static MRESULT SetParams(HWND hwnd, WNDPARAMS *parm)
{
   SKINBUTTON   *cfg  = WinQueryWindowPtr(hwnd,0);

   switch(parm->fsStatus)
   {
   case WPM_TEXT:
      if(cfg->text)
         free(cfg->text);
      cfg->text = malloc(strlen(parm->pszText)+1);
      if(!cfg->text)
         return (MRESULT) FALSE;
      strcpy(cfg->text,parm->pszText);
      break;

   default:
      return (MRESULT) FALSE;
   }

   WinInvalidateWindow(hwnd,FALSE);


   return (MRESULT) TRUE;
}

static MRESULT GetParams(HWND hwnd, WNDPARAMS *parm)
{
   SKINBUTTON   *cfg  = WinQueryWindowPtr(hwnd,0);

   switch(parm->fsStatus)
   {
   case WPM_TEXT:
      strcpy(parm->pszText,cfg->text);
      break;

   default:
      return (MRESULT) FALSE;
   }

   return (MRESULT) TRUE;

}


