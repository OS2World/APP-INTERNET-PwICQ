/*
 * os2url.c - Abre uma URL (versao OS/2)
 */

 #pragma strings(readonly)

 #define INCL_WIN

 #include <string.h>

 #include <pwMacros.h>
 #include <icqtlkt.h>

/*---[ Prototipos ]-------------------------------------------------------------------------------------------*/


/*---[ Implementacao ]----------------------------------------------------------------------------------------*/

 int EXPENTRY icqOpenURL(HICQ icq, const char *url)
 {
    PROGDETAILS pDetails;
    HAPP        happ;
    ULONG       saf                     = SAF_INSTALLEDCMDLINE;
    char        browser[0x0100];
    char        parms[0x0200];

#ifdef DEBUG
    icqLoadString(icq,"browser","s:\\Mozilla\\bin\\mozilla.exe",browser,0xFF);
#else
    icqLoadString(icq,"browser","mozilla.exe",browser,0xFF);
#endif

    strcpy(parms,"Opening ");
    strncpy(parms,url,0x01FF);
    icqWriteSysLog(icq,PROJECT,parms);

    strcpy(parms," \"");
    strncat(parms,url,0x01FE);
    strncat(parms,"\"",0x01FE);

    memset(&pDetails,0,sizeof(PROGDETAILS));

    pDetails.Length                      = sizeof(PROGDETAILS);

    pDetails.pszTitle                    = (PSZ) url;
    pDetails.pszExecutable               = browser;
    pDetails.pszParameters               = parms;
    pDetails.pszStartupDir               = "";
    pDetails.swpInitial.fl               = SWP_ACTIVATE;        /* Window positioning   */
    pDetails.swpInitial.hwndInsertBehind = HWND_TOP;
    pDetails.progt.fbVisible             = SHE_VISIBLE;
    pDetails.progt.progc                 = PROG_DEFAULT;

    switch(icqLoadValue(icq,"BrowserMode",0))
    {
    case 0:
       DBGMessage("Visible");
       pDetails.progt.fbVisible  = SHE_VISIBLE;
       break;

    case 1:
       DBGMessage("Minimized");
       pDetails.progt.fbVisible  = SHE_VISIBLE;
       saf                      |= SAF_MINIMIZED;
       break;

    default:
       pDetails.progt.fbVisible  = SHE_INVISIBLE;
       saf                      |= SAF_BACKGROUND;
       DBGMessage("Invisible");
    }

    happ = WinStartApp(NULLHANDLE,&pDetails,NULL,NULL,saf);

    return 0;
 }



