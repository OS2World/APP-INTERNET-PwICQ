/*
 * ICQKERNEL.H
 */

#ifndef PWICQKERNEL_INCLUDED

   #define PWICQKERNEL_INCLUDED "2.0"

#ifdef __cplusplus
   extern "C" {
#endif

   #ifdef __OS2__
      #define INCL_DOSQUEUES
      #include <os2.h>
   #endif

   #ifndef PWICQDEF_INCLUDED
      #define HICQ    void *
      #define HUSER   void *
      #define HSEARCH void *
      #include <icqdefs.h>
      #undef  HICQ
      #undef  HUSER
      #undef  HSEARCH
   #endif

   /*---[ Macros ]---------------------------------------------------------*/


   #define ALWAYS_HIDE_USER(icq,usr) ((usr->u.groupMasc&USRG_MPT) && (icq->cntlFlags&ICQFC_MPT))

   /*---[ Estruturas e definicoes ]----------------------------------------*/

   #define PASSWORD_SIZE        15

   #pragma pack(1)

   typedef struct icqitem
   {
      struct icqitem *up;
      struct icqitem *down;
   } ICQITEM;

   typedef struct _icqlist
   {
     ICQITEM *first;
     ICQITEM *last;
     UCHAR   lock;
   } ICQLIST;

   typedef ICQLIST *    HLIST;

   typedef struct _user
   {
      ICQUSER   u;

      const struct c2sprotmgr  *c2s;            /* C2S Protocol manager for this user           */
      const void               *peer;           /* Peer protocol manager for this user          */
      PEERDATA		       *peerData;	/* Peer protocol manager data block             */

      long      ip[2];
      short     port;
      short     blinkTimer;

      HLIST     msg;

   } USERINFO;

   typedef USERINFO * HUSER;

   typedef struct _mqueue       /* Pending messages list */
   {
      ULONG     uin;
      USHORT    type;
      UCHAR     mode;
      ULONG     id;
   } MQUEUE;

   typedef struct eventlistener
   {
      char      timer;
      void      (* _System listener)(void *, void *, ULONG, char, USHORT, ULONG);
      void      *parm;
   } EVENTLISTENER;

   typedef struct _icq
   {
      USHORT                    sz;
      USHORT                    usz;
      const struct icqftable    *ftable;
      BOOL                      ready;
      ULONG                     uin;

      USHORT			        shortSeq;
      USHORT					minutes;

      char                      pwd[PASSWORD_SIZE+1];

#ifndef linux
      UCHAR                     programPath[0x0100];
#endif

      HLIST                     threads;
      HLIST                     plugins;
      HLIST                     msg;
      HLIST                     listeners;
      HLIST                     guiListeners;
      HLIST                     pendingSearchs;
      HLIST                     requests;
      char                      *badWords;

      ICQSHAREMEM               *shareBlock;
#ifdef linux
      int                                                    shmID;
#endif

      ICQINSTANCE               *info;

      /* User management */
      HLIST                     users;
      const USERDBMGR           *userDB;

      /* Searchs */
      USHORT                    searchs;        /* Number of active searchs */

      /* Service status */
      ULONG                     srvcStatus;
      int                       myPID;

      /* Control flags */
      ULONG                     cntlFlags;
      #define ICQFC_SAVEUSER            0x00000001
      #define ICQFC_CLEAR               0x00000002
      #define ICQFC_LOGONCOMPLETE       0x00000004
      #define ICQFC_EXTRAMODULES        0x00000008
      #define ICQFC_SAVEQUEUES          0x00000010
      #define ICQFC_STARTUPOK           0x00000020
      #define ICQFC_NOGUI               0x00000040
      #define ICQFC_MPT                 0x00000080
      #define ICQFC_SAVECONFIG 		    0x00000100
      #define ICQFC_SAVEPASSWORD        0x00000200

      ULONG                     modeCaps;

      /* Connection modes */
      USHORT                    connctTimer;
      ULONG                     currentMode;
      ULONG                     onlineFlags;

      /* Message management */
      void                      (* _System convertCpg)(struct _icq *,ULONG,ULONG,UCHAR *,int);
      HLIST                     msgQueue;
      char                      *awayMsg;
      const MSGMGR              **msgMgr;

      /* Client<->Server communication */
      int			loginRetry;
      const C2SPROTMGR          *c2s;
      const ICQMODETABLE        *offline;      /* Offline code */
      long                      ipConfirmed;
      USHORT                    c2sPort;

      /* Client<->Client communication */
      const PEERPROTMGR *peer;                  /* Default peer protocol manager */
      void		*peerData;		/* Default peer protocol manager data block */

      /* Skin Manager */
      const SKINMGR             *skin;
      void                      *skinData;
      const char		        *skinRequired;

      /* Blacklists */
      time_t                    blkListTime;
      time_t     			    badWordTime;
      ULONG                     *blacklist;
      ULONG                     *ignore;

      /* Command line parameters */
      int                       numpar;
      char                      **param;

      /* Timers */
      int                       autoSaveTimer;

      /* OS specifics */

#ifdef __OS2__
      HQUEUE                    queue;
      HINI                      ini;
      HAB                       hab;
      USHORT                    countryCode;    /* Pa¡s atual                   */
      ULONG                     cpgIn;          /* Codepage de entrada          */
      ULONG                     cpgOut;         /* Codepage de saida            */
      HMODULE                   icqNetW;        /* ICQNETW.DLL module handle    */
#endif

#ifdef BEOS
      HLIST                     config;
#endif

#ifdef linux
      HLIST                     config;
      BOOL                      saveConfig;
      int                                          timerTick;
      int                       seconds;
      int                                                         queue;
#endif

   } ICQ;

/*   typedef ICQ * HICQ; */
   #define HICQ ICQ *

   #include <icqqueue.h>

   typedef struct icqplugin
   {
      HMODULE           module;

      USHORT            id;
      void              *dataBlock;
      const MSGMGR      **msgmgr;


      void (* _System ConfigPage)(HICQ, void *, ULONG, USHORT, HWND, const DLGINSERT *, char *);

      void (* _System Menu)(HICQ, void *, HUSER,USHORT,HWND);

      int  (* _System ExternCMD)(HICQ, void *,int,char *);

      void (* _System SystemEvent)(HICQ, void *,short);
      void (* _System UserEvent)(HICQ, void *,HUSER,short);
      void (* _System ProcessEvent)(HICQ, void *, ULONG, char, short, ULONG);
      BOOL (* _System Packet)(HICQ, void *,USHORT, void *, int, long, int);
      void (* _System PreProcessMessage)(HICQ, void *, HUSER, ULONG, HMSG, BOOL);
      int  (* _System rpcRequest)(HICQ, struct icqpluginipc *);
      int  (* _System chkCommand)(HICQ, const char *, const char *);

      void (* _System snd2Option)(HICQ, hWindow, USHORT, BOOL, const MSGMGR *, int ( * _System)(HWND, USHORT, const char *, SENDTOCALLBACK *));

      void (* _System Timer)(HICQ, void *);


   } PLUGIN;

   typedef PLUGIN * HPLUGIN;


#ifdef __OS2__
   #define isProcValid(p,x) (x)
#endif

#ifdef linux
   #define isProcValid(p,x) ( (p->module!=0) && ((unsigned long)x) != 0xFFFFFFFF)
#endif

   typedef struct sysmsg                /* Message in the system queue */
   {
      ULONG     uin;                    /* From... */
      ICQMSG    m;                      /* The message */
   } SYSMSG;

#ifdef __cplusplus
   }
#endif

   /*---[ Search ]---------------------------------------------------------*/

   typedef struct search
   {
      USHORT 		sz;
      ULONG  		uin;
      ULONG		flags;

      #define ICQSEARCH_EVENTS	0x00000001

      USHORT 		idle;
      ULONG  		sequence;
      HWND   		hwnd;
      SEARCHCALLBACK 	callBack;
   } ICQSEARCH;

   #define HSEARCH struct search *

   /*---[ Requests ]-------------------------------------------------------*/

   typedef struct request
   {
      USHORT		sz;
      ULONG             session;
      ULONG             uin;
      ULONG             id;
      BOOL		out;
      const REQUESTMGR  *mgr;
      void		*data;
   } REQUEST;

   /*---[ Macros ]---------------------------------------------------------*/

   #include <pwmacros.h>
   #include <icqreserved.h>
   #include <icqtlkt.h>

   #pragma pack()

   void icqLoadMenus(HICQ);

#endif


