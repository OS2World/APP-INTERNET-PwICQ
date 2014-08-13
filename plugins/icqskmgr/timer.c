/*
 * TIMER.C - Funcoes de temporizacao
 */

 #define INCL_WIN
 #include <os2.h>

 #include <skinapi.h>


 #include "icqskmgr.h"

/*---[ Prototipos ]---------------------------------------------------------------------------------*/



/*---[ Implementacao ]------------------------------------------------------------------------------*/

 void doTimer(HWND hwnd)
 {
    ICQWNDINFO  *ctl = WinQueryWindowPtr(hwnd,0);
    RECTL       rcl;
    HWND        h;

    sknDoTimer(hwnd,ctl->skn);

    /* Botao de connecting */

    if(ctl->flags & WFLG_CONNECTING)
    {
       if(ctl->modeTimer++ > ctl->modeDiv)
       {
          ctl->modeTicks++;
          ctl->modeTimer = 0;
          h = WinWindowFromID(hwnd,101);
          WinQueryWindowRect(h,&rcl);
          rcl.xRight = rcl.xLeft + (rcl.yTop - rcl.yBottom);
          WinInvalidateRect(h, &rcl, TRUE);
       }
    }

    /* Processa temporizacao */

    if(ctl->ticks++ < ctl->divisor)
       return;

    ctl->ticks = 0;
    ctl->timer++;

    if(ctl->flags & WFLG_BLINKTEXT)
    {
       ctl->flags &= ~WFLG_BLINKTEXT;
       WinInvalidateRect(WinWindowFromID(hwnd,1003),NULL,TRUE);
    }

    if(ctl->flags & WFLG_BLINK)
       sknBlinkControl(hwnd,1003,ctl->timer&1);

    if(ctl->eventIcon != ctl->modeIcon)
    {
       h = WinWindowFromID(hwnd,103);
       WinQueryWindowRect(h,&rcl);
       rcl.xRight = rcl.xLeft + ctl->iconSize;
       WinInvalidateRect(h, &rcl, TRUE);
    }
 }

