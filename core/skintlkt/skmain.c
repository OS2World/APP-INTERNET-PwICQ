/*
 * SKMAIN.C - Gerenciamento do SKIN
 */

#define INCL_GPI
#define INCL_WIN
#include <os2.h>

#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#include "skin.h"

/*---[ Constantes ]---------------------------------------------------------------*/

/* extern const char *mainWindowName = "pwSKNWin.wnd"; */

/*---[ Implementacao ]------------------------------------------------------------*/

HSKIN EXPENTRY sknCreate(HWND hwnd, const SKINELEMENT *winElements, USHORT cx, USHORT cy, const char *pszText)
{
   SKINCONFIG           *cfg            = malloc(sizeof(SKINCONFIG));
   const SKINELEMENT    *cmd;

   if(!cfg)
      return cfg;

   memset(cfg,0,sizeof(SKINCONFIG));

   /* Pre-configura */

   cfg->cx = cx;
   cfg->cy = cy;

   /* Cria janelas filhas */

   for(cmd = winElements;cmd->type;cmd++)
   {
      switch(cmd->type)
      {
      case CMD_BUTTON:
         DBGMessage("Criar botao");
         WinCreateWindow(       hwnd,
                                (PSZ) buttonWindowName,
                                (PSZ) "",
                                cmd->lValue|CS_MOVENOTIFY|CS_SIZEREDRAW,
                                0,  0,
                                0,  0,
                                hwnd,
                                HWND_BOTTOM,
                                cmd->pos,
                                NULL,
                                NULL );
         break;

      case CMD_SLIDER:
         DBGMessage("Criar slider");
         DBGTrace(cmd->pos);
         WinCreateWindow(       hwnd,
                                (PSZ) scbarWindowName,
                                (PSZ) "",
                                cmd->lValue|CS_MOVENOTIFY|CS_SIZEREDRAW,
                                0,  0,
                                0,  0,
                                hwnd,
                                HWND_BOTTOM,
                                cmd->pos,
                                NULL,
                                NULL );
         break;

      case CMD_LISTBOX:
         DBGMessage("Criar listbox");
         DBGTrace(cmd->pos);
         WinCreateWindow(       hwnd,
                                (PSZ) listboxWindowName,
                                (PSZ) "",
                                cmd->lValue|CS_MOVENOTIFY|CS_SIZEREDRAW,
                                0,  0,
                                0,  0,
                                hwnd,
                                HWND_BOTTOM,
                                cmd->pos,
                                NULL,
                                NULL );
         break;
      }
   }

   DBGTracex(pszText);

   if(pszText)
      sknLoad(hwnd,cfg,winElements,NULL);

   DBGTracex(cfg);

   return cfg;
}

void EXPENTRY sknPaint(HPS hps, HSKIN cfg, PRECTL rcl)
{
/*
   paintMainBackGround(cfg, hps, cfg->cx, cfg->cy);
*/

   POINTL       p;

   if(cfg->BITMAP_BACKGROUND)
   {
      p.x = rcl->xLeft;
      p.y = rcl->yBottom;

      WinDrawBitmap(    hps,
                        cfg->BITMAP_BACKGROUND,
                        rcl,
                        &p,
                        SKC_FOREGROUND,
                        SKC_BACKGROUND,
                        DBM_NORMAL);
   }
   else
   {
      WinFillRect(hps, rcl, CLR_WHITE);
   }

}


void EXPENTRY sknDestroy(HSKIN cfg)
{
   if(!cfg)
      return;

   if(cfg->menuFont)
   {
      free(cfg->menuFont);
      cfg->menuFont = NULL;
   }

   destroyImages(cfg);

   /* Libera bloco de controle */
   free(cfg);

}

void destroyImages(HSKIN cfg)
{
   /* Destroi todos os bitmaps criados */
   int       f;

   for(f=0;f<BITMAPS_MAINWIN;f++)
   {
      if(cfg->b[f])
      {
         GpiDeleteBitmap(cfg->b[f]);
         cfg->b[f] = 0;
      }
   }

   for(f=0;f<XBITMAPS;f++)
   {
      if(cfg->xbm[f].masc)
      {
         GpiDeleteBitmap(cfg->xbm[f].masc);
         cfg->xbm[f].masc = 0;
      }
      if(cfg->xbm[f].image)
      {
         GpiDeleteBitmap(cfg->xbm[f].image);
         cfg->xbm[f].image = 0;
      }
   }

   for(f=0;f<ICONBARS;f++)
   {
      if(cfg->iconbar[f].masc)
      {
         GpiDeleteBitmap(cfg->iconbar[f].masc);
         cfg->iconbar[f].masc = 0;
      }
      if(cfg->iconbar[f].image)
      {
         GpiDeleteBitmap(cfg->iconbar[f].image);
         cfg->iconbar[f].image = 0;
      }
   }

}

void EXPENTRY sknDoTimer(HWND hwnd, HSKIN cfg)
{
   POINTL        p;
   HWND          h;
   SHORT         fx;
   SHORT         fy;
   SWP           swp;
   ULONG         flag   = 0;

   switch(cfg->drgMode)
   {
   case 1:      /* Resize */
      WinQueryPointerPos(HWND_DESKTOP,&p);
      WinQueryWindowPos(h = WinQueryWindow(hwnd,QW_PARENT), &swp);

      fx = p.x - cfg->drgPos.x;
      fy = p.y - cfg->drgPos.y;

      swp.y  += fy;
      swp.cx += fx;
      swp.cy -= fy;
      flag    = SWP_MOVE|SWP_SIZE;
      break;

   case 2:      /* Move */
      WinQueryPointerPos(HWND_DESKTOP,&p);
      WinQueryWindowPos(h = WinQueryWindow(hwnd,QW_PARENT), &swp);

      fx = p.x - cfg->drgPos.x;
      fy = p.y - cfg->drgPos.y;

      swp.x += fx;
      swp.y += fy;
      flag   = SWP_MOVE;
      break;

   }

   if(flag)
   {
      /* Ajusta tamanho minimo */

      /*
      fx = max(swp.cx,cfg->minSize.x);
      fy = max(swp.cy,cfg->minSize.y);

      if(cfg->maxSize.x)
         fx = min(fx,cfg->maxSize.x);

      if(cfg->maxSize.y)
         fy = min(fy,cfg->maxSize.y);

      if(fx == swp.cx && fy == swp.cy)
      {
         // Corrige a janela
         cfg->drgPos.x = p.x;
         cfg->drgPos.y = p.y;
         WinSetWindowPos(h, 0, swp.x, swp.y, swp.cx, swp.cy, flag);
      }
      */

      if(swp.cx > cfg->maxSize.x)
         swp.cx = cfg->maxSize.x;

      if(swp.cy > cfg->maxSize.y)
         swp.cy = cfg->maxSize.y;

      if(swp.cx < cfg->minSize.x)
         swp.cx = cfg->minSize.x;

      if(swp.cy < cfg->minSize.y)
         swp.cy = cfg->minSize.y;

      cfg->drgPos.x = p.x;
      cfg->drgPos.y = p.y;

      WinSetWindowPos(h, 0, swp.x, swp.y, swp.cx, swp.cy, flag);
   }

}


void EXPENTRY sknSetDrag(HWND hwnd, HSKIN cfg, USHORT modo, char fator)
{
   if(modo)
   {
      DBGMessage("Iniciou o resize");
      cfg->drgMode = fator;
      WinQueryPointerPos(HWND_DESKTOP,&cfg->drgPos);
      DBGTrace(cfg->drgPos.x);
      DBGTrace(cfg->drgPos.y);
      WinSetCapture(HWND_DESKTOP,hwnd);
   }
   else
   {
      if(cfg->drgMode)
      {
         WinInvalidateRect(hwnd, 0, TRUE);
         WinSetCapture(HWND_DESKTOP,NULLHANDLE);
      }
      cfg->drgMode = 0;
   }
}

MRESULT EXPENTRY sknDefaultButtons(HWND hwnd, HSKIN cfg, short id)
{
   SWP  swp;
   HWND h;

   switch(id)
   {
   case DID_CLOSEBUTTON:                /* Close */
      WinPostMsg(hwnd,WM_CLOSE,0,0);
      break;

   case DID_RESIZEBUTTON:               /* Resize */
      break;

   case DID_MINIMIZEBUTTON:             /* Minimize */
      WinSetWindowPos(WinQueryWindow(hwnd,QW_PARENT),0,0,0,0,0,SWP_MINIMIZE);
      break;

   case DID_MAXIMIZEBUTTON:             /* Maximize/Restore */
      WinQueryWindowPos(h = WinQueryWindow(hwnd,QW_PARENT),&swp);
      if( (swp.fl & (SWP_MAXIMIZE|SWP_MINIMIZE)) != 0 )
         WinSetWindowPos(h,0,0,0,0,0,SWP_RESTORE);
      else
         WinSetWindowPos(h,0,0,0,0,0,SWP_MAXIMIZE);
      break;

   }
   return 0;
}

void EXPENTRY sknConnectListBox(HWND hwnd, USHORT LBox, USHORT SBar, USHORT up, USHORT down)
{
   HWND hBar  = WinWindowFromID(hwnd,SBar);
   HWND hUp   = WinWindowFromID(hwnd,up);
   HWND hDown = WinWindowFromID(hwnd,down);
   HWND hBox  = WinWindowFromID(hwnd,LBox);

   if(hUp)
      WinSendMsg(hUp,WMSKN_CONNECT,MPFROMLONG(hBar),MPFROMLONG(WMSKN_UP));

   if(hDown)
      WinSendMsg(hDown,WMSKN_CONNECT,MPFROMLONG(hBar),MPFROMLONG(WMSKN_DOWN));

   if(hBar)
      WinSendMsg(hBar,WMSKN_CONNECT,MPFROMLONG(hBox),MPFROMLONG(LM_SETTOPINDEX));

   if(hBox)
      WinSendMsg(hBox,WMSKN_CONNECT,MPFROMLONG(hBar),MPFROMLONG(WMSKN_SETVALUE));
}

void notifyClients(HWND hwnd,const SKINELEMENT *elements, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   const SKINELEMENT *cmd;

   for(cmd = elements;cmd->type;cmd++)
   {
      if(cmd->type == CMD_BUTTON || cmd->type == CMD_SLIDER || cmd->type == CMD_LISTBOX)
         WinSendDlgItemMsg(hwnd,cmd->pos,msg,mp1,mp2);
   }

}

void EXPENTRY sknReloadSkin(HWND hwnd, char *name, HSKIN cfg, const SKINELEMENT *elm)
{
/*

   SWP 		swp;
   HPOINTER	ptr;

   ptr = WinQueryPointer(HWND_DESKTOP);

   WinSetPointer(HWND_DESKTOP,WinQuerySysPointer(HWND_DESKTOP,SPTR_WAIT,FALSE));

   WinQueryWindowPos(hwnd,&swp);
   WinSetWindowPos(hwnd,0,swp.x,swp.y,0,0,SWP_SIZE);
   WinSendMsg(hwnd,WMSKN_LOADSKIN,0,0);
   WinSetWindowPos(hwnd,0,swp.x,swp.y,swp.cx,swp.cy,SWP_SIZE);
   WinInvalidateRect(hwnd,0,TRUE);

   WinSetPointer(HWND_DESKTOP,ptr);
*/

   sknLoadNewSkin(hwnd,elm,name);

}

