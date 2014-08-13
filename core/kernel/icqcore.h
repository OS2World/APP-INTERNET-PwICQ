/*
 * icqcore.h
 */

 #include <icqftable.h>

/*---[ Macros ]-------------------------------------------------------------*/

 #define threadLog(t,s)  icqWriteThreadLog(t,PROJECT,s);

#ifdef DEBUG_LOG

 #define sysLog(i,s) icqWriteExtendedLog(i,PROJECT,s,__FILE__,__LINE__);

 void icqWriteExtendedLog(HICQ,const char *, const char *, const char *, int);

#else

 #define sysLog(i,s)     icqWriteSysLog(i,PROJECT,s);

#endif


/*---[ Constantes ]---------------------------------------------------------*/

 extern const struct icqftable pwICQLibraryEntryPoints;

/*---[ Prototipos ]---------------------------------------------------------*/

 void initialize(HICQ, int,char **, STATUSCALLBACK *);
 void terminate(HICQ);

 int  oneMinute(HICQ);
 int  oneSecond(HICQ);

 void loadPlugins(HICQ);
 void unloadPlugins(HICQ);
 void startPlugins(HICQ);

 void loadUserList(HICQ);
 void saveUserList(HICQ, BOOL);

 char *loadTextFile(HICQ, const char *, time_t *);
 BOOL searchTextFile(const char *, const char *);

 void ajustUserIcon(HICQ, HUSER, BOOL);
 void loadBlackList(HICQ);
 void loadIgnoreList(HICQ);

 int  sendMessage(HICQ, ULONG, USHORT, unsigned char *, BOOL);

 void icqChkConfigFile(HICQ,char *);

 void           loadMessageList(HICQ);
 void           saveMessageList(HICQ);
// const char *   searchMessageManager(HICQ, HUSER, ULONG, HMSG, BOOL);
 HMSG           addInputMessage(HICQ, ULONG, HUSER *, USHORT, time_t, int, const char *,BOOL);

 BOOL           isEmpty(const char *);

 void           openUserMessageWindow(HICQ, HUSER, BOOL);



