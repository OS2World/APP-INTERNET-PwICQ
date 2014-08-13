/*
 * os2login.C - Processa dialogo de logon
 */

 #pragma strings(readonly)

 #define INCL_WIN
 #include <pwicqgui.h>
 #include <stdlib.h>
 #include <stdio.h>

/*---[ Definicoes ]-----------------------------------------------------------------------------------*/

 struct parm
 {
    USHORT	sz;
    HICQ	icq;
 };

/*---[ Prototipos ]-----------------------------------------------------------------------------------*/

 static void    WinCentralize(HWND);
 static void	initialize(HWND,HICQ);
 static void 	ProcButton(HWND, HICQ, short);

/*---[ Implementacao ]--------------------------------------------------------------------------------*/

 MRESULT EXPENTRY lgnProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    switch(msg)
    {
    case WM_COMMAND:
       if(SHORT1FROMMP(mp2) == CMDSRC_PUSHBUTTON)
          ProcButton(hwnd,WinQueryWindowPtr(hwnd,0),SHORT1FROMMP(mp1));
       break;

    case WM_INITDLG:
       initialize(hwnd,((struct parm *) PVOIDFROMMP(mp2))->icq);
       break;

    default:
       return WinDefDlgProc(hwnd, msg, mp1, mp2);
    }

    return 0;
 }

 void EXPENTRY icqskin_logonDialog(HICQ icq)
 {
    struct parm p;

    p.sz  = sizeof(struct parm);
    p.icq = icq;

    WinDlgBox(  HWND_DESKTOP,
                HWND_DESKTOP,
                lgnProc,
                module,
                20,
                &p );
 }

 static void WinCentralize(HWND hwnd)
 {
     SWP win;
     SWP dkt;

     WinQueryWindowPos(HWND_DESKTOP,&dkt);
     WinQueryWindowPos(hwnd,&win);

     dkt.cx >>= 1;
     dkt.cy >>= 1;

     win.cx >>= 1;
     win.cy >>= 1;

     dkt.cx -= win.cx;
     dkt.cy -= win.cy;

     WinSetWindowPos(    hwnd,
                         0,
                         dkt.cx, dkt.cy,
                         0,0,
                         SWP_MOVE|SWP_SHOW|SWP_ACTIVATE );

 }

 static void initialize(HWND hwnd, HICQ icq)
 {
    char buffer[80];
    ULONG uin	= icqQueryUIN(icq);

    WinSetWindowPtr(hwnd,QWL_USER, icq);

    if(uin)
    {
       sprintf(buffer,"%d",uin);
       WinSetDlgItemText(hwnd,100,buffer);
    }

    WinSendDlgItemMsg(hwnd,101,EM_SETTEXTLIMIT,MPFROMSHORT(8),0);

    WinCentralize(hwnd);
 }

 static void ProcButton(HWND hwnd, HICQ icq, short id)
 {
    char 	buffer[20];
    ULONG	uin;

    switch(id)
    {
    case 103:
       DBGMessage("Logon");
       WinQueryDlgItemText(hwnd,100,19,buffer);
       uin = atol(buffer);
       if(uin)
       {
          WinQueryDlgItemText(hwnd,101,19,buffer);
          DBGTrace(uin);
          DBGMessage(buffer);
          DBGTrace(WinQueryButtonCheckstate(hwnd,102));

          if(*buffer)
          {
             if(!icqSetCurrentUser(icq, uin, buffer, WinQueryButtonCheckstate(hwnd,102)))
                WinDismissDlg(hwnd,1);
          }
          else
          {
             WinSetFocus(HWND_DESKTOP,WinWindowFromID(hwnd,101));
          }

       }
       else
       {
          WinSetFocus(HWND_DESKTOP,WinWindowFromID(hwnd,100));
       }
       break;

    case 104:
       DBGMessage("New User");
       break;

    case 105:
       WinDismissDlg(hwnd,0);
       break;

    }

 }
