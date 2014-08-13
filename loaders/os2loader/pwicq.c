/*
 * PWICQ.C - pwICQ for OS/2 GUI Loader
 */

 #pragma strings(readonly)

 #define INCL_WIN
 #define INCL_WINDIALOGS         /* Window Dialog Mgr Functions  */
 #define INCL_WINPOINTERS        /* Window Pointer Functions     */
 #define INCL_DOSPROCESS
 #define INCL_GPIBITMAPS

 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>
 #include <stdlib.h>
 #include <icqtlkt.h>

/*---[ Definicoes ]--------------------------------------------------------------*/


/*---[ Prototipos ]--------------------------------------------------------------*/

 static void            WinCentralize(HWND);
 static void            mainThread(void *);
 static void _System    setStatus(HICQ,int);
 static int             start(void);
 static void            setString(HWND, SHORT);
 static void            setBitmap(HWND, int);
 static void            blinkWarning(void);
 static void            chkParm(int,char **);

 MRESULT EXPENTRY splashProc(HWND, ULONG, MPARAM, MPARAM);

/*---[ Constantes ]--------------------------------------------------------------*/


/*---[ Statics ]-----------------------------------------------------------------*/

 static HWND    hwnd            = NULLHANDLE;
 static USHORT  splashCX        = 0;
 static USHORT  splashCY        = 0;

/*---[ Implementacao ]-----------------------------------------------------------*/

 int main(int numpar, char *param[])
 {
    HICQ    		icq;
    HMQ     		hmq;
    ULONG   		sknParm;
    const SKINMGR   	*skin;

    chkParm(numpar,param);

    DBGMessage( "---[ " PROJECT " ]----------------------------------------------");

    if(start())
    {
       CHKPoint();

       icq = icqCoreStartup(numpar,param,(STATUSCALLBACK *) setStatus);
       DBGTracex(icq);

       if(icq)
       {
          if( (hmq = WinCreateMsgQueue (icqQueryAnchorBlock(icq),0 )) != NULLHANDLE)
          {
             skin = icqQuerySkinManager(icq);
             DBGTracex(skin);

             if(skin)
             {
                DBGTracex(skin->sz != sizeof(SKINMGR));

                if(skin->sz != sizeof(SKINMGR))
                {
                   setStatus(icq,-5);
                }
                else
                {
                   sknParm = skin->loadSkin(icq,(STATUSCALLBACK *) setStatus,numpar,param);

                   DBGTrace(icqQueryLogonStatus(icq));

                   if(sknParm && icqQueryLogonStatus(icq))
                   {
                      if(hwnd)
                      {
                         /* Check if the configuration window fix is necessary */
                         icqSaveProfileValue(icq, "MAIN","Splash:cx",splashCX);
                         icqSaveProfileValue(icq, "MAIN","Splash:cy",splashCY);
                         if(icqLoadProfileValue(icq,"MAIN","SmallFonts",0x80) & 0x80)
                            icqSaveProfileValue(icq,"MAIN","SmallFonts",splashCX > 200 ? 0x80 : 0x81);
                         WinPostMsg(hwnd,WM_CLOSE,0,0);
                      }

                      /* Skin manager assumes from here */
                      skin->executeSkin(icq,sknParm);
                   }
                   else if(hwnd)
                   {
                      WinSendMsg(hwnd,WM_CLOSE,0,0);
                   }
                }
             }
             else
             {
                setStatus(icq,-4);
             }
             CHKPoint();
             WinDestroyMsgQueue(hmq);
          }
       }

       CHKPoint();
       icqCoreTerminate(icq);

       if(hwnd)
       {
          /* The splash screen is open, wait for 10 seconds */
          DosSleep(1000);
          blinkWarning();
       }

    }

    DBGMessage("Finished");

    return 0;
 }

 static int start(void)
 {
    int f;

    if(_beginthread(mainThread, NULL, 8192, NULL) < 1)
       return 0;

    for(f=0;f<100 && !hwnd; f++)
       DosSleep(10);

    DBGTracex(hwnd);

    if(!hwnd)
    {
       fprintf(stderr,"*** Error loading the startup dialog box");
       DosBeep(3000,50);
       DosSleep(100);
       DosBeep(3000,50);
       DosSleep(100);
       DosBeep(3000,50);
       DosSleep(100);

       DosBeep(3000,100);
       DosSleep(100);
       DosBeep(3000,100);
       DosSleep(100);
       DosBeep(3000,100);
       DosSleep(100);

       DosBeep(3000,50);
       DosSleep(100);
       DosBeep(3000,50);
       DosSleep(100);
       DosBeep(3000,50);
       DosSleep(100);
    }

    return hwnd != NULLHANDLE;
 }

 static void mainThread(void *p)
 {
    HMQ         hmq;
    QMSG        qmsg;
    HAB         hab;

    CHKPoint();

    if( (hab = WinInitialize(0)) != NULLHANDLE)
    {
       CHKPoint();
       if( (hmq = WinCreateMsgQueue (hab,0 )) != NULLHANDLE)
       {
          CHKPoint();
          WinDlgBox(HWND_DESKTOP,HWND_DESKTOP,splashProc,0L,3,NULL);
          CHKPoint();
          WinDestroyMsgQueue(hmq);
       }
       WinTerminate(hab);
    }

    CHKPoint();

 }

 static void _System setStatus(HICQ icq, int status)
 {
    switch(status)
    {
    case -1000:
       if(hwnd)
          WinPostMsg(hwnd,WM_CLOSE,0,0);
       break;

    case -1001:
       if(!hwnd)
          start();
       break;

    default:
       WinPostMsg(hwnd,WM_USER+10,MPFROMSHORT(status),0);
    }
    DosSleep(100);
 }

 MRESULT EXPENTRY splashProc(HWND dlg, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    switch(msg)
    {
    case WM_USER+10:
       setString(hwnd,SHORT1FROMMP(mp1));
       break;

    case WM_USER+11:
       WinShowWindow(WinWindowFromID(hwnd,200),((ULONG) mp1) & 1 ? TRUE : FALSE);
       break;

    case WM_DESTROY:
       hwnd = NULLHANDLE;
       return WinDefDlgProc(hwnd, msg, mp1, mp2);

    case WM_INITDLG:
       CHKPoint();
       hwnd = dlg;
       WinCentralize(dlg);
       break;

    default:
       return WinDefDlgProc(hwnd, msg, mp1, mp2);
    }

    return 0;
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

     DBGTrace(win.cx); // 188 small fonts, 314 large fonts
     DBGTrace(win.cy); // 72 small fonts, 99 large fonts

     splashCX   = win.cx;
     splashCY   = win.cy;

     WinSetWindowPos(    hwnd,
                         0,
                         dkt.cx, dkt.cy,
                         0,0,
                         SWP_MOVE|SWP_SHOW|SWP_ACTIVATE );

 }

 static void setString(HWND hwnd, short vlr)
 {
    char        buffer[0x0100];

    WinQueryDlgItemText(hwnd,202,0xFF,buffer);
    WinSetDlgItemText(hwnd,203,buffer);

    WinQueryDlgItemText(hwnd,201,0xFF,buffer);
    WinSetDlgItemText(hwnd,202,buffer);

    WinQueryDlgItemText(hwnd,200,0xFF,buffer);
    WinSetDlgItemText(hwnd,201,buffer);

    DBGTrace(vlr);

    if(vlr > 0)
    {
       *buffer = 0;
       WinLoadString(WinQueryAnchorBlock(hwnd), NULLHANDLE, 1000 + vlr, 0xFF, (PSZ) buffer);
       WinSetDlgItemText(hwnd,200,buffer);
       setBitmap(hwnd,vlr);
    }
    else
    {
       *buffer = 0;
       WinLoadString(WinQueryAnchorBlock(hwnd), NULLHANDLE, 1499 - vlr, 0xFF, (PSZ) buffer);
       WinSetDlgItemText(hwnd,200,buffer);
    }

    DBGMessage(buffer);

 }

 static void setBitmap(HWND hwnd, int vlr)
 {
    HBITMAP     old     =       LONGFROMMR(WinSendDlgItemMsg(hwnd,101,SM_QUERYHANDLE,0,0));
    HBITMAP     btm;
    HPS         hps;

    if(vlr >= (ICQSTARTUP_USER-1))
    {
       vlr = 116;
    }
    else
    {
       vlr = 100 + ((vlr * 17)/ICQSTARTUP_USER);

       if(vlr > 116)
          vlr = 116;
    }

    hps = WinGetPS(hwnd);
    btm = GpiLoadBitmap(hps,NULLHANDLE,vlr,138,128);
    WinReleasePS(hps);

    if(btm)
       WinSendDlgItemMsg(hwnd,101,SM_SETHANDLE,MPFROMLONG(btm),0);
    else if(old)
       GpiDeleteBitmap(old);

 }

 static void blinkWarning(void)
 {
    int f;

    for(f=0;f < 20;f++)
    {
       WinPostMsg(hwnd,WM_USER+11,(MPARAM) f, 0);
       DosSleep(500);
    }

    WinPostMsg(hwnd,WM_CLOSE,0,0);

    while(hwnd)
       DosSleep(100);

 }

 static void chkParm(int numpar,char **param)
 {
    const char *ptr;

    while(--numpar > 0)
    {
       param++;

       if(!strncmp(*param,"--debug",7))
       {
          ptr = strchr(*param,'=');

          if(ptr)
             ptr++;

          if(!*ptr)
             ptr = "pwicq.dbg";

          DBGMessage(ptr);

          freopen(ptr,"w",stderr);
          CHKPoint();
       }

       if(!strncmp(*param,"--log",5))
       {
          ptr = strchr(*param,'=');

          if(ptr)
             ptr++;

          if(!*ptr)
             ptr = "pwicq.log";

          DBGMessage(ptr);

          freopen(ptr,"a",stdout);
       }
    }
 }

