/*
 * RXAPI.C - Interface REXX para o pwICQ IPC
 */

 #define INCL_REXXSAA

#ifdef __OS2__

 #pragma strings(readonly)

 #define INCL_ERRORS
 #include <os2.h>
 #include <rexxsaa.h>

#else
 #include <rexx.h>

#endif

 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include <netdb.h>

 #include <pwMacros.h>
 #include <icqqueue.h>

 #include "rxapi.h"

/*---[ Structs ]-------------------------------------------------------------------------------------------*/

 #pragma pack(1)

 typedef struct fncEntry
 {
    PSZ     rxName;
    PSZ     cName;
 } fncEntry, *fncEntryPtr;

 #pragma pack()

/*---[ Statics ]-------------------------------------------------------------------------------------------*/

 static fncEntry RxFncTable[] =
 {
      { "rxICQQueryBuild",       "rxICQQueryBuild"        },
      { "rxICQSetMode",          "rxICQSetMode"           },
      { "rxICQQueryUIN",         "rxICQQueryUIN"          },
      { "rxICQExecute",          "rxICQExecute"           },
      { "rxICQOpenFirstMessage", "rxICQOpenFirstMessage"  },
      { "rxICQShowPopupMenu",    "rxICQShowPopupMenu"     },
      { "rxICQShowUserList",     "rxICQShowUserList"      },
      { "rxICQPluginRequest",    "rxICQPluginRequest"     },
      { "rxICQMPTRequest",       "rxICQMPTRequest"        },
      { "rxICQQueryOnlineMode",  "rxICQQueryOnlineMode"   },
      { "rxICQConvertCodePage",  "rxICQConvertCodePage"   },
      { "rxICQSetPeerInfo",      "rxICQSetPeerInfo"       },
      { "rxICQPeerConnect",      "rxICQPeerConnect"       },
      { "rxICQQueryAvailable",   "rxICQQueryAvailable"    },
      { "rxICQSendFile",         "rxICQSendFile"          },
      { "rxICQAddUser",          "rxICQAddUser"           },
	  { "rxICQOpenURL",			 "rxICQOpenURL"           },
	  { "rxICQOSD",    			 "rxICQOSD"               },
	  { "rxICQSendURL",			 "rxICQSendURL"           },
      { "rxICQDropFuncs",        "rxICQDropFuncs"         }
 };

 static const char *DLLNAME = "icqipc";


/*---[ Implementacao ]-------------------------------------------------------------------------------------*/

ULONG APIENTRY rxICQLoadFuncs(  CHAR *name,
                                ULONG numargs,
                                RXSTRING args[],
                                CHAR *queuename,
                                RXSTRING *retstr )
  {
    int         entries;
    int         j;
    int         rc      = 0;

    CHKPoint();

    entries           = sizeof( RxFncTable ) / sizeof( fncEntry );
    retstr->strlength = 0;

    for( j = 0; j < entries && !rc; ++j )
       rc = RexxRegisterFunctionDll( RxFncTable[ j ].rxName,(PSZ) DLLNAME,RxFncTable[ j ].cName );

    CopyInteger(rc, retstr );

    return( VALID_ROUTINE );

}

ULONG APIENTRY rxICQQueryBuild( CHAR *name,
                                ULONG numargs,
                                RXSTRING args[],
                                CHAR *queuename,
                                RXSTRING *retstr )
  {
     CHKPoint();
     CopyString( BUILD, retstr);
     return( VALID_ROUTINE );
  }

ULONG EXPENTRY rxICQDropFuncs(  CHAR *name,
                                ULONG numargs,
                                RXSTRING args[],
                                CHAR *queuename,
                                RXSTRING *retstr )
  {
    int entries;
    int j;

    retstr->strlength = 0;

    if( numargs > 0 )
        return( INVALID_ROUTINE );

    entries = sizeof( RxFncTable ) / sizeof( fncEntry );

    for( j = 0; j < entries; ++j )
        RexxDeregisterFunction( RxFncTable[ j ].rxName );

    RexxDeregisterFunction( "rxICQLoadFuncs" );

    CopyInteger(entries, retstr );


    return( VALID_ROUTINE );

}

ULONG EXPENTRY rxICQSetMode(    CHAR *name,
                                ULONG numargs,
                                RXSTRING args[],
                                CHAR *queuename,
                                RXSTRING *retstr )
  {
     int rc;

     if( numargs != 2 )
        return( INVALID_ROUTINE );

     DBGMessage(args[0].strptr);
     DBGMessage(args[1].strptr);

     rc = icqIPCSetModeByName(atoi(args[0].strptr),args[1].strptr);

     DBGTrace(rc);

     CopyInteger(rc, retstr );

     return( VALID_ROUTINE );
  }

ULONG EXPENTRY rxICQQueryUIN(   CHAR *name,
                                ULONG numargs,
                                RXSTRING args[],
                                CHAR *queuename,
                                RXSTRING *retstr )
  {
     ULONG      uin;

     if( numargs )
        return( INVALID_ROUTINE );

     if(icqIPCQueryInstance(&uin))
        CopyInteger(0, retstr );
     else
        CopyInteger(uin, retstr );

     return( VALID_ROUTINE );
  }

ULONG EXPENTRY rxICQExecute(    CHAR *name,
                                ULONG numargs,
                                RXSTRING args[],
                                CHAR *queuename,
                                RXSTRING *retstr )
  {
     int rc;

     if( numargs != 2 )
        return( INVALID_ROUTINE );

     DBGMessage(args[0].strptr);
     DBGMessage(args[1].strptr);

     rc = icqIPCExecuteCommand(atoi(args[0].strptr),args[1].strptr);

     DBGTrace(rc);

     CopyInteger(rc, retstr );

     return( VALID_ROUTINE );
  }


ULONG EXPENTRY rxICQOpenFirstMessage( CHAR *name,
                                                                          ULONG numargs,
                                                                          RXSTRING args[],
                                                                          CHAR *queuename,
                                                                   RXSTRING *retstr )
  {
     int   rc;
         ULONG uin      = 0;

     if( numargs > 1 )
        return( INVALID_ROUTINE );
         else if(numargs == 1)
                uin = atoi(args[0].strptr);

     rc = icqIPCOpenFirstMessage(uin);

     CopyInteger(rc, retstr );

     return( VALID_ROUTINE );
  }

ULONG EXPENTRY rxICQShowPopupMenu( CHAR *name,
                                                                   ULONG numargs,
                                                                   RXSTRING args[],
                                                                   CHAR *queuename,
                                                                   RXSTRING *retstr )
  {
     int   rc;

     if( numargs != 2 )
        return( INVALID_ROUTINE );

     rc = icqIPCShowPopupMenu(atoi(args[0].strptr), atoi(args[1].strptr));

     CopyInteger(rc, retstr );

     return( VALID_ROUTINE );
  }


ULONG EXPENTRY rxICQShowUserList( CHAR *name,
                                                                          ULONG numargs,
                                                                          RXSTRING args[],
                                                                          CHAR *queuename,
                                                                   RXSTRING *retstr )
  {
     int   rc;
     ULONG uin      = 0;

     if( numargs > 1 )
        return( INVALID_ROUTINE );
     else if(numargs == 1)
        uin = atoi(args[0].strptr);

     rc = icqIPCShowUserList(uin);

     CopyInteger(rc, retstr );

     return( VALID_ROUTINE );
  }

ULONG EXPENTRY rxICQPluginRequest( CHAR *name,  ULONG numargs,
                                                RXSTRING args[],
                                                CHAR *queuename,
                                                RXSTRING *retstr )
  {
     ULONG  uin         = 0;
     int    rc;

     if( numargs < 2  || numargs > 3)
        return(INVALID_ROUTINE);

     uin = atoi(args[0].strptr);

     if(numargs == 2)
        rc = icqIPCPluginRequest(uin,args[1].strptr, "");
     else
        rc = icqIPCPluginRequest(uin,args[1].strptr, args[2].strptr);

     CopyInteger(rc, retstr );
     return( VALID_ROUTINE );

  }

ULONG EXPENTRY rxICQQueryOnlineMode( CHAR *name,  ULONG numargs,
                                                RXSTRING args[],
                                                CHAR *queuename,
                                                RXSTRING *retstr )
  {
     ULONG uin	= 0;
     char  buffer[0x0100];


     if(numargs > 1)
        return(INVALID_ROUTINE);

     if(numargs > 0)
        uin = atoi(args[0].strptr);

     icqIPCQueryOnlineMode(0,0xFF,buffer);

     CopyString(buffer, retstr);
     return( VALID_ROUTINE );

  }

ULONG EXPENTRY rxICQConvertCodePage(    CHAR *name,
                                        ULONG numargs,
                                        RXSTRING args[],
                                        CHAR *queuename,
                                        RXSTRING *retstr )
  {
//     int  rc;
     char *string;

     if( numargs != 3 )
        return( INVALID_ROUTINE );

     DBGMessage(args[0].strptr);
     DBGMessage(args[1].strptr);
     DBGMessage(args[2].strptr);

     string = malloc(strlen(args[3].strptr)+5);

     if(!string)
     {
        return( INVALID_ROUTINE );
     }
     strcpy(string,args[3].strptr);

     icqIPCConvertCodePage(atoi(args[0].strptr), atoi(args[1].strptr), string, -1);

     CopyString(string, retstr);

     free(string);

     return( VALID_ROUTINE );
  }

ULONG EXPENTRY rxICQQueryAvailable(    CHAR *name,ULONG numargs,RXSTRING args[],CHAR *queuename, RXSTRING *retstr )
{

     switch(numargs)
     {
     case 0:
        CopyInteger(icqIPCQueryAvailable(0), retstr );
        break;

     case 1:
        CopyInteger(icqIPCQueryAvailable(atoi(args[0].strptr)), retstr );
        break;

     default:
        return( INVALID_ROUTINE );
     }

     return( VALID_ROUTINE );
  }

ULONG EXPENTRY rxICQSendFile(CHAR *name, ULONG numargs, RXSTRING args[], CHAR *queuename, RXSTRING *retstr)
{
   if(numargs < 3)
      return( INVALID_ROUTINE );

    switch(numargs)
    {
    case 3:
       CopyInteger(icqIPCSendFile(atoi(args[0].strptr), atoi(args[1].strptr), args[2].strptr, NULL), retstr);
       break;

    case 4:
       CopyInteger(icqIPCSendFile(atoi(args[0].strptr), atoi(args[1].strptr), args[2].strptr, args[3].strptr), retstr);
       break;

    default:
       return( INVALID_ROUTINE );
    }
    return( VALID_ROUTINE );
}


BEGINREXXFUNCTION(rxICQMPTRequest)
     REQPARMS(3);
ENDREXXFUNCTION(icqIPCMPTRequest(INTVALUE(0), INTVALUE(1), INTVALUE(2)));

BEGINREXXFUNCTION(rxICQSetPeerInfo)

     struct hostent *host;
     long           ip;

     REQPARMS(5);

     host = gethostbyname(STRINGVALUE(2));

     if(host == NULL)
     {
        RETURN_VALID(-1);
     }

     ip = *( (long *) host->h_addr );
     DBGTracex(ip);

     if(!ip || ip == 0xFFFFFFFF)
        RETURN_INVALID();

ENDREXXFUNCTION(icqIPCSetPeerInfo(LONGVALUE(0), LONGVALUE(1), ip, INTVALUE(3), INTVALUE(4)));

BEGINREXXFUNCTION(rxICQPeerConnect)
     REQPARMS(2);
ENDREXXFUNCTION(icqIPCPeerConnect(LONGVALUE(0), LONGVALUE(1)));

BEGINREXXFUNCTION(rxICQAddUser)
     REQPARMS(2);
ENDREXXFUNCTION(icqIPCAddUser(LONGVALUE(0), LONGVALUE(1)));

BEGINREXXFUNCTION(rxICQOpenURL)
     REQPARMS(2);
ENDREXXFUNCTION(icqIPCOpenURL(LONGVALUE(0), STRINGVALUE(1)));

BEGINREXXFUNCTION(rxICQOSD)
     REQPARMS(2);
ENDREXXFUNCTION(icqIPCPluginRequest(LONGVALUE(0),"icqosd", STRINGVALUE(1)));

BEGINREXXFUNCTION(rxICQSendURL)
     REQPARMS(2);
ENDREXXFUNCTION(icqIPCOpenURL(LONGVALUE(0), STRINGVALUE(1)));


