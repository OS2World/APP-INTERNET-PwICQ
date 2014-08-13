/*
 * ICQTLKT.H - pwICQ V2 developer's toolkit
 */

#ifndef ICQTLKT_INCLUDED

   #define ICQTLKT_INCLUDED "2.0"

#ifdef MEMWATCH
   #include <memwatch.h>
#else
   #define  TRACE(x) /* x */
#endif

   #ifndef BUILD
      #define BUILD __DATE__ " " __TIME__
   #endif

   #ifndef PROJECT
      #define PROJECT __FILE__
   #endif

   #ifndef PWICQKERNEL_INCLUDED
      #define HICQ      void *
      #define HLIST     void *
   #endif

   #ifndef PWICQDEF_INCLUDED
      #include <icqdefs.h>
   #endif

   #pragma pack(1)

#ifdef __cplusplus
   extern "C" {
#endif

   /*---[ Get/Set Core informations ]------------------------------------------------*/

   char                 * ICQAPI icqQueryPath(HICQ, const char *, char *, int);
   const char           * ICQAPI icqQueryProgramPath(HICQ, const char *, char *, int);
   char                 * ICQAPI icqQueryCoreVersion(void);
   const ICQMODETABLE   * ICQAPI icqQueryModeTable(HICQ);
   ULONG                  ICQAPI icqQueryUIN(HICQ);
   ULONG                  ICQAPI icqQueryOnlineMode(HICQ);
   BOOL                   ICQAPI icqCheckBlacklist(HICQ, ULONG);
   BOOL                   ICQAPI icqCheckBadWord(HICQ, const char *);
   BOOL                   ICQAPI icqCheckIgnoreList(HICQ, ULONG);

   int                    ICQAPI icqSetCurrentUser(HICQ, ULONG, const char *, BOOL);
   BOOL                   ICQAPI icqQueryLogonStatus(HICQ);
   BOOL                   ICQAPI icqIsActive(HICQ);
   USHORT                 ICQAPI icqQueryModeIcon(HICQ, ULONG);
   ULONG                  ICQAPI icqQueryOfflineMode(HICQ);
   BOOL                   ICQAPI icqSetLocalIP(HICQ, long);
   ULONG                * ICQAPI icqQueryOnlineFlags(HICQ);
   USHORT                 ICQAPI icqSetC2SPort(HICQ,USHORT);
   int                    ICQAPI icqQueryMaxMessageLength(HICQ, HUSER);
   BOOL                   ICQAPI icqClearToSend(HICQ);

   long                   ICQAPI icqQueryLocalIP(HICQ);

   ULONG                  ICQAPI icqSetCaps(HICQ, ULONG);
   ULONG                  ICQAPI icqGetCaps(HICQ);

   void                   ICQAPI icqLogonDialog(HICQ);

   void                   ICQAPI icqAbend(HICQ, ULONG);
   void                   ICQAPI icqSetServerBusy(HICQ, BOOL);

   const char           * ICQAPI icqQueryAwayMessage(HICQ);
   int                    ICQAPI icqSetAwayMessage(HICQ, const char *);
   BOOL				      ICQAPI icqQueryTextModeFlag(HICQ);

   ULONG                  ICQAPI icqQueryModeFlags(HICQ);
   const char *           ICQAPI icqQuerySkinManagerName(HICQ);

   const C2SPROTMGR     * ICQAPI icqQueryProtocolManager(HICQ, USHORT);

   void 		          ICQAPI icqQueryPassword(HICQ, char *);
   int                    ICQAPI icqSetPassword(HICQ,USHORT,const char *, const char *);

   const SKINMGR *        ICQAPI icqQuerySkinManager(HICQ);

   USHORT                 ICQAPI icqQueryShortSequence(HICQ);
   ULONG				  ICQAPI icqQueryBuild(void);

   const void *           ICQAPI icqQueryInternalTable(HICQ, USHORT, char *, int);

   int                    ICQAPI icqSetConnectTimer(HICQ,USHORT);

   #ifdef __OS2__
      HAB       ICQAPI icqQueryAnchorBlock(HICQ);
      HINI      ICQAPI icqQueryINIFile(HICQ);
      USHORT    ICQAPI icqQueryRemoteCodePage(HICQ);
      USHORT    ICQAPI icqSetRemoteCodePage(HICQ, USHORT);
      int       ICQAPI icqOpenWindow(HICQ, HUSER, const char *, OPENWINCALLBACK *,void *);
      void      ICQAPI icqTranslateCodePage(HICQ, ULONG, ULONG, UCHAR *, int);
      HICQ      ICQAPI icqQueryHandler(void);
      int       ICQAPI icqQueryCountryCode(HICQ);
      USHORT    ICQAPI icqQueryLocalCodePage(HICQ);

      #ifdef __DLL__
         #define DLL
      #endif
   #endif

   /*---[ List management ]----------------------------------------------------------*/
   HLIST        ICQAPI icqCreateList(void);
   HLIST        ICQAPI icqDestroyList(HLIST);
   void         ICQAPI icqClearList(HLIST);
   void *       ICQAPI icqAddElement(HLIST, int);
   int          ICQAPI icqRemoveElement(HLIST, void *);
   void *       ICQAPI icqQueryFirstElement(HLIST);
   void *       ICQAPI icqQueryLastElement(HLIST);
   void *       ICQAPI icqQueryNextElement(HLIST, void *);
   void *       ICQAPI icqQueryPreviousElement(HLIST, void *);
   void *       ICQAPI icqResizeElement(HLIST, int, void *);
   void         ICQAPI icqLockList(HLIST);
   void         ICQAPI icqUnlockList(HLIST);

   /*---[ Online mode ]--------------------------------------------------------------*/
   ULONG        ICQAPI icqSetOnlineMode(HICQ,ULONG);
   ULONG        ICQAPI icqReconnect(HICQ);
   ULONG        ICQAPI icqDisconnect(HICQ);
   ULONG        ICQAPI icqSetModeByName(HICQ, const char *);
   int          ICQAPI icqIsOnline(HICQ);

   /*---[ Event Launchers ]----------------------------------------------------------*/
   void         ICQAPI icqSystemEvent(HICQ,USHORT);
   void         ICQAPI icqUserEvent(HICQ,HUSER,USHORT);
   void         ICQAPI icqEvent(HICQ, ULONG, char, USHORT, ULONG);

   /*---[ Users-list management ]----------------------------------------------------*/
   HUSER        ICQAPI icqInsertUser(HICQ, ULONG, const char *, const char *, const char *, const char *, const char *, ULONG);
   ULONG        ICQAPI icqRemoveUser(HICQ, HUSER);
   ULONG        ICQAPI icqRejectUser(HICQ, HUSER, BOOL);
   int          ICQAPI icqBlacklistUser(HICQ, ULONG);

   HUSER        ICQAPI icqUpdateUserInfo(HICQ, ULONG, BOOL, const char *, const char *, const char *, const char *, BOOL);
   HUSER        ICQAPI icqAddNewUser(HICQ, ULONG, const char *, const char *, const char *, const char *, BOOL);

   HUSER        ICQAPI icqInsertTemporaryUser(HICQ, ULONG);

   HUSER        ICQAPI icqQueryUserHandle(HICQ, ULONG);
   HUSER        ICQAPI icqQueryUserByNick(HICQ, const char *);

   HUSER        ICQAPI icqUserOnline(HICQ, ULONG, ULONG, long, long, short, long, ULONG);
   HUSER        ICQAPI icqSetUserOnlineMode(HICQ, ULONG, ULONG);
   HUSER        ICQAPI icqQueryNextUser(HICQ, HUSER);

   ULONG        ICQAPI icqUpdateUserFlag(HICQ, HUSER, BOOL, ULONG);
   USHORT       ICQAPI icqQueryUserPosLimit(HICQ);

   HUSER        ICQAPI icqAddUserInCache(HICQ, ULONG);

   const char * ICQAPI icqLoadUserDB(HICQ, HUSER, const char *, const char *, char *, int);
   const char * ICQAPI icqSaveUserDB(HICQ, HUSER, const char *, char *);

   const char * ICQAPI icqQueryUserNick(HUSER);
   const char * ICQAPI icqQueryUserEmail(HUSER);
   const char * ICQAPI icqQueryUserFirstName(HUSER);
   const char * ICQAPI icqQueryUserLastName(HUSER);
   const char * ICQAPI icqQueryUserSound(HUSER);

   int          ICQAPI icqRequestUserUpdate(HICQ, ULONG);
   int          ICQAPI icqRequestUserInfo(HICQ, ULONG);

   int          ICQAPI icqQueryUserDataBlockLength(void);

   void         ICQAPI icqRegisterUserAction(HUSER);

   long         ICQAPI icqQueryUserIP(HICQ, HUSER);
   int          ICQAPI icqQueryUserPort(HICQ, HUSER);
   long         ICQAPI icqQueryUserGateway(HICQ, HUSER);

   void         ICQAPI icqSetUserIP(HICQ, HUSER, long, long);
   HUSER        ICQAPI icqSetUserIPandPort(HICQ, ULONG, long, long, USHORT);

   int          ICQAPI icqAddUserInContactList(HICQ, ULONG);

   void         ICQAPI icqAjustUserIcon(HICQ, HUSER, BOOL);

   #define icqQueryFirstUser(i) icqQueryNextUser(i,NULL)


   /*---[ Thread management ]--------------------------------------------------------*/
   typedef struct icqThread
   {
      HICQ        icq;
#ifdef __OS2__
      TID         tid;
#endif

#ifdef linux
      pthread_t   tid;
      void        (* _System start_address) (struct icqThread *);
#endif

#ifdef BEOS
      thread_id   tid;
#endif

      void        *parm;
      CHAR        id[3];
      USHORT      timer;

      /*
       * CallBack procedures, register it if necessary
       */

      /* Called by the core in case of need to stop */
      void        (* _System stopthread) (struct icqThread *);

      /* Thread event processor, used to notify the thread of pwICQ events */
      void        (* _System event) (struct icqThread *, ULONG, char, USHORT, ULONG);

   } ICQTHREAD;

   typedef ICQTHREAD * HTHREAD;


   int          ICQAPI icqCreateThread(HICQ, void (* _System)(ICQTHREAD *), unsigned int, int, void *, const char *);

   #define icqStart(i,f,s,b,p,n) icqStartThread(i, (void (* _System)(ICQTHREAD *, void *)) f, s, b, p, n)
   int          ICQAPI icqStartThread(HICQ, void (* _System start_address)(ICQTHREAD *, void *), unsigned int, int, const void *, const char *);

   int          ICQAPI icqKillThread(HICQ,ICQTHREAD *);

   /*---[ Logging ]------------------------------------------------------------------*/
   void         ICQAPI icqWriteThreadLog(ICQTHREAD *, const char *, const char *);
   void         ICQAPI icqWriteSysLog(HICQ, const char *, const char *);
   void         ICQAPI icqPrintLog(HICQ, const char *, const char *, ...);
   void         ICQAPI icqWriteUserLog(HICQ,const char *,HUSER, const char *);
   void         ICQAPI icqWriteSysRC(HICQ, const char *, int, const char *);
   const char * ICQAPI icqIP2String(long,char *);
   void 	ICQAPI icqWarning(HICQ, HUSER, const char *, USHORT, const char *);

   /*---[ Network ]------------------------------------------------------------------*/

   #include <icqnetw.h>

   /*---[ Plugin support ]-----------------------------------------------------------*/
   HPLUGIN      ICQAPI icqLoadPlugin(HICQ, const char *,char *);
   int          ICQAPI icqStartPlugin(HICQ, HPLUGIN);
   int          ICQAPI icqStopPlugin(HICQ, HPLUGIN);
   HPLUGIN      ICQAPI icqQueryPlugin(HICQ, const char *);

   int          ICQAPI icqInsertMenuOption(HICQ, USHORT, const char *, USHORT, MENUCALLBACK *, ULONG);
   HWND         ICQAPI icqOpenConfigDialog(HICQ, ULONG, USHORT);
   void         ICQAPI icqInsertConfigPage(HICQ, ULONG, USHORT, HWND, const DLGINSERT *, char *);

   int          ICQAPI icqUnloadPlugin(HICQ,HPLUGIN);
   BOOL         ICQAPI icqPacket(HICQ, USHORT, void *, int, long, int);

   void         ICQAPI icqPreProcessMessage(HICQ,HUSER,ULONG,HMSG);

   void *       ICQAPI icqGetPluginDataBlock(HICQ, HMODULE);
   void *       ICQAPI icqQueryPluginDataBlock(HICQ, HPLUGIN);

   void         ICQAPI icqInsertSendToOptions(HICQ, hWindow, USHORT, BOOL, const MSGMGR *, int ( * _System)(HWND, USHORT, const char *, SENDTOCALLBACK *));
   int          ICQAPI icqLoadSymbol(HPLUGIN, const char *, void **);

   HPLUGIN      ICQAPI icqQueryNextPlugin(HICQ, HPLUGIN);

   #define icqQueryFirstPlugin(i) icqQueryNextPlugin(i,NULL)

   /*---[ Event listeners ]----------------------------------------------------------*/

   /* Gui listeners are called when requested by the skin manager (Asincronous and sequenced in one single thread) */
   void         ICQAPI icqExecuteGuiListeners(HICQ, ULONG, char, USHORT, ULONG);
   int          ICQAPI icqAddGuiEventListener(HICQ, void *, HWND);
   int          ICQAPI icqRemoveGuiEventListener(HICQ, void *, HWND);

   /* Regular listeners are called in the same thread context of the event generation (Sincronized) */
   int          ICQAPI icqAddEventListener(HICQ, void (* _System)(HICQ, void *, ULONG, char, USHORT, ULONG), void *);
   int          ICQAPI icqRemoveEventListener(HICQ, void (* _System)(HICQ, void *, ULONG, char, USHORT, ULONG), void *);

   /*---[ Special service plugins (Core extensions) ]--------------------------------*/
   void         ICQAPI icqSetServiceStatus(HICQ,int,BOOL);

   BOOL         ICQAPI icqRegisterServerProtocolManager(HICQ, const C2SPROTMGR *);
   BOOL         ICQAPI icqDeRegisterServerProtocolManager(HICQ, const C2SPROTMGR *);

   BOOL         ICQAPI icqRegisterPeerProtocolManager(HICQ,const PEERPROTMGR *, void *);
   BOOL         ICQAPI icqDeRegisterPeerProtocolManager(HICQ, const PEERPROTMGR *);
   int          ICQAPI icqSetPeerSession(HICQ, ULONG, ULONG, const PEERPROTMGR *, PEERDATA *);
   PEERDATA *   ICQAPI icqGetPeerSession(HICQ, ULONG, const PEERPROTMGR *);

   int          ICQAPI icqRegisterDefaultMessageProcessor(HICQ, USHORT, const MSGMGR **);

   BOOL         ICQAPI icqRegisterSkinManager(HICQ, const SKINMGR *);
   int          ICQAPI icqInitialize(HICQ, STATUSCALLBACK * );
   int          ICQAPI icqTerminate(HICQ);
   UCHAR *      ICQAPI icqConvertCodePage(HICQ, BOOL, UCHAR *, int);

   BOOL         ICQAPI icqRegisterUserDBManager(HICQ, const USERDBMGR *);
   BOOL         ICQAPI icqDeRegisterUserDBManager(HICQ, const USERDBMGR *);


   void *       ICQAPI icqGetSkinDataBlock(HICQ);
   void         ICQAPI icqSetSkinDataBlock(HICQ, void *);

   int          ICQAPI icqValidateMsgEditHelper(HICQ, USHORT);

   typedef void (_System CPGCONV)(HICQ, ULONG, ULONG, UCHAR *, int);

   CPGCONV *    ICQAPI icqSetCodePageConverter(HICQ, CPGCONV *);

#ifdef linux
   void         ICQAPI icqkrnl_Timer(HICQ);
   int          ICQAPI icqChkCmdLine(int, char **);
   int          ICQAPI icqGetString(HICQ, HMODULE, USHORT, char *, int);
   const void * ICQAPI _icqGetDialogDescriptor(HICQ, HMODULE, USHORT, int *, int);
   const void * ICQAPI _icqGetResourceDescriptor(HICQ, HMODULE,int);
#endif

   long         ICQAPI icqQueryPeerPort(HICQ);
   long         ICQAPI icqQueryPeerAddress(HICQ);
   ULONG        ICQAPI icqSetPeerCookie(HICQ, USHORT, ULONG);
   ULONG        ICQAPI icqGetPeerCookie(HICQ, USHORT);

   /*---[ Access to the configuration file ]-----------------------------------------*/
   char *       ICQAPI icqLoadProfileString(HICQ, const char *, const char *, const char *, char *, int);
   void         ICQAPI icqSaveProfileString(HICQ, const char *, const char *, const char *);

   ULONG        ICQAPI icqLoadProfileValue(HICQ, const char *, const char *, ULONG);
   ULONG        ICQAPI icqSaveProfileValue(HICQ, const char *, const char *, ULONG);

   ULONG        ICQAPI icqLoadValue(HICQ, const char *, ULONG);
   char *       ICQAPI icqLoadString(HICQ, const char *, const char *, char *, int);

   ULONG        ICQAPI icqSaveValue(HICQ, const char *, ULONG);
   void         ICQAPI icqSaveString(HICQ, const char *, const char *);

   void         ICQAPI icqDeleteProfile(HICQ,const char *);

#ifdef __OS2__
   void         ICQAPI icqRestoreWindow(HWND, HICQ, ULONG, const char *, short, short);
   void         ICQAPI icqStoreWindow(HWND, HICQ, ULONG, const char *);
   void         ICQAPI icqChangeMsgEditmode(HWND,HICQ,ULONG,USHORT,BOOL);
#endif

   /*---[ Messages ]----------------------------------------------------------------*/
   BOOL         ICQAPI icqInsertMessage(HICQ, ULONG, ULONG, USHORT, time_t, ULONG, int, const char *);
   BOOL         ICQAPI icqInsertRTFMessage(HICQ, ULONG, ULONG, USHORT, time_t, ULONG, const char *);
   void         ICQAPI icqProcessServerConfirmation(HICQ, ULONG, ULONG);
   int          ICQAPI icqSendMessage(HICQ, ULONG, USHORT, unsigned char *);
   void         ICQAPI icqOpenMessage(HICQ, ULONG, HMSG);
   BOOL         ICQAPI icqOpenUserMessage(HICQ, ULONG);
   BOOL         ICQAPI icqOpenSystemMessage(HICQ);
   void         ICQAPI icqOpenUserMessageWindow(HICQ,HUSER);
   char *       ICQAPI icqQueryMessageTitle(HICQ, ULONG, BOOL, HMSG, char *, int);
   HMSG         ICQAPI icqQueryUserMessage(HICQ, ULONG);
   BOOL         ICQAPI icqRemoveMessage(HICQ, ULONG, HMSG);
   void         ICQAPI icqSetUserAutoOpen(HICQ, HUSER, BOOL);
   void         ICQAPI icqNewUserMessage(HICQ, ULONG, USHORT, const char *, const char *);
   int          ICQAPI icqNewMessageWithManager(HICQ, ULONG, const MSGMGR *);
   int          ICQAPI icqNewMultiUserMessage(HICQ, USHORT, const char *, const char *);

   HMSG         ICQAPI icqQueryMessage(HICQ, ULONG);
   int          ICQAPI icqQueryFirstMessage(HICQ, ULONG *, HMSG *);
   HMSG         ICQAPI icqQueryNextMessage(HICQ, ULONG, HMSG);

   int          ICQAPI icqSendConfirmation(HICQ, ULONG, USHORT, BOOL, char *);
   BOOL         ICQAPI icqInsertAwayMessage(HICQ, ULONG, const char *, char *);
   int          ICQAPI icqSendQueued(HICQ);
   int          ICQAPI icqRemoveMessageByID(HICQ, ULONG);

   const MSGMGR * ICQAPI icqQueryMessageManager(HICQ, USHORT);

   /*---[ User search ]-------------------------------------------------------------*/
   HSEARCH      ICQAPI icqBeginSearch(HICQ, ULONG, ULONG, HWND, BOOL, SEARCHCALLBACK);
   int          ICQAPI icqEndSearch(HICQ, ULONG);
   ULONG        ICQAPI icqQuerySearchUIN(HICQ, ULONG);
   HSEARCH      ICQAPI icqQuerySearchHandle(HICQ, ULONG);
   int          ICQAPI icqSetSearchResponse(HICQ, ULONG, const ICQSEARCHRESPONSE *);

   int 		ICQAPI icqSearchByICQ(HICQ, ULONG, HWND, SEARCHCALLBACK);
   int          ICQAPI icqSearchByMail(HICQ, const char *, HWND, SEARCHCALLBACK);
   int          ICQAPI icqSearchRandom(HICQ, USHORT, HWND, SEARCHCALLBACK);
   int          ICQAPI icqSearchByInformation(HICQ,HWND,const char *, const char *,const char *, const char *,BOOL,SEARCHCALLBACK);

   HUSER	ICQAPI icqCacheSearchResult(HICQ,USHORT,ULONG,BOOL);

   /*---[ Requests ]----------------------------------------------------------------*/

   void *       ICQAPI   icqCreateRequest(HICQ, ULONG, BOOL, ULONG, ULONG, const REQUESTMGR *, void *, int);
   int          ICQAPI   icqSetRequestSession(HICQ, ULONG, BOOL, ULONG);

   int          ICQAPI   icqRefuseRequest(HICQ, ULONG, const char *);
   int          ICQAPI   icqAcceptRequest(HICQ, ULONG, const char *);

   int          ICQAPI   icqRequestRefused(HICQ, ULONG, int, int, const char *);
   int          ICQAPI   icqRequestAccepted(HICQ, ULONG, int, int, const char *);

   int          ICQAPI   icqDestroyRequest(HICQ, ULONG, BOOL);
   int          ICQAPI   icqDestroySessionRequests(HICQ, ULONG);
   void *       ICQAPI   icqQueryNextRequest(HICQ, ULONG, ULONG *);

   /*---[ Misc ]--------------------------------------------------------------------*/

   int          ICQAPI   icqOpenURL(HICQ, const char *);
   int          ICQAPI   icqExecuteCmd(HICQ, char *);
   int          ICQAPI   icqShowPopupMenu(HICQ, ULONG, USHORT, USHORT, USHORT);
   int          ICQAPI   icqPopupUserList(HICQ);
   int 		    ICQAPI   icqAbortSearchByWindow(HICQ, HWND);
   int          ICQAPI   icqConnectUser(HICQ,ULONG,int);
   int          ICQAPI   icqBeginPeerSession(HICQ, HUSER);
   const char * ICQAPI   icqQueryURL(HICQ, const char *);

   void 	ICQAPI   icqDumpPacket(HICQ, HUSER, const char *, int, unsigned char *);

   const PEERPROTMGR * ICQAPI icqQueryPeerProtocolManager(HICQ, USHORT);
   void *              ICQAPI icqQueryPeerProtocolData(HICQ, USHORT);

#ifdef __OS2__
   int          ICQAPI   icqSetEA(const char *, const char *, int, const char *);
   int          ICQAPI   icqGetEA(const char *, const char *, int, char *);
#endif

   int          ICQAPI   icqSendFile(HICQ, ULONG, const char *, const char *);

   /*---[ Extended loader ]------------------------------------------------*/

   HICQ         ICQAPI  icqCoreStartup(int, char **, STATUSCALLBACK * );
   int          ICQAPI  icqCoreTerminate(HICQ);

#ifdef linux

   struct coreEntryPoints
   {
          USHORT        sz;             /* Size of this structure */
          USHORT        id;             /* Size of ICQ */
          HICQ          (* _System icqCoreStartup)(int, char **, STATUSCALLBACK *);
          int           (* _System icqCoreTerminate)(HICQ);
          int           (* _System icqTerminate)(HICQ);
          int           (* _System icqChkCmdLine)(int, char **);
          void          (* _System icqkrnl_Timer)(HICQ);
   };

#endif



   #pragma pack()

#ifdef __cplusplus
   }
#endif

#ifndef PWMACROS_INCLUDED

    #ifndef DEBUG
       #define DBGMessage(x) /* x */
       #define DBGTrace(x) /* x */
       #define DBGTracex(x) /* x */
       #define CHKPoint()  /* */
       #define DBGPrint(x,y) /* x y */
    #else
       #define DBGMessage(x) fprintf(stderr,"%s(%d):\t%s\n",__FILE__,__LINE__,x);fflush(stderr);
       #define DBGTrace(x)   fprintf(stderr,"%s(%d):\t%s = %ld\n",__FILE__,__LINE__,#x, (unsigned long) x);fflush(stderr);
       #define DBGTracex(x)  fprintf(stderr,"%s(%d):\t%s = %08lx\n",__FILE__,__LINE__,#x, (unsigned long) x);fflush(stderr);
       #define CHKPoint()    fprintf(stderr,"%s(%d):\t%s\t\t(" __DATE__ " " __TIME__")\n",__FILE__,__LINE__,__FUNCTION__);fflush(stderr);
       #define DBGPrint(x,y) fprintf(stderr,"%s(%d):\t" x "\n",__FILE__,__LINE__,y);fflush(stderr);
    #endif

 #endif

 #ifdef USEICQLIB
    #include <icqftable.h>
 #endif

#endif

