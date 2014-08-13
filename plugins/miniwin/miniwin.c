/*
 * icqminiwin.c - pwICQ Mini status Window - Plugin Entry points
 *
 */

 #pragma strings(readonly)

 #include <string.h>
 #include <stdio.h>

 #include "icqminiw.h"

/*----------------------------------------------------------------------------*/

 HMODULE        module          = NULLHANDLE;
 static  HWND   hwnd            = NULLHANDLE;

/*----------------------------------------------------------------------------*/

 int EXPENTRY icqminiw_Configure(HICQ icq, HMODULE mod)
 {
    CHKPoint();
    module = mod;
    return sizeof(HWND);
 }

 int EXPENTRY icqminiw_Start(HICQ icq, HPLUGIN p, HWND *hwnd)
 {
    CHKPoint();
    icqWriteSysLog(icq, PROJECT,"Mini Status Window plugin starting (Build " BUILD ")");
    *hwnd = 0;

    icqminiw_initializeGui(icq);

    return 0;
 }



