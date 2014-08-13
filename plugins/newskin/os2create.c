/*
 * os2create.c - Create OS2 Windows
 */

 #pragma strings(readonly)

 #define INCL_WIN
 #include <string.h>
 #include <pwicqgui.h>

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static int configWindow(SKINDATA *);

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

#ifdef STANDARD_GUI

 int _System icqskin_CreateStandardWindow(HICQ icq, SKINDATA *cfg)
 {
    ULONG   ulFrameFlags = FCF_TITLEBAR|FCF_SIZEBORDER|FCF_MINMAX|FCF_TASKLIST|FCF_SYSMENU|FCF_NOMOVEWITHOWNER;
//    |FCF_ICON
//    |FCF_ACCELTABLE;

    cfg->frame = WinCreateStdWindow(   HWND_DESKTOP,
    								   0,
    								   &ulFrameFlags,
    								   (PSZ) icqFrameWindowClass,
    								   (PSZ) "pwICQ",
    								   WS_VISIBLE,
    								   (HMODULE) module,
    								   1000,
    								   &cfg->mainWindow
    							   );

    DBGTracex(cfg->frame);
    DBGTracex(WinQueryWindow(cfg->mainWindow,QW_PARENT));

    if(configWindow(cfg))
       return -1;

    CHKPoint();

    if(icqskin_loadIcons(cfg,"icons.gif"))
    {
       icqskin_closeWindow(cfg->mainWindow);
       return -1;
    }

    return 0;

 }

#endif

#ifdef SKINNED_GUI

 int icqskin_LoadSkinFromFile(HICQ icq, SKINDATA *cfg, SKINFILESECTION *sk)
 {
    /* Install skin file */
    char       buffer[0x0100];
    const char *ptr;
    int        f;
    const struct icqButtonTable *btn = icqgui_MainButtonTable;

    ptr = icqskin_getSkinFileEntry(sk, "Description", "Name");

    if(ptr)
    {
       strcpy(buffer,"Loading ");
       strncat(buffer,ptr,0xFF);

       DBGMessage(buffer);
       icqWriteSysLog(icq,PROJECT,buffer);
    }

    f = icqskin_loadImage(icq, icqskin_getSkinFileEntry(sk,"Main","Icons"), &cfg->iconImage, &cfg->iconMasc);

    if(f < 1)
       icqskin_loadIcons(cfg, icqskin_getSkinFileEntry(sk,"Main","Icons"));
    else
       cfg->iconSize = f;


    icqskin_loadSkin(cfg->mainWindow,"Main",sk);
    icqskin_loadSkin(icqskin_getDlgItemHandle(cfg->mainWindow,ID_USERLIST), "Main",sk);
    icqskin_loadSkin(icqskin_getDlgItemHandle(cfg->mainWindow,ID_SCROLLBAR),"Main",sk);

    for(f=0;f < icqgui_MainButtonTableElements;f++)
    {
       DBGTrace(f);
       icqskin_loadSkin(icqskin_getDlgItemHandle(cfg->mainWindow,btn->id),"Main",sk);
       btn++;
    }

    for(f=0;f<ICQMNU_COUNTER;f++)
       icqskin_loadSkin(cfg->menu[f],"Menu.Main",sk);

    return 0;
 }

 int _System icqskin_CreateSkinnedWindow(HICQ icq, SKINDATA *cfg)
 {
    ULONG           ulFrameFlags = FCF_TASKLIST|FCF_NOMOVEWITHOWNER|FCF_NOBYTEALIGN;

    cfg->frame = WinCreateStdWindow(   HWND_DESKTOP,
    								   0,
    								   &ulFrameFlags,
    								   (PSZ) icqFrameWindowClass,
    								   (PSZ) "pwICQ",
    								   WS_VISIBLE,
    								   (HMODULE) module,
    								   1000,
    								   &cfg->mainWindow
    							   );

    DBGTracex(cfg->frame);
    DBGTracex(WinQueryWindow(cfg->mainWindow,QW_PARENT));

    if(configWindow(cfg))
    {
       CHKPoint();
       return -1;
    }

    icqskin_openSkinFile(cfg);
    icqskin_LoadSkinFromFile(icq,cfg,cfg->skin);

    DBGTracex(icqDDEQueryServer(icq,"MOZILLA", "WWW_OpenURL"));

    return 0;
 }

#endif

 static int configWindow(SKINDATA *cfg)
 {
    static const char *menuName[ICQMNU_COUNTER] = { STANDARD_MENU_NAMES };
    HWND uList;
    HWND scBar;
    char buffer[0x0100];
    int  f;

#ifdef DEBUG
    icqSaveString(cfg->icq, "pos:MainWindow", NULL);
#endif

    if(!(cfg->frame||cfg->mainWindow))
       return -1;

    WinSetWindowPtr(cfg->frame,QWL_USER,WinQueryWindowPtr(cfg->mainWindow,0));

    icqskin_setICQHandle(cfg->mainWindow,cfg->icq);
    icqskin_setFrameName(cfg->mainWindow,"mainwindow");

    icqskin_setButtons(cfg->mainWindow,icqgui_MainButtonTable,icqgui_MainButtonTableElements);

    uList = icqSkin_createStandardUserList(cfg->icq, cfg->mainWindow, ID_USERLIST);
#ifdef STANDARD_GUI
    icqskin_setButtonSize(uList,3,78,-6,-78);
#else
    icqskin_setButtonSize(uList,3,78,-18,-82);
#endif

    DBGTracex(cfg->icq);
    icqLoadString(cfg->icq, "UserListFont", "8.Helv.Bold", buffer, 0xFF);
    DBGMessage(buffer);
    icqskin_setButtonFont(uList,buffer);

    icqLoadString(cfg->icq, "ButtonFont", "8.Helv", buffer, 0xFF);
    DBGMessage(buffer);

    for(f=0;f<ICQMNU_COUNTER;f++)
    {
       cfg->menu[f] = icqskin_CreateMenu(cfg->icq, f, cfg);
       icqskin_setButtonFont(cfg->menu[f],buffer);
       icqskin_setButtonName(cfg->menu[f],menuName[f]);
    }

    scBar = icqSkin_createStandardScrollBar(cfg->icq, cfg->mainWindow, ID_SCROLLBAR);
    icqskin_setButtonSize(scBar,-4,78,9,-82);

#ifdef STANDARD_GUI
    WinShowWindow(scBar,FALSE);
#endif

    icqskin_connectScrollBar(uList,scBar);

    return 0;
 }


