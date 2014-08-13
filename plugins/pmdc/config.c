
 #pragma strings(readonly)

 #include <string.h>
 #include <stdio.h>
 #include <ctype.h>

 #include "icqpmdc.h"

/*---[ Configuration data ]----------------------------------------------------------------------------------*/

 #pragma pack(1)

 struct config
 {
    USHORT      selectedMode;
    USHORT      onlineMode;
    USHORT      offlineMode;
    BOOL        enabled;
 };

 #pragma pack()

/*---[ Prototipes ]------------------------------------------------------------------------------------------*/

 static int   _System load(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int   _System save(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int   _System configure(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static void  _System selected(const DLGHELPER *,HWND,HICQ,ULONG,  USHORT, ULONG, char *);

/*---[ Constants ]-------------------------------------------------------------------------------------------*/

 static const DLGMGR pmdcConfig = {    sizeof(DLGMGR),
                                       sizeof(DIALOGCALLS),
                                       sizeof(struct config),
                                       configure,              // Configure
                                       load,                   // Load
                                       save,                   // Save
                                       NULL,                   // Cancel
                                       NULL,                   // Event
                                       selected,               // Selected
                                       NULL,                   // Click
                                       NULL,                   // Changed
                                       NULL		               // sysButton
                                  };

/*---[ Constants ]-------------------------------------------------------------------------------------------*/

 void EXPENTRY icqpmdc_ConfigPage(HICQ icq, void *dunno, ULONG uin, USHORT type, HWND hwnd, const DLGINSERT *dlg, char *buffer)
 {
    if(type == 1)
       dlg->loadPage(hwnd, module, 625, &pmdcConfig, CFGWIN_ADVANCED);
 }

 static int _System configure(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    const ICQMODETABLE  *tbl    = icqQueryModeTable(icq);

    if(!tbl)
       return -1;

    DBGTracex(tbl);

    while(tbl->descr)
    {
       DBGMessage(tbl->descr);
       dlg->listBoxInsert(hwnd,102,tbl->mode,tbl->descr,FALSE);
       tbl++;
    }

    return sizeof(struct config);
 }

 static int _System load(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    struct config *cfg = (struct config *) dlg->getUserData(hwnd);

    cfg->onlineMode  = icqQueryModeTable(icq)->mode & 0x0000FFFF;
    cfg->offlineMode = icqQueryOfflineMode(icq) & 0x0000FFFF;
    cfg->enabled     = FALSE;
    dlg->readCheckBox(hwnd,113,PROJECT ":DropBox",FALSE);
    return 0;
 }

 static int _System save(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    struct config *cfg = (struct config *) dlg->getUserData(hwnd);

    if(cfg->enabled)
    {
       sprintf(buffer,"pmdc:%04x.enabled",(int) cfg->selectedMode);
       dlg->writeRadioButton(hwnd,103,buffer,3);
    }
    dlg->writeCheckBox(hwnd, 113, PROJECT ":DropBox");

    return 0;
 }

 static void  _System selected(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, USHORT id, ULONG parm, char *buffer)
 {
    struct config       *cfg    = (struct config *) dlg->getUserData(hwnd);

    save(dlg,hwnd,icq,uin,buffer);

    parm &= 0x0000FFFF;
    cfg->selectedMode = parm;

    sprintf(buffer,"pmdc:%04x.enabled",(int) cfg->selectedMode);

    dlg->readRadioButton(hwnd,103, buffer, 3,2);

    dlg->setEnabled(hwnd,103,TRUE);
    dlg->setEnabled(hwnd,104,TRUE);
    dlg->setEnabled(hwnd,105,TRUE);

    cfg->enabled = TRUE;

 }


