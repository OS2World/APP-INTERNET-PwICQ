/*
 * icqRexx.c - Suporte REXX/Helpers
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>

 #include "icqrexx.h"

/*---[ Prototipes ]--------------------------------------------------------------------------*/


/*---[ Public ]------------------------------------------------------------------------------*/

 HMODULE module = NULLHANDLE;

/*---[ Implement ]---------------------------------------------------------------------------*/

 int EXPENTRY icqrexx_Configure(HICQ icq, HMODULE hmod)
 {
#ifdef DEBUG
    icqWriteSysLog(icq,PROJECT,"Loading Helper/Script manager plugin Build " __DATE__ " " __TIME__ " (Debug Version)");
#else
    icqWriteSysLog(icq,PROJECT,"Loading Helper/Script manager plugin Build  " BUILD);
#endif

    if(icqQueryUserDataBlockLength() != sizeof(ICQUSER))
    {
       icqWriteSysLog(icq,PROJECT,"Can't start message manager due to invalid pwICQ core version");
       return -1;
    }

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"Extensive logging enabled");
#endif

    icqSetCaps(icq,ICQMF_HELPERS);        // Turn helper capability on

    module = hmod;

    return 0;
 }

 void EXPENTRY icqrexx_ConfigPage(HICQ icq, void *dummy, ULONG uin, USHORT type, HWND hwnd, const DLGINSERT *dlg, char *buffer)
 {
/*
    if(type != 1)
       return;
	
#ifdef __OS2__	
    dlg->insert(hwnd, module, 625, &evtConfig,   CFGWIN_EVENTS);
    dlg->insert(hwnd, module, 626, NULL,         CFGWIN_ADVANCED);
#else
    dlg->insertTable(hwnd, CFGWIN_EVENTS, event_table, &evtConfig, "Scripts\nEvent Scripts");
#endif
*/
	
 }

