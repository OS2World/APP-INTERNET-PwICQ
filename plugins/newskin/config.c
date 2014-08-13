/*
 * config.c - Common configuration window processing
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_WIN
#endif

 #include <malloc.h>
 #include <string.h>
 #include <time.h>
 #include <pwicqgui.h>

 #ifdef __OS2__
   #define CHILD_WINDOW(h,i) WinWindowFromID(h,DLG_ELEMENT_ID+i)
 #else
   #define CHILD_WINDOW(h,i) icqskin_getDlgItemHandle(h,i)
 #endif

/*---[ Structures ]-----------------------------------------------------------*/

 #pragma pack(1)

 typedef struct cfgedithelper
 {
    USHORT     sz;
    ULONG      build;


 } CFGEDITHELPER;


 #pragma pack()

/*---[ Constants ]------------------------------------------------------------*/

 static const USHORT userBox[] =   {	MSGID_NICK,
   										MSGID_NICKENTRY,
   										MSGID_NAME,
   										MSGID_NAMEENTRY,
   										MSGID_USERMODE,
	                                    MSGID_MESSAGE,
	                                    MSGID_ABOUTBUTTON,
	                                    MSGID_USERMODE,
	                                    MSGID_NAMEBOX
   							       };


 static const USHORT userSections[] = { CFGWIN_INFO,
 										CFGWIN_EVENTS,
 										CFGWIN_OPTIONS
 									  };
 									
 static const USHORT sysSections[]  = { CFGWIN_NETWORK,
 										CFGWIN_EVENTS,
 										CFGWIN_OPTIONS,
										CFGWIN_USERS,
										CFGWIN_SECURITY
								      };

 const char *icqskin_configsections[CFGWIN_USER] =
                                      { "*",
										"Network",
										"Mode",
										"Multimedia",
										"Users",
										"Options",
										"Security",
										"Miscelaneous",
										"Advanced",
										"About",
										"Information",
										"Events"
									  };


/*---[ Prototipes ]-----------------------------------------------------------*/

 static void ICQCALLBACK closeButton(hWindow, hWindow);
 static void ICQCALLBACK saveButton(hWindow, hWindow);
 static void ICQCALLBACK reloadButton(hWindow, hWindow);
 static void ICQCALLBACK refreshButton(hWindow, hWindow);
 static void ICQCALLBACK sendMessage(hWindow, hWindow);
 static void ICQCALLBACK aboutUser(hWindow, hWindow);

/*---[ Implementation ]-------------------------------------------------------*/

 hWindow _System icqskin_openConfig(HICQ icq, ULONG uin, USHORT type)
 {
    char            buffer[0x0100];
    SKINDATA        *skn           = icqskin_getDataBlock(icq);
    ICQCONFIGDIALOG *cfg;
    const char      *name          = "ConfigWindow";
    hWindow         hwnd;
    int             f;
    HUSER           usr;

    for(cfg = skn->firstCfgWin;cfg;cfg = cfg->down)
    {
       if(cfg->uin == uin && cfg->type == type)
       {
          icqskin_showFrame(cfg->hwnd);
          return 0;
       }
    }

    DBGMessage("Abrir dialogo de configuracao");

    hwnd = icqskin_createConfigWindow(icq,uin,icqgui_cfgUserBoxButtonTableElements,icqgui_cfgUserBoxButtonTable);

    if(!hwnd)
       return 0;

    if(!type)
    {
       usr = icqQueryUserHandle(icq,uin);
       if(!usr)
          icqAddUserInCache(icq, uin);
    }

    /* Configure chained list */

    cfg       = icqskin_getWindowDataBlock(hwnd);
    cfg->hwnd = hwnd;
    cfg->type = type;
    cfg->up   = skn->lastCfgWin;
	
    if(cfg->up)
       cfg->up->down = cfg;
    else
       skn->firstCfgWin = cfg;

    skn->lastCfgWin = cfg;

    /* Configure window */
    icqAddGuiEventListener(icq, (void *) icqskin_cfgEventProcessor, (HWND) hwnd);

    icqskin_setMsgButtonCallback(hwnd, MSGID_CLOSEBUTTON, closeButton);
    icqskin_setMsgButtonCallback(hwnd, MSGID_SAVE,        saveButton);
    icqskin_setMsgButtonCallback(hwnd, MSGID_RELOAD,      reloadButton);
    icqskin_setMsgButtonCallback(hwnd, MSGID_REFRESH,     refreshButton);

    DBGTrace(type);

    switch(type)
    {
    case ICQCONFIGW_USER: /* User configuration Window */

       DBGMessage("Opening user configuration Window");
       icqWriteSysLog(icq,PROJECT,"Opening user configuration Window");
       name = "UserConfig";

       sprintf(buffer,"%s's Information",icqQueryUserNick(icqQueryUserHandle(icq,uin)));
	   DBGMessage(buffer);
	
       icqskin_setFrameTitle(hwnd,buffer);
       icqskin_setFrameIcon(hwnd, skn, ICQICON_USERCONFIG);
	
	   DBGTracex(CHILD_WINDOW(hwnd,MSGID_USERMODE));
	   icqskin_setDlgItemIcon(hwnd,MSGID_USERMODE,ICQICON_OFFLINE);
	
       CHKPoint();

   	   icqskin_setMsgButtonCallback(hwnd, MSGID_MESSAGE,         sendMessage);
       CHKPoint();

       icqskin_setMsgButtonCallback(hwnd, MSGID_ABOUTBUTTON,     aboutUser);
       CHKPoint();

       for(f=0;f<sizeof(userSections)/sizeof(USHORT);f++)
          icqskin_insertConfigPage(hwnd, CFGWIN_ROOT, 0, userSections[f], icqskin_configsections[userSections[f]]);

       CHKPoint();
	
       break;

    case ICQCONFIGW_SYSTEM:	/* Main configuration Window */

       DBGMessage("Opening main configuration Window");
       icqWriteSysLog(icq,PROJECT,"Opening main configuration Window");
       name = "SysConfig";

       icqLoadString(icq,"NickName","John Doe",buffer+0x80,79);
       sprintf(buffer,"%s's pwICQ Configuration",buffer+0x80);
	   DBGMessage(buffer);
	
       icqskin_setFrameTitle(hwnd,buffer);
       icqskin_setFrameIcon(hwnd, skn, ICQICON_CONFIGURE);

       for(f=0;f < (sizeof(userBox)/sizeof(USHORT)); f++)
          icqskin_hide(CHILD_WINDOW(hwnd,userBox[f]));

       for(f=0;f<sizeof(sysSections)/sizeof(USHORT);f++)
          icqskin_insertConfigPage(hwnd, CFGWIN_ROOT, 0, sysSections[f], icqskin_configsections[sysSections[f]]);

//#ifdef DEBUG
//       icqstatic_setText(CHILD_WINDOW(hwnd,MSGID_TITLE),"About pwICQ");
//#endif

       break;

    default:
       DBGMessage("******** UNEXPECTED CONFIG WINDOW TYPE REQUEST");
       icqWriteSysLog(icq,PROJECT,"Unexpected configuration window type request");
       break;

    }

	DBGMessage(name);
    icqskin_setFrameName(hwnd, name);

#ifdef SKINNED_GUI
       icqskin_loadSkin(hwnd,"Config",skn->skin);
#endif

    /* Call plugin for pages */
	CHKPoint();
    icqInsertConfigPage(icq, uin, type, hwnd, &icqskin_dialogCalls, buffer);
	CHKPoint();

    /* Send event to the window */

    if(icqIsOnline(icq))
       icqskin_cfgEventProcessor(icq, uin, 'S', ICQEVENT_ONLINE,  0, hwnd);
    else
       icqskin_cfgEventProcessor(icq, uin, 'S', ICQEVENT_OFFLINE, 0, hwnd);

    CHKPoint();
    icqskin_cfgEventProcessor(icq, uin, 'U', ICQEVENT_UPDATED, 0,hwnd);
    icqskin_cfgEventProcessor(icq, uin, 'U', ICQEVENT_CHANGED, 0,hwnd);
    CHKPoint();

    icqconfig_setWindowType(hwnd,type);
    	
#if defined(GTK1) || defined(GTK2)
    gtk_widget_show(GTK_WIDGET(hwnd));
#endif	
	
	DBGTracex(hwnd);

    icqskin_restoreFrame(hwnd,name,578,475);
    icqconfig_initialize(hwnd);

    return hwnd;
 }

 void icqskin_destroyCfgWindow(hWindow hwnd)
 {
    ICQCONFIGDIALOG *cfg =  icqskin_getWindowDataBlock(hwnd);
    SKINDATA        *skn;

    if(!cfg)
       return;

    DBGMessage("Configuration window closed");
    icqRemoveGuiEventListener(cfg->fr.icq, (void *) icqskin_cfgEventProcessor, (HWND) hwnd);

    skn = icqskin_getDataBlock(cfg->fr.icq);

    if(cfg->up)
       cfg->up->down = cfg->down;
    else
       skn->firstCfgWin = cfg->down;

    if(cfg->down)
       cfg->down->up = cfg->up;
    else
       skn->lastCfgWin = cfg->up;

 }

 static void ICQCALLBACK closeButton(hWindow btn, hWindow hwnd)
 {
    icqskin_closeWindow(hwnd);
 }

 static void ICQCALLBACK saveButton(hWindow btn, hWindow hwnd)
 {
    CHKPoint();
    icqconfig_save(hwnd);
 }

 static void ICQCALLBACK reloadButton(hWindow btn, hWindow hwnd)
 {
    icqconfig_load(hwnd);
 }

 static void ICQCALLBACK refreshButton(hWindow btn, hWindow hwnd)
 {
    CHKPoint();
 }

 static void ICQCALLBACK sendMessage(hWindow btn, hWindow hwnd)
 {
	ICQCONFIGDIALOG *cfg = icqskin_getWindowDataBlock(hwnd);
    icqNewUserMessage(cfg->fr.icq, cfg->uin, 0, 0, 0);
 }

 static void ICQCALLBACK aboutUser(hWindow btn, hWindow hwnd)
 {
	ICQCONFIGDIALOG *cfg = icqskin_getWindowDataBlock(hwnd);
	if( !(cfg&&cfg->uin&&cfg->fr.icq) )
	   return;
    icqskin_openAboutUser(cfg->fr.icq, cfg->uin, 0, 0);
 }


