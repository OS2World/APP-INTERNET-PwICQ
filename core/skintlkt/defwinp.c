/*
 * Processamento default de mensagems da janela de skin
 */

 #define INCL_WIN
 #include <os2.h>
 #include <pwMacros.h>
 #include "skin.h"

 #include <malloc.h>
 #include <string.h>
 #include <stdlib.h>

/*---[ Definicoes ]---------------------------------------------------------------*/


/*---[ Prototipos ]---------------------------------------------------------------*/

 static void     paint(HWND,HSKIN);
 static void     erase(HWND,HSKIN,HPS,PRECTL);
 static void     drawItem(HWND,HSKIN,USHORT,POWNERITEM);
 static void     setSize(HWND,HSKIN,USHORT,USHORT);

/*---[ Implementacao ]------------------------------------------------------------*/

 MRESULT EXPENTRY sknDefWindowProc(HWND hwnd, HSKIN skn, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    switch(msg)
    {
    /*---[ Manipulacao do SKIN ]-----------------------------------*/

    case WMSKN_QUERYSKIN:
       return (MRESULT) skn;

    case WMSKN_QUERYMENUFONT:
       return (MRESULT) skn->menuFont;

    case WMSKN_FONTCHANGED:
//       DBGMessage("Janela principal foi avisada da mudanca de fonte");
       break;

    case WMSKN_LOADNEWSKIN:
       DBGMessage("Carregar novo skin");
       return (MRESULT) sknLoad(hwnd, skn, (const SKINELEMENT *) mp2, (const char *) mp1);

    case WMSKN_SETSIZE:
       setSize(hwnd,skn,SHORT1FROMMP(mp1),SHORT2FROMMP(mp2));
       break;

    /*---[ Acoes ]-------------------------------------------------*/

    case WM_TIMER:
       sknDoTimer(hwnd,skn);
       break;

    case WM_COMMAND:
       if(SHORT1FROMMP(mp2) == CMDSRC_PUSHBUTTON)
          return sknDefaultButtons(hwnd, skn, SHORT1FROMMP(mp1));
       return WinDefWindowProc(hwnd, msg, mp1, mp2);


    /*---[ Desenho dos controles ]---------------------------------*/

    case WM_MEASUREITEM: /* MP1=ID, MP2=Index */
       return 0;

    case WM_DRAWITEM:
       drawItem(hwnd,skn,SHORT1FROMMP(mp1),PVOIDFROMMP(mp2));
       break;

    /*---[ Movimentacao da janela ]--------------------------------*/

    case WMSKN_SETSIZING:   /* Processa resize MP1=ON/OFF */
       sknSetDrag(hwnd,skn,SHORT1FROMMP(mp1),1);
       break;

    case WM_BUTTON2UP:
    case WM_ENDDRAG:
       sknSetDrag(hwnd,skn,0,0);
       break;

    case WM_BEGINDRAG:
       sknSetDrag(hwnd,skn,1,2);
       break;

    /*---[ Drag & Drop ]-------------------------------------------*/

    case DM_DRAGOVER:
       DBGTrace(WinQueryWindowUShort(hwnd,QWS_ID));

       if(WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),WMSKN_QUERYDRAG,(MPARAM) hwnd, MPFROM2SHORT(WinQueryWindowUShort(hwnd,QWS_ID),0)))
       {
          DBGMessage("DragOver aceito");
          return WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),msg,mp1,mp2);
       }
       else
       {
          DBGMessage("DragOver rejeitado");
          return WinDefWindowProc(hwnd, msg, mp1, mp2);
       }
       break;

    case DM_DRAGLEAVE:
       DBGMessage("DragLeave");
       return WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),msg,mp1,mp2);

    case DM_DROP:
       DBGMessage("Drop");
       return WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),msg,mp1,mp2);


    /*---[ Manipulacao da janela ]---------------------------------*/

    case WM_PAINT:
       paint(hwnd,skn);
       break;

    case WM_ERASEBACKGROUND:
       erase(hwnd,skn,LONGFROMMP(mp1),PVOIDFROMMP(mp2));
       break;

    case WM_DESTROY:
       sknDestroy(skn);
       return WinDefWindowProc(hwnd, msg, mp1, mp2);


    default:
       return WinDefWindowProc(hwnd, msg, mp1, mp2);

    }
    return 0;
 }

 static void erase(HWND hwnd,HSKIN skn,HPS hps,PRECTL rcl)
 {
    hps = WinBeginPaint(hwnd,NULLHANDLE,rcl);
    sknPaint(hps,skn,rcl);
    WinEndPaint(hps);
 }

 static void paint(HWND hwnd,HSKIN skn)
 {
    RECTL        rcl;
    HPS          hps;
    hps = WinBeginPaint(hwnd,NULLHANDLE,&rcl);
    sknPaint(hps,skn,&rcl);
    WinEndPaint(hps);
 }

 static void drawItem(HWND hwnd, HSKIN skn, USHORT id, POWNERITEM i)
 {

    if(i->hItem && !(i->fsState&0x80000000))
    {
       i->rclItem.xLeft  += 4;
       i->rclItem.xRight -= 4;

       WinDrawText(   i->hps,
                      -1,
                      (char *) i->hItem,
                      &i->rclItem,
                      i->fsState & 1 ? SKC_SELECTEDFOREGROUND : ( i->fsState & 2 ? SKC_USER1 : SKC_FOREGROUND),
                      2,
                      DT_LEFT|DT_VCENTER);
    }

 }

 static void setSize(HWND hwnd, HSKIN cfg, USHORT cx, USHORT cy)
 {
   SWP  swp;

   hwnd = WinQueryWindow(hwnd,QW_PARENT);

   WinQueryWindowPos(hwnd,&swp);

   DBGTrace(swp.cx);
   DBGTrace(swp.cy);

   if(swp.cx > cfg->maxSize.x)
      swp.cx = cfg->maxSize.x;

   if(swp.cy > cfg->maxSize.y)
      swp.cy = cfg->maxSize.y;

   if(swp.cx < cfg->minSize.x)
      swp.cx = cfg->minSize.x;

   if(swp.cy < cfg->minSize.y)
      swp.cy = cfg->minSize.y;

   DBGTrace(swp.cx);
   DBGTrace(swp.cy);

   WinSetWindowPos(hwnd,0,swp.x,swp.y,swp.cx,swp.cy,SWP_SIZE);

/*
    DBGTracex(hwnd);

    hwnd = WinQueryWindow(hwnd,QW_PARENT);

    DBGTrace(cx);
    DBGTrace(cy);

    if(cx > cfg->maxSize.x)
       cx = cfg->maxSize.x;

    if(cy > cfg->maxSize.y)
       cy = cfg->maxSize.y;

    if(cx < cfg->minSize.x)
       cx = cfg->minSize.x;

    if(cy < cfg->minSize.y)
       cy = cfg->minSize.y;

    DBGTrace(cx);
    DBGTrace(cy);

    WinSetWindowPos(hwnd,0,0,0,cx,cy,SWP_SIZE);
*/

 }


