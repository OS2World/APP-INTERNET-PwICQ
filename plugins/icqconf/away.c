/*
 * away dialog
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdlib.h>
 #include <string.h>
 #include <stdio.h>

 #include "icqconf.h"

/*---[ Configuration ]---------------------------------------------------------------------------------------*/

 #define CHECKBOXES     6

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

 static int   _System initialize(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static void  _System selected(const DLGHELPER *,HWND,HICQ,ULONG,  USHORT, ULONG, char *);
 static int   _System load(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int   _System save(const DLGHELPER *,HWND,HICQ,ULONG,  char *);

/*---[ Constants ]-------------------------------------------------------------------------------------------*/

 const DLGMGR away = {        sizeof(DLGMGR),
                              sizeof(DLGHELPER),
                              sizeof(struct config),
                              initialize,	// Configure
                              load,		    // Load
                              save,		    // Save
                              NULL,         // Cancel
                              NULL,         // Event
                              selected,		// Selected
                              NULL,         // Click
                              NULL,         // changed
                              NULL          // sysbtn
                         };



/*---[ Implementing ]----------------------------------------------------------------------------------------*/

 static int initialize(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    const ICQMODETABLE  *tbl    = icqQueryModeTable(icq);

    DBGTracex(tbl);

    if(!tbl)
       return -1;

    while(tbl->descr)
    {
       DBGMessage(tbl->descr);
       dlg->listBoxInsert(hwnd,102,tbl->mode,tbl->descr,FALSE);
       tbl++;
    }

    return 0;

 }

 static int load(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    struct config       *cfg   = (struct config *) dlg->getUserData(hwnd);
    int f;

    cfg->onlineMode  = icqQueryModeTable(icq)->mode & 0x0000FFFF;
    cfg->offlineMode = icqQueryOfflineMode(icq) & 0x0000FFFF;

    for(f=0;f<CHECKBOXES;f++)
       dlg->setEnabled(hwnd,200+f,FALSE);

    dlg->setEnabled(hwnd,102,TRUE);
    dlg->setEnabled(hwnd,103,FALSE);

    return 0;
 }

 static void  _System selected(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, USHORT id, ULONG parm, char *buffer)
 {
    char                key[20];
    struct config       *cfg    = (struct config *) dlg->getUserData(hwnd);
    int                 f;
    ULONG               flag;
    ULONG               masc    = 0x00000001;
    ULONG               caps    = icqGetCaps(icq);

    DBGTracex(id);
    DBGTracex(cfg);

    if(id != 102 || !cfg)
       return;

    DBGTracex(parm);

    save(dlg,hwnd,icq,uin,buffer);

    parm &= 0x0000FFFF;
    cfg->selectedMode = parm;

    sprintf(key,"Away:%04x.msg",(int) parm);
    icqLoadString(icq, key, "", buffer, 0xFF);
    dlg->setString(hwnd,103,buffer);

    strcpy(key+10,"flg");
    flag = icqLoadValue(icq, key, 0);

    for(f=0;f<CHECKBOXES;f++)
    {
       dlg->setCheckBox(hwnd,200+f,flag&masc);
       dlg->setEnabled(hwnd,200+f,caps&masc);
       masc <<= 1;
    }

    cfg->enabled = TRUE;

    if(parm == cfg->onlineMode || parm == cfg->offlineMode)
    {
       dlg->setEnabled(hwnd,103,FALSE);
       dlg->setEnabled(hwnd,200,FALSE);
       dlg->setEnabled(hwnd,202,parm == cfg->onlineMode);
    }
    else
    {
       dlg->setEnabled(hwnd,103,TRUE);
    }

 }

 static int _System save(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin,  char *buffer)
 {
    char                key[20];
    struct config       *cfg    = (struct config *) dlg->getUserData(hwnd);
    ULONG               flag    = 0;
    ULONG               masc    = 0x00000001;
    int                 f;

    if(!cfg || !cfg->enabled)
       return 0;

    sprintf(key,"Away:%04x.msg",cfg->selectedMode);

    if(cfg->selectedMode != cfg->onlineMode && cfg->selectedMode != cfg->offlineMode )
    {
       dlg->getString(hwnd,103,0xFF,buffer);
       icqSaveString(icq, key, buffer);
    }

    strcpy(key+10,"flg");

    for(f=0;f<CHECKBOXES;f++)
    {
       if(dlg->getCheckBox(hwnd,200+f))
          flag |= masc;
       masc <<= 1;
    }
    icqSaveValue(icq,key,flag);

    return 0;
 }


