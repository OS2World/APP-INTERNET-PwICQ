/*
 * LISTBOX.C - Controle listbox
 */

#define INCL_WIN
#define INCL_GPI
#include <os2.h>

#include <stdlib.h>

#include <malloc.h>
#include <string.h>

#include "skin.h"

/*---[ Definicoes ]---------------------------------------------------------------*/

#define _ALLOC_STEP_    100
#define UPDATE_TIME     50

/*---[ Prototipos ]---------------------------------------------------------------*/

 static void     create(HWND, PCREATESTRUCT);
 static void     destroy(HWND);
 static void     paint(HWND);
 static void     sizePos(HWND,SHORT, SHORT, SHORT, SHORT);
 static void     reposic(HWND, USHORT, USHORT);
 static void     addItemHandle(HWND,ULONG);
 static void     doConnect(HWND, HWND, ULONG);
 static void     setTop(HWND,USHORT);
 static BOOL     btnDown(HWND,ULONG,ULONG);
 static BOOL     btnUp(HWND,ULONG,ULONG);
 static BOOL     mouseMove(HWND, USHORT, USHORT);
 static void     selectItem(HWND, LBDATA *, short);
 static void     chkForMenu(HWND);
 static void     ajustBackground(HWND, PSWP);
 static MRESULT  queryHandle(HWND,USHORT);
 static HWND     assignMenu(HWND, USHORT, ULONG);
 static void     menuCommand(HWND, ULONG, MPARAM, MPARAM);
 static void     clearList(HWND);
 static void     resize(HWND, USHORT, USHORT);
 static void     blink(HWND,USHORT);
 static void     redraw(HWND,LBDATA *);
 static void     drawImage(HWND,HPS,PRECTL,USHORT);
 static ULONG    querySelectedHandle(LBDATA *);
 static void     moveCursor(HWND,LONG);
 static int      sort(HWND);
 static int      queryByHandler(HWND,ULONG);
 static MRESULT  dragOver(HWND, PDRAGINFO, MPARAM);
 static MRESULT  dragLeave(HWND, LBDATA *, MPARAM, MPARAM);
 static MRESULT  drop(HWND, LBDATA *, MPARAM, MPARAM);


/*---[ Constantes ]---------------------------------------------------------------*/

 extern const char *listboxWindowName = "pwLISTBOX.wnd";

/*---[ Implementacao ]------------------------------------------------------------*/

 MRESULT EXPENTRY lbsProcedure(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    switch(msg)
    {
    case WM_CREATE:
       create(hwnd,PVOIDFROMMP(mp2));
       return WinDefWindowProc(hwnd, msg, mp1, mp2);

    case WM_DESTROY:
       destroy(hwnd);
       return WinDefWindowProc(hwnd, msg, mp1, mp2);

    case WM_PAINT:
       paint(hwnd);
       break;

    case WM_BUTTON2DOWN:
       WinSetFocus(HWND_DESKTOP,WinQueryWindow(hwnd, QW_OWNER));
       if(btnDown(hwnd,LONGFROMMP(mp1),LBF_BUTTON2))
          chkForMenu(hwnd);
       break;

    case WM_BUTTON2UP:
       btnUp(hwnd,LONGFROMMP(mp1),LBF_BUTTON2);
       break;

    case WM_BUTTON1DOWN:
       WinSetFocus(HWND_DESKTOP,WinQueryWindow(hwnd, QW_OWNER));
       btnDown(hwnd,LONGFROMMP(mp1),0);
       break;

    case WM_BUTTON1UP:
       btnUp(hwnd,LONGFROMMP(mp1),0);
       break;

    case WM_BUTTON1DBLCLK:      /* Double click - Avisa a mÆe */
       btnUp(hwnd, 0, 0);
       WinPostMsg(       WinQueryWindow(hwnd, QW_OWNER),
                         WM_CONTROL,
                         MPFROM2SHORT(WinQueryWindowUShort(hwnd,QWS_ID),LN_ENTER),
                         MPFROMLONG(hwnd) );
       break;

    case WM_MOUSEMOVE:
       mouseMove(hwnd, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1));
       break;

    case LM_SETTOPINDEX:
       setTop(hwnd,SHORT1FROMMP(mp1));
       break;

    case WMSKN_AJUSTSIZEPOS:
       sizePos(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1),SHORT1FROMMP(mp2),SHORT2FROMMP(mp2));
       break;

    case WMSKN_AJUSTPOS:
       reposic(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1));
       break;

    case WMSKN_CONNECT:  /* MP1=Handle */
       doConnect(hwnd,LONGFROMMP(mp1),LONGFROMMP(mp2));
       break;

    case WMSKN_INSERTHANDLE:   /* MP1=Handle */
       addItemHandle(hwnd,LONGFROMMP(mp1));
       break;

    case WM_WINDOWPOSCHANGED:
       ajustBackground(hwnd,PVOIDFROMMP(mp1));
       return WinDefWindowProc(hwnd, msg, mp1, mp2);

    case WMSKN_ASSIGNMENU: /* MP1=Menu ID, MP2=Style */
       return MRFROMLONG(assignMenu(hwnd,SHORT1FROMMP(mp1),LONGFROMMP(mp2)));

    case WM_PRESPARAMCHANGED:
       ajustPresParam(hwnd,LONGFROMMP(mp1));
       break;

    case WMSKN_INSERTMENUITEM: /* MP1=Text, MP2=ID */
    case WMSKN_INSERTHANDLER:
    case WMSKN_SHOWMENU:
       menuCommand(hwnd,msg,mp1,mp2);
       break;

    case WMSKN_QUERYMENU:
       return (MRESULT) ((LBDATA *) WinQueryWindowPtr(hwnd,0))->hMenu;

    case WMSKN_QUERYMENUFONT:
       DBGMessage("Responder a pedido de fonte");
       break;

    case WMSKN_BLINK:   /* Redesenhar a imagem base */
       blink(hwnd,SHORT1FROMMP(mp1));
       break;

    case WMSKN_REDRAW:   /* Redesenhar a imagem base */
       redraw(hwnd,WinQueryWindowPtr(hwnd,0));
       break;

    case WMSKN_CLEAR:
       clearList(hwnd);
       break;

    case WM_SIZE:
       resize(hwnd, SHORT1FROMMP(mp2), SHORT2FROMMP(mp2));
       break;

    case WM_USER+1:      /* Pintar a janela piscando MP1=HPS, MP2=PRECTL */
       drawImage(hwnd,LONGFROMMP(mp1),PVOIDFROMMP(mp2), 0);
       break;

    case LM_QUERYSELECTION:
       return MRFROMSHORT( ((LBDATA *) WinQueryWindowPtr(hwnd,0))->selected );

    case WMSKN_QUERYSELECTEDHANDLE:
       return (MRESULT) querySelectedHandle(WinQueryWindowPtr(hwnd,0));

    case WMSKN_UP:
       moveCursor(hwnd, -1);
       break;

    case WMSKN_DOWN:
       moveCursor(hwnd, 1);
       break;

    case WMSKN_SORT:            /* Reordenar a lista */
       return (MRESULT) sort(hwnd);

    case WMSKN_SETSORTCALLBACK: /* Ajusta funcao de reorder */
       ((LBDATA *) WinQueryWindowPtr(hwnd,0))->reorder = (int (* _System)(ULONG, ULONG)) mp1;
       WinPostMsg(hwnd,WMSKN_SORT,0,0);
       break;

    case WMSKN_QUERYBYHANDLER:  /* Localizar pelo handler MP1=Handler*/
       return (MRESULT) queryByHandler(hwnd,LONGFROMMP(mp1));

    /*---[ Drag & Drop ]-------------------------------------------*/

    case DM_DRAGOVER:
       return dragOver(hwnd, (PDRAGINFO) mp1, mp2);

    case DM_DRAGLEAVE:
       return dragLeave(hwnd,WinQueryWindowPtr(hwnd,0),mp1,mp2);

    case DM_DROP:
       return drop(hwnd,WinQueryWindowPtr(hwnd,0),mp1,mp2);

    default:
       return WinDefWindowProc(hwnd, msg, mp1, mp2);

    }
    return 0;
 }

 static ULONG querySelectedHandle(LBDATA *cfg)
 {
    if(cfg->selected == LIT_NONE)
       return 0;
    return (cfg->list+cfg->selected)->hItem;
 }

 static void create(HWND hwnd, PCREATESTRUCT info)
 {
    LBDATA       *cfg            = malloc(sizeof(LBDATA));

    WinSetWindowPtr(hwnd,0,cfg);

    if(!cfg)
    {
       WinPostMsg(hwnd,WM_CLOSE,0,0);
       return;
    }

    memset(cfg,0,sizeof(LBDATA));

    cfg->flags    |= (LBF_REDRAW1|LBF_REDRAW0);
    cfg->selected  =
    cfg->top       = LIT_NONE;
 }

 static void destroy(HWND hwnd)
 {
    LBDATA       *cfg = WinQueryWindowPtr(hwnd,0);
    int          f;

    if(cfg->bg)
       GpiDeleteBitmap(cfg->bg);

    if(cfg->cache[0])
       GpiDeleteBitmap(cfg->cache[0]);

    if(cfg->cache[1])
       GpiDeleteBitmap(cfg->cache[1]);

    if(cfg->list)
       free(cfg->list);

    free(cfg);
 }

 static void blink(HWND hwnd, USHORT modo)
 {
    LBDATA *cfg = WinQueryWindowPtr(hwnd,0);
    RECTL  rcl;

    if(cfg->flags & LBF_DRAG)
       return;

    if(modo)
       cfg->drawFlags |= 2;
    else
       cfg->drawFlags &= ~2;

    if(cfg->szImage)
    {
       WinQueryWindowRect(hwnd,&rcl);
       rcl.xRight = cfg->szImage;
       WinInvalidateRect(hwnd, &rcl, TRUE);
    }
    else
    {
       WinInvalidateRect(hwnd, 0, TRUE);
    }

 }

 static void redraw(HWND hwnd, LBDATA *cfg)
 {
    cfg->flags   |= (LBF_REDRAW1|LBF_REDRAW0);
    cfg->szImage  = 0;
    WinInvalidateRect(hwnd, 0, TRUE);
 }

 static void paint(HWND hwnd)
 {
    LBDATA       *cfg     = WinQueryWindowPtr(hwnd,0);
    HPS          hps;
    POINTL       p        = { 0, 0 };
    RECTL        rcl;
    OWNERITEM    itn;
    HSKIN        skn;
    LONG         alTable[SKC_MAXCOLORS];
    HBITMAP      bg       = cfg->bg;
    int          f;
    BOOL         useCache = FALSE;

    if( (WinQueryWindowULong(hwnd,QWL_STYLE) & WMSKS_CACHEDLB))
    {
       useCache = TRUE;

       if(cfg->drawFlags & 2)
       {
          /* Pinta imagem piscando */
          bg = cfg->cache[1];

          if(cfg->flags & LBF_REDRAW1)
          {
             paintWindow(hwnd, cfg->cache[1], cfg->bg);
             cfg->flags &= ~LBF_REDRAW1;
          }
       }
       else
       {
          /* Pinta imagem pura */
          bg = cfg->cache[0];

          if(cfg->flags & LBF_REDRAW0)
          {
             paintWindow(hwnd, cfg->cache[0], cfg->bg);
             cfg->flags &= ~LBF_REDRAW0;
          }
       }
    }

    hps = WinBeginPaint(hwnd,NULLHANDLE,&rcl);

    p.x = rcl.xLeft;
    p.y = rcl.yBottom;
    WinDrawBitmap(hps, bg, &rcl, &p, CLR_WHITE, CLR_BLACK, DBM_NORMAL);

    skn = (HSKIN) WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),WMSKN_QUERYSKIN,0,0);

    GpiQueryLogColorTable(hps, 0L, 0L, SKC_MAXCOLORS, alTable);
    for(f=0;f<SKC_MAXCOLORS;f++)
       alTable[f] = skn->color[f];
    GpiCreateLogColorTable(hps,0L,LCOLF_CONSECRGB,0L,SKC_MAXCOLORS,alTable);

    WinQueryWindowRect(hwnd,&rcl);
    drawImage(hwnd, hps, &rcl, useCache ? 2 : 1);

    WinEndPaint(hps);

 }

 static void drawImage(HWND hwnd, HPS hps, PRECTL rcl, USHORT modo)
 {
    LBDATA       *cfg            = WinQueryWindowPtr(hwnd,0);
    SHORT        atual;
    int          row             = rcl->yTop;
    OWNERITEM    itn;
    USHORT       id              = WinQueryWindowUShort(hwnd,QWS_ID);
    HWND         owner           = WinQueryWindow(hwnd, QW_OWNER);
    POINTL       p;

    if(cfg->top != LIT_NONE)
    {
       itn.rclItem.yTop = row;
       for(atual = cfg->top;atual < cfg->itens && itn.rclItem.yTop > 0; atual++)
       {
          cfg->bottom = atual;

          memset(&itn,0,sizeof(OWNERITEM));

          itn.hwnd                    = hwnd;
          itn.hps                     = hps;
          itn.idItem                  = atual;
          itn.hItem                   = (cfg->list + atual)->hItem;
          (cfg->list+atual)->yTop     =
          itn.rclItem.yTop            = row;

          row -= queryItemSize(hps,owner,id,(const char *) itn.hItem,atual,NULL);

          (cfg->list+atual)->yBottom  =
          itn.rclItem.yBottom         = row;

          row--;

          itn.rclItem.xLeft   = rcl->xLeft;
          itn.rclItem.xRight  = rcl->xRight;

          switch(modo)
          {
          case 0:        /* Desenhando so a imagem */
             itn.fsState = cfg->drawFlags|0x80000000;
             break;

          case 1:        /* Desenhando tudo */
             itn.fsState = cfg->drawFlags |0xC0000000;
             break;

          case 2:        /* Desenhando so o texto */
             itn.fsState = cfg->drawFlags|0x40000000;
             itn.rclItem.xLeft   = (cfg->list + atual)->xLeft;
             itn.rclItem.xRight  = (cfg->list + atual)->xRight;
             break;

          }

          if(atual == cfg->selected)
          {
             if(cfg->flags & LBF_DRAG)
             {
                WinSendMsg(owner,WM_DRAWITEM,MPFROMSHORT(id),MPFROMP(&itn));

                GpiSetColor(hps,SKC_SELECTEDBACKGROUND);
                p.y = (cfg->list+atual)->yBottom+1;
                p.x = rcl->xLeft;
                GpiMove(hps,&p);
                p.y = (cfg->list+atual)->yTop-1;
                p.x = rcl->xRight-1;
                GpiBox(hps,DRO_OUTLINE,&p,1,1);
             }
             else
             {
                WinFillRect(hps, &itn.rclItem, SKC_SELECTEDBACKGROUND);
                itn.fsState |= 1;
                WinSendMsg(owner,WM_DRAWITEM,MPFROMSHORT(id),MPFROMP(&itn));
             }
          }
          else
          {
             WinSendMsg(owner,WM_DRAWITEM,MPFROMSHORT(id),MPFROMP(&itn));
          }


          if(!modo)
          {
             /* Desenhou somente a imagem */
             cfg->szImage = max(cfg->szImage,itn.rclItem.xLeft);
             (cfg->list + atual)->xLeft  = itn.rclItem.xLeft;
             (cfg->list + atual)->xRight = itn.rclItem.xRight;
          }

          itn.rclItem.yTop = (cfg->list+atual)->yTop;

       }
    }

 }

 static void sizePos(HWND hwnd,SHORT x, SHORT y, SHORT cx, SHORT cy)
 {
    LBDATA *cfg = WinQueryWindowPtr(hwnd,0);

//    DBGTrace(cx);
//    DBGTrace(cy);

    cfg->x  = x;
    cfg->y  = y;
    cfg->cx = cx;
    cfg->cy = cy;
 }

 static void reposic(HWND hwnd,USHORT mCx,USHORT mCy)
 {
    LBDATA    *cfg = WinQueryWindowPtr(hwnd,0);
    SHORT     x,y,cx,cy;

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

 static void clearList(HWND hwnd)
 {
    LBDATA  *cfg = WinQueryWindowPtr(hwnd,0);

    if(!cfg->list)
       return;

    cfg->itens = 0;

    WinPostMsg(hwnd,WMSKN_REDRAW,0,0);

 }

 static void addItemHandle(HWND hwnd, ULONG hdl)
 {
    LBDATA  *cfg = WinQueryWindowPtr(hwnd,0);
    ITNDATA *i;

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

    i = (cfg->list + cfg->itens);

    i->hItem     = hdl;

    i->yTop     =
    i->yBottom  = 0;

    if(cfg->hConnect)
       WinPostMsg(cfg->hConnect,WMSKN_SETLIMITS,0,MPFROMLONG(cfg->itens));

    cfg->itens++;

    if(cfg->top == LIT_NONE)
    {
       cfg->top = 0;
       if(cfg->hConnect)
          WinPostMsg(cfg->hConnect,WMSKN_SETVALUE,0,MPFROMLONG(cfg->top));
    }

    WinPostMsg(hwnd,WMSKN_REDRAW,0,0);

 }

 static void doConnect(HWND hwnd, HWND to, ULONG msg)
 {
    LBDATA *cfg = WinQueryWindowPtr(hwnd,0);
    cfg->hConnect = to;
    cfg->mConnect = msg;
 }

 static void setTop(HWND hwnd,USHORT top)
 {
    LBDATA  *cfg = WinQueryWindowPtr(hwnd,0);

    if(top < cfg->itens && top != cfg->top)
    {
       cfg->top = top;
       if(cfg->selected != LIT_NONE)
       {
         (cfg->list+cfg->selected)->yBottom =
         (cfg->list+cfg->selected)->yTop    = 0;
       }
       WinPostMsg(hwnd,WMSKN_REDRAW,0,0);
    }
 }

 static BOOL btnDown(HWND hwnd, ULONG ps, ULONG masc)
 {
    LBDATA  *cfg = WinQueryWindowPtr(hwnd,0);
    POINTS  *p   = (POINTS *) & ps;

    cfg->flags |= (LBF_BUTTONDOWN|masc);

    WinSetCapture(HWND_DESKTOP,hwnd);
    return mouseMove(hwnd, p->x, p->y);
 }

 static BOOL btnUp(HWND hwnd, ULONG p, ULONG masc)
 {
    LBDATA  *cfg = WinQueryWindowPtr(hwnd,0);
    BOOL    ret  = (cfg->flags & masc) != 0;
    WinSetCapture(HWND_DESKTOP,NULLHANDLE);

    cfg->flags &= ~(LBF_BUTTONDOWN|masc);

    WinInvalidateRect(hwnd, 0, TRUE);

    return ret;
 }

 static BOOL mouseMove(HWND hwnd, USHORT x, USHORT y)
 {
    LBDATA       *cfg    = WinQueryWindowPtr(hwnd,0);
    SHORT        atual;

    if(!(cfg->flags & LBF_BUTTONDOWN) )
       return FALSE;

    if(!cfg->list)
       return FALSE;

    for(atual = cfg->top;atual < cfg->itens; atual++)
    {
       if( y <= (cfg->list + atual)->yTop && y >= (cfg->list + atual)->yBottom)
       {
          selectItem(hwnd,cfg,atual);
          return TRUE;
       }
    }
    return FALSE;
 }

 static void selectItem(HWND hwnd, LBDATA *cfg, short atual)
 {
    short  old;
    RECTL  rcl;

    if(atual != cfg->selected)
    {
       old               = cfg->selected;
       cfg->selected     = atual;
       cfg->flags       |= (LBF_REDRAW1|LBF_REDRAW0);    /* Invalida o cache */

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

 static void chkForMenu(HWND hwnd)
 {
    /* Se existir um menu associado, ativa-o */
    LBDATA       *cfg    = WinQueryWindowPtr(hwnd,0);

    if(cfg->hMenu)
       WinPostMsg(cfg->hMenu,WMSKN_SHOWMENU,0,0);

 }

 static void ajustBackground(HWND hwnd, PSWP swp)
 {
    LBDATA *cfg  = WinQueryWindowPtr(hwnd,0);
    createControlBackground(hwnd, swp, &cfg->bg, NULL);
 }

 int queryItemSize(HPS hps, HWND owner, USHORT id, const char *text, short atual, USHORT *cols)
 {
    MRESULT      sizes;
    POINTL       aptl[TXTBOX_COUNT];
    int          ret;

    sizes = WinSendMsg(owner,WM_MEASUREITEM,MPFROMSHORT(id),MPFROMSHORT(atual));

    if(sizes)
    {
       ret = SHORT1FROMMR(sizes);
       if(cols)
          *cols = SHORT2FROMMR(sizes);
    }
    else
    {
       GpiQueryTextBox(    hps,
                           strlen(text),
                           (char *) text,
                           TXTBOX_COUNT,
                           aptl);
       ret = (aptl[TXTBOX_TOPRIGHT].y - aptl[TXTBOX_BOTTOMRIGHT].y);

       if(cols)
          *cols = (aptl[TXTBOX_TOPRIGHT].x - aptl[TXTBOX_TOPLEFT].x);
    }
    return ret;
 }

 static HWND assignMenu(HWND hwnd, USHORT id, ULONG style)
 {
    LBDATA *cfg  = WinQueryWindowPtr(hwnd,0);

    DBGTracex(cfg);

    if(cfg->hMenu)
       return NULLHANDLE;
    cfg->hMenu = CreatePopupMenu(WinQueryWindow(hwnd, QW_OWNER), id, style);
    DBGTracex(cfg->hMenu);
    return cfg->hMenu;
 }

 static void menuCommand(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    LBDATA *cfg  = WinQueryWindowPtr(hwnd,0);
    if(cfg->hMenu)
       WinSendMsg(cfg->hMenu,msg,mp1,mp2);
 }

 void ajustPresParam(HWND hwnd, ULONG parm)
 {
    char         buffer[40];

    switch(parm)
    {
    case PP_FOREGROUNDCOLOR:
       return;

    case PP_BACKGROUNDCOLOR:
       return;

    case PP_FONTNAMESIZE:
       *buffer = 0;
       WinQueryPresParam(      hwnd,
                               parm,
                               0,
                               NULL,
                               40,
                               buffer,
                               0);

//       DBGMessage(buffer);

       if(*buffer)
       {
          WinSendMsg(    WinQueryWindow(hwnd, QW_OWNER),
                         WMSKN_FONTCHANGED,
                         MPFROMSHORT(WinQueryWindowUShort(hwnd,QWS_ID)),
                         MPFROMP(buffer) );
       }
       break;

    default:
       return;

    }
    WinPostMsg(hwnd,WMSKN_REDRAW,0,0);

 }

 static void resize(HWND hwnd, USHORT cx, USHORT cy)
 {
    LBDATA *cfg  = WinQueryWindowPtr(hwnd,0);

    if(cfg->cache[0])
    {
       GpiDeleteBitmap(cfg->cache[0]);
       cfg->cache[0] = NULLHANDLE;
    }

    if(cfg->cache[1])
    {
       GpiDeleteBitmap(cfg->cache[1]);
       cfg->cache[1] = NULLHANDLE;
    }

    cfg->drawFlags &= ~2;

    if( (WinQueryWindowULong(hwnd,QWL_STYLE) & WMSKS_CACHEDLB) )
    {
       cfg->cache[0] = createBaseImage(hwnd,cx,cy);
       cfg->cache[1] = createBaseImage(hwnd,cx,cy);
    }

    WinPostMsg(hwnd,WMSKN_REDRAW,0,0);
 }

 static void moveCursor(HWND hwnd, LONG step)
 {
    LBDATA *cfg  = WinQueryWindowPtr(hwnd,0);
    SHORT  sel   = cfg->selected;

    if(cfg->itens < 1)
       return;

    if(sel == LIT_NONE)
    {
       if(step > 0)
          sel = 0;
       else
          sel = cfg->itens;
    }
    else
    {
       sel += step;
    }

    sel = sel < 0               ? 0             : sel;
    sel = sel > cfg->itens      ? cfg->itens    : sel;

    selectItem(hwnd,cfg,sel);

    /* Verifica se esta acima da area visivel */
    if(cfg->selected < cfg->top)
    {
       cfg->top     = cfg->selected;
       redraw(hwnd,cfg);
    }

    /* Verifica se esta abaixo da area visivel */
    if(cfg->selected > cfg->bottom)
    {
       DBGMessage("********* INCOMPLETO - Rodar a listbox para cima **********");
    }


 }

 static int sort(HWND hwnd)
 {
    LBDATA  *cfg        = WinQueryWindowPtr(hwnd,0);
    ITNDATA *first;
    ITNDATA temp;
    int     changed     = 0;
    int     s1;
    int     s2;

    DBGMessage("Reordenar a lista");

    if(cfg->itens < 2 || !cfg->reorder)
       return 0;

    for(s1=0;s1<(cfg->itens-1);s1++)
    {
       first = (cfg->list+s1);

       for(s2=s1+1;s2<cfg->itens;s2++)
       {
          if(cfg->reorder(first->hItem,(cfg->list+s2)->hItem) > 0)
             first = (cfg->list+s2);
       }

       if(first != (cfg->list+s1) )
       {
          temp            = *(cfg->list+s1);
          *(cfg->list+s1) = *first;
          *first          = temp;
          changed++;
       }
    }

    if(changed)
       WinPostMsg(hwnd,WMSKN_REDRAW,0,0);

    return changed;
 }

 static int queryByHandler(HWND hwnd, ULONG handler)
 {
    LBDATA      *cfg        = WinQueryWindowPtr(hwnd,0);
    int         f;

    for(f=0;f<(cfg->itens-1);f++)
    {
       if( (cfg->list+f)->hItem == handler )
          return f;
    }

    return -1;
 }

 static MRESULT dragOver(HWND hwnd, PDRAGINFO dragInfo, MPARAM mp2)
 {
    LBDATA      *wnd    = WinQueryWindowPtr(hwnd,0);
    MRESULT     rc;
    HWND        owner   = WinQueryWindow(hwnd, QW_OWNER);
    POINTL      ptl;
    SHORT       atual;
    SHORT       select  = -1;

    if(!(wnd->flags & LBF_DRAG))
    {
       if(WinSendMsg(owner,WMSKN_QUERYDRAG,(MPARAM) hwnd, MPFROM2SHORT(WinQueryWindowUShort(hwnd,QWS_ID),0)))
       {
          wnd->flags |= LBF_DRAG;
       }
       else
       {
          return 0;
       }
    }

    rc = WinSendMsg(owner,DM_DRAGOVER,(MPARAM) dragInfo,mp2);

    if(SHORT1FROMMR(rc) == DOR_DROP)
    {

       DrgAccessDraginfo(dragInfo);
       ptl.x = dragInfo->xDrop;
       ptl.y = dragInfo->yDrop;
       DrgFreeDraginfo(dragInfo);

       WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptl, 1);

       for(atual = wnd->top;atual < wnd->itens && select == -1; atual++)
       {
          if( ptl.y <= (wnd->list + atual)->yTop && ptl.y >= (wnd->list + atual)->yBottom)
             select = atual;
       }

       if(select != wnd->selected)
       {
          wnd->flags       |= (LBF_REDRAW1|LBF_REDRAW0);    /* Invalida o cache */
          wnd->selected     = select;

          if(select == -1)
             wnd->flags &= ~LBF_DRAG;

          WinInvalidateRect(hwnd, NULL, FALSE);
       }

       if(select == -1)
          rc = MRFROM2SHORT(DOR_NODROP,0);

    }

    return rc;
 }

 static MRESULT dragLeave(HWND hwnd, LBDATA *wnd, MPARAM mp1, MPARAM mp2)
 {
    DBGMessage("DragLeave");
    wnd->flags &= ~LBF_DRAG;
    wnd->flags |= (LBF_REDRAW1|LBF_REDRAW0);    /* Invalida o cache */
    WinInvalidateRect(hwnd, NULL, FALSE);
    return WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),DM_DRAGLEAVE,mp1,mp2);
 }

 static MRESULT drop(HWND hwnd, LBDATA *wnd, MPARAM mp1, MPARAM mp2)
 {
    DBGMessage("Drop");
    wnd->flags &= ~LBF_DRAG;
    wnd->flags |= (LBF_REDRAW1|LBF_REDRAW0);    /* Invalida o cache */
    WinInvalidateRect(hwnd, NULL, FALSE);
    return WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),DM_DROP,mp1,mp2);
 }

