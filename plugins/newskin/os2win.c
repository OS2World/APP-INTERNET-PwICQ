/*
 * os2win.c - Loader for the pwICQ OS2 GUI
 */

 #pragma strings(readonly)

 #include <pwicqgui.h>

#ifndef GUINAME
   #ifdef STANDARD_GUI
       #define GUINAME "Standard"
   #else
       #define GUINAME "Skinned"
   #endif
#endif

/*---[ Skin Manager entry points ]------------------------------------------------------------------------*/

 static ULONG _System loadSkin(HICQ, STATUSCALLBACK *, int, char **);
 static int   _System executeSkin(HICQ,ULONG);
 static int   _System event(HICQ, ULONG, char, USHORT, ULONG, SKINDATA *);
 static int   _System timer(HICQ,SKINDATA *,char);
 static int   _System validateEdit(USHORT);
 static int   _System popupUserList(HICQ);

/*---[ Skin Manager definition table ]--------------------------------------------------------------------*/

 const SKINMGR icqgui_descr =
 {
    sizeof(SKINMGR),
    0,                  // flags
    GUINAME,
    loadSkin,
    executeSkin,
    (int (_System *)(HICQ,void *,char)) timer,               // timer,
    NULL,                  // warning
    icqskin_viewMessage,   // view
    icqskin_newMessage,    // newMessage
    icqskin_newWithMgr,    // msgWithMgr
    NULL,                  // msgMultDest
    NULL,                  // awayMessage
    icqskin_popupMenu,     // popupmenu
    popupUserList,         // popupUserList
    icqskin_insertMenu,    // insertMenu,
    icqskin_openConfig,    // openConfig,
    validateEdit,          // validateEdit,
    (int (_System *)(HICQ,ULONG,char,USHORT,ULONG,void *)) event,      // event,
#ifdef __OS2__
    NULL,
    NULL,
    NULL
#endif
 };

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 static ULONG _System loadSkin(HICQ icq, STATUSCALLBACK *status, int argc, char **argv)
 {
    SKINDATA *cfg = icqGetPluginDataBlock(icq,module);

    if(!cfg)
       return 0;

    icqskin_registerWindowClass(icq);
    icqskin_Initialize(icq,cfg);

    CHKPoint();

    if(!icqQueryLogonStatus(icq))
    {
       status(icq,-1000);
       icqskin_logonDialog(icq);
       if(!icqQueryLogonStatus(icq))
          return -2;
       status(icq,-1001);
       status(icq,ICQSTARTUP_LOADINGSKIN);
    }

    if(icqInitialize(icq,status))
    {
       icqskin_closeWindow(cfg->frame);
    }

    return (ULONG) cfg;
 }

 static int  _System executeSkin(HICQ icq, ULONG parm)
 {
   QMSG     qmsg;
   SKINDATA *cfg = icqskin_Start(icq, parm);
   HAB      hab  = icqQueryAnchorBlock(icq);
   	
   DBGTracex(cfg);

   if(!cfg)
      return -1;	

#ifndef NDEBUG
//    icqInsertMessage(icq,  27241234, 0, MSG_NORMAL,     0, 0, -1, "Teste de inclusao de mensagem");
//    icqInsertMessage(icq,  27241234, 0, MSG_URL,        0, 0, -1, "Teste de inclusao de URLþhttp://www.os2brasil.com.br");
//    icqInsertMessage(icq,  27241234, 0, MSG_ADD,        0, 0, -1, "SalþRobertoþSalomonþsalomon@scr.com.brþ0");
//    icqInsertMessage(icq,  27241234, 0, MSG_REQUEST,    0, 0, -1, "SalþRobertoþSalomonþsalomon@scr.com.brþ0þResolve, Pô!!!");
//    icqInsertMessage(icq,  27241234, 1, MSG_FILE,       0, 0, -1, "SalþRobertoþSalomonþsalomon@scr.com.brþ0þResolve, Pô!!!");
//    icqInsertMessage(icq,  27241234, 0, MSG_AUTHORIZED, 0, 0, -1, "Autorizado!!");
//    icqInsertMessage(icq,  27241234, 0, MSG_REFUSED,    0, 0, -1, "Voce nao e bom o bastante para entrar na minha lista");
#endif

   icqskin_restoreFrame(cfg->mainWindow,"pos:MainWindow", 150, 375);

//#ifdef DEBUG
//   popupUserList(icq);
//#endif

   while(WinGetMsg(hab,&qmsg,0,0,0))
      WinDispatchMsg(hab,&qmsg);
	
   icqskin_storeFrame(cfg->mainWindow,"pos:MainWindow");

   return icqskin_Stop(icq,cfg);

 }

 int icqskin_postEvent(HICQ icq, ULONG uin, char id, USHORT event, ULONG parm)
 {
    SKINDATA *cfg = icqskin_getDataBlock(icq);

    if(cfg && cfg->mainWindow)
    {
       DBGTracex(MPFROMSH2CH(event,id,1));

       if(uin)
          WinPostMsg(cfg->mainWindow,WMICQ_EVENT,MPFROMSH2CH(event,id,1),MPFROMLONG(uin));
       else
          WinPostMsg(cfg->mainWindow,WMICQ_EVENT,MPFROMSH2CH(event,id,0),MPFROMLONG(parm));

    }
    return 0;
 }

 static int _System event(HICQ icq, ULONG uin, char id, USHORT event, ULONG parm, SKINDATA *cfg)
 {
    if(cfg && cfg->mainWindow)
    {
       if(id == 'S')
          WinPostMsg(cfg->mainWindow,WMICQ_EVENT,MPFROMSH2CH(event,id,0),MPFROMLONG(parm));
       else
          WinPostMsg(cfg->mainWindow,WMICQ_EVENT,MPFROMSH2CH(event,id,1),MPFROMLONG(uin));
    }
    return 0;
 }

 static int _System timer(HICQ icq,SKINDATA *skn,char flags)
 {
    if(skn && skn->sz == sizeof(SKINDATA))
    {
       if(skn->tick++ < 5)
          return 0;

       skn->tick = 0;

       skn->animate++;
       skn->animate &= 0x07;

       event(icq,0,'g',ICQEVENT_TIMER,0,skn);
    }
    return 0;
 }

 static int _System validateEdit(USHORT sz)
 {
    if(sz != sizeof(DIALOGCALLS))
       return 2;
    return 0;
 }

 int _System popupUserList(HICQ icq)
 {
#ifdef DEBUG
    icqskin_postEvent(icq, 0, 'g', ICQEVENT_POPUP, 0);
#endif
    return 0;
 }

