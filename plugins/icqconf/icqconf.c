/*
 * ICQCONF.C - Modulo principal do plugin de configuracao
 */

#ifdef __OS2__
 #define INCL_WIN
#endif

 #include <stdio.h>
 #include <string.h>
 #include <time.h>

 #include "icqconf.h"

/*---[ Prototipes ]------------------------------------------------------------------------------------------*/

 static int  sysConfig(HICQ, ULONG, HWND, const DLGINSERT *, char *);
 static int  userInfo(HICQ, ULONG, HWND, const DLGINSERT *, char *);

/*---[ Publics ]---------------------------------------------------------------------------------------------*/

 HMODULE module = NULLHANDLE;

/*---[ Implementing ]----------------------------------------------------------------------------------------*/

 int EXPENTRY icqconf_Configure(HICQ icq, HMODULE hmod)
 {
    module = hmod;
    return 0;
 }

 void EXPENTRY icqconf_ConfigPage(HICQ icq, void *dunno, ULONG uin, USHORT type, HWND hwnd, const DLGINSERT *dlg, char *buffer)
 {
    static const struct entryPoints
    {
       int (*exec)(HICQ,ULONG,HWND,const DLGINSERT *,char *);
    } ep[] =
	{
	   { userInfo },
	   { sysConfig }
    };

    if(dlg->sz != sizeof(DLGINSERT))
    {
       icqWriteSysLog(icq,PROJECT,"Invalid dialog loader");
       return;
    }

#ifdef EXTENDED_LOG
    sprintf(buffer,"Loading dialog boxes for configuration window type %d (Table size: %d)",(int) type, (int) (sizeof(ep)/sizeof(struct entryPoints)) );
    icqWriteSysLog(icq,PROJECT,buffer);
#endif

    if(type < (sizeof(ep)/sizeof(struct entryPoints)) )
       (ep+type)->exec(icq,uin,hwnd,dlg,buffer);

    return;
 }

 static int sysConfig(HICQ icq, ULONG uin, HWND hwnd, const DLGINSERT *dlg, char *buffer)
 {

    /* Insert System Configuration pages */
    CHKPoint();

    dlg->loadPage(hwnd, module, 202, &away,         CFGWIN_OPTIONS);
    dlg->loadPage(hwnd, module, 100, &antiSpam,     CFGWIN_SECURITY);
    dlg->loadPage(hwnd, module, 101, &request,      CFGWIN_SECURITY);
    dlg->loadPage(hwnd, module, 204, &groupNames,   CFGWIN_USERS);

#ifdef __OS2__
    dlg->loadPage(hwnd, module, 203, &codepage,   CFGWIN_OPTIONS);
#endif

    return 0;

 }

 static int userInfo(HICQ icq, ULONG uin, HWND hwnd, const DLGINSERT *dlg, char *buffer)
 {

    /* Insert User configuration pages */
    CHKPoint();

    dlg->loadPage(hwnd, module, 201, &usrBasic,     CFGWIN_INFO);
    dlg->loadPage(hwnd, module, 206, &miscOptions,  CFGWIN_OPTIONS);
    dlg->loadPage(hwnd, module, 205, &userGroups,   CFGWIN_OPTIONS);

    return 0;
 }


