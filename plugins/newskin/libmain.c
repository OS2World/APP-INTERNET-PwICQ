/*
 * LIBMAIN.C - OS2Win compatible entry points
 */

#ifdef __OS2__
 #pragma strings(readonly)
 int _CRT_init(void);
#endif

 #include <string.h>
 #include <malloc.h>
 #include <stdio.h>

 #include <pwicqgui.h>

/*---[ Data block ]---------------------------------------------------------------------------------------*/

 SKINDATA icqskin_dataBlock = { 0 };

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 unsigned long _System _DLL_InitTerm(unsigned long hModule, unsigned long ulFlag)
 {
    switch (ulFlag)
    {
    case 0 :
       if (_CRT_init() == -1)
          return 0UL;
       memset(&icqskin_dataBlock,0,sizeof(icqskin_dataBlock));
       break;

    case 1 :
       break;

    default  :
       return 0UL;
    }

    module = hModule;
    return 1UL;
 }

 APIRET EXPENTRY icqStartWindowTools(HICQ icq, HAB hab)
 {
    if(icqQueryUserDataBlockLength() != sizeof(ICQUSER))
    {
       icqWriteSysLog(icq,PROJECT,"Invalid core version detected");
       return -1;
    }

    icqWriteSysLog(icq,PROJECT,"OS/2 PM layer version " ICQGUI_INCLUDED " Build " __DATE__ " " __TIME__ " by " USER "@" HOSTNAME);

    /* Register Window Classes */
    icqskin_registerWindowClass(icq);

    memset(&icqskin_dataBlock,0,sizeof(SKINDATA));
    icqskin_dataBlock.sz  = sizeof(SKINDATA);
    icqskin_dataBlock.icq = icq;

#ifdef __OS2__
    icqskin_dataBlock.iconImage = NO_IMAGE;
    icqskin_dataBlock.iconMasc  = NO_IMAGE;
#endif

    icqskin_loadIcons(&icqskin_dataBlock,"icons.gif");

    return 0;
 }

 int EXPENTRY icqUpdateSysTray(HICQ icq)
 {
    HWND     sysTray = icqQuerySystray(icq);
    SKINDATA *skn    = icqskin_getDataBlock(icq);

    if(sysTray)
    {
       WinPostMsg(sysTray,WM_TRAYICON,(MPARAM) skn->mainWindow,0);
       return 0;
    }

    return -1;
 }

 APIRET EXPENTRY icqExecuteWindowLoop(HICQ icq, HWND hwnd)
 {
    QMSG        qmsg;
    HAB         hab     = icqQueryAnchorBlock(icq);

    icqskin_dataBlock.mainWindow = hwnd;

    DBGTracex(icqDDEQueryServer(icq,"MOZILLA", "WWW_OpenURL"));
    icqQuerySystray(icq);

    while(WinGetMsg(hab,&qmsg,0,0,0))
       WinDispatchMsg(hab,&qmsg);

    return 0;
 }


 APIRET EXPENTRY icqStopWindowTools(HICQ icq)
 {
    CHKPoint();

    return 0;
 }

 void  EXPENTRY icqDefaultMsgPreProcessor(HICQ icq, HUSER usr, ULONG uin, struct icqmsg *msg, BOOL convert)
 {

 }

 void EXPENTRY icqAjustDefaultMsgDialog(HWND hwnd, HICQ icq, ULONG flags, USHORT type, BOOL out)
 {

 }

 const MSGMGR * EXPENTRY icqQueryDefaultMsgMgr(HICQ icq)
 {

    return NULL;
 }

 void EXPENTRY icqAjustFrameTitle(HICQ icq, HWND hwnd, const char *txt, HPOINTER *ptrMode)
 {
    icqWriteSysLog(icq,PROJECT,"Deprecated call to icqAjustFrameTitle()");
 }
 static void getTitleByMode(HICQ icq, ULONG sel, char *buffer)
 {
    int                 pos;
    const ICQMODETABLE  *mode;

    /* Ajusta pelo modo atual */

    pos = 0;
    for(mode = icqQueryModeTable(icq);mode && mode->descr;mode++)
    {
       if(mode->mode == sel)
       {
          /* Achei o modo */
          strncat(buffer," - ",0xFF);
          strncat(buffer,mode->descr,0xFF);
          return;
       }
       pos++;
    }
 }

 static USHORT getModeIcon(HICQ icq, ULONG sel)
 {
    const ICQMODETABLE  *mode;

    for(mode = icqQueryModeTable(icq);mode && mode->descr; mode++)
    {
       if(mode->mode == sel)
          return mode->icon;
    }
    return ICQICON_ONLINE;
 }

 void EXPENTRY icqAjustTaskList(HICQ icq, HWND hwnd, const char *txt)
 {
    ULONG               sel             = icqQueryOnlineMode(icq);
    HMSG                msg             = NULL;
    ULONG               uin;
    int                 pos;
    int                 task            = icqLoadValue(icq,"TaskTITLE",0);
    char                *buffer         = NULL;

    CHKPoint();

    icqQueryFirstMessage(icq, &uin, &msg);

    DBGTracex(txt);

    buffer = malloc(1024);

    if(buffer)
    {
       if(task && txt)
       {
          CHKPoint();
          sprintf(buffer,"%lu - %s",icqQueryUIN(icq),txt);
       }
       else
       {
          sprintf(buffer,"%lu",icqQueryUIN(icq));

          switch(task)
          {
          case 0:                  // No change
             break;

          case 1:
             getTitleByMode(icq,sel,buffer);
             break;

          case 2:
             if(msg)
             {
                strncat(buffer," - ",0xFF);
                pos = strlen(buffer);
                CHKPoint();
                icqQueryMessageTitle(icq, uin, FALSE, msg, buffer + pos, 0xFF - pos);
                CHKPoint();
             }
             else
             {
                getTitleByMode(icq,sel,buffer);
             }
             break;
          }
       }
       DBGMessage(buffer);
       DBGTrace(strlen(buffer));

       WinSendMsg(hwnd,WM_USER+1040,(MPARAM) buffer, 0);

       CHKPoint();
       free(buffer);
    }
    else
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when changing window title");
    }

    switch(icqLoadValue(icq,"TaskICON",0))
    {
    case 0:		// No change
       break;

    case 1:		// By mode
       WinPostMsg(hwnd,WM_USER+1039,(MPARAM) getModeIcon(icq, sel), 0);
       break;

    case 2:		// By message
       if(msg && msg->mgr)
          WinPostMsg(hwnd,WM_USER+1039,(MPARAM) msg->mgr->icon[0], 0);
       else
          WinPostMsg(hwnd,WM_USER+1039,(MPARAM) getModeIcon(icq, sel), 0);
       break;

    }

    icqUpdateSysTray(icq);

    CHKPoint();
 }

 int EXPENTRY icqValidateDefaultEditHelper(USHORT sz)
 {
    return sz == sizeof(MSGEDITHELPER) ? 0 : 99;
 }

 int EXPENTRY icqLoadIconTable(HICQ icq, HBITMAP *img, HBITMAP *msk)
 {
    *img = icqskin_dataBlock.iconImage;
    *msk = icqskin_dataBlock.iconMasc;
    return 0;
 }

 int icqskin_postEvent(HICQ icq, ULONG uin, char id, USHORT event, ULONG parm)
 {
    if(!icqskin_dataBlock.mainWindow)
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected call to icqskin_postEvent()");
       return -1;
    }

    switch(id)
    {
    case 'S':
       WinPostMsg(icqskin_dataBlock.mainWindow,WMICQ_SYSTEMEVENT,MPFROMSHORT(event),(MPARAM) parm);
       break;

    case 'U':
       WinPostMsg(icqskin_dataBlock.mainWindow,WMICQ_USEREVENT,MPFROMSHORT(event),(MPARAM) uin);
       break;

    default:
       WinPostMsg(icqskin_dataBlock.mainWindow,WMICQ_GUIEVENT,MPFROM2SHORT(event,id),(MPARAM) parm);
    }

    return 0;
 }

 HWND EXPENTRY icqOpenDefaultConfigurationWindow(HICQ icq, ULONG uin, USHORT type)
 {
    return icqskin_openConfig(icq, uin, type);
 }

 void EXPENTRY icqLogonDialog(HICQ icq)
 {
    CHKPoint();
    icqskin_logonDialog(icq);
    CHKPoint();
 }

 APIRET EXPENTRY icqRegisterConfigurationWindow(HICQ icq, HAB hab)
 {
    return 0;
 }

 int EXPENTRY icqOpenSearchWindow(HICQ icq, USHORT id)
 {
    return 0;
 }

 HPOINTER EXPENTRY icqCreatePointer(HICQ icq, int id)
 {
    return icqskin_CreatePointer(icqskin_getDataBlock(icq), id);
 }


 int EXPENTRY icqProcessGuiEvent(HICQ icq, ULONG uin, UCHAR type, USHORT event, ULONG parm)
 {
    #pragma pack(1)

    struct _parm
    {
   	   void (* _System callback)(ULONG);
    } *p = (struct _parm *) parm;

    #pragma pack()

    if(type == 'g' && event == ICQEVENT_CALLBACK)
    {
       if(p->callback)
          p->callback(parm);
    }

    return 0;
 }

 int EXPENTRY icqSetBitmap(HICQ icq, hWindow hwnd, const char *fileName)
 {
    return icqskin_setBitmap(icq, hwnd, fileName);
 }

