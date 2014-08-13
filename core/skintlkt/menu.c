/*
 * MENU.C - Objeto para controle de menus
 */

/*
 * LISTBOX.C - Controle listbox
 */

#define INCL_WIN
#define INCL_GPI
#include <os2.h>

#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include "skin.h"

/*---[ Definicoes ]---------------------------------------------------------------*/

#define _ALLOC_STEP_    10
#define yBORDER         4
#define xBORDER         3

/*---[ Macros ]-------------------------------------------------------------------*/

/*   WinShowWindow(hwnd,FALSE); */

#define HIDE(hwnd)   WinSetWindowPos(hwnd,0,0,0,0,0,SWP_DEACTIVATE|SWP_HIDE);
#define SHOW(hwnd)   WinSetWindowPos(hwnd,0,0,0,0,0,SWP_ACTIVATE|SWP_SHOW);

/*---[ Prototipos ]---------------------------------------------------------------*/

static void     create(HWND, PCREATESTRUCT);
static void     destroy(HWND);
static void     paint(HWND);
static void     addItem(HWND, const char *, ULONG, ULONG, short);
static void     showMenu(HWND);
static BOOL     mouseMove(HWND, USHORT, USHORT);
static void     selectItem(HWND, MNUDATA *, SHORT);
static void     chkPointerPos(HWND);
static void     doClick(HWND);
static void     configure(MNUDATA *,HWND,USHORT);
static void     addHandler(HWND,MNUHANDLER *);


/*---[ Constantes ]---------------------------------------------------------------*/

extern const char *menuWindowName = "pwSKNMENU.wnd";

/*---[ Implementacao ]------------------------------------------------------------*/

MRESULT EXPENTRY mnuProcedure(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch(msg)
   {
   case WM_USER+10:
      configure(WinQueryWindowPtr(hwnd,0),LONGFROMMP(mp1),SHORT1FROMMP(mp2));
      break;

   case WM_CREATE:
      create(hwnd,PVOIDFROMMP(mp2));
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   case WM_PAINT:
      paint(hwnd);
      break;

   case WM_DESTROY:
      destroy(hwnd);
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   case WM_MOUSEMOVE:
      mouseMove(hwnd, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1));
      break;

   case WMSKN_INSERTMENUITEM: /* MP1=Text, MP2=ID */
      addItem(hwnd,PVOIDFROMMP(mp1),LONGFROMMP(mp2),0,-1);
      break;

   case WMSKN_INSERTHANDLER: /* MP1= MNUHANDLER * */
      addHandler(hwnd,PVOIDFROMMP(mp1));
      break;

   case WM_BUTTON1CLICK:
      doClick(hwnd);
      break;

   case WMSKN_SHOWMENU:
      showMenu(hwnd);
      break;

/*
   case WM_PRESPARAMCHANGED:
      ajustPresParam(hwnd,LONGFROMMP(mp1));
      break;
*/

   case 0x41F:
   case 0x41E:
      chkPointerPos(hwnd);
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   case WM_UPDATEFRAME:
      DosBeep(3000,100);
      return (MRESULT) TRUE;

   default:
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   }
   return 0;
}

static void create(HWND hwnd, PCREATESTRUCT info)
{
   MNUDATA      *cfg            = malloc(sizeof(MNUDATA));

   WinSetWindowPtr(hwnd,0,cfg);

   if(!cfg)
   {
      WinPostMsg(hwnd,WM_CLOSE,0,0);
      return;
   }

   memset(cfg,0,sizeof(MNUDATA));

}

static void configure(MNUDATA *cfg, HWND owner, USHORT id)
{
   DBGMessage("Configurar o menu");

   cfg->owner = owner;
   cfg->id    = id;

}

static void destroy(HWND hwnd)
{
   MNUDATA      *cfg    = WinQueryWindowPtr(hwnd,0);
   int          f;
   ITNDATA      *i      = cfg->list;

   for(f=0;f<cfg->itens;f++)
   {
      if(i->hItem)
         free( (void *) i->hItem );
      i++;
   }

   if(cfg->bg)
      GpiDeleteBitmap(cfg->bg);

   if(cfg->list)
      free(cfg->list);

   free(cfg);
}

static void paint(HWND hwnd)
{
   RECTL        rcl;
   USHORT       top;
   USHORT       bottom;

   MNUDATA      *cfg    = WinQueryWindowPtr(hwnd,0);
   HSKIN        skn     = (HSKIN) WinSendMsg(cfg->owner,WMSKN_QUERYSKIN,0,0);
   HPS          hps;
   POINTL       p;
   int          f;
   int          row;
   OWNERITEM    itn;
   ITNDATA      *i;
   LONG         alTable[SKC_MAXCOLORS];

   hps = WinBeginPaint(hwnd,NULLHANDLE,&rcl);
   top    = rcl.yTop;
   bottom = rcl.yBottom;

   GpiQueryLogColorTable(hps, 0L, 0L, SKC_MAXCOLORS, alTable);

   for(f=0;f<SKC_MAXCOLORS;f++)
      alTable[f] = skn->color[f];

   alTable[SKC_BACKGROUND]                      = alTable[SKC_MENUBACKGROUND];
   alTable[SKC_FOREGROUND]                      = alTable[SKC_MENUFOREGROUND];
   alTable[SKC_SELECTEDBACKGROUND]              = alTable[SKC_MENUSELECTEDBACKGROUND];
   alTable[SKC_SELECTEDFOREGROUND]              = alTable[SKC_MENUSELECTEDFOREGROUND];

   GpiCreateLogColorTable(hps,0L,LCOLF_CONSECRGB,0L,SKC_MAXCOLORS,alTable);

   WinQueryWindowRect(hwnd, &rcl);
   WinFillRect(hps, &rcl, SKC_BACKGROUND);

   row = rcl.yTop-yBORDER;

   i = cfg->list;
   for(f=0;f<cfg->itens;f++)
   {
      memset(&itn,0,sizeof(OWNERITEM));
      itn.hwnd             = hwnd;
      itn.hps              = hps;
      itn.idItem           = i->itemID;
      itn.hItem            = i->hItem;
      itn.rclItem.xLeft    = rcl.xLeft+xBORDER;
      itn.rclItem.xRight   = rcl.xRight-xBORDER;

      i->yTop              =
      itn.rclItem.yTop     = row;

      row -= queryItemSize(hps,cfg->owner,cfg->id,(const char *) itn.hItem,f,NULL);

      i->yBottom           =
      itn.rclItem.yBottom  = --row;

      if( (itn.rclItem.yTop <= top && itn.rclItem.yTop >= bottom) || (itn.rclItem.yBottom <= top && itn.rclItem.yBottom >= bottom))
      {
         /* Tem que pintar */

         if(f == cfg->selected)
         {
            /* Item esta selecionado */
            WinFillRect(hps, &itn.rclItem, SKC_SELECTEDBACKGROUND);
            itn.fsState |= 1;
         }
/*
         else
         {
            WinFillRect(hps, &itn.rclItem, SKC_BACKGROUND);
         }
*/

         WinSendMsg(cfg->owner,WM_DRAWITEM,MPFROMSHORT(cfg->id),MPFROMP(&itn));
      }

      i++;
   }

/*
   if(!cfg->bg)
      WinDrawBorder(hps, &rcl, 5, 5, SKC_BACKGROUND, SKC_BACKGROUND, DB_DLGBORDER );
*/

   GpiSetColor(hps,SKC_MENUBORDER);

   p.x =
   p.y = 0;
   GpiMove( hps, &p );

   p.x = rcl.xRight-1;
   p.y = rcl.yTop-1;
   GpiBox(hps, DRO_OUTLINE, &p, 0, 0);

   WinEndPaint(hps);

}

static void addHandler(HWND hwnd, MNUHANDLER *hdl)
{
   addItem(hwnd,hdl->text,hdl->itemID,hdl->handler,hdl->after);
}

static void addItem(HWND hwnd, const char *text, ULONG id, ULONG handler, short pos)
{
   MNUDATA      *cfg = WinQueryWindowPtr(hwnd,0);
   ITNDATA      *i;
   USHORT       cx, cy;
   USHORT       x,y;
   HPS          hps;
   int          f;
   char         *font = WinSendMsg(cfg->owner,WMSKN_QUERYMENUFONT,0,0);

   if(!text)
      text = "";

   if(font)
      WinSetPresParam(hwnd,PP_FONTNAMESIZE,(ULONG) strlen(font)+1,(PVOID) font);

   if(!cfg->list)
   {
      DBGMessage("Alocar nova tabela de dados");
      cfg->tSize = _ALLOC_STEP_;
      cfg->list  = malloc( _ALLOC_STEP_ * sizeof(ITNDATA));
   }
   else if(cfg->itens >= cfg->tSize)
   {
      DBGMessage("Aumentar o tamanho da tabela de dados");
      cfg->tSize += _ALLOC_STEP_;
      cfg->list   = realloc(cfg->list,cfg->tSize * sizeof(ITNDATA));
   }

   if(!cfg->list)
      return;

   i   = (cfg->list + cfg->itens);

   if(pos >= 0)
   {
      i->itemID = 0xFFFFFFFF;

      for(f=0;f < cfg->itens && (cfg->list+f)->itemID != pos; f++);
      while(f < cfg->itens && (cfg->list+f)->itemID == pos)
         f++;

      DBGTrace(f);

      if(f < cfg->itens)
      {
         /* Achou a posicao */
         x = f;
         for(f = cfg->itens; f > x;f--)
            cfg->list[f] = cfg->list[f-1];
         i = (cfg->list+x);
      }
      else
      {
         /* Nao achou a posicao */
         DBGTrace(cfg->itens);
         for(f=cfg->itens;f>0;f--)
         {
            DBGTrace(f);
            cfg->list[f] = cfg->list[f-1];
         }
         i = cfg->list;
      }
   }

   i->hItem    = (ULONG) malloc(strlen(text)+1);

   if(i->hItem)
      strcpy((void *) i->hItem,text);

   i->yTop      =
   i->yBottom   = 0;

   i->handler   = handler;
   i->itemID    = id;

   cfg->itens++;

   cx = cy = 0;

   hps = WinGetPS(hwnd);

   for(f=0;f<cfg->itens;f++)
   {
      x = 0;
      y = queryItemSize(hps, cfg->owner, cfg->id, (const char *) (cfg->list+f)->hItem, f, &x);

      cx  = max(cx,x);
      cy += (y+1);
   }
   WinReleasePS(hps);

   cx += 10;

   WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT), 0, 0, 0, cx+xBORDER+xBORDER+xBORDER, cy+yBORDER+yBORDER, SWP_SIZE);
   WinInvalidateRect(hwnd, 0, TRUE);
}

static void showMenu(HWND hwnd)
{
   MNUDATA      *cfg = WinQueryWindowPtr(hwnd,0);
   POINTL       pos;
   SWP          desktop;
   SWP          current;

   DBGMessage("Mostrar o menu");
   DBGTracex(hwnd);

   cfg->selected = 0xFFFF;

   WinQueryPointerPos(HWND_DESKTOP,&pos);
   pos.x -=10;
   pos.y -=10;

   WinQueryWindowPos(HWND_DESKTOP,&desktop);
   WinQueryWindowPos(hwnd,&current);

   DBGTrace(current.cx);
   DBGTrace(current.cy);

   if( (pos.x+current.cx) > desktop.cx)
      pos.x = (desktop.cx - current.cx) - 5;

   if( (pos.y+current.cy) > desktop.cy)
      pos.y = (desktop.cy - current.cy) - 5;

   DBGTrace(pos.x);
   DBGTrace(pos.y);

   WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT),0,pos.x,pos.y,0,0,SWP_ACTIVATE|SWP_MOVE|SWP_SHOW);
}

static BOOL mouseMove(HWND hwnd, USHORT x, USHORT y)
{
   MNUDATA      *cfg = WinQueryWindowPtr(hwnd,0);
   SHORT        atual;
   SWP          current;

   for(atual = 0;atual < cfg->itens; atual++)
   {
      if( y <= (cfg->list + atual)->yTop && y >= (cfg->list + atual)->yBottom)
      {
         selectItem(hwnd,cfg,atual);
         return TRUE;
      }
   }
   return FALSE;
}

static void selectItem(HWND hwnd, MNUDATA *cfg, SHORT atual)
{
   short  old;
   RECTL  rcl;

  if(atual != cfg->selected)
  {
     old            = cfg->selected;
     cfg->selected  = atual;

     WinQueryWindowRect(hwnd, &rcl);

     if(old >= 0)
     {
        rcl.yTop    = (cfg->list+old)->yTop;
        rcl.yBottom = (cfg->list+old)->yBottom;
        WinInvalidateRect(hwnd, &rcl, FALSE);
     }

     if(atual >= 0)
     {
        rcl.yTop    = (cfg->list+atual)->yTop;
        rcl.yBottom = (cfg->list+atual)->yBottom;
        WinInvalidateRect(hwnd, &rcl, FALSE);
     }

  }
}

static void chkPointerPos(HWND hwnd)
{
   POINTL pos;
   SWP    win;

   hwnd = WinQueryWindow(hwnd, QW_PARENT);

   WinQueryWindowPos(hwnd,&win);
   WinQueryPointerPos(HWND_DESKTOP,&pos);

   win.cx += win.x;
   win.cy += win.y;

   if( (pos.x >= win.x) && (pos.x <= win.cx) && (pos.y >= win.y ) && (pos.y <= win.cy) )
      return;

   HIDE(hwnd);
}

static void doClick(HWND hwnd)
{
   MNUDATA *cfg = WinQueryWindowPtr(hwnd,0);
   USHORT  id;

  if(cfg->selected < cfg->itens)
   {
      if( (cfg->list+cfg->selected)->handler != 0)
      {
         /* Enviar uma a‡ao de handler */
         CHKPoint();
         WinPostMsg(    cfg->owner,
                        WMSKN_ACTIVATEHANDLER,
                        MPFROMLONG((cfg->list+cfg->selected)->handler),
                        MPFROMLONG((cfg->list+cfg->selected)->itemID));


      }
      else if( (WinQueryWindowULong(hwnd,QWL_STYLE) & WMSKS_SENDACTION) )
      {
         CHKPoint();
         WinPostMsg(    cfg->owner,
                        WMSKN_ACTION,
                        MPFROMSHORT(cfg->id),
                        MPFROMLONG((cfg->list+cfg->selected)->itemID));
      }
      else
      {
         CHKPoint();
         WinPostMsg(    cfg->owner,
                        WM_COMMAND,
                        MPFROM2SHORT(cfg->id,(cfg->list+cfg->selected)->itemID),
                        MPFROM2SHORT(CMDSRC_MENU,TRUE));
      }
   }

   HIDE(WinQueryWindow(hwnd, QW_PARENT));
}


