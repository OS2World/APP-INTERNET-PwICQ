/*
 * os2dde.c - OS/2 dde information
 */

 #pragma strings(readonly)

 #define INCL_WIN

 #include <string.h>
 #include <pwicqgui.h>

/*---[ Defines ]------------------------------------------------------------------------------------------*/


/*---[ Button control data structure ]--------------------------------------------------------------------*/


/*---[ Statics ]------------------------------------------------------------------------------------------*/

 static BOOL busy                                = FALSE;
 static HWND ddeServer                           = NULLHANDLE;
 static void *callBackParam                      = NULL;
 static BOOL ( * _System callBack)(HWND, void *) = NULL;

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/


/*---[ Implementation ]-----------------------------------------------------------------------------------*/

// mainWindow

 HWND EXPENTRY icqDDEQueryServer(HICQ icq, const char *app, const char *topic)
 {
    SKINDATA    *cfg = icqskin_getDataBlock(icq);
    CONVCONTEXT context;

    if(!(cfg && cfg->mainWindow))
    {
       icqWriteSysLog(icq,PROJECT,"Invalid DDE Request");
       return 0;
    }

    if(busy)
    {
       icqWriteSysLog(icq,PROJECT,"DDE service is busy");
       return 0;
    }

    busy      = TRUE;
    ddeServer = 0;

    memset(&context,0,sizeof(CONVCONTEXT));
    context.cb = sizeof(CONVCONTEXT);

    DBGMessage(app);
    DBGMessage(topic);

    WinDdeInitiate(cfg->mainWindow, (PSZ) app, (PSZ) topic, &context);

    DBGTracex(ddeServer);

    busy = FALSE;

    return ddeServer;

 }

 void EXPENTRY icqDDEData(HWND hwnd, PDDESTRUCT dde)
 {
    CHKPoint();
 }

 BOOL EXPENTRY icqDDEInitiateAck(HWND hwnd, HWND server, PDDEINIT ddeInit)
 {
    ddeServer = server;

    if(callBack)
       return callBack(server,callBackParam);

    return FALSE;
 }

 void icqskin_ddeData(HWND hwnd, PDDESTRUCT dde)
 {
    CHKPoint();
 }

 static BOOL _System SystrayCallBack(HWND hwnd, void *icq)
 {
#ifdef EXTENDED_LOG
    char     logBuffer[0x0100];
#endif

    SKINDATA *cfg = icqskin_getDataBlock((HICQ) icq);

    cfg->TrayServer = hwnd;

    if(cfg->TrayServer)
    {
#ifdef EXTENDED_LOG
       sprintf(logBuffer,"System Tray found at window #%08lx",cfg->TrayServer);
       icqWriteSysLog((HICQ) icq,PROJECT,logBuffer);
#endif
       WinPostMsg(cfg->TrayServer,WM_TRAYADDME,(MPARAM) cfg->mainWindow,0);
    }

    return TRUE;

 }

 HWND EXPENTRY icqQuerySystray(HICQ icq)
 {
    SKINDATA *cfg = icqskin_getDataBlock((HICQ) icq);

    if(cfg->TrayServer)
       return cfg->TrayServer;

    callBackParam = (void *) icq;
    callBack      = SystrayCallBack;

    icqDDEQueryServer(icq, "SystrayServer", "TRAY");

    callBackParam = 0;
    callBack      = 0;

    return cfg->TrayServer;
 }

 int EXPENTRY icqDisableSysTray(HICQ icq)
 {
   SKINDATA *cfg = icqskin_getDataBlock((HICQ) icq);

   cfg->TrayServer = 0;

#ifdef EXTENDED_LOG
   icqWriteSysLog(icq,PROJECT,"System tray disabled");
#endif

   return 0;
 }


