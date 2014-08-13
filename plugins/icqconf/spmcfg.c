/*
 * Anti-Spam dialog
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include "icqconf.h"

/*---[ Screen ]----------------------------------------------------------------------------------------------*/

/*---[ Constants ]-------------------------------------------------------------------------------------------*/

 static int _System load(const DIALOGCALLS *, HWND, HICQ, ULONG, char *);
 static int _System save(const DIALOGCALLS *, HWND, HICQ, ULONG, char *);

 DECLARE_SIMPLE_DIALOG_MANAGER(antiSpam,load,save);

/*---[ Implementing ]----------------------------------------------------------------------------------------*/

 static int load(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {

    dlg->readRadioButton(hwnd,110, "Filter.K",      3,0);
    dlg->readRadioButton(hwnd,114, "Filter.U",      3,1);
    dlg->readCheckBox(hwnd,   102, "IgnorePager",   TRUE);
    dlg->readCheckBox(hwnd,   103, "IgnoreMail",    TRUE);

    dlg->readCheckBox(hwnd,   118, "ignoreEmpty",   FALSE);
    dlg->readCheckBox(hwnd,   119, "ignoreURL",     FALSE);
    dlg->readCheckBox(hwnd,   120, "badWord.K",     FALSE);
    dlg->readCheckBox(hwnd,   121, "badWord.U",     FALSE);

    dlg->readCheckBox(hwnd,   123, "badWord.Strip", TRUE);

    return 0;
 }

 static int save(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {

    dlg->writeRadioButton(hwnd, 110, "Filter.K", 3);
    dlg->writeRadioButton(hwnd, 114, "Filter.U", 3);

    dlg->writeCheckBox(hwnd,    102, "IgnorePager");
    dlg->writeCheckBox(hwnd,    103, "IgnoreMail");

    dlg->writeCheckBox(hwnd,    118, "ignoreEmpty");
    dlg->writeCheckBox(hwnd,    119, "ignoreURL");
    dlg->writeCheckBox(hwnd,    120, "badWord.K");
    dlg->writeCheckBox(hwnd,    121, "badWord.U");
    dlg->writeCheckBox(hwnd,    123, "badWord.Strip");

    return 0;
 }






