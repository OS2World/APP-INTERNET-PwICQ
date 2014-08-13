/*
 * ICQDEFS.H - pwICQ Toolkit definitions
 */

#ifndef PWICQDEF_INCLUDED

   #define PWICQDEF_INCLUDED 2.0

   #define PWICQVERSION         "2.00"
   #define PWICQVERSIONID       0x00002000

   #define PWICQICONBARSIZE     53
   #define PWICQGROUPCOUNTER    18

   #define SYSTEM_UIN           1000

   #ifndef ICQMSGWIN

      #ifdef linux
         #define ICQMSGWIN void *
      #endif

      #ifdef __OS2__
         #define ICQMSGWIN HWND
      #endif

   #endif

   #ifdef __OS2__
      #include <os2.h>
      #define sysSleep(x) DosSleep(x*1000)
      #define OS2_TYPEDEFS_INCLUDED 1
      #ifndef DLLENTRY
         #define DLLENTRY EXPENTRY
      #endif

      #define PATH_SEPARATOR '\\'

   #endif

   #ifdef BEOS
      #include <threads.h>
   #endif

   #ifdef linux

      #include <pthread.h>

      #ifndef HWND
         #define HWND     void *
      #endif

      #ifndef DLLENTRY
         #define DLLENTRY
      #endif

      #define sysSleep(x) usleep(x*1000000)
      #define PATH_SEPARATOR '/'

   #endif

   #ifndef OS2_TYPEDEFS_INCLUDED

      #define OS2_TYPEDEFS_INCLUDED 1

      typedef unsigned short    USHORT;
      typedef unsigned char     UCHAR;
      typedef unsigned long     ULONG;
      typedef char              CHAR;
	
	  #ifndef BOOL
	     #define BOOL unsigned char
      #endif

      typedef void *            HMODULE;

      #ifndef FALSE
         #define FALSE          0x00
      #endif

      #ifndef TRUE
         #define TRUE          !FALSE
      #endif

      #define EXPENTRY
      #define DLLENTRY
      #define _System
      #define ICQCALLBACK

      #define sysSleep(x) usleep(x*1000000)

   #else

      #define ICQCALLBACK _System

   #endif

   #ifndef ICQAPI
      #define ICQAPI DLLENTRY
   #endif

   #include <time.h>

   #ifndef PWICQKERNEL_INCLUDED
      typedef void * HPLUGIN;
      typedef void * HSEARCH;
   #endif

   #pragma pack(1)

   /*---[ Message fields ]--------------------------------------------------------------*/

   #define      MAX_MSG_SIZE            32768

   #define      ICQ_FIELD_SEPARATOR     '\x01'

   /*---[ CORE service flags ]----------------------------------------------------------*/

   #define      ICQSRCV_CORE      0x00000001       /* Main Core service                 */
   #define      ICQSRVC_C2S       0x00000002       /* Client<->Server service           */

   /*---[ CORE online flags ]-----------------------------------------------------------*/

   #define      ICQF_STARTING     0x00000001
   #define      ICQF_CONNECTING   0x00000002
   #define      ICQF_CONTACTED    0x00000004
   #define      ICQF_CONTACTLIST  0x00000008
   #define      ICQF_ONLINE       0x00000010

   /*---[ Standard icon codes ]---------------------------------------------------------*/

   #define ICQICON_ONLINE              5
   #define ICQICON_AWAY                6
   #define ICQICON_NA                 40
   #define ICQICON_BUSY               12
   #define ICQICON_DND                42
   #define ICQICON_FREEFORCHAT        13

   #define ICQICON_INACTIVE           10
   #define ICQICON_OFFLINE             7
   #define ICQICON_CONNECTING         39
   #define ICQICON_ANIMATE            18
   #define ICQICON_DISCONNECT         38
   #define ICQICON_SEARCH             11
   #define ICQICON_UPDATE			  16
   #define ICQICON_SHUTDOWN           17
   #define ICQICON_REMOVE             17

   #define ICQICON_USERINFO           46
   #define ICQICON_ABOUT              50

   #define ICQICON_MESSAGERECEIVED    26
   #define ICQICON_MESSAGESENT        37
   #define ICQICON_FILE               47

   #define ICQICON_CONFIGURE          27
   #define ICQICON_USERCONFIG         31

   #define ICQICON_UNREADMESSAGE       0
   #define ICQICON_READMESSAGE         1
   #define ICQICON_UNREADURL           2
   #define ICQICON_READURL             3

   /*---[ Configuration window sections ]-----------------------------------------------*/

   enum CFGWINDOW_SECTIONS
   {
      CFGWIN_ROOT,
      CFGWIN_NETWORK,
      CFGWIN_MODE,
      CFGWIN_MULTIMEDIA,
      CFGWIN_USERS,
      CFGWIN_OPTIONS,
      CFGWIN_SECURITY,
      CFGWIN_MISC,
      CFGWIN_ADVANCED,
      CFGWIN_ABOUT,
      CFGWIN_INFO,
      CFGWIN_EVENTS,

      CFGWIN_USER
   };

   /*---[ OS/2 Window messages ]--------------------------------------------------------*/

   #ifdef WM_USER

      #define WMICQ_EVENT          WM_USER+5000    /* pwICQ event                         */

      #define WMICQ_SYSTEMEVENT    WM_USER+3000
      #define WMICQ_USEREVENT      WM_USER+3001
      #define WMICQ_GUIEVENT       WM_USER+3002


      #ifndef WS_TOPMOST
         #define WS_TOPMOST  0x00200000
      #endif

   #endif

   /*---[ Events and messages ]---------------------------------------------------------*/

   enum STARTUPMESSAGES
   {
      ICQSTARTUP_ENVIRONMENT,                   /* Configuring base environment         */

#ifdef __OS2__
      ICQSTARTUP_MPTS,                          /* Verifying network service level      */
#endif

      ICQSTARTUP_PARMS,                         /* Processing command line parameters   */
      ICQSTARTUP_CONFIG,                        /* Loading configuration file           */
      ICQSTARTUP_FILTERS,                       /* Loading filter files                 */
      ICQSTARTUP_PLUGINS,                       /* Loading plugins                      */
      ICQSTARTUP_PLUGINSOK,                     /* Plugin folder loaded                 */

      ICQSTARTUP_STARTINGSKIN,                  /* Starting skin manager                */
      ICQSTARTUP_LOADINGSKIN,                   /* Loading skin                         */
      ICQSTARTUP_SKINLOADED,                    /* Skin loaded                          */

      ICQSTARTUP_LOADINGUSERS,                  /* Loading contact-list                 */
      ICQSTARTUP_STARTINGPLUGINS,               /* Starting plugins                     */
      ICQSTARTUP_COREOK,                        /* Main core is ready                   */

      ICQSTARTUP_USER
   };

   #define ICQMNU_USER		0
   #define ICQMNU_MODES 	1
   #define ICQMNU_SYSTEM	2
   #define ICQMNU_COUNTER	3

   enum ICQMESSAGES                                                       /* Used in icqAbend(i,m) */
   {
      ICQMSG_INVALIDPASSWORD,                    /* Bad password, can't connect         */
      ICQMSG_MEMORYERROR,                        /* Allocation memory error             */


      ICQMESSAGE_USER
   };

   enum ICQEVENTS
   {
        ICQEVENT_STARTUP,                          /*  0 Startup process complete                    */
        ICQEVENT_STOPPING,                         /*  1 Stopping                                    */
        ICQEVENT_CONTACT,                          /*  2 Server/User contacted                       */
        ICQEVENT_CONFIRMED,                        /*  3 Confirmed by the other side                 */
        ICQEVENT_ONLINE,                           /*  4 Online                                      */
        ICQEVENT_OFFLINE,                          /*  5 Offline                                     */
        ICQEVENT_CHANGED,                          /*  6 Status change                               */
        ICQEVENT_PLISTCLEAR,                       /*  7 All pending packets sent                    */
        ICQEVENT_IPCHANGED,                        /*  8 External IP Adress was changed              */
        ICQEVENT_MESSAGECHANGED,                   /*  9 Message queue was changed                   */
        ICQEVENT_GOAWAY,                           /* 10 Server said 'Go Away'                       */
        ICQEVENT_RETRY,                            /* 11 Server said 'Retry in a few moments'        */
        ICQEVENT_USERADDED,                        /* 12 New user added in the list                  */
        ICQEVENT_ICONCHANGED,                      /* 13 Icon has changed                            */
        ICQEVENT_POSCHANGED,                       /* 14 Position changed                            */
        ICQEVENT_CLOSING,                          /* 15 Main window is closing                      */
        ICQEVENT_UPDATED,                          /* 16 Information changed                         */
        ICQEVENT_CONNECTING,                       /* 17 Connecting with the server                  */
        ICQEVENT_IGNORED,                          /* 18 Message was ignored (blacklist,badword,etc) */
        ICQEVENT_BEGINSEARCH,                      /* 19 Search started                              */
        ICQEVENT_ENDSEARCH,                        /* 20 Search complete                             */
        ICQEVENT_LOGONCOMPLETE,                    /* 21 Logon process complete                      */
        ICQEVENT_FINDINGSERVER,                    /* 22 Finding server to connect (gethostbyname)   */
        ICQEVENT_SERVERERROR,                      /* 23 Disconnecting because of some server error  */
        ICQEVENT_REJECTED,                         /* 24 User rejected from the server               */
        ICQEVENT_DELETED,                          /* 25 Removed                                     */
        ICQEVENT_ABEND,                            /* 26 Abnormal ending                             */
        ICQEVENT_SECONDARY,                        /* 27 Secondary startup process complete          */
        ICQEVENT_BEGINPLAY,                        /* 28 Starting sound event                        */
        ICQEVENT_ENDPLAY,                          /* 29 Stopping sound event                        */
        ICQEVENT_MODIFIED,                         /* 30 Information was changed                     */
        ICQEVENT_HIDE,                             /* 31 Hide user/Window                            */
        ICQEVENT_SHOW,                             /* 32 Show user/Window                            */
        ICQEVENT_WILLCHANGE,                       /* 33 User handle will be changed                 */
        ICQEVENT_SETOFFLINE,                       /* 34 Set user icon to offline mode               */
        ICQEVENT_TIMER,                            /* 35 One per second                              */
        ICQEVENT_SAVE,                             /* 36 Save information                            */
        ICQEVENT_CANTREMOVE,                       /* 37 Can't remove                                */
        ICQEVENT_LOADUSERS,                        /* 38 Load user list                              */
        ICQEVENT_SEARCHTIMEOUT,                    /* 39 Search timeout                              */
        ICQEVENT_ABORTSEARCH,                      /* 40 Search aborted                              */
        ICQEVENT_PEERBEGIN,                        /* 41 Peer session registered                     */
        ICQEVENT_PEEREND,                          /* 42 Peer session deregistered                   */
        ICQEVENT_CONNCTOK,                         /* 43 Connection accepted                         */
        ICQEVENT_CONNCTNOK,                        /* 44 Connection refused                          */
        ICQEVENT_REQCREATED,                       /* 45 Request was created                         */
        ICQEVENT_REQDELETED,                       /* 46 Request was deleted                         */
        ICQEVENT_REQACCEPTED,                      /* 47 Request was accepted                        */
        ICQEVENT_REQREFUSED,                       /* 48 Request was refused                         */
        ICQEVENT_CANTCONNECT,                      /* 49 Cant connect                                */
        ICQEVENT_TERMINATE,                        /* 50 Close system                                */
		ICQEVENT_CALLBACK,                         /* 51 Serialize callbacks                         */
        ICQEVENT_MESSAGEADDED,                     /* 52 Message was added in the queue              */
		ICQEVENT_POPUP,                            /* 53 Show popup window                           */


        ICQEVENT_USER
   };

   enum ICQSEARCHEVENTS                            /* Search Events */
   {
        ICQSEARCH_BASIC,
        ICQSEARCH_MORE,

        ICQSEARCH_USER
   };

   #define icqSearchEvent(i,u,e) icqEvent(i, u,'F', e,0)

   /*---[ MSG Types ]--------------------------------------------------------------*/

   #define MSG_NORMAL     0x0001                   /* Normal                       */
   #define MSG_CHAT       0x0002                   /* Chat Request                 */
   #define MSG_FILE       0x0003                   /* File transfer request        */
   #define MSG_URL        0x0004                   /* URL                          */
   #define MSG_REQUEST    0x0006                   /* Authorization request        */
   #define MSG_REFUSED    0x0007                   /* Authorization denied         */
   #define MSG_ACCEPTED   0x0008                   /* Authorized                   */
   #define MSG_SYSREQ     0x0009                   /* System request               */
   #define MSG_ADD        0x000C                   /* "You were added"             */
   #define MSG_PAGER      0x000D                   /* ICQ-Pager message            */
   #define MSG_MAIL       0x000E                   /* Mail Express                 */
   #define MSG_CONTACT    0x0013                   /* Contact-list message         */
   #define MSG_SMS        0x001A
   #define MSG_GREET      0x001A

   #define MSG_AUTHORIZED MSG_ACCEPTED

   /*---[ IPC resources ]----------------------------------------------------------*/

#ifdef __OS2__
   #define PWICQ_SHAREMEM         "\\SHAREMEM\\pwICQ2"
   #define PWICQ_MAX_INSTANCES    10
#endif

#ifdef linux
   #define PWICQ_SHAREMEM         "/tmp/pwicq.shared"
   #define PWICQ_MAX_INSTANCES    5
#endif

#ifdef WIN32
   #define PWICQ_MAX_INSTANCES    5
#endif

   typedef struct icqinstance
   {
      ULONG uin;                                   /* Instance's UIN               */
      ULONG mode;                                  /* Instance's online mode       */
      int   pid;                                   /* Instance's PID               */

      UCHAR modeIcon;                              /* Current online-mode icon     */
      UCHAR eventIcon;                             /* First pending message icon   */

      ULONG eventCounter;                          /* Event counter                */

      ULONG flags;
      #define ICQINSTANCEFLAG_READY 0x00000001

   } ICQINSTANCE;

   typedef struct icqsharemem
   {
      USHORT            sz;                        /* Size of the structure        */
      USHORT            si;                        /* Size of ICQINSTANCE          */
      USHORT            instances;                 /* Running instances            */
      ULONG             reserved;
      ICQINSTANCE       i[PWICQ_MAX_INSTANCES];    /* Current active instances     */
   } ICQSHAREMEM;

   /*---[ Mode information ]--------------------------------------------------------------*/

   typedef struct modetable
   {
      ULONG       mode;
      USHORT      icon;
      const char  *descr;
   } ICQMODETABLE;

   /*---[ Mode Flags used by icqSet/GetCaps ]---------------------------------------------------*/

   #define ICQMF_ONLINE         0x80000000

   #define ICQMF_ASKMESSAGE     0x00000001      /* Ask user for the away message                */
   #define ICQMF_AUTOMODE       0x00000002      /* Enable automatic mode changes                */
   #define ICQMF_AUTOOPEN       0x00000004      /* Enable auto-open                             */
   #define ICQMF_SOUND          0x00000008      /* Enable sounds                                */
   #define ICQMF_HELPERS        0x00000030      /* Enable helpers                               */
   #define ICQMF_NORMALHELPER   0x00000010      /* Enable normal helpers                        */
   #define ICQMF_EXTENDEDHELPER 0x00000020      /* Enable extended helpers                      */
   #define ICQMF_HISTORY        0x00010000      /* Enable user-logs/History                     */
   #define ICQMF_RTF            0x00020000      /* Can show RTF messages                        */
   #define ICQMF_FILE           0x00040000      /* Can do file transfers                        */
   #define ICQMF_EXTENDEDMENU   0x00080000      /* Can manage extended menus                    */

   /*---[ User Information ]--------------------------------------------------------------------*/

   #ifndef PEERDATA
      #define PEERDATA void
   #endif

   typedef struct icquser
   {
      USHORT            sz;                     /* Structure's size                             */

      ULONG             uin;

      UCHAR             type;                   /* User type                                    */
      #define USRT_ICQ          1               /* This user is using ICQ's protocol            */

      ULONG             flags;
      #define USRF_ONLIST       0x00000001      /* Send the user in the contact-list            */
      #define USRF_INVISIBLE    0x00000002      /* Always invisible for this user               */
      #define USRF_VISIBLE      0x00000004      /* Always visible for this user                 */
      #define USRF_AUTOOPEN     0x00000008      /* Auto-open messages from this user            */
      #define USRF_IGNORE       0x00000010      /* Ignore messages from this user               */
      #define USRF_TEMPORARY    0x00000020      /* User not in the contact-list file            */
      #define USRF_REFRESH      0x00000040      /* Refresh user information                     */
      #define USRF_WAITING      0x00000080      /* Waiting for authorization                    */
      #define USRF_AUTHORIZE    0x00000100      /* Authorization needed                         */
      #define USRF_SEARCHING    0x00000200      /* User information was requested               */
      #define USRF_HIDEONLIST   0x00000400      /* Hide user in the contact-list                */
      #define USRF_REJECTED     0x00000800      /* User was rejected (Spammer?)                 */
      #define USRF_EXTENDED     0x00001000      /* User has extended information                */
      #define USRF_BLINK        0x00002000      /* Blink user in the contact-list               */
      #define USRF_HISTORY      0x00004000      /* Save user history/log                        */
      #define USRF_DISABLED     0x00008000      /* Disabled in the contact-list                 */
      #define USRF_ONLINE       0x00010000      /* User is online                               */
      #define USRF_CANRECEIVE   0x00020000      /* User can receive files                       */
      #define USRF_CANCHAT      0x00040000      /* User can receive chat requests               */
      #define USRF_FILEDIR      0x00080000      /* Save received files in user's path           */
      #define USRF_AUTOACCEPT   0x00100000      /* Always accept files from this user           */


      #define USRF_DELETED      0x80000000      /* User is deleted                              */


      #define USRF_CACHED       USRF_DISABLED   /* Only for caching user information            */

      UCHAR             pos;                    /* Ordering indicator                           */

      ULONG             mode;                   /* Current user mode                            */
      USHORT            onlineIcon;             /* Current user online icon                     */
      USHORT            modeIcon;               /* Current user mode icon                       */
      USHORT            offlineIcon;            /* Offline user icon                            */
      USHORT            eventIcon;              /* Current user event icon                      */
      USHORT            index;                  /* Position in the list */

      USHORT            idle;                   /* IDLE timer                                   */
      UCHAR             wndCounter;             /* Window counter                               */

      /* C2S Protocol manager information */
      ULONG             c2sFlags;               /* Reserved for C2S Protocol Manager            */
      USHORT            c2sGroup;               /* C2S's User group number                      */
      USHORT            c2sIDNumber;            /* C2S's User id                                */
      ULONG             onlineSince;
      ULONG             memberSince;

      /* Peer protocol manager information */
      ULONG             peerVersion;            /* Version of the Peer protocol for this user   */
      ULONG		        peerCookie;		        /* Peer protocol cookie                         */

      /* Skin manager information */
      void              *listItem;              /* Reserved for the skin manager                */

      /* User timers */
      ULONG             lastOnline;             /* When the user goes online for the last time  */
      ULONG             lastMessage;            /* When the last message was received           */
      ULONG             lastAction;             /* Last user activity                           */

      /* User information */
      ULONG             clientVersion;          /* Wich ICQ Client the user is using?           */
      UCHAR             sex;                    /*                                              */
      UCHAR             age;                    /*                                              */
      ULONG             groupMasc;              /* User groups (bitmask)                        */
      #define USRG_MASC 0x0007FFFF              /* Regular user group masc                      */
      #define USRG_MPT  0x80000000              /* XPTO's filter                                */


   } ICQUSER;

   #ifndef PWICQKERNEL_INCLUDED
      typedef ICQUSER * HUSER;
   #endif

   #ifndef HICQ
      #define HICQ void *
   #endif

   /*---[ Request manager ]--------------------------------------------------------------*/

   typedef struct requestmgr
   {
      USHORT            sz;

      /* Request accepted/refused HICQ, ID, DataBlock, SESSION, PORT, PARAM */
      int               (* _System accepted)(HICQ, ULONG, void *, ULONG, int, void *);
      int               (* _System refused )(HICQ, ULONG, void *, ULONG, int, void *);

      int               (* _System accept  )(HICQ, ULONG, void *, ULONG, void *, const char *);
      int               (* _System refuse  )(HICQ, ULONG, void *, ULONG, void *, const char *);

   } REQUESTMGR;

   /*---[ Peer protocol manager ]--------------------------------------------------------*/

   typedef struct peerprotmgr                    /* Peer Protocol manager descriptor             */
   {
      USHORT            sz;                      /* Structure size                               */
      USHORT            version;                 /* Protocol version                             */
      ULONG             build;                   /* Module build                                 */
      const char        *descr;                  /* Peer protocol manager description            */

      USHORT            maxMsgSize;              /* Maximum message size                         */

      ULONG             flags;                   /* Peer protocol flags                          */

      /* Session management */
      USHORT            (* _System queryStatus)(HICQ, HUSER, PEERDATA *);
      int               (* _System startListener)(HICQ, PEERDATA *);
      int               (* _System stopListener)(HICQ, PEERDATA *);

      /* Send message to user */
      USHORT            (* _System sendMessage)(HICQ, HUSER, USHORT, const char *, ULONG *, PEERDATA *);
      USHORT            (* _System sendFileReq)(HICQ, HUSER, PEERDATA *, const char *, int, const char *, ULONG );
      USHORT            (* _System sendChatReq)(HICQ, HUSER, PEERDATA *, const char *);

      /* Peer cookie */
      ULONG		        (* _System setCookie)(HICQ,void *, ULONG);
      ULONG		        (* _System getCookie)(HICQ,void *);

      /* Network actions */
      long	            (* _System getPort)(HICQ, void *);
      long	            (* _System getAddress)(HICQ, void *);

      /* Session management */
      int               (* _System beginSession)(HICQ, void *, HUSER, int);
      int		        (* _System closeAll)(HICQ,ULONG);

   } PEERPROTMGR;

   /*---[ Search ]-----------------------------------------------------------------------*/

   #define ICQSEARCHRESPONSE_ITEMS 4

   typedef struct icqSearchResponse
   {
      USHORT    id;                                     /* sizeof(ICQSEARCHRESPONSE)    */
      USHORT    sz;                                     /* size of text[]               */
      ULONG     uin;                                    /* User uin                     */
      ULONG     flags;
      #define   ICQSR_ALWAYSAUTH        0x00000001      /* Always authorize             */
      #define   ICQSR_DLGFLAG           0x00000002      /* for Dialog Manager plugin    */
      #define   ICQSR_ONLIST            0x00000004      /* Already in contact-list      */
      #define   ICQSR_CACHED            0x00000008      /* Already in cache-list        */
      #define   ICQSR_FAILED            0x00000010      /* Search failed                */

      #define   ICQSR_LAST              0x80000000      /* Is the last result           */
      #define   ICQSR_BEGIN             0x40000000      /* Begin search                 */
      #define   ICQSR_END               0x20000000      /* End search                   */

      UCHAR     sex;                                    /* User sex                     */
      UCHAR     age;                                    /* User age                     */

      UCHAR     status;

      USHORT    offset[ICQSEARCHRESPONSE_ITEMS];                                /* Results offsets              */
      char      text[1];                                /* The results                  */

   } ICQSEARCHRESPONSE;

   #define getSearchNickname(x)  (x->text+x->offset[0])
   #define getSearchFirstname(x) (x->text+x->offset[1])
   #define getSearchLastname(x)  (x->text+x->offset[2])
   #define getSearchEMail(x)     (x->text+x->offset[3])

   typedef int (_System * SEARCHCALLBACK)(HICQ, ULONG, USHORT, HWND, const ICQSEARCHRESPONSE *);

   /*---[ Server protocol manager ]------------------------------------------------------*/

   typedef struct c2sprotmgr                    /* Protocol manager descriptor                  */
   {
      USHORT             sz;                    /* Structure size                               */
      USHORT             version;               /* Protocol version                             */
      ULONG              build;                 /* Module build                                 */
      const char         *descr;                /* Protocol manager description                 */
      USHORT             maxSize;               /* Maximum message size                         */

      /* Mode button table */
      const ICQMODETABLE *modeButton;

      /* Set the current online mode */
      ULONG             (* _System setMode)(HICQ,ULONG,ULONG);

      /* Send command to the server */
      int               (* _System sendCmd)(HICQ, unsigned short, void *, int);

      /* Request basic user information (0=Ok) */
      HSEARCH           (* _System requestUserUpdate)(HICQ, ULONG, int);

      /* Request Complete user information (0=Ok) */
      int               (* _System requestUserInfo)(HICQ, ULONG, int);

      /* Send message to one user */
      USHORT            (* _System sendMessage)(HICQ, ULONG, USHORT, const char *, ULONG *);

      /* Send confirmation message */
      int               (* _System sendConfirmation)(HICQ, ULONG, USHORT, BOOL, char *);

      /* Contact-list management */
      int               (* _System addUser)(HICQ, HUSER);
      int               (* _System delUser)(HICQ, HUSER);

      /* Query mode icon */
      USHORT            (* _System queryModeIcon)(HICQ, ULONG);

      /* Management */
      int               (* _System setPassword)(HICQ,const char *, const char *);

      /* Is online with the server ?  */
      int               (* _System isOnline)(HICQ);

      /* Searchs */
      int               (* _System searchByICQ)(HICQ, ULONG, HWND, SEARCHCALLBACK);
      int               (* _System searchByMail)(HICQ, const char *, HWND, SEARCHCALLBACK);
      int               (* _System searchRandom)(HICQ, USHORT, HWND, SEARCHCALLBACK);
      int               (* _System searchByInformation)(HICQ,HWND, const char *, const char *,const char *, const char *,BOOL, SEARCHCALLBACK );

   } C2SPROTMGR;

   /*---[ User DB manager ]--------------------------------------------------------------------*/

   typedef struct userdbmgr
   {
      USHORT    sz;                             /* Struct size (for checking) */
      int       (* _System writeUserDB)(HICQ, HUSER, const char *, char *);
      int       (* _System readUserDB)(HICQ, HUSER, const char *, const char *, char *, int);
      int       (* _System deleteUserDB)(HICQ, HUSER);
   } USERDBMGR;

   /*---[ Event processors ]--------------------------------------------------------*/

   struct icqEventProcessors
   {
      UCHAR  type;
      USHORT event;
      int    (* _System exec)(void *,ULONG,ULONG);
   };

   #define icqDeclareSystemEventProcessor(e)     int _System icqSysEvent_##e(HICQ,ULONG,ULONG)
   #define icqSystemEventProcessor(e)            { 'S', e, (int(* _System)(void *,ULONG,ULONG)) icqSysEvent_##e }
   #define icqBeginSystemEventProcessor(m,i,u,p) int _System icqSysEvent_##m(HICQ i,ULONG u,ULONG p)

   #define icqDeclareUserEventProcessor(e)       int _System icqUserEvent_##e(HICQ,ULONG,ULONG)
   #define icqUserEventProcessor(e)              { 'U', e, (int(* _System )(void *,ULONG,ULONG)) icqUserEvent_##e }
   #define icqBeginUserEventProcessor(m,i,u,p)   int _System icqUserEvent_##m(HICQ i,ULONG u,ULONG p)

   #define icqDeclareGuiEventProcessor(e)        int _System icqGuiEvent_##e(HICQ,ULONG,ULONG)
   #define icqGuiEventProcessor(e)               { 'g', e, (int(* _System )(void *,ULONG,ULONG)) icqGuiEvent_##e }
   #define icqBeginGuiEventProcessor(m,i,u,p)    int _System icqGuiEvent_##m(HICQ i,ULONG u,ULONG p)

   /*---[ Message management ]------------------------------------------------------------*/

   typedef struct icqmsg                        /* Message descriptor                           */
   {
      USHORT           sz;                      /* Structure size (compatibility check)         */

      USHORT           type;                    /* Message type                                 */

      ULONG            msgFlags;                /* Message flags                                */
      #define MSGF_AUTHORIZE     0x00000001
      #define MSGF_IGNORE        0x00000002
      #define MSGF_CANSAVE       0x00000004
      #define MSGF_BLACKLIST     0x00000008     /* Blacklisted User/IP address                  */
      #define MSGF_EMPTY         0x00000010     /* Message without text                         */
      #define MSGF_ACCEPTED      0x00000020     /* Message was auto-accepted                    */
      #define MSGF_OPEN          0x00000040     /* Open message edit window                     */
      #define MSGF_UNHIDE        0x00000080     /* Unhide user                                  */
      #define MSGF_RTF           0x00000100     /* Message is in RTF format                     */
      #define MSGF_SYSTEM        0x00000200     /* Message added to the system queue            */
      #define MSGF_URGENT   	 0x00000400     /* Urgent message                               */
      #define MSGF_OUTPUT        0x00000800
      #define MSGF_USECHATWINDOW 0x00001000     /* Use chat window                              */
      #define MSGF_READ          0x00002000     /* Message was read, remove it                  */

      ULONG               sequenceNumber;       /* Used by protocol manager                     */
      time_t              msgTime;

      const struct msgmgr *mgr;                 /* Message manager description                  */

      USHORT              msgSize;              /* Message size                                 */

   } ICQMSG;

   #define HMSG ICQMSG *

/*
   #ifndef MSGEDITWINDOW
      #ifdef __OS2__
         #define MSGEDITWINDOW HWND
      #else
         #define MSGEDITWINDOW void *
      #endif
   #endif
*/

   #ifndef hWindow
      #ifdef __OS2__
         #define hWindow HWND
      #else
         #define hWindow void *
      #endif
   #endif

   typedef struct dlgmgr DLGMGR;

   typedef struct _dialogcalls
   {
      USHORT       sz;
      ULONG        build;

	  /* Frame Window/Dialogs */
      int          (* _System setTitle)(hWindow,const char *);
      int          (* _System setVisible)(hWindow, USHORT, BOOL);
      int          (* _System getVisible)(hWindow, USHORT);
      int          (* _System setEnabled)(hWindow, USHORT, BOOL);
      int          (* _System setString)(hWindow,USHORT,const char *);
      int          (* _System getString)(hWindow,USHORT,int,char *);
      int          (* _System setCheckBox)(hWindow,USHORT,BOOL);
      int          (* _System getCheckBox)(hWindow,USHORT);
      int          (* _System setSpinButton)(hWindow,USHORT,int,int,int);
      int          (* _System getSpinButton)(hWindow,USHORT);

      void         (* _System loadString)(hWindow, USHORT, USHORT);
      void         (* _System setTime)(hWindow, USHORT, time_t);
	  void         (* _System setEditable)(hWindow, USHORT, BOOL);
	  void         (* _System setIcon)(hWindow,USHORT,USHORT);
	  void         (* _System setTextLimit)(hWindow,USHORT,USHORT);
	  void         (* _System listBoxInsert)(hWindow,USHORT,ULONG,const char *,BOOL);
	
	  /* Frame Window/Dialogs + Configuration files */
	  void         (* _System readCheckBox)(hWindow,USHORT,const char *,BOOL);
	  void         (* _System readRadioButton)(hWindow,USHORT,const char *, USHORT, USHORT);
	  void         (* _System readString)(hWindow,USHORT,const char *,const char *);
	
	  void         (* _System writeCheckBox)(hWindow,USHORT,const char *);
	  void         (* _System writeRadioButton)(hWindow,USHORT,const char *, USHORT);
	  void         (* _System writeString)(hWindow,USHORT,const char *);

      /* Dialog box only */
      void *       (* _System getUserData)(hWindow);

      /* Miscellaneous */	
      int          (* _System queryClipboard)(HICQ, int, char *);
      int          (* _System populateEventList)(HICQ, hWindow, USHORT);
      int          (* _System selectFile)(HWND, USHORT, BOOL, const char *, const char *, const char *, char *, int (* _System)(const struct _dialogcalls *,HWND,HICQ,char *));
      int          (* _System setBitmap)(HICQ, HWND, HMODULE, USHORT, const char *);
     	
	  /* Message dialog only */
      int          (* _System setMessage)(hWindow, USHORT, BOOL, HMSG);
      int          (* _System loadUserFields)(hWindow);
      void         (* _System setReply)(hWindow, USHORT);

      /* Configuration boxes only */
      int          (* _System insertPage)(hWindow,USHORT,hWindow,USHORT,const char *);
      hWindow      (* _System loadPage)(hWindow, HMODULE, USHORT, const struct dlgmgr *, USHORT);

      /* Resource management */
      hWindow      (* _System loadDialog)(HICQ,hWindow,HMODULE,USHORT,const struct dlgmgr *);

   } DIALOGCALLS;

   #define DLGHELPER     DIALOGCALLS
   #define MSGEDITHELPER DIALOGCALLS
   #define DLGINSERT     DIALOGCALLS

   #define DECLARE_SIMPLE_DIALOG_MANAGER(n,l,s) const DLGMGR n = {sizeof(DLGMGR),sizeof(DLGHELPER),0,NULL,l,s,NULL,NULL,NULL,NULL,NULL,NULL}

   struct dlgmgr
   {
      USHORT    sz;
      USHORT    szCalls;	  /* Size of DIALOGCALLS, for version checking */
      USHORT    userDataSize; /* Size of user data block                   */

      int       (* _System initialize)( const DIALOGCALLS *, HWND, HICQ, ULONG, char *);
      int       (* _System load)(       const DIALOGCALLS *, HWND, HICQ, ULONG, char *);
      int       (* _System save)(       const DIALOGCALLS *, HWND, HICQ, ULONG, char *);
      int       (* _System cancel)(     const DIALOGCALLS *, HWND, HICQ, ULONG, char *);
      void      (* _System event)(      const DIALOGCALLS *, HWND, HICQ, ULONG, char,USHORT,char *);
      void      (* _System selected)(   const DIALOGCALLS *, HWND, HICQ, ULONG, USHORT, ULONG, char *);
      int       (* _System click)(      const DIALOGCALLS *, HWND, HICQ, ULONG, USHORT, char * );
      int       (* _System changed)(    const DIALOGCALLS *, HWND, HICQ, ULONG, USHORT, USHORT, char * );

      int       (* _System sysbtn)(     const DIALOGCALLS *, HWND, HICQ, ULONG, HWND, USHORT, SEARCHCALLBACK, char *);

   };


   /* Send message 0 = OK */
   typedef int (_System MSGSENDER)(const DIALOGCALLS *, hWindow, HICQ, ULONG);

   /* Format message dialog window */
   typedef int (_System MSGFORMATTER)(const DIALOGCALLS *, hWindow, HICQ, ULONG, USHORT, BOOL, HMSG);

   /* Button processor */
   typedef BOOL (_System MSGBUTTON)(const DIALOGCALLS *, hWindow, HICQ, ULONG, USHORT);

   typedef struct msgmgr                        /* Message manager descriptor                   */
   {
      USHORT            sz;
      UCHAR             type;                   /* Message type                                 */

      USHORT            reply;                  /* type of reply for this kind of message       */
                                                /* 0xFFFF means *no reply*                      */

      ULONG             flags;                  /* Message processor flags                      */
      #define ICQMSGMF_LOADUSERFIELDS   0x00000001
      #define ICQMSGMF_SYSTEMMESSAGE    0x00000002
      #define ICQMSGMF_REQUESTMESSAGE   0x00000004
      #define ICQMSGMF_DISABLESEND      0x00000008
      #define ICQMSGMF_USECHATWINDOW    0x00000010
      #define ICQMSGMF_URLCOMPATIBLE    0x00000020
      #define ICQMSGMF_SENDTO           0x00000040

      USHORT            icon[2];                /* 0 = Unread, 1 = Read                         */

      /* Message pre-processor */
      void               (* _System preProcess)(HICQ, UCHAR *, ICQMSG *);

      /* Query the message title */
      const char *      (* _System title)(HICQ, HUSER, ULONG, BOOL, ICQMSG *, char *, int);

      /* format message fields */
      void              (* _System formatLog)(HICQ, BOOL, ICQMSG *, char *, int);
      int               (* _System getField)(HICQ, ICQMSG *, int, int, char *);

      /* View/edit message, if not defined the internal MSG Edit box is called */
      int               (* _System edit)(HICQ, ULONG, BOOL, ICQMSG *, const char *, const char *);

      /* Show/Hide default msg-edit window controls, fill values from the message */
      MSGFORMATTER      *formatWindow;

      /* Button override, return FALSE if the button wasn't processed */
      MSGBUTTON         *action;

      /* Read default message dialog controls, format packet and send */
      MSGSENDER         *send;

      /* Send pre-formated text */
      int               (* _System sendText)(HICQ,ULONG,const char *);

      /* Executes the send to... button */
      void              (* _System sendTo)(HICQ, ULONG, const char *, const char *);

      /* Called when the window is receiving another message */
      void              (* _System exit)(const DIALOGCALLS *, hWindow, HICQ, ULONG, USHORT, USHORT);

      /* Event processor for the message-edit window */
      void              (* _System event)(const DIALOGCALLS *, hWindow, HICQ, ULONG, char, USHORT, ULONG);

   } MSGMGR;

   typedef int (_System SENDTOCALLBACK)(HICQ, ULONG, const MSGMGR *, const char *, const char *);

   /*---[ Skin manager ]-------------r----------------------------------------------*/

   /* Menu callback function: ICQ Handle, Uin, ICON id, callback parameter */
   typedef int (_System MENUCALLBACK)(HICQ, ULONG, USHORT,ULONG);
   typedef int (_System STATUSCALLBACK)(HICQ, int);

#ifdef __OS2__
   typedef int (_System OPENWINCALLBACK)(HICQ, HWND, const char *, void *);
#endif

   typedef struct skinmgr                       /* Skin manager descriptor      */
   {
      USHORT    sz;

      /* Skin manager flags */
      ULONG     flags;
      #define SKINMGR_HASTIMER  0x00000001      /* Disable pwICQ Timer system (LINUX ONLY) */
      #define SKINMGR_NOMENUS   0x00000002      /* Skin doesn't create menus               */

      /* Skin manager identification */
      const char *id;

      /* Initialize skin manager 0 = failed */
      ULONG    (* _System loadSkin)(HICQ, STATUSCALLBACK *, int, char **);

      /* Process the window loop */
      int      (* _System executeSkin)(HICQ, ULONG);

      /* Timer tick */
      int      (* _System timer)(HICQ,void *,char);

      /* Open warning window */
      void     (* _System warning)(HICQ, HUSER, USHORT, void *, const char *);

      /* Default message viewer */
      void     (* _System view)(HICQ, ULONG, HMSG);

      /* Open new message window */
      int      (* _System newMessage)(HICQ, ULONG, USHORT, const char *, const char *);
      int      (* _System msgWithMgr)(HICQ, ULONG, const MSGMGR *, HMSG);
	  int      (* _System msgMultDest)(HICQ,USHORT,const char *,const char *);

      /* Show away message */
      BOOL     (* _System awayMessage)(HICQ, ULONG, const char *, char *);

      /* Open Popup menu */
      int      (* _System popupMenu)(HICQ, ULONG, USHORT, USHORT, USHORT);

      /* Popup user list */
      int      (* _System popupUserList)(HICQ);

      /* Insert menu option (HICQ,Menu-ID,text, icon,CallBack,Parameter) */
/*
#ifdef linux
      #define ICQMENU_USER         0
      #define ICQMENU_MODES        1
      #define ICQMENU_SYSTEM       2
      #define ICQMENU_COUNT            3
#else
      #define ICQMENU_USER      1003
      #define ICQMENU_MODES      101
      #define ICQMENU_SYSTEM     102
#endif
*/
      int      (* _System insertMenu)(HICQ, USHORT, const char *, USHORT, MENUCALLBACK *, ULONG);

      /* Create the configuration dialog */
      #define ICQCONFIGW_USER   0
      #define ICQCONFIGW_SYSTEM 1
      #define ICQCONFIGW_SEARCH 2
      HWND     (* _System openConfig)(HICQ, ULONG, USHORT);

      /* Get the current MSGMGR struct size */
      int      (* _System validateEditHelper)(USHORT);

      /* Skin event processor */
      int      (* _System event)(HICQ,ULONG,char,USHORT,ULONG,void *);

#ifdef __OS2__

      /* Image loader */
      HBITMAP  (* _System LoadBitmap)(const char *);

      /* Send a DDE Request */
      int      (* _System ddeRequest)(HICQ, const char *, const char *, const char *);

      /* Open child window */
      int      (* _System openWindow)(HICQ, HUSER, const char *, OPENWINCALLBACK *,void *);

#endif

   } SKINMGR;

   #pragma pack()

   /*---[ Usefull macros ]--------------------------------------------------------------*/

   #define icqWalkList(V,L)       for(L=icqQueryFirstElement(V);L;L=icqQueryNextElement(V,L))
   #define icqWalkUsers(i,u)      for(u=icqQueryFirstUser(i);u;u=icqQueryNextUser(i,u))
   #define icqWalkMessages(i,u,m) for(m=icqQueryNextMessage(i,u,NULL);m;m=icqQueryNextMessage(i,u,m))

#ifdef PROJECT
   #define icqThreadLog(t,s)  icqWriteThreadLog(t,PROJECT,s)
   #define icqSysLog(i,s)     icqWriteSysLog(i,PROJECT,s)
   #define NOT_IMPLEMENTED    icqWriteSysLog(NULL, PROJECT, __FUNCTION__ );
#else
   #define icqThreadLog(t,s)  icqWriteThreadLog(t,__FILE__,s)
   #define icqSysLog(i,s)     icqWriteSysLog(i,__FILE__,s)
   #define NOT_IMPLEMENTED    icqWriteSysLog(NULL, __FILE__, __FUNCTION__ );
#endif

#endif
