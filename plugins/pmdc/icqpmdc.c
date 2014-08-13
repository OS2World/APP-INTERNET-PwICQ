/*
 * icqpmdc.C - PMDC Interface plugin
 *
 */

 #pragma strings(readonly)

 #include <os2.h>

 #include <string.h>
 #include <stdio.h>

 #include "icqpmdc.h"

 #include <icqqueue.h>

/*----------------------------------------------------------------------------*/

 struct msgtype
 {
    USHORT      type;
    char        *masc;
 };

/*----------------------------------------------------------------------------*/


#ifdef SYSMENU
 static int  _System startDownloads(HICQ, ULONG, USHORT, ULONG);
 static int  _System stopDownloads(HICQ, ULONG, USHORT, ULONG);
#endif

 static void writePipe(HICQ, const char *);

/*----------------------------------------------------------------------------*/

 HMODULE        module          = NULLHANDLE;
 static int     enableDownloads = TRUE;

/*----------------------------------------------------------------------------*/

 int EXPENTRY icqpmdc_Configure(HICQ icq, HMODULE mod)
 {
    /*
     *
     * PLUGIN configuration routine, the first called by pwICQ, must return 0
     * or the a size for the plugin data-area to be allocated by the core
     *
     * * If returns a negative value the plugin is unloaded
     *
     */
    module = mod;

    CHKPoint();

    return 0;
 }

 int EXPENTRY icqpmdc_Start(HICQ icq, HPLUGIN p, void *dunno)
 {
    icqWriteSysLog(icq, PROJECT,"PMDC interface plugin starting (Build " BUILD ")");

/*
#ifdef SYSMENU
    icqInsertMenuOption(icq, ICQMENU_SYSTEM, "Stop Downloads",  48, 48, stopDownloads);
    icqInsertMenuOption(icq, ICQMENU_SYSTEM, "Start Downloads", 47, 47, startDownloads);
#endif
*/
    icqpmdc_initializeGui(icq);

    return 0;
 }

 void writePipe(HICQ icq, const char *text)
 {
    FILE *f = fopen("\\PIPE\\PMDOWNLOADER","w");

    if(f)
    {
       fprintf(f,"%s\n",text);
       fclose(f);
    }
    else
    {
       icqWriteSysLog(icq, PROJECT,"Can't open pipe");
    }

    #ifdef LOG_ALL
       icqWriteSysLog(icq, PROJECT,text);
    #endif

 }

 static int queryStatusByMode(HICQ icq)
 {
    char        key[80];
    sprintf(key,"pmdc:%04x.enabled",(int) icqQueryOnlineMode(icq) & 0xFFFF);
    DBGMessage(key);
    return icqLoadValue(icq,key,2);
 }

 static void setStatusByMode(HICQ icq)
 {
    switch(queryStatusByMode(icq))
    {
    case 0:
#ifdef LOG_ALL
       icqWriteSysLog(icq, PROJECT,"Enable downloads");
#endif
       enableDownloads = TRUE;
       writePipe(icq,"/STARTALL");
       break;

    case 1:
#ifdef LOG_ALL
       icqWriteSysLog(icq, PROJECT,"Disable downloads");
#endif
       enableDownloads = FALSE;
       writePipe(icq,"/STOPALL");
       break;

    }
 }

#ifdef SYSMENU
 static int _System startDownloads(HICQ icq, ULONG uin, USHORT type,ULONG dummy)
 {
    icqWriteSysLog(icq, PROJECT,"Enabling PMDC");
    writePipe(icq,"/STARTALL");
    return 0;
 }

 static int _System stopDownloads(HICQ icq, ULONG uin, USHORT type,ULONG dummy)
 {
    icqWriteSysLog(icq, PROJECT,"Disabling PMDC");
    writePipe(icq,"/STOPALL");
    return 0;
 }
#endif

 void EXPENTRY icqpmdc_rpcRequest(HICQ icq, struct icqpluginipc *req)
 {
    if(req->h.sz <= sizeof(struct icqpluginipc))
    {
       req->h.rc = -1;
       icqWriteSysLog(icq,PROJECT,"Invalid IPC request");
       return;
    }

    icqpmdc_downloadFile(icq,(const char *)(req+1));
    req->h.rc = 0;
 }

 void icqpmdc_downloadFile(HICQ icq, const char *url)
 {

/*
--- 30/09/2002 10:17:16 Message from ErOs2 (ICQ#167811206) ---

Oh, I undestand you incorrectly, and implement it as commands to start and stop added downloads.

However, I will not change this behaviour. I will implement understanding commands like

"/URL=http://something/.../   /STATUS=P"

where status will be P - paused, A - active.
*/

    char buffer[0x0100];

    strcpy(buffer,"/URL=");
    strncat(buffer,url,0xFF);
    strncat(buffer," /STATUS=",0xFF);

    if(enableDownloads)
       strncat(buffer,"A",0xFF);
    else
       strncat(buffer,"P",0xFF);

    writePipe(icq, buffer);

 }

 static int _System downloadMenu(HICQ icq, ULONG uin, const MSGMGR *mgr, const char *text, const char *url)
 {
    if(url)
       icqpmdc_downloadFile(icq,url);
    return 0;
 }

 void EXPENTRY icqpmdc_addSendTo(HICQ icq, hWindow hwnd, USHORT type, BOOL out, const MSGMGR *mgr, int ( * _System addOption)(HWND, USHORT, const char *, SENDTOCALLBACK *))
 {
    /*
     * Add option in the Send to button
     */
    if(!out)
       addOption(hwnd, 1, "Download Center", downloadMenu);

 }

 void EXPENTRY icqpmdc_Event(HICQ icq, void *block, ULONG uin, char id, USHORT eventCode, ULONG parm)
 {
    if(id != 'S')
       return;

    if(eventCode == ICQEVENT_SECONDARY)
       icqpmdc_createWindow(icq);

    if(eventCode == ICQEVENT_ONLINE || eventCode == ICQEVENT_OFFLINE || eventCode == ICQEVENT_CHANGED)
       setStatusByMode(icq);
 }

