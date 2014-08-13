/*
 * OS2Start.c - Start session
 */

 #pragma strings(readonly)

 #define INCL_WIN

 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>

 #include "icqrexx.h"

//#ifdef DEBUG
//    #define NOEXEC
//#endif

/*---[ Prototipes ]--------------------------------------------------------------------------*/


/*---[ Public ]------------------------------------------------------------------------------*/


/*---[ Implement ]---------------------------------------------------------------------------*/

 void EXPENTRY icqrexx_Event(HICQ icq, void *dataBlock, ULONG uin, char id, USHORT event, ULONG parm)
 {
    char        key[20];
    char        title[0x0100];
    char        program[0x0100];
    char        parms[0x0100];
    ULONG       flag;

    sprintf(key,"helper:F%c%03d",id,(int) event);
	
    flag = icqLoadValue(icq, key, 0);

    if(!(flag & 0x80000000))    // No helper fileName for this event
       return;

    DBGMessage(key);

#ifdef EXTENDED_LOG
    sprintf(title,"Key: %s Flags: %08lx %08lx %08lx",key,flag,flag&0x7Fc0>>6,flag&0x20);
    icqWriteSysLog(icq,PROJECT,title);
#endif

    if(!(flag & 0x20))
    {
       if( (flag & 0x10) && !(icqQueryModeFlags(icq) & ICQMF_EXTENDEDHELPER) )
#ifdef EXTENDED_LOG
       {
          icqWriteSysLog(icq,PROJECT,"Extended helper disabled");
          return;
       }
#else
          return;
#endif
       else if(!(icqQueryModeFlags(icq) & ICQMF_NORMALHELPER))
#ifdef EXTENDED_LOG
       {
          icqWriteSysLog(icq,PROJECT,"Normal Helper disabled");
          return;
       }
#else
          return;
#endif
    }
#ifdef EXTENDED_LOG
    else
    {
       icqWriteSysLog(icq,PROJECT,"Always run");
    }
#endif

    *(key+7) = 'T';
    icqLoadString(icq,key,"",title,0xFF);
    if(!*title)
       strcpy(title,"pwICQ Helper");

    *(key+7) = 'P';
    icqLoadString(icq,key,"",parms,0xFF);

    *(key+7) = 'N';
    icqLoadString(icq,key,"",program,0xFF);

    DBGMessage(title);
    DBGMessage(program);

    icqrexx_expandSystemMacros(icq,event,uin,parms);
    icqrexx_expandSystemMacros(icq,event,uin,title);

    DBGTracex(flag);

    if(uin)
    {
       if(icqrexx_expandUserMacros(icq, icqQueryUserHandle(icq, uin), (flag&0x7Fc0) >> 6, parms))
          return;

       if(icqrexx_expandUserMacros(icq, icqQueryUserHandle(icq, uin), (flag&0x7Fc0) >> 6, title))
          return;

    }

    DBGTracex(flag);
    DBGMessage(parms);

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,title);
    icqWriteSysLog(icq,PROJECT,program);
    icqWriteSysLog(icq,PROJECT,parms);
#endif

    icqrexx_startHelper(icq, flag, title, program, parms );

 }

 int EXPENTRY icqrexx_startHelper(HICQ icq, ULONG flags, UCHAR *PgmTitle, UCHAR *PgmName, UCHAR *p)
 {
    PROGDETAILS pDetails;
    HAPP        happ;
    ULONG       saf     = SAF_INSTALLEDCMDLINE;
    char        parms[0x0100];

    *parms = ' ';
    strncpy(parms+1,p,0xFE);

    DBGMessage(PgmName);
    DBGMessage(parms);

    memset(&pDetails,0,sizeof(PROGDETAILS));

    pDetails.Length                      = sizeof(PROGDETAILS);

    pDetails.pszTitle                    = PgmTitle;
    pDetails.pszExecutable               = PgmName;
    pDetails.pszParameters               = parms;
    pDetails.pszStartupDir               = "";
//    pDetails.pszEnvironment              = "WORKPLACE\0\0";
    pDetails.swpInitial.fl               = SWP_ACTIVATE;        /* Window positioning   */
//    pDetails.swpInitial.cy               = 0;                   /* Width of window      */
//    pDetails.swpInitial.cx               = 0;                   /* Height of window     */
//    pDetails.swpInitial.y                = 0;                   /* Lower edge of window */
//    pDetails.swpInitial.x                = 0;                   /* Left edge of window  */
    pDetails.swpInitial.hwndInsertBehind = HWND_TOP;
//    pDetails.swpInitial.hwnd             = NULLHANDLE;
//    pDetails.swpInitial.ulReserved1      = 0;
//    pDetails.swpInitial.ulReserved2      = 0;

    switch(flags & 0x000c)
    {
    case 0x0004:
       DBGMessage("Visible");
       pDetails.progt.fbVisible  = SHE_VISIBLE;
//       saf                    |= SAF_STARTCHILDAPP;
       break;
    case 0x0008:
       DBGMessage("Minimized");
       pDetails.progt.fbVisible  = SHE_VISIBLE;
       saf                      |= SAF_MINIMIZED;
       break;
    default:
       pDetails.progt.fbVisible  = SHE_INVISIBLE;
       saf                      |= SAF_BACKGROUND;
       DBGMessage("Invisible");
    }

    switch(flags & 0x0003)
    {
    case 0x0001:
       DBGMessage("FullScreen");
       pDetails.progt.progc = PROG_FULLSCREEN;
       break;

    case 0x0002:
       DBGMessage("PM");
       pDetails.progt.progc = PROG_PM;
       break;

    default:
       DBGMessage("OS/2 Window");
       pDetails.progt.progc = PROG_WINDOWABLEVIO;
       break;
    }

    happ = WinStartApp(NULLHANDLE,&pDetails,NULL,NULL,saf);

    DBGTracex(happ);

    return 0;
 }

