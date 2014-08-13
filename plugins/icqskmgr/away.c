/*
 * AWAY.C - User away dialog box
 */

 #pragma strings(readonly)

 #include <stdio.h>
 #include <string.h>

 #include "icqskmgr.h"

/*---[ Prototipos ]---------------------------------------------------------------------------------*/

 static void configure(HWND,struct awayDialogParameters *);

/*---[ Implementacao ]------------------------------------------------------------------------------*/

 MRESULT EXPENTRY dlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    if(msg == WM_INITDLG)
       configure(hwnd,PVOIDFROMMP(mp2));

    return WinDefDlgProc(hwnd,msg,mp1,mp2);
 }


 void openUserAwayDialog(HWND hwnd, struct awayDialogParameters *parm)
 {
    DBGMessage("*** Abrir dialogo de user away");

    WinDlgBox(  HWND_DESKTOP,
                hwnd,
                dlgProc,
                module,
                21,
                parm);
 }

 static void configure(HWND hwnd,struct awayDialogParameters *parm)
 {
    char        temp[0x0100];

    CHKPoint();

    if(!parm || parm->sz != sizeof(struct awayDialogParameters))
    {
       icqSysLog(NULL,"Invalid parameter block passed to the User Away dialog box");
       return;
    }

    if(parm->title)
       WinSetWindowText(hwnd,(PSZ) parm->title);

    sprintf(temp,"%lu",parm->uin);
    WinSetDlgItemText(hwnd,108,temp);

    if(parm->usr)
    {
       WinSetDlgItemText(hwnd,106,(PSZ) icqQueryUserNick(parm->usr));
       WinSetDlgItemText(hwnd,107,(PSZ) icqQueryUserEmail(parm->usr));
       strncpy(temp,(PSZ) icqQueryUserFirstName(parm->usr),0xFF);
       strncat(temp," ",0xFF);
       strncat(temp,(PSZ) icqQueryUserLastName(parm->usr),0xFF);
       WinSetDlgItemText(hwnd,109,temp);
    }
    else
    {
       WinSetDlgItemText(hwnd,106,temp);
    }

    if(parm->txt)
       WinSetDlgItemText(hwnd,113,parm->txt);

    parm->flag = 0;

 }

