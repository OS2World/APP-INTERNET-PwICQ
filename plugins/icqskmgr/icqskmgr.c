/*
 * ICQSKMGR.C - pwICQ GUI Manager Plugin (Version with Skin support)
 */

 #define INCL_DOSPROCESS

 #pragma strings(readonly)

 #include <time.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stdio.h>
 #include <io.h>

 #include "icqskmgr.h"

/*---[ Prototipos ]--------------------------------------------------------------------------*/

 static int    _System executeSkin(HICQ icq,ULONG);
 static void   _System warning(HICQ, HUSER, USHORT, void *, const char *);

 static ULONG  _System loadSkin(HICQ, STATUSCALLBACK, int, char ** );
 static void   _System view(HICQ, ULONG, HMSG);
 static int    _System newMessage(HICQ, ULONG, USHORT, const char *, const char *);
 static void   _System ajustMsgDialog(HWND, HICQ, ULONG, USHORT, BOOL);
 static int    _System insertMenu(HICQ, USHORT, const char *, USHORT, MENUCALLBACK *, ULONG);
 static HWND   _System openConfig(HICQ, ULONG, USHORT);
 static BOOL   _System awayMessage(HICQ, ULONG, const char *, char *);
 static int    _System validateEditHelper(USHORT);
 static int    _System ddeRequest(HICQ, const char *, const char *, const char *);
 static int    _System popupMenu(HICQ, ULONG, USHORT, USHORT, USHORT);
 static int    _System openWindow(HICQ, HUSER, const char *, int (* _System)(HICQ, HWND, const char *, void *),void *);
 static int    _System newWithMgr(HICQ, ULONG, const MSGMGR *, HMSG);

/*---[ Constantes ]--------------------------------------------------------------------------*/

 static const int menu_ids[] = { 1003, 101, 102 };

 static const SKINMGR caps = {          sizeof(SKINMGR),
                                        0,
                                        PROJECT,
                                        loadSkin,
                                        executeSkin,
                                        NULL,
                                        warning,
                                        view,
                                        newMessage,
                                        newWithMgr,
                                        NULL,                   // msgMultDest
                                        awayMessage,
                                        popupMenu,
                                        NULL,                   // popupUserList
                                        insertMenu,
                                        openConfig,
                                        validateEditHelper,
                                        NULL,                   // Event
                                        NULL,					// Bitmap loader
                                        ddeRequest,
                                        openWindow              // int (* _System openWindow)(HICQ, HUSER, const char *, int (* _System)(HICQ, HWND, void *),void *);

                             };


 extern const SKINELEMENT       winElements[] =
 {
        STANDARD_SKIN_STUFF

        CMD_ICONBAR,    0,              0,                      "Icons",
        CMD_XBITMAP,    0,              0,                      "Logo",

        CMD_SLIDER,  1000,              0,                      "ScrollBar",

        CMD_BUTTON,  1001,              0,                      "Up",
        CMD_BUTTON,  1002,              0,                      "Down",

        CMD_FONT,    1003,              0,                      "UserListFont",
        CMD_FONT,    101,               0,                      "ModeButtonFont",
        CMD_FONT,    102,               0,                      "SystemButtonFont",
        CMD_FONT,    103,               0,                      "SysMessageFont",

        CMD_LISTBOX, 1003,              WMSKS_CACHEDLB,         "UserList",

        CMD_PALLETE, SKC_USER1,         0,                      "OfflineColor",
        CMD_PALLETE, SKC_USER3,         0,                      "OnlineColor",
        CMD_PALLETE, SKC_USER4,         0,                      "SysMessageColor",
        CMD_PALLETE, SKC_USER5,         0,                      "OfflineWaitingColor",
        CMD_PALLETE, SKC_USER6,         0,                      "OnlineWaitingColor",

        CMD_BUTTON,  101,               WS_VISIBLE,             "ModeButton",
        CMD_BUTTON,  102,               WS_VISIBLE,             "SysButton",
        CMD_BUTTON,  103,               WS_VISIBLE,             "SysMessage",

        0,             0,               0,                      NULL
 };

/*---[ Statics ]-----------------------------------------------------------------------------*/

 static HWND    mainFrame           = NULLHANDLE;

 extern HWND    mainWindow          = NULLHANDLE;
 extern HMODULE module              = NULLHANDLE;

/*---[ Implementacao ]-----------------------------------------------------------------------*/

 int EXPENTRY icqskmgr_Configure(HICQ icq, HMODULE hmod)
 {
    char        key[20];
    char        *ptr;

#ifdef DEBUG
    icqWriteSysLog(icq,PROJECT,"Loading pwICQ Skin manager " __DATE__ " " __TIME__ " (Debug Version)");
#else
    icqWriteSysLog(icq,PROJECT,"Loading pwICQ Skin manager Build " BUILD);
#endif

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"Extensive logging enabled");
#endif

    CHKPoint();

    if(icqQueryUserDataBlockLength() != sizeof(ICQUSER))
    {
       CHKPoint();
       icqWriteSysLog(icq,PROJECT,"Can't start skin manager due to invalid pwICQ core version");
       return -1;
    }

    if(!icqRegisterSkinManager(icq,&caps))
    {
       CHKPoint();
       icqWriteSysLog(icq,PROJECT,"Can't register skin manager");
       return -2;
    }

    icqSetSkinDataBlock(icq,0);

    module = hmod;

    key[19] = 0;
    strncpy(key,sknQueryIDString(),19);
    for(ptr = key;*ptr && *ptr != ':';ptr++);
    *ptr = 0;

    for(ptr = (char *) sknQueryIDString();*ptr && *ptr != ':';ptr++);

    if(*ptr)
       icqWriteSysLog(icq,key,ptr+1);

    return 0;
 }

 void EXPENTRY icqskmgr_Timer(HICQ icq, void *lixo)
 {
    if(mainWindow)
       WinPostMsg(mainWindow,WM_USER+12,0,0);
 }

 void EXPENTRY icqskmgr_Event(HICQ icq, void *dunno, ULONG uin, UCHAR id, USHORT event, ULONG parm)
 {
    if(!mainWindow)
       return;

    if(id == 'S')
       WinPostMsg(mainWindow,WMICQ_SYSTEMEVENT,MPFROMSHORT(event),0);
    else if(id == 'U')
       WinPostMsg(mainWindow,WMICQ_USEREVENT,MPFROMSHORT(event),MPFROMLONG(uin));
 }

 int _System newWithMgr(HICQ icq, ULONG uin, const MSGMGR *mgr, HMSG msg)
 {

    DBGTracex(mgr);
    DBGTracex(msg);

    icqOpenMsgWindow(icq, uin, mgr->type, msg, mgr, NULL, NULL);
    return 0;
 }

 void _System view(HICQ icq, ULONG uin, HMSG m)
 {
    icqOpenMsgWindow(icq,uin,0,m,0,0,0);
 }

 char *querySkinName(HICQ icq, char *skin)
 {
    char        *ptr;
    int         sz;
    char        buffer[0x0100];

    icqLoadString(icq, "skin", "", skin, 0xFF);

    if(!*skin || access(skin,0))
    {
       icqLoadString(icq, "skinpath", "", skin, 0xFF);

       if(!*skin)
          icqQueryPath(icq,"skins\\",skin,0xFF);

       sz  = strlen(skin);
       ptr = skin + sz;
       sz  = 0xFF - sz;

       DBGMessage(skin);
       icqLoadString(icq, "skin", "default.skn", ptr, sz);

    }

    DBGMessage(skin);

    if(access(skin,0))
    {
       strcpy(buffer,"Can't find ");
       strncat(buffer,skin,0xFF);
       icqWriteSysLog(icq,PROJECT,buffer);
       return NULL;
    }
    strcpy(buffer,"Loading ");
    strncat(buffer,skin,0xFF);
    icqWriteSysLog(icq,PROJECT,buffer);

    DBGMessage(buffer);
    DBGMessage(skin);

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"Skin definitions complete");
#endif

    return skin;
 }

 ULONG _System loadSkin(HICQ icq, STATUSCALLBACK *startupStage, int numpar, char **param )
 {
    HAB         hab     = icqQueryAnchorBlock(icq);
    char        skin[0x0100];

    DBGMessage("Iniciando skin manager");
    icqWriteSysLog(icq,PROJECT,"Starting Skin Manager");

    startupStage(icq,ICQSTARTUP_STARTINGSKIN);

    if(sknInitialize(hab) && sknRegisterClass(hab,"pwICQ2Skin",sknProcedure,sizeof(void *)))
    {
       icqStartWindowTools(icq,hab);

       startupStage(icq,ICQSTARTUP_LOADINGSKIN);

       DBGTrace(icqQueryLogonStatus(icq));

       if(!icqQueryLogonStatus(icq))
       {
          startupStage(icq,-1000);
          icqLogonDialog(icq);
          if(!icqQueryLogonStatus(icq))
             return -2;
          startupStage(icq,-1001);
          startupStage(icq,ICQSTARTUP_LOADINGSKIN);
       }

       mainWindow = sknCreateWindow("pwICQ2Skin",querySkinName(icq, skin),&mainFrame,"pwICQ",module,FCF_TASKLIST|FCF_NOMOVEWITHOWNER|FCF_ICON|FCF_ACCELTABLE|FCF_NOBYTEALIGN,1);
       DBGTracex(mainWindow);

       if(mainWindow)
       {
          icqRegisterConfigurationWindow(icq,hab);

          WinSendMsg(mainWindow,WM_USER+10,MPFROMP(icq),MPFROMLONG(mainFrame));
          startupStage(icq,ICQSTARTUP_SKINLOADED);
          if(icqInitialize(icq,startupStage))
             WinPostMsg(mainWindow,WM_CLOSE,0,0);
          WinPostMsg(mainWindow,WM_USER+19,0,0);
          return (ULONG) mainWindow;
       }
    }
    return 0;
 }

#ifdef DEBUG

 int _System testMenu(HICQ icq, ULONG uin, USHORT type, ULONG id)
 {
    DBGMessage("Menu callback");
    DBGTracex(type);
    DBGTrace(uin);

    CHKPoint();
    icqNewMessageWithManager(icq, uin, icqQueryMessageManager(icq, MSG_NORMAL));
    CHKPoint();

    return 0;
 }

#endif

 int _System executeSkin(HICQ icq, ULONG parm)
 {

    if(parm != mainWindow)
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected parameter received");
       return -1;
    }

    icqRestoreWindow(mainFrame, icq, 0, "pos:MainWindow", 150, 375);

    icqExecuteWindowLoop(icq,parm);

    icqStoreWindow(mainFrame, icq, 0, "pos:MainWindow");

    icqStopWindowTools(icq);
    icqTerminate(icq);

    DBGMessage("Skin terminado");

    icqWriteSysLog(icq,PROJECT,"Skin Manager stopped");
    DBGMessage("Skin manager terminado");

    return 0;
 }

 static void _System warning(HICQ icq, HUSER usr, USHORT id, void *x1, const char *x2)
 {
    ULONG uin   = 0;

    if(usr)
       uin = usr->uin;

    if(mainWindow)
       WinPostMsg(mainWindow,WM_USER+15,MPFROMLONG(uin),MPFROMSHORT(id));

 }

 int _System newMessage(HICQ icq, ULONG uin, USHORT type, const char *p1, const char *p2)
 {
#ifdef DEBUG

    DBGTracex(p1);
    if(p1)
       DBGMessage(p1);

    DBGTracex(p2);
    if(p2)
       DBGMessage(p2);

#endif

    return (int) icqOpenMsgWindow(icq, uin, type, NULL, NULL, p1, p2);

 }

 static HWND _System openConfig(HICQ icq, ULONG uin, USHORT type)
 {
    return icqOpenDefaultConfigurationWindow(icq,uin,type);
 }

// static int    _System insertMenu(HICQ, USHORT, const char *, USHORT, MENUCALLBACK *, ULONG);

 static int _System insertMenu(HICQ icq, USHORT menu, const char *text, USHORT icon, MENUCALLBACK *cbk, ULONG parm)
 {
    MNUHANDLER mnu;

    if(menu > 2)
    {
       icqWriteSysLog(icq,PROJECT,"Invalid menu ID when inserting option");
       return -1;
    }

    DBGMessage("Pediu insercao manual de menu");
    DBGTrace(menu);
    DBGTracex(WinWindowFromID(mainWindow,menu_ids[menu]));

    memset(&mnu,0,sizeof(MNUHANDLER));

    mnu.text    = text;
    mnu.after   = 9999;
    mnu.handler = (ULONG) cbk;
    mnu.itemID  = icon;

    sknInsertMenuHandler(mainWindow,menu_ids[menu],&mnu);
    return 0;
 }

 static BOOL   _System awayMessage(HICQ icq, ULONG uin, const char *title, char *txt)
 {
    struct awayDialogParameters p;
    int                         f;

    CHKPoint();

    p.sz        = sizeof(struct awayDialogParameters);
    p.flag      = 0xFF;
    p.uin       = uin;
    p.title     = title;
    p.txt       = txt;
    p.icq       = icq;
    p.usr       = icqQueryUserHandle(icq,uin);

    WinPostMsg(mainWindow,WM_USER+18,MPFROMP(&p),0);

    for(f=0;p.flag && f<200;f++)
       DosSleep(10);

    if(p.flag)
    {
       icqSysLog(icq,"Timeout passing information to the main window");
       icqAbend(icq,0);
       return FALSE;
    }

    return TRUE;
 }

 static int _System validateEditHelper(USHORT sz)
 {
    if(sz != sizeof(DIALOGCALLS))
       return 2;
    return icqValidateDefaultEditHelper(sz);
 }

 static int _System ddeRequest(HICQ icq, const char *application, const char *topic, const char *request)
 {
    icqWriteSysLog(icq,PROJECT,"Inactive DDE Requests");
    icqWriteSysLog(icq,PROJECT,request);
    return -1;
 }

 static int _System popupMenu(HICQ icq, ULONG uin, USHORT menu, USHORT x, USHORT y)
 {
    DBGMessage("Popup menu");

    if(!mainWindow)
       return -1;

    if(menu > 2)
    {
       icqWriteSysLog(icq,PROJECT,"Invalid popup menu request");
       return -2;
    }

    WinPostMsg(mainWindow,WM_USER+21,MPFROM2SHORT(menu_ids[menu],0),MPFROMLONG(uin));

    return 0;
 }

 static int _System openWindow(HICQ icq, HUSER usr, const char *id, int (* _System callback)(HICQ, HWND, const char *, void *),void *arg)
 {
    struct openWindowParameters parm = {        sizeof(struct openWindowParameters),
                                                TRUE,
                                                usr,
                                                id,
                                                callback,
                                                arg
                                        };
    int f;


    if(!mainWindow)
       return -1;

    DBGTracex(mainWindow);

    CHKPoint();
    WinPostMsg(mainWindow,WM_USER+22,(MPARAM) &parm,0);

    /* Espera a janela pegar o parametro */

    for(f=0;f<100 && parm.flag;f++)
       DosSleep(10);

    if(parm.flag)
    {
       icqWriteSysLog(icq,PROJECT,"Error passing message to the mainwindow");
       icqAbend(icq,0);
    }

    return 0;
 }

 MRESULT EXPENTRY icqskmgr_sendMsg(ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    if(mainWindow)
       return WinSendMsg(mainWindow,msg,mp1,mp2);
    return 0;
 }

