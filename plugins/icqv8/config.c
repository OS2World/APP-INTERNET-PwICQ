/*
 * OS2CONF.C - Dialogo de configuracao para o plugin de protocolo V7
 */

 #include "icqv8.h"

/*---[ Prototipes ]------------------------------------------------------------------------------------------*/

 static int _System loginLoad(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int _System loginSave(const DLGHELPER *,HWND,HICQ,ULONG,char *);

 static int _System optionsLoad(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int _System optionsSave(const DLGHELPER *,HWND,HICQ,ULONG,char *);

 static int _System advancedLoad(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int _System advancedSave(const DLGHELPER *,HWND,HICQ,ULONG,char *);

/*---[ Publics ]---------------------------------------------------------------------------------------------*/

 static DECLARE_SIMPLE_DIALOG_MANAGER(login,    loginLoad,    loginSave);
 static DECLARE_SIMPLE_DIALOG_MANAGER(options,  optionsLoad,  optionsSave);
 static DECLARE_SIMPLE_DIALOG_MANAGER(advanced, advancedLoad, advancedSave);

/*
 const DLGMGR login   = {       sizeof(DLGMGR),
                                sizeof(DLGHELPER),
                                0,
                                NULL,           // Configure
                                loginLoad,      // Load
                                loginSave,      // Save
                                NULL,           // Cancel
                                NULL,           // Event
                                NULL,           // Selected
                                NULL,           // Click
                                NULL            // Changed
                        };

 const DLGMGR options = {       sizeof(DLGMGR),
                                sizeof(DLGHELPER),
                                0,
                                NULL,           // Configure
                                optionsLoad,    // Load
                                optionsSave,    // Save
                                NULL,           // Cancel
                                NULL,           // Event
                                NULL,           // Selected
                                NULL,           // Click
                                NULL            // Changed
                        };

 const DLGMGR advanced = {      sizeof(DLGMGR),
                                sizeof(DLGHELPER),
                                0,
                                NULL,           // Configure
                                advancedLoad,   // Load
                                advancedSave,   // Save
                                NULL,           // Cancel
                                NULL,           // Event
                                NULL,           // Selected
                                NULL,           // Click
                                NULL            // Changed
                        };
*/

/*---[ Implementing ]----------------------------------------------------------------------------------------*/


 void EXPENTRY icqv8_ConfigPage(HICQ icq, void *lixo, ULONG uin, USHORT type, HWND hwnd, const DLGINSERT *dlg, char *buffer)
 {
    if(type != 1)
       return;

    dlg->loadPage(hwnd, module, 1070, &login,    CFGWIN_NETWORK);
    dlg->loadPage(hwnd, module, 1071, &options,  CFGWIN_SECURITY);
    dlg->loadPage(hwnd, module, 1072, &advanced, CFGWIN_ADVANCED);

 }

 static int loginLoad(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    dlg->readString(hwnd,101,"loginServer","login.icq.com");

    dlg->setSpinButton(hwnd,105,icqLoadValue(icq,"reconnect",0)/60,0,3600);

    dlg->readCheckBox(hwnd,106,"hideWhenOffline",  FALSE);
    dlg->readCheckBox(hwnd,114,"Keep-Alive",  FALSE);

    return 0;
 }

 static int loginSave(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    dlg->writeString(hwnd,101,"loginServer");

	icqSaveValue(icq,"reconnect",dlg->getSpinButton(hwnd,105)*60);

    dlg->writeCheckBox(hwnd,106,"hideWhenOffline");
    dlg->writeCheckBox(hwnd,114,"Keep-Alive");

    return 0;
 }


 static int optionsLoad(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    dlg->readCheckBox(hwnd,101,"ip:contact",       TRUE);
    dlg->readCheckBox(hwnd,102,"ip:request",       TRUE);
    dlg->readCheckBox(hwnd,112,"ip:show",          FALSE);
    dlg->readCheckBox(hwnd,103,"webAware",         FALSE);
    dlg->readCheckBox(hwnd,110,"notifyIfRemoved",  FALSE);

    dlg->readString(hwnd,111,"AutoRemoveMsg","");

    dlg->readRadioButton(hwnd,106,"ifUpdateFails",3,0);

    return 0;
 }

 static int optionsSave(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    dlg->writeCheckBox(hwnd,101,"ip:contact");
    dlg->writeCheckBox(hwnd,102,"ip:request");
    dlg->writeCheckBox(hwnd,112,"ip:show");
    dlg->writeCheckBox(hwnd,103,"webAware");
    dlg->writeCheckBox(hwnd,110,"notifyIfRemoved");

    dlg->writeString(hwnd,111,"AutoRemoveMsg");
    dlg->writeRadioButton(hwnd,106,"ifUpdateFails",3);

    return 0;
 }

 static int advancedLoad(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    dlg->readCheckBox(hwnd,101,"cacheOnSpecial",  FALSE);
    dlg->readCheckBox(hwnd,102,"newMsgFormat",    TRUE);
    dlg->readCheckBox(hwnd,103,"updateNick",      TRUE);
    dlg->readCheckBox(hwnd,104,"c2s.watchDog",    TRUE);
    dlg->readCheckBox(hwnd,106,"URLAsText",       TRUE);

    return 0;
 }

 static int advancedSave(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    dlg->writeCheckBox(hwnd,101,"cacheOnSpecial");
    dlg->writeCheckBox(hwnd,102,"newMsgFormat");
    dlg->writeCheckBox(hwnd,103,"updateNick");
    dlg->writeCheckBox(hwnd,104,"c2s.watchDog");
    dlg->writeCheckBox(hwnd,106,"URLAsText");
    return 0;
 }


