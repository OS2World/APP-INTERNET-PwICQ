/*
 * BUtton Processor
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdio.h>

 #include "icqminiw.h"


/*---[ Definitions ]-------------------------------------------------------------------------------------*/


/*---[ Statics ]-----------------------------------------------------------------------------------------*/

 static void statusChanged(HWND,HICQ);

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/

/*---[ Implementation ]----------------------------------------------------------------------------------*/


 void icqminiw_sysButton(HWND wnd, HICQ icq)
 {
    CHKPoint();
    icqShowPopupMenu(icq, 0, ICQMNU_SYSTEM, 0, 0);
 }

 void icqminiw_modeButton(HWND wnd, HICQ icq)
 {
    CHKPoint();
    icqShowPopupMenu(icq, 0, ICQMNU_MODES, 0, 0);
 }

 void icqminiw_openButton(HWND wnd, HICQ icq)
 {
    HMSG        msg	= 0;
    ULONG       uin;

    if(icqQueryFirstMessage(icq,&uin,&msg))
    {
       CHKPoint();
       icqPopupUserList(icq);
       return;
    }
    CHKPoint();
    icqOpenMessage(icq, uin, msg);
 }



