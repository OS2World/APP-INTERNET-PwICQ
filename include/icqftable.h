/*
 * ICQFTABLE.H - Tabela estatica de funcoes
 */

#ifdef __cplusplus
   extern "C" {
#endif

 #pragma pack(1)

 #define ICQFTABLE_VERSION 1

 #include <icqqueue.h>

 struct icqftable
 {
    USHORT               sz;                     /* Tamanho dessa estrutura                      */
    USHORT               Version;                /* Versao da tabela                             */
    ULONG                build;                  /* icqkrnl build                                */
    const char           *ident;
    USHORT               szKrnl;                 /* Tamanho da estrutura KRNL                    */
    USHORT               szUser;                 /* Tamanho da estrutura de usuario              */
    ULONG                pwICQVersion;           /* Versao do pwICQ                              */

    /*---[ Skin manager ]-------------------------------------------------------------*/
    const SKINMGR *      ( * _System icqQuerySkinManager)(HICQ);
    BOOL                 ( * _System icqRegisterSkinManager)(HICQ, const SKINMGR *);
    const char *         ( * _System icqQuerySkinManagerName)(HICQ);

    void *               ( * _System icqGetSkinDataBlock)(HICQ);
    void                 ( * _System icqSetSkinDataBlock)(HICQ,void *);

    BOOL                 ( * _System icqRegisterServerProtocolManager)(HICQ, const C2SPROTMGR *);
    BOOL                 ( * _System icqDeRegisterServerProtocolManager)(HICQ, const C2SPROTMGR *);

    BOOL                 ( * _System icqRegisterPeerProtocolManager)(HICQ, const PEERPROTMGR *, void *);
    BOOL                 ( * _System icqDeRegisterPeerProtocolManager)(HICQ, const PEERPROTMGR *);

    int                  ( * _System icqShowPopupMenu)(HICQ, ULONG, USHORT, USHORT, USHORT);
    int                  ( * _System icqInsertMenuOption)(HICQ, USHORT, const char *, USHORT, MENUCALLBACK *, ULONG);
    HWND                 ( * _System icqOpenConfigDialog)(HICQ, ULONG, USHORT);

    /*---[ Users-list management ]----------------------------------------------------*/
    HUSER                ( * _System icqQueryUserHandle)(HICQ, ULONG);
    HUSER                ( * _System icqQueryUserByNick)(HICQ, const char *);
    HUSER                ( * _System icqQueryNextUser)(HICQ, HUSER);

    const char *         ( * _System icqQueryUserNick)(HUSER);
    const char *         ( * _System icqQueryUserEmail)(HUSER);
    const char *         ( * _System icqQueryUserFirstName)(HUSER);
    const char *         ( * _System icqQueryUserLastName)(HUSER);
    const char *         ( * _System icqQueryUserSound)(HUSER);
    long	         ( * _System icqQueryUserIP)(HICQ,HUSER);
    long    		 ( * _System icqQueryUserGateway)(HICQ, HUSER);

    HUSER                ( * _System icqSetUserOnlineMode)(HICQ, ULONG, ULONG);
    HUSER	         ( * _System icqSetUserIPandPort)(HICQ, ULONG, long, long, USHORT);

    const char *         ( * _System icqLoadUserDB)(HICQ, HUSER, const char *, const char *, char *, int);
    const char *         ( * _System icqSaveUserDB)(HICQ, HUSER, const char *, char *);

    HUSER                ( * _System icqUserOnline)(HICQ, ULONG, ULONG, long, long, short, long, ULONG);

    void                 ( * _System icqRegisterUserAction)(HUSER);

    HUSER                ( * _System icqUpdateUserInfo)(HICQ, ULONG, BOOL, const char *, const char *, const char *, const char *, BOOL);

    int                  ( * _System icqAddUserInContactList)(HICQ, ULONG);
    HUSER                ( * _System icqAddUserInCache)(HICQ, ULONG);

    int                  ( * _System icqBlacklistUser)(HICQ, ULONG);

    ULONG                ( * _System icqRemoveUser)(HICQ,HUSER);
    ULONG                ( * _System icqRejectUser)(HICQ,HUSER,BOOL);

    int                  ( * _System icqRequestUserUpdate)(HICQ, ULONG);

    HUSER                ( * _System icqAddNewUser)(HICQ, ULONG, const char *, const char *, const char *, const char *, BOOL);
    HUSER                ( * _System icqInsertUser)(HICQ, ULONG, const char *, const char *, const char *, const char *, const char *, ULONG);

    ULONG                ( * _System icqUpdateUserFlag)(HICQ, HUSER, BOOL, ULONG);

    void                 ( * _System icqAjustUserIcon)(HICQ, HUSER, BOOL);

    /*---[ Event Launchers ]----------------------------------------------------------*/
    void                 ( * _System icqSystemEvent)(HICQ,USHORT);
    void                 ( * _System icqUserEvent)(HICQ,HUSER,USHORT);
    void                 ( * _System icqEvent)(HICQ, ULONG, char, USHORT, ULONG);
    void                 ( * _System icqAbend)(HICQ, ULONG);

    /*---[ Configuration file ]-------------------------------------------------------*/
    ULONG                ( * _System icqLoadValue)(HICQ, const char *, ULONG);
    char *               ( * _System icqLoadString)(HICQ, const char *, const char *, char *, int);
    char *               ( * _System icqLoadProfileString)(HICQ, const char *, const char *, const char *, char *, int);

    ULONG                ( * _System icqSaveValue)(HICQ, const char *, ULONG);
    void                 ( * _System icqSaveString)(HICQ, const char *, const char *);
    void                 ( * _System icqSaveProfileString)(HICQ, const char *, const char *, const char *);

    /*---[ Plugin management ]--------------------------------------------------------*/
    int                  ( * _System icqLoadSymbol)(HPLUGIN, const char *, void **);
    HPLUGIN              ( * _System icqQueryPlugin)(HICQ, const char *);
    void *               ( * _System icqGetPluginDataBlock)(HICQ, HMODULE);
    void * 		 ( * _System icqQueryPluginDataBlock)(HICQ, HPLUGIN);
    HPLUGIN 		 ( * _System icqQueryNextPlugin)(HICQ, HPLUGIN);

    /*---[ Protocol ]-----------------------------------------------------------------*/
    ULONG                ( * _System icqReconnect)(HICQ);
    ULONG                ( * _System icqSetOnlineMode)(HICQ, ULONG);
    BOOL                 ( * _System icqClearToSend)(HICQ);

    void                 ( * _System icqSetServerBusy)(HICQ, BOOL);
    long                 ( * _System icqQueryPeerPort)(HICQ);
    long                 ( * _System icqQueryPeerAddress)(HICQ);

    BOOL                 ( * _System icqSetLocalIP)(HICQ, long);

    void                 ( * _System icqSetServiceStatus)(HICQ,int, BOOL);
    int                  ( * _System icqIsOnline)(HICQ);
    ULONG                ( * _System icqDisconnect)(HICQ);
    int                  ( * _System icqSetPeerSession)(HICQ, ULONG, ULONG, const PEERPROTMGR *, PEERDATA *);
    PEERDATA *           ( * _System icqGetPeerSession)(HICQ, ULONG, const PEERPROTMGR *);

    int                  ( * _System icqBeginPeerSession)(HICQ, HUSER);

    /*---[ Miscellaneous (Core) ]-----------------------------------------------------*/
    int                  ( * _System icqInitialize)(HICQ , STATUSCALLBACK * );
    int                  ( * _System icqSetCurrentUser)(HICQ, ULONG, const char *, BOOL);

    BOOL                 ( * _System icqIsActive)(HICQ);
    int                  ( * _System icqCreateThread)(HICQ, void (* _System)(ICQTHREAD *), unsigned int, int, void *, const char *);
    int                  ( * _System icqStartThread)(HICQ, void (* _System)(ICQTHREAD *, void *), unsigned int, int, const void *, const char *);

    void                 ( * _System icqWriteSysLog)(HICQ, const char *, const char *);
    void                 ( * _System icqPrintLog)(HICQ, const char *, const char *, ...);
    void                 ( * _System icqWriteSysRC)(HICQ, const char *, int, const char *);

    void 		         ( * _System icqWarning)(HICQ,HUSER,const char *,USHORT,const char *);
    void                 ( * _System icqWriteUserLog)(HICQ,const char *, HUSER, const char *);

    int                  ( * _System icqQueryUserDataBlockLength)(void);
    UCHAR *              ( * _System icqConvertCodePage)(HICQ, BOOL, UCHAR *, int);
    void                 ( * _System icqQueryPassword)(HICQ, char *);
    ULONG                ( * _System icqQueryOnlineMode)(HICQ);
    void                 ( * _System icqProcessRemoteBlock)(HICQ, ICQQUEUEBLOCK *);
    CPGCONV *            ( * _System icqSetCodePageConverter)(HICQ, CPGCONV *);
    const char *         ( * _System icqQueryProgramPath)(HICQ, const char *, char *, int);
    const ICQMODETABLE * ( * _System icqQueryModeTable)(HICQ);
    BOOL                 ( * _System icqQueryLogonStatus)(HICQ);
    int                  ( * _System icqValidateMsgEditHelper)(HICQ, USHORT);
    int                  ( * _System icqRegisterDefaultMessageProcessor)(HICQ, USHORT, const MSGMGR **);
    char *               ( * _System icqQueryPath)(HICQ, const char *, char *, int);
    ULONG                ( * _System icqSetPeerCookie)(HICQ, USHORT, ULONG);
    ULONG                ( * _System icqQueryModeFlags)(HICQ);
    int                  ( * _System icqQueryMaxMessageLength)(HICQ,HUSER);

    BOOL                 ( * _System icqRegisterUserDBManager)(HICQ, const USERDBMGR *);
    BOOL                 ( * _System icqDeRegisterUserDBManager)(HICQ, const USERDBMGR *);

    ULONG                ( * _System icqQueryOfflineMode)(HICQ);
    ULONG                ( * _System icqGetCaps)(HICQ);
    ULONG                ( * _System icqSetCaps)(HICQ,ULONG);

    const C2SPROTMGR *   ( * _System icqQueryProtocolManager)(HICQ, USHORT);
    long		 ( * _System icqQueryLocalIP)(HICQ);

    BOOL		 ( * _System icqQueryTextModeFlag)(HICQ);
    BOOL                 ( * _System icqCheckBlacklist)(HICQ, ULONG);
    BOOL                 ( * _System icqCheckIgnoreList)(HICQ, ULONG);
    USHORT               ( * _System icqQueryShortSequence)(HICQ);
    ULONG                ( * _System icqQueryBuild)(void);
    int                  ( * _System icqSetConnectTimer)(HICQ, USHORT);

    /*---[ Messages ]-----------------------------------------------------------------*/
    BOOL                 ( * _System icqInsertMessage)(HICQ, ULONG, ULONG, USHORT, time_t, ULONG, int, const char *);
    BOOL                 ( * _System icqInsertRTFMessage)(HICQ, ULONG, ULONG, USHORT, time_t, ULONG, const char *);

    BOOL                 ( * _System icqInsertAwayMessage)(HICQ, ULONG, const char *, char *);
    const char *         ( * _System icqQueryAwayMessage)(HICQ);
    int                  ( * _System icqSetAwayMessage)(HICQ, const char *);

    const MSGMGR *       ( * _System icqQueryMessageManager)(HICQ, USHORT);

    void                 ( * _System icqProcessServerConfirmation)(HICQ, ULONG, ULONG);

    BOOL                 ( * _System icqRemoveMessage)(HICQ, ULONG, HMSG);
    HMSG                 ( * _System icqQueryMessage)(HICQ, ULONG);
    void                 ( * _System icqNewUserMessage)(HICQ, ULONG, USHORT, const char *, const char *);
    int                  ( * _System icqNewMessageWithManager)(HICQ, ULONG, const MSGMGR *);
    int                  ( * _System icqNewMultiUserMessage)(HICQ, USHORT, const char *, const char *);

    BOOL                 ( * _System icqOpenSystemMessage)(HICQ);
    void                 ( * _System icqOpenUserMessageWindow)(HICQ, HUSER);
    int                  ( * _System icqSendMessage)(HICQ, ULONG, USHORT, unsigned char *);
    int                  ( * _System icqSendConfirmation)(HICQ, ULONG, USHORT, BOOL, char *);
    char *               ( * _System icqQueryMessageTitle)(HICQ, ULONG, BOOL, ICQMSG *, char *, int);
    int                  ( * _System icqSendQueued)(HICQ);
    int                  ( * _System icqRemoveMessageByID)(HICQ, ULONG);

    int                  ( * _System icqQueryFirstMessage)(HICQ, ULONG *, HMSG *);
    HMSG                 ( * _System icqQueryNextMessage)(HICQ, ULONG, HMSG);

    ULONG                ( * _System icqQueryUIN)(HICQ);

    /*---[ Event listeners ]----------------------------------------------------------*/

    void                 ( * _System icqExecuteGuiListeners)(HICQ, ULONG, char, USHORT, ULONG);
    int                  ( * _System icqAddGuiEventListener)(HICQ, void *, HWND);
    int                  ( * _System icqRemoveGuiEventListener)(HICQ, void *, HWND);

    /*---[ List management ]----------------------------------------------------------*/
    HLIST                ( * _System icqCreateList)(void);
    HLIST                ( * _System icqDestroyList)(HLIST);
    void                 ( * _System icqClearList)(HLIST);
    void *               ( * _System icqAddElement)(HLIST, int);
    int                  ( * _System icqRemoveElement)(HLIST, void *);
    void *               ( * _System icqQueryFirstElement)(HLIST);
    void *               ( * _System icqQueryLastElement)(HLIST);
    void *               ( * _System icqQueryNextElement)(HLIST, void *);
    void *               ( * _System icqQueryPreviousElement)(HLIST, void *);
    void *               ( * _System icqResizeElement)(HLIST, int, void *);

    /*---[ Searchs ]------------------------------------------------------------------*/
    HSEARCH              ( * _System icqBeginSearch)(HICQ, ULONG, ULONG, HWND, BOOL, SEARCHCALLBACK);
    int                  ( * _System icqEndSearch)(HICQ, ULONG);
    ULONG                ( * _System icqQuerySearchUIN)(HICQ, ULONG);
    int                  ( * _System icqSetSearchResponse)(HICQ, ULONG, const ICQSEARCHRESPONSE *);

    int                  ( * _System icqSearchByICQ)(HICQ, ULONG, HWND, SEARCHCALLBACK);
    int                  ( * _System icqSearchByMail)(HICQ, const char *, HWND, SEARCHCALLBACK);
    int                  ( * _System icqSearchRandom)(HICQ, USHORT, HWND, SEARCHCALLBACK);
    int                  ( * _System icqSearchByInformation)(HICQ, HWND, const char *, const char *,const char *, const char *, BOOL, SEARCHCALLBACK );

    /*---[ Requests ]----------------------------------------------------------------*/
    void *               ( * _System icqCreateRequest)(HICQ, ULONG, BOOL, ULONG, ULONG, const REQUESTMGR *, PEERDATA *, int);
    int                  ( * _System icqDestroyRequest)(HICQ, ULONG, BOOL);
    int                  ( * _System icqAcceptRequest)(HICQ, ULONG, const char *);
    int                  ( * _System icqRefuseRequest)(HICQ, ULONG, const char *);
    int                  ( * _System icqDestroySessionRequests)(HICQ, ULONG);
    int                  ( * _System icqRequestAccepted)(HICQ, ULONG, int, int, const char *);
    int                  ( * _System icqRequestRefused)(HICQ, ULONG, int, int, const char *);
    void *               ( * _System icqQueryNextRequest)(HICQ, ULONG, ULONG *);
    int                  ( * _System icqSetRequestSession)(HICQ, ULONG, BOOL, ULONG);

#ifdef linux
    /*---[ Resource management ]-----------------------------------------------------*/
    int                  ( * _System icqGetString)(HICQ, HMODULE, USHORT, char *, int);
    const void *         ( * _System icqGetResourceDescriptor)(HICQ, HMODULE,int);
    const void *         ( * _System icqGetDialogDescriptor)(HICQ, HMODULE, USHORT, int *, int);
#endif

    /*---[ Miscellaneous ]-----------------------------------------------------------*/

    int                  ( * _System icqOpenURL)(HICQ, const char *);
    void                 ( * _System icqInsertSendToOptions)(HICQ, hWindow, USHORT, BOOL, const MSGMGR *, int ( * _System)(HWND, USHORT, const char *, SENDTOCALLBACK *));
    void                 ( * _System icqInsertConfigPage)(HICQ, ULONG, USHORT, HWND, const DLGINSERT *, char *);
    int                  ( * _System icqExecuteCmd)(HICQ, char *);
    void                 ( * _System icqDumpPacket)(HICQ, HUSER, const char *, int, unsigned char *);
    const char *         ( * _System icqIP2String)(long, char *);
    int 		 ( * _System icqAbortSearchByWindow)(HICQ, HWND);
    HUSER 		 ( * _System icqCacheSearchResult)(HICQ, USHORT, ULONG, BOOL);
    int                  ( * _System icqConnectUser)(HICQ,ULONG,int);
    const PEERPROTMGR *  ( * _System icqQueryPeerProtocolManager)(HICQ, USHORT);
    void *               ( * _System icqQueryPeerProtocolData)(HICQ, USHORT);

    const void *         ( * _System icqQueryInternalTable)(HICQ, USHORT, char *, int);
    int                  ( * _System icqSendFile)(HICQ, ULONG, const char *, const char *);
    const char *         ( * _System icqQueryURL)(HICQ, const char *);
	
 };

 int EXPENTRY pwICQConfigureLibrary(const struct icqftable *);

#ifdef USEICQLIB

   #define ENTRY pwICQStaticLibraryEntryPoints

   extern const struct icqftable *pwICQStaticLibraryEntryPoints;

   /*---[ Skin manager ]-------------------------------------------------------------*/
   #define icqQuerySkinManager(i)					    ENTRY->icqQuerySkinManager(i);
   #define icqRegisterSkinManager(h, s)                 ENTRY->icqRegisterSkinManager(h, s)
   #define icqQuerySkinManagerName(h)                   ENTRY->icqQuerySkinManagerName(h)
   #define icqGetSkinDataBlock(h)                       ENTRY->icqGetSkinDataBlock(h)
   #define icqSetSkinDataBlock(h,d)                     ENTRY->icqSetSkinDataBlock(h,d)
   #define icqShowPopupMenu(h, u, m, x, y)              ENTRY->icqShowPopupMenu(h, u, m, x, y)
   #define icqInsertMenuOption(i, u, a, b, c, d)        ENTRY->icqInsertMenuOption(i, u, a, b, c, d)
   #define icqOpenConfigDialog(i,u,n)                   ENTRY->icqOpenConfigDialog(i,u,n)

   /*---[ Protocol managers ]--------------------------------------------------------*/
   #define icqRegisterServerProtocolManager(h, c)       ENTRY->icqRegisterServerProtocolManager(h, c)
   #define icqDeRegisterServerProtocolManager(h, c)     ENTRY->icqDeRegisterServerProtocolManager(h, c)
   #define icqRegisterPeerProtocolManager(i,p,d)        ENTRY->icqRegisterPeerProtocolManager(i,p,d)
   #define icqDeRegisterPeerProtocolManager(i,p)        ENTRY->icqDeRegisterPeerProtocolManager(i,p)

   /*---[ Users-list management ]----------------------------------------------------*/
   #define icqQueryUserHandle(i,u)                      ENTRY->icqQueryUserHandle(i,u)
   #define icqQueryUserByNick(i,n)                      ENTRY->icqQueryUserByNick(i,n)
   #define icqQueryNextUser(i,u)                        ENTRY->icqQueryNextUser(i,u)
   #define icqQueryUserNick(u)                          ENTRY->icqQueryUserNick(u)
   #define icqQueryUserEmail(u)                         ENTRY->icqQueryUserEmail(u)
   #define icqQueryUserFirstName(u)                     ENTRY->icqQueryUserFirstName(u)
   #define icqQueryUserLastName(u)                      ENTRY->icqQueryUserLastName(u)
   #define icqQueryUserSound(u)							ENTRY->icqQueryUserSound(u)
   #define icqQueryUserIP(i,u)							ENTRY->icqQueryUserIP(i,u)
   #define icqQueryUserGateway(i,u)			            ENTRY->icqQueryUserGateway(i,u)

   #define icqSetUserOnlineMode(h, u, m)                ENTRY->icqSetUserOnlineMode(h, u, m)
   #define icqSetUserIPandPort(i,u,g,a,p)		        ENTRY->icqSetUserIPandPort(i,u,g,a,p)

   #define icqLoadUserDB(i,u,a,b,c,d)                   ENTRY->icqLoadUserDB(i,u,a,b,c,d)
   #define icqSaveUserDB(i,u,a,b)                       ENTRY->icqSaveUserDB(i,u,a,b)

   #define icqUserOnline(i,u,m,a,b,c,d,e)               ENTRY->icqUserOnline(i,u,m,a,b,c,d,e)
   #define icqRegisterUserAction(u)                     ENTRY->icqRegisterUserAction(u)

   #define icqUpdateUserInfo(i,u,f,a,b,c,d,e)           ENTRY->icqUpdateUserInfo(i,u,f,a,b,c,d,e)

   #define icqAddUserInContactList(i,u)                 ENTRY->icqAddUserInContactList(i,u)
   #define icqAddUserInCache(i,u)                       ENTRY->icqAddUserInCache(i,u)
   #define icqBlacklistUser(i,u)                        ENTRY->icqBlacklistUser(i,u)
   #define icqRemoveUser(i,u)                           ENTRY->icqRemoveUser(i,u);
   #define icqRejectUser(i,u,s)                         ENTRY->icqRejectUser(i,u,s)
   #define icqRequestUserUpdate(i,u)                    ENTRY->icqRequestUserUpdate(i,u)
   #define icqAddNewUser(i,u,a,b,c,d,e)                 ENTRY->icqAddNewUser(i,u,a,b,c,d,e)
   #define icqInsertUser(i,u,a,b,c,d,e,f)               ENTRY->icqInsertUser(i,u,a,b,c,d,e,f)
   #define icqUpdateUserFlag(i,u,a,b)                   ENTRY->icqUpdateUserFlag(i,u,a,b)
   #define icqAjustUserIcon(i,u,f)                      ENTRY->icqAjustUserIcon(i,u,f)

   /*---[ Event Launchers ]----------------------------------------------------------*/
   #define icqSystemEvent(i,e)                          icqEvent(i,0,'S',e,0)
   #define icqEvent(i,u,t,e,p)                          ENTRY->icqEvent(i,u,t,e,p)
   #define icqUserEvent(i,u,e)                          ENTRY->icqUserEvent(i,u,e)

   /*---[ Configuration file ]-------------------------------------------------------*/
   #define icqLoadValue(i,k,d)                          ENTRY->icqLoadValue(i,k,d)
   #define icqLoadString(i,k,d,b,s)                     ENTRY->icqLoadString(i,k,d,b,s)
   #define icqLoadProfileString(i,k,a,b,c,d)            ENTRY->icqLoadProfileString(i,k,a,b,c,d)
   #define icqSaveValue(i,a,b)                          ENTRY->icqSaveValue(i,a,b)
   #define icqSaveString(i,a,b)                         ENTRY->icqSaveString(i,a,b)
   #define icqSaveProfileString(i,a,b,c)                ENTRY->icqSaveProfileString(i,a,b,c)

   /*---[ Plugin management ]--------------------------------------------------------*/
   #define icqLoadSymbol(p,a,b)                         ENTRY->icqLoadSymbol(p,a,b)
   #define icqQueryPlugin(i,k)                          ENTRY->icqQueryPlugin(i,k)
   #define icqGetPluginDataBlock(i,m)                   ENTRY->icqGetPluginDataBlock(i,m)
   #define icqQueryPluginDataBlock(i,p) 		ENTRY->icqQueryPluginDataBlock(i,p)
   #define icqQueryNextPlugin(i,p)			ENTRY->icqQueryNextPlugin(i,p)

   /*---[ Protocol ]-----------------------------------------------------------------*/
   #define icqReconnect(i)                              ENTRY->icqReconnect(i)
   #define icqSetOnlineMode(i,u)                        ENTRY->icqSetOnlineMode(i,u)
   #define icqClearToSend(i)                            ENTRY->icqClearToSend(i)
   #define icqSetServerBusy(h,f)                        ENTRY->icqSetServerBusy(h,f)
   #define icqQueryPeerPort(i)                          ENTRY->icqQueryPeerPort(i)
   #define icqQueryPeerAddress(i)                       ENTRY->icqQueryPeerAddress(i)
   #define icqSetLocalIP(i,l)                           ENTRY->icqSetLocalIP(i,l)
   #define icqSetServiceStatus(i,s,f)                   ENTRY->icqSetServiceStatus(i,s,f)
   #define icqIsOnline(i)                               ENTRY->icqIsOnline(i)
   #define icqDisconnect(i)                             ENTRY->icqDisconnect(i)
   #define icqSetPeerSession(i,u,s,p,d)                 ENTRY->icqSetPeerSession(i,u,s,p,d)
   #define icqGetPeerSession(i,u,p)                     ENTRY->icqGetPeerSession(i,u,p)
   #define icqBeginPeerSession(i,u)                     ENTRY->icqBeginPeerSession(i,u)

   /*---[ Miscellaneous (Core) ]-----------------------------------------------------*/
   #define icqInitialize(i,c)                           ENTRY->icqInitialize(i,c)
   #define icqSetCurrentUser(i,u,p,f)                   ENTRY->icqSetCurrentUser(i,u,p,f)
   #define icqIsActive(i)                               ENTRY->icqIsActive(i)

   #define icqCreateThread(i,e,a,b,c,d)                 ENTRY->icqCreateThread(i,e,a,b,c,d)
   #define icqStartThread(i,e,a,b,c,d)                  ENTRY->icqStartThread(i,e,a,b,c,d)
   #define icqWarning(h,u,i,t,m)			ENTRY->icqWarning(h,u,i,t,m)

   #define icqWriteUserLog(i,a,b,c)                     ENTRY->icqWriteUserLog(i,a,b,c)

   #define icqQueryUserDataBlockLength()                ENTRY->icqQueryUserDataBlockLength()
   #define icqConvertCodePage(i,b,t,s)                  ENTRY->icqConvertCodePage(i,b,t,s)

   #define icqQueryPassword(i,b)                        ENTRY->icqQueryPassword(i,b)
   #define icqQueryOnlineMode(i)                        ENTRY->icqQueryOnlineMode(i)
   #define icqProcessRemoteBlock(i,q)                   ENTRY->icqProcessRemoteBlock(i,q)
   #define icqSetCodePageConverter(i,c)                 ENTRY->icqSetCodePageConverter(i,c)
   #define icqQueryProgramPath(i,a,b,c)                 ENTRY->icqQueryProgramPath(i,a,b,c)
   #define icqQueryModeTable(i)                         ENTRY->icqQueryModeTable(i)
   #define icqQueryLogonStatus(i)                       ENTRY->icqQueryLogonStatus(i)
   #define icqValidateMsgEditHelper(i,u)                ENTRY->icqValidateMsgEditHelper(i,u)
   #define icqRegisterDefaultMessageProcessor(i,a,b)    ENTRY->icqRegisterDefaultMessageProcessor(i,a,b)
   #define icqQueryPath(i,a,b,c)                        ENTRY->icqQueryPath(i,a,b,c)
   #define icqSetPeerCookie(i,v,u)                      ENTRY->icqSetPeerCookie(i,v,u)
   #define icqQueryModeFlags(i)                         ENTRY->icqQueryModeFlags(i)
   #define icqRegisterUserDBManager(i,d)                ENTRY->icqRegisterUserDBManager(i,d)
   #define icqDeRegisterUserDBManager(i,d)              ENTRY->icqDeRegisterUserDBManager(i,d)

   #define icqQueryOfflineMode(i)                       ENTRY->icqQueryOfflineMode(i)
   #define icqGetCaps(i)                                ENTRY->icqGetCaps(i)
   #define icqSetCaps(i,u)                              ENTRY->icqSetCaps(i,u)

   #define icqQueryProtocolManager(i,t)                 ENTRY->icqQueryProtocolManager(i,t)
   #define icqQueryLocalIP(i)                           ENTRY->icqQueryLocalIP(i)


   #define icqQueryMaxMessageLength(i,u)                ENTRY->icqQueryMaxMessageLength(i,u)
   #define icqQueryTextModeFlag(i)						ENTRY->icqQueryTextModeFlag(i)

   #define icqCheckBlacklist(i,u)                       ENTRY->icqCheckBlacklist(i,u)
   #define icqCheckIgnoreList(i,u)                      ENTRY->icqCheckIgnoreList(i,u)
   #define icqQueryShortSequence(i)                     ENTRY->icqQueryShortSequence(i)
   #define icqQueryBuild()								ENTRY->icqQueryBuild()
   #define icqSetConnectTimer(i,t)                      ENTRY->icqSetConnectTimer(i,t)

   /*---[ Messages ]-----------------------------------------------------------------*/
   #define icqInsertMessage(i,u,s,t,m,a,b,c)            ENTRY->icqInsertMessage(i,u,s,t,m,a,b,c)
   #define icqInsertRTFMessage(i,u,s,t,a,b,c)           ENTRY->icqInsertRTFMessage(i,u,s,t,a,b,c)
   #define icqInsertAwayMessage(i,a,b,c)                ENTRY->icqInsertAwayMessage(i,a,b,c)
   #define icqQueryAwayMessage(i)                       ENTRY->icqQueryAwayMessage(i)
   #define icqSetAwayMessage(i,t)                       ENTRY->icqSetAwayMessage(i,t)
   #define icqQueryMessageManager(i,t)                  ENTRY->icqQueryMessageManager(i,t)
   #define icqProcessServerConfirmation(t,a,b)          ENTRY->icqProcessServerConfirmation(t,a,b)
   #define icqRemoveMessage(i,u,m)                      ENTRY->icqRemoveMessage(i,u,m)
   #define icqQueryMessage(i,u)                         ENTRY->icqQueryMessage(i,u)
   #define icqNewUserMessage(i,u,t,a,b)                 ENTRY->icqNewUserMessage(i,u,t,a,b)
   #define icqNewMessageWithManager(i, u, m)            ENTRY->icqNewMessageWithManager(i, u, m)
   #define icqNewMultiUserMessage(i, t, m, u)           ENTRY->icqNewMultiUserMessage(i, t, m, u)

   #define icqOpenSystemMessage(i)                      ENTRY->icqOpenSystemMessage(i)
   #define icqOpenUserMessageWindow(i,u)                ENTRY->icqOpenUserMessageWindow(i,u)
   #define icqSendMessage(i,u,t,m)                      ENTRY->icqSendMessage(i,u,t,m)
   #define icqSendConfirmation(i,u,a,b,c)               ENTRY->icqSendConfirmation(i,u,a,b,c)
   #define icqQueryMessageTitle(i,a,b,c,d,e)            ENTRY->icqQueryMessageTitle(i,a,b,c,d,e)
   #define icqSendQueued(i)                             ENTRY->icqSendQueued(i)
   #define icqRemoveMessageByID(i,h)                    ENTRY->icqRemoveMessageByID(i,h)
   #define icqQueryFirstMessage(i,u,m)                  ENTRY->icqQueryFirstMessage(i,u,m)
   #define icqQueryNextMessage(i,u,m)                   ENTRY->icqQueryNextMessage(i,u,m)

   #define icqQueryUIN(i)                               ENTRY->icqQueryUIN(i)

   /*---[ Event listeners ]----------------------------------------------------------*/
   #define icqExecuteGuiListeners(i,u,t,e,p)            ENTRY->icqExecuteGuiListeners(i,u,t,e,p)
   #define icqAddGuiEventListener(i,c,h)                ENTRY->icqAddGuiEventListener(i,c,h)
   #define icqRemoveGuiEventListener(i,c,h)             ENTRY->icqRemoveGuiEventListener(i,c,h)

   /*---[ List management ]----------------------------------------------------------*/
   #define icqCreateList()                              ENTRY->icqCreateList()
   #define icqDestroyList(h)                            ENTRY->icqDestroyList(h)
   #define icqClearList(h)                              ENTRY->icqClearList(h)
   #define icqAddElement(h,i)                           ENTRY->icqAddElement(h,i)
   #define icqRemoveElement(h,p)                        ENTRY->icqRemoveElement(h,p)
   #define icqQueryFirstElement(h)                      ENTRY->icqQueryFirstElement(h)
   #define icqQueryLastElement(h)                       ENTRY->icqQueryLastElement(h)
   #define icqQueryNextElement(h,p)                     ENTRY->icqQueryNextElement(h,p)
   #define icqQueryPreviousElement(h,p)                 ENTRY->icqQueryPreviousElement(h,p)
   #define icqResizeElement(h,s,p)                      ENTRY->icqResizeElement(h,s,p)

   /*---[ Searchs ]------------------------------------------------------------------*/
   #define icqBeginSearch(i,a,b,c,d,e)                  ENTRY->icqBeginSearch(i,a,b,c,d,e)
   #define icqEndSearch(i,s)                            ENTRY->icqEndSearch(i,s)
   #define icqQuerySearchUIN(i,s)                       ENTRY->icqQuerySearchUIN(i,s)
   #define icqSetSearchResponse(i,a,b)                  ENTRY->icqSetSearchResponse(i,a,b)
   #define icqSearchByICQ(i,a,b,c)                      ENTRY->icqSearchByICQ(i,a,b,c)
   #define icqSearchByMail(i,a,b,c)                     ENTRY->icqSearchByMail(i,a,b,c)
   #define icqSearchRandom(i,a,b,c)                     ENTRY->icqSearchRandom(i,a,b,c)
   #define icqSearchByInformation(i,a,b,c,d,e,f,g)      ENTRY->icqSearchByInformation(i,a,b,c,d,e,f,g)

   /*---[ Requests ]----------------------------------------------------------------*/
   #define icqCreateRequest(i, u, o, s, x, p, d, b)     ENTRY->icqCreateRequest(i, u, o, s, x, p, d, b)
   #define icqDestroyRequest(i, x, o)                   ENTRY->icqDestroyRequest(i, x, o)
   #define icqAcceptRequest(i, x, r)                    ENTRY->icqAcceptRequest(i, x, r)
   #define icqRefuseRequest(i, x, r)                    ENTRY->icqRefuseRequest(i, x, r)
   #define icqDestroySessionRequests(i, s)              ENTRY->icqDestroySessionRequests(i, s)
   #define icqRequestAccepted(i, x, p, b, r)            ENTRY->icqRequestAccepted(i, x, p, b, r)
   #define icqRequestRefused(i, x, p, b, r)             ENTRY->icqRequestRefused(i, x, p, b, r)
   #define icqQueryNextRequest(i,s,r)                   ENTRY->icqQueryNextRequest(i,s,r)
   #define icqSetRequestSession(i, n, o, s)             ENTRY->icqSetRequestSession(i, n, o, s)

   /*---[ Miscellaneous ]-----------------------------------------------------------*/
   #define icqOpenURL(i,u)                              ENTRY->icqOpenURL(i,u)
   #define icqInsertSendToOptions(i,m,t,f,h,c)          ENTRY->icqInsertSendToOptions(i,m,t,f,h,c)
   #define icqInsertConfigPage(i,u,t,h,d,a)             ENTRY->icqInsertConfigPage(i,u,t,h,d,a)
   #define icqExecuteCmd(i,c)                           ENTRY->icqExecuteCmd(i,c)
   #define icqDumpPacket(i,u,a,b,c)                     ENTRY->icqDumpPacket(i,u,a,b,c)
   #define icqIP2String(i,b)                            ENTRY->icqIP2String(i,b)
   #define icqAbortSearchByWindow(i,h)                  ENTRY->icqAbortSearchByWindow(i,h)
   #define icqCacheSearchResult(i,s,u,f)                ENTRY->icqCacheSearchResult(i,s,u,f)
   #define icqConnectUser(i,u,s)                        ENTRY->icqConnectUser(i,u,s)
   #define icqAbend(i,m)                                ENTRY->icqAbend(i,m)
   #define icqWriteSysLog(i,p,t)                        ENTRY->icqWriteSysLog(i,p,t)
   #define icqPrintLog                                  ENTRY->icqPrintLog
   #define icqWriteSysRC(i, p, r, t)                    ENTRY->icqWriteSysRC(i, p, r, t)

   #define icqQueryPeerProtocolManager(i,v)             ENTRY->icqQueryPeerProtocolManager(i,v)
   #define icqQueryPeerProtocolData(i,v)                ENTRY->icqQueryPeerProtocolData(i,v)
   #define icqQueryInternalTable(i,v,b,s)               ENTRY->icqQueryInternalTable(i,v,b,s)
   #define icqSendFile(i,u,f,m)                         ENTRY->icqSendFile(i,u,f,m)
   #define icqQueryURL(i,t)                             ENTRY->icqQueryURL(i,t)

#ifdef linux   
   #define icqGetString(i,m,n,b,s)                      ENTRY->icqGetString(i,m,n,b,s)
   #define icqGetResourceDescriptor(i,m)                ENTRY->icqGetResourceDescriptor(i,m,sizeof(struct icqResourceDescriptor))
   #define icqGetDialogDescriptor(i,m,n,e)              ENTRY->icqGetDialogDescriptor(i,m,n,&e,sizeof(struct icqDialogDescriptor))

#endif	

#endif


#ifdef __cplusplus
   }
#endif
