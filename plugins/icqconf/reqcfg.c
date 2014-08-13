/*
 * Anti-Spam dialog
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include "icqconf.h"

/*---[ Constants ]-------------------------------------------------------------------------------------------*/

 static int _System load(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int _System save(const DLGHELPER *,HWND,HICQ,ULONG,char *);

 DECLARE_SIMPLE_DIALOG_MANAGER(request,load,save);

/*---[ Implementing ]----------------------------------------------------------------------------------------*/

 static int load(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    dlg->readCheckBox(  hwnd, 104, "AutoRefuse",    FALSE);
    dlg->readString(    hwnd, 106, "AutoRefuseMsg", "");
    dlg->readCheckBox(  hwnd, 108, "AutoAccept",    FALSE);
    dlg->readCheckBox(  hwnd, 117, "RefuseIgnored", FALSE);
    return 0;
 }

 static int save(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    dlg->writeCheckBox( hwnd, 104, "AutoRefuse");
    dlg->writeString(   hwnd, 106, "AutoRefuseMsg");
    dlg->writeCheckBox( hwnd, 108, "AutoAccept");
    dlg->writeCheckBox( hwnd, 117, "RefuseIgnored");
    return 0;
 }






