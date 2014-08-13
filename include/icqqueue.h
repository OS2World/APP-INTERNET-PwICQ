/*
 * ICQQUEUE.H - pwICQ V2 IPC Calls
 */


#ifndef PWICQQUEUE_INCLUDED

 #define PWICQQUEUE_INCLUDED 2.0

/*---[ Commands ]----------------------------------------------------------------------------------------*/

 enum ICQCMD_TABLE
 {
    ICQCMD_SETMODE,             /* 00 Set online mode                              */
    ICQCMD_SHOW,                /* 01 Show main window                             */
    ICQCMD_HIDE,                /* 02 Hide main window                             */
    ICQCMD_PLUGIN,              /* 03 Send request to plugin                       */
    ICQCMD_MODEBYNAME,          /* 04 Set mode by name                             */
    ICQCMD_WRITELOG,            /* 05 Write an entry in logfile                    */
    ICQCMD_QUERYBUILD,          /* 06 Query the build number of core module        */
    ICQCMD_QUERYCONFIGSTRING,   /* 07 Query a string from the config file          */
    ICQCMD_EXEC,                /* 08 Execute command                              */
    ICQCMD_OPENFIRST,           /* 09 Open first message                           */
    ICQCMD_OPENMENU,            /* 10 Open popup menu                              */
    ICQCMD_POPUPUSERLIST,       /* 11 Open popup user list                         */
    ICQCMD_MPTREQUEST,          /* 12 Reserved                                     */
    ICQCMD_QUERYONLINEMODE,	    /* 13 Query online mode				               */
    ICQCMD_TRANSLATECODEPAGE,   /* 14 Translate codepage                           */
    ICQCMD_SETPEERINFO,         /* 15 Set peer info                                */
    ICQCMD_PEERCONNECT,         /* 16 Begin peer session with the user             */
    ICQCMD_SENDFILE,            /* 17 Send file to user                            */
    ICQCMD_ADDUSER,             /* 18 Add user to contact-list                     */
	ICQCMD_OPENURL,             /* 19 Open URL                                     */
	ICQCMD_SENDURL,             /* 20 Send URL                                     */

    ICQCMD_USER
 };


/*---[ Structs ]-----------------------------------------------------------------------------------------*/

 #pragma pack(1)

 typedef struct icqqueueblock
 {
    USHORT      szPrefix;                       /* Size of ICQQUEUEBLOCK (Version check)    */
    USHORT      sz;                             /* Size of datablock                        */
    UCHAR       cmd;                            /* Command type                             */
    USHORT      rc;                             /* Return code                              */
    USHORT      status;                         /* Status code                              */
 } ICQQUEUEBLOCK;

 struct icqqueuesetmode
 {
    ICQQUEUEBLOCK       h;
    ULONG               mode;
 };

 struct icqpluginipc
 {
    ICQQUEUEBLOCK       h;
    char                plugin[11];             /* Plugin name                  */
 };

 struct icqqueuemptblock
 {
    ICQQUEUEBLOCK       h;
    UCHAR               request;
    ULONG               uin;
 };

 struct icqpeerinfo
 {
    ICQQUEUEBLOCK       h;
    ULONG               uin;
    USHORT              peerVersion;
    ULONG               peerCookie;
    long                ip;
    USHORT              port;
 };


#ifdef linux

 typedef struct _icqquemessage
 {
    USHORT         sz;
    int            shmID;
    USHORT         shmSize;
 } ICQQUE_MESSAGE;

 typedef struct _icqqueblock
 {
    ULONG          uin;
    ICQQUE_MESSAGE msg;
 } ICQQUE_PACKET;

#endif

 #pragma pack()

/*---[ Functions in pwICQ core ]-------------------------------------------------------------------------*/

#ifdef HICQ
 void EXPENTRY icqProcessRemoteBlock(HICQ,ICQQUEUEBLOCK *);
#endif

/*---[ Functions in pwICQ IPC library ]------------------------------------------------------------------*/

 int   EXPENTRY icqIPCSendCommand(ULONG, ICQQUEUEBLOCK *);
 int   EXPENTRY icqIPCSetMode(ULONG,ULONG);
 int   EXPENTRY icqIPCSendCMD(ULONG, UCHAR);
 int   EXPENTRY icqIPCSetModeByName(ULONG,const char *);
 int   EXPENTRY icqIPCWriteLog(ULONG,const char *);
 ULONG EXPENTRY icqIPCQueryBuild(void);
 int   EXPENTRY icqIPCQueryInstance(ULONG *);
 int   EXPENTRY icqIPCQueryString(ULONG, const char *, int, char *);
 int   EXPENTRY icqIPCExecuteCommand(ULONG, const char *);
 int   EXPENTRY icqIPCShowPopupMenu(ULONG, USHORT);
 int   EXPENTRY icqIPCOpenFirstMessage(ULONG);
 int   EXPENTRY icqIPCShowUserList(ULONG);
 int   EXPENTRY icqIPCPluginRequest(ULONG,const char *, const char *);
 int   EXPENTRY icqIPCMPTRequest(ULONG, UCHAR, ULONG);
 ULONG EXPENTRY icqIPCQueryOnlineMode(ULONG,int,char *);
 int   EXPENTRY icqIPCConvertCodePage(ULONG, BOOL, UCHAR *, int);
 int   EXPENTRY icqIPCSetPeerInfo(ULONG,ULONG,long,USHORT, USHORT); /* User, UIN, ip, port, version */
 int   EXPENTRY icqIPCPeerConnect(ULONG, ULONG);
 ULONG EXPENTRY icqIPCQueryAvailable(ULONG);
 int   EXPENTRY icqIPCSendFile(ULONG,ULONG,const char *, const char *);
 int   EXPENTRY icqIPCAddUser(ULONG,ULONG);
 int   EXPENTRY icqIPCOpenURL(ULONG,const char *);
 int   EXPENTRY icqIPCSendURL(ULONG, const char *);

 #ifdef PWICQDEF_INCLUDED
    ICQSHAREMEM * EXPENTRY icqIPCGetSharedBlock(void);
    int           EXPENTRY icqIPCReleaseSharedBlock(ICQSHAREMEM *);
 #endif

#endif

