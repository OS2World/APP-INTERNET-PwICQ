/*
 * ftable.c - Tabela callback para facilitar a montagem de plugins em linux
 */

#ifdef __OS2__
   #pragma strings(readonly)
#endif

 #include <icqkernel.h>
 #include "icqcore.h"

/*---[ Implementacao da tabela ]-----------------------------------------------------------------*/

 const struct icqftable pwICQLibraryEntryPoints =
 {
    sizeof(struct icqftable),
    ICQFTABLE_VERSION,
    XBUILD,
#ifdef HOSTNAME
    PROJECT " version " PWICQVERSION " Build " BUILD " by " USER "@" HOSTNAME,
#else
    PROJECT " version " PWICQVERSION " Build " BUILD " by " USER,
#endif
    sizeof(ICQ),
    sizeof(USERINFO),
    PWICQVERSIONID,

    /*---[ Skin manager ]-------------------------------------------------------------*/
    icqQuerySkinManager,
    icqRegisterSkinManager,
    icqQuerySkinManagerName,
    icqGetSkinDataBlock,
    icqSetSkinDataBlock,

    icqRegisterServerProtocolManager,
    icqDeRegisterServerProtocolManager,

    icqRegisterPeerProtocolManager,
    icqDeRegisterPeerProtocolManager,

    icqShowPopupMenu,
    icqInsertMenuOption,
    icqOpenConfigDialog,

    /*---[ Users-list management ]----------------------------------------------------*/
    icqQueryUserHandle,
    icqQueryUserByNick,
    icqQueryNextUser,
    icqQueryUserNick,
    icqQueryUserEmail,
    icqQueryUserFirstName,
    icqQueryUserLastName,
    icqQueryUserSound,
    icqQueryUserIP,
    icqQueryUserGateway,
    icqSetUserOnlineMode,
    icqSetUserIPandPort,
    icqLoadUserDB,
    icqSaveUserDB,
    icqUserOnline,
    icqRegisterUserAction,
    icqUpdateUserInfo,
    icqAddUserInContactList,
    icqAddUserInCache,
    icqBlacklistUser,
    icqRemoveUser,
    icqRejectUser,
    icqRequestUserUpdate,
    icqAddNewUser,
    icqInsertUser,
    icqUpdateUserFlag,
    icqAjustUserIcon,

    /*---[ Event Launchers ]----------------------------------------------------------*/
    icqSystemEvent,
    icqUserEvent,
    icqEvent,
    icqAbend,

    /*---[ Configuration file ]-------------------------------------------------------*/
    icqLoadValue,
    icqLoadString,
    icqLoadProfileString,

    icqSaveValue,
    icqSaveString,
    icqSaveProfileString,

    /*---[ Plugin management ]--------------------------------------------------------*/
    icqLoadSymbol,
    icqQueryPlugin,
    icqGetPluginDataBlock,
    icqQueryPluginDataBlock,
    icqQueryNextPlugin,

    /*---[ Protocol ]-----------------------------------------------------------------*/
    icqReconnect,
    icqSetOnlineMode,
    icqClearToSend,
    icqSetServerBusy,
    icqQueryPeerPort,
    icqQueryPeerAddress,
    icqSetLocalIP,

    icqSetServiceStatus,
    icqIsOnline,
    icqDisconnect,
    icqSetPeerSession,
    icqGetPeerSession,
    icqBeginPeerSession,

    /*---[ Miscellaneous (Core) ]-----------------------------------------------------*/
    icqInitialize,
    icqSetCurrentUser,
    icqIsActive,
    icqCreateThread,
    icqStartThread,
    icqWriteSysLog,
    icqPrintLog,
    icqWriteSysRC,
    icqWarning,
    icqWriteUserLog,
    icqQueryUserDataBlockLength,
    icqConvertCodePage,
    icqQueryPassword,
    icqQueryOnlineMode,
    icqProcessRemoteBlock,
    icqSetCodePageConverter,
    icqQueryProgramPath,
    icqQueryModeTable,
    icqQueryLogonStatus,
    icqValidateMsgEditHelper,
    icqRegisterDefaultMessageProcessor,
    icqQueryPath,
    icqSetPeerCookie,
    icqQueryModeFlags,
    icqQueryMaxMessageLength,
	
    icqRegisterUserDBManager,
    icqDeRegisterUserDBManager,

    icqQueryOfflineMode,
    icqGetCaps,
    icqSetCaps,

    icqQueryProtocolManager,
    icqQueryLocalIP,

    icqQueryTextModeFlag,
    icqCheckBlacklist,
    icqCheckIgnoreList,
    icqQueryShortSequence,
    icqQueryBuild,
    icqSetConnectTimer,

    /*---[ Messages ]-----------------------------------------------------------------*/
    icqInsertMessage,
    icqInsertRTFMessage,
    icqInsertAwayMessage,
    icqQueryAwayMessage,
    icqSetAwayMessage,
    icqQueryMessageManager,
    icqProcessServerConfirmation,
    icqRemoveMessage,
    icqQueryMessage,
    icqNewUserMessage,
    icqNewMessageWithManager,
    icqNewMultiUserMessage,
    icqOpenSystemMessage,
    icqOpenUserMessageWindow,
    icqSendMessage,
    icqSendConfirmation,
    icqQueryMessageTitle,
    icqSendQueued,
    icqRemoveMessageByID,
    icqQueryFirstMessage,
    icqQueryNextMessage,

    icqQueryUIN,

    /*---[ Event listeners ]----------------------------------------------------------*/
    icqExecuteGuiListeners,
    icqAddGuiEventListener,
    icqRemoveGuiEventListener,

    /*---[ List management ]----------------------------------------------------------*/
    icqCreateList,
    icqDestroyList,
    icqClearList,
    icqAddElement,
    icqRemoveElement,
    icqQueryFirstElement,
    icqQueryLastElement,
    icqQueryNextElement,
    icqQueryPreviousElement,
    icqResizeElement,

    /*---[ Searchs ]------------------------------------------------------------------*/
    icqBeginSearch,
    icqEndSearch,
    icqQuerySearchUIN,
    icqSetSearchResponse,

    icqSearchByICQ,
    icqSearchByMail,
    icqSearchRandom,
    icqSearchByInformation,

    /*---[ Requests ]-----------------------------------------------------------------*/
    icqCreateRequest,
    icqDestroyRequest,
    icqAcceptRequest,
    icqRefuseRequest,
    icqDestroySessionRequests,
    icqRequestAccepted,
    icqRequestRefused,
    icqQueryNextRequest,
    icqSetRequestSession,

#ifdef linux
    /*---[ Resource management ]-----------------------------------------------------*/
    icqGetString,
    _icqGetResourceDescriptor,
    _icqGetDialogDescriptor,
#endif

    /*---[ Miscellaneous ]-----------------------------------------------------------*/
    icqOpenURL,
    icqInsertSendToOptions,
    icqInsertConfigPage,
    icqExecuteCmd,
    icqDumpPacket,
    icqIP2String,
    icqAbortSearchByWindow,
    icqCacheSearchResult,
    icqConnectUser,
    icqQueryPeerProtocolManager,
    icqQueryPeerProtocolData,
    icqQueryInternalTable,
    icqSendFile,
    icqQueryURL

 };



