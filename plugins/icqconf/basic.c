/*
 * Basic user information dialog
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdio.h>

 #include "icqconf.h"

/*---[ Prototipes ]------------------------------------------------------------------------------------------*/

 static int   _System load(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int   _System save(const DLGHELPER *,HWND,HICQ,ULONG,char *);

/*---[ Constants ]-------------------------------------------------------------------------------------------*/

 DECLARE_SIMPLE_DIALOG_MANAGER(usrBasic,load,save);

/*---[ Implementing ]----------------------------------------------------------------------------------------*/

 static int _System load(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    HUSER usr = icqQueryUserHandle(icq,uin);

    sprintf(buffer,"%lu", uin);
    dlg->setString(hwnd,200,buffer);

    if(usr)
    {
       dlg->setString(hwnd,201,icqQueryUserNick(usr));
       dlg->setString(hwnd,202,icqQueryUserFirstName(usr));
       dlg->setString(hwnd,203,icqQueryUserLastName(usr));

       sprintf(buffer,"%d",usr->age);
       dlg->setString(hwnd,150,buffer);

       dlg->setCheckBox(hwnd,120+(usr->sex%4),TRUE);

    }

    return 0;
 }

 static int _System save(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {

    return 0;
 }


