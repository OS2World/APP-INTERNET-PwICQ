/*
 * ishared.h
 */

#ifndef ISCLIENT_INCLUDED

   #define ISCLIENT_INCLUDED 1

#ifdef __OS2__
   #define INCL_DOSPROCESS
#endif

#ifdef PWICQ
   #include <icqtlkt.h>
   extern HMODULE module;
#else
   #include <icqnetw.h>
#endif

#ifdef WIN32
   #include <windows.h>
   #define _System WINAPI
#endif

/*---[ Base types ]--------------------------------------------------------------------------------------*/

   #include <stdio.h>
   #include <i-share.h>

/*---[ Defines ]-----------------------------------------------------------------------------------------*/

 #ifndef CACHE_SEARCHS
    #define CACHE_SEARCHS    100
 #endif

 #ifndef CACHE_USERS
    #define CACHE_USERS      50
 #endif

 #ifndef PPS_LIMIT
    #define PPS_LIMIT	     10
 #endif

 #ifdef WINICQ
    #define WINICQAPI_VERSION 0x00010002
 #endif

 #ifndef BUILD
    #define BUILD __DATE__ " " __TIME__
 #endif

/*---[ Macros ]------------------------------------------------------------------------------------------*/

#ifdef __OS2__
   #define INTER_PACKET_DELAY  DosSleep(5);
   #define yield()	       DosSleep(0)
   #define THREAD_ID           int

   #define delayloop(f,t,c)    for(f=0;f<t && c;f++) DosSleep(10);

#endif

#ifdef linux

   #include <unistd.h>
   #include <sched.h>

   #define INTER_PACKET_DELAY  usleep(500);
//   #define SHUTDOWN_DELAY      usleep(100);
   #define strlwr(x)           ishare_strlwr(x)
   #define yield()             sched_yield()
   #define THREAD_ID           int
   #define delayloop(f,t,c)    for(f=0;f<t && c;f++) usleep(100);

   void ICQAPI ishare_strlwr(char *);
#endif

#ifdef WIN32
   #define INTER_PACKET_DELAY  Sleep(10);
   #define yield()	       Sleep(0)
   #define THREAD_ID           HANDLE
   #define delayloop(f,t,c)    for(f=0;f<t && c;f++) Sleep(10);
#endif

/*---[ Structures ]--------------------------------------------------------------------------------------*/

   #pragma pack(1)

   typedef struct _list_header
   {
      struct _list_header 	*up;
      struct _list_header 	*down;
   } ISHARED_LISTHEADER;

   typedef struct _list_id
   {
      struct _list_header 	*first;
      struct _list_header 	*last;
   } ISHARED_LIST;

#ifdef CLIENT

   typedef struct _file_descriptor
   {
      ISHARED_LISTHEADER	l;
      USHORT			fileID;
      ULONG			fileSize;
      UCHAR			md5[16];
      UCHAR			offset;
      UCHAR			filePath[1];
   } ISHARED_FILEDESCR;

   typedef struct _result_descriptor
   {
      ISHARED_LISTHEADER	l;
      long			userIP;
      USHORT			userPort;
      ULONG		        size;		/* File size					*/
      UCHAR		        md5[16];	/* File MD5 checksum				*/
      ULONG		        uin;		/* The user UIN (Can be 0)			*/
      USHORT		        fileID;	        /* File sequence				*/
      UCHAR		        filename[1];	/* The filename found				*/
   } ISHARED_RESULT;

   typedef struct _search_descriptor
   {
      ISHARED_LISTHEADER l;
      USHORT             id;
      USHORT		 idle;
      USHORT		 resultCount;
#ifdef ADMIN
      struct _admin_user *adm;
#endif
      ISHARED_LIST	 result;
		
      UCHAR              key[1];
   } ISHARED_SEARCHDESCR;

   typedef struct _ishared_cached
   {
      long      userIP;			/* IP from person who's searching the file	*/
      USHORT    userPort;		/* Port from person who's searching the file	*/
      USHORT	idle;			/* Idle time                                    */
      USHORT	id;	     		/* Search ID					*/
   } ISHARED_CACHED;

   typedef struct _ishared_user
   {
      long   ip;
      USHORT port;
      UCHAR  idle;
      UCHAR  kpl;
      USHORT seed;
   } ISHARED_USER;

#else

   #define ISHARED_USER void

#endif

   typedef struct _admin_user
   {
      ISHARED_LISTHEADER  	l;
      USHORT			type;	// 0 = Admin session 1 = File transfer
      int		 	sock;
      int			level;
      USHORT		        rc;
      THREAD_ID		        thread;
      FILE			*hFile;
      struct _ishared_config	*cfg;
   } ISHARED_ADMIN;

   typedef struct _transfer_file
   {
      ISHARED_ADMIN		adm;
      ULONG			fileSize;
      ULONG			fileKey;
      UCHAR			md5[16];
      USHORT			fileID;
      char			filename[1];
   } ISHARED_TRANSFER;

#ifdef WINICQ

   #define MAX_WINICQ_USER 200

   typedef struct _winicq_user
   {
      ISHARED_LISTHEADER l;
      ULONG                     uin;
      long                      ip;
   } ISHARED_WINICQUSER;
#endif

   typedef struct _ishared_config
   {
      USHORT 			sz;
      BOOL   			active;
      int    			sock;
      short  			kplTimer;
      ULONG  			myIP;
      int			packetCounter;
      int			packetSequence;

#ifdef CLIENT
      USHORT			fileID;
      USHORT			searchID;
      ISHARED_LIST		sharedFiles;
      ISHARED_LIST		mySearches;
      int               	cachePtr;
      ISHARED_CACHED    	cached[CACHE_SEARCHS];
      ISHARED_USER		users[CACHE_USERS];
#endif

#ifdef WINICQ
      int			winICQTimer;
      BOOL			winICQEnabled;
      ISHARED_WINICQUSER	winICQUsers[MAX_WINICQ_USER];
#endif

#ifdef ADMIN
      int			admSock;
      ISHARED_LIST		admUsers;
#endif

#ifdef PWICQ
      THREAD_ID			mainThread;
      int               msgSequence;
      HICQ   			icq;
#else
      ISHARED_LIST  	        config;
#endif

      ISHARE_STATUS		*st;

   } ISHARED_CONFIG;

   #pragma pack()

/*---[ Macros ]------------------------------------------------------------------------------------------*/

#ifdef PWICQ

   #define log(c,x) 			icqWriteSysLog(cfg->icq,PROJECT,x)
   #define netlog(c,t) 			icqWriteNetworkErrorLog(c->icq, PROJECT, t)
   #define loadNickname(c,b,s) 		icqLoadString(c->icq, "NickName", "", b, s);
   #define loadString(c,k,d,b,s) 	icqLoadString(c->icq, "ISH:" k, d, b, s);
   #define loadValue(c,k,d) 		icqLoadValue(c->icq, "ISH:" k, d)
   #define queryUIN(c) 			icqQueryUIN(cfg->icq)
   #define isActive(c) 			c->active && icqIsActive(c->icq)
   #define localIP(c)			icqQueryLocalIP(c->icq)

   #define queryPath(c,v,b,s)	        icqQueryPath(c->icq,v,b,s);

#else

   #define log(c,x)			ishare_log(PROJECT,x)
   #define netlog(c,x)			ishare_log(PROJECT,x)
   #define loadNickname(c,b,s)		ishare_loadString(c,"nickname","John Doe",b,s)
   #define loadString(c,k,d,b,s)	ishare_loadString(c,k,d,b,s)
   #define loadValue(c,k,d)		ishare_loadValue(c,k,d)
   #define queryUIN(c)			0
   #define isActive(c)			c->active
   #define localIP(c)			c->myIP
   #define queryPath(c,v,b,s)	        strncpy(b,v,s)

   void         ICQAPI ishare_log(const char *, const char *);
   void         ICQAPI ishare_loadConfig(ISHARED_CONFIG *);
   const char * ICQAPI ishare_loadString(ISHARED_CONFIG *, const char *, const char *, char *, int);
   int          ICQAPI ishare_loadValue(ISHARED_CONFIG *, const char *, int);

#endif

#ifdef USEICQNETW
   #include <icqnetw.h>
   #define ishare_RecvFrom(s,i,p,b,q)   icqRecvFrom(s,i,p,b,q)
   #define ishare_OpenUDP(p) 		icqOpenUDP(p)
   #define ishare_OpenTCP(p) 		icqOpenTCP(p)
   #define ishare_CloseSocket(s)	icqCloseSocket(s)
   #define ishare_ShutdownSocket(s)	icqShutdownSocket(s)
   #define ishare_EnableBroadcast(s)    icqEnableBroadcast(s)
   #define ishare_Listen(s)		icqListen(s)
   #define ishare_Accept(s,i,p)         icqAcceptConnct(s,i,p)
   #define ishare_GetPort(s)    	icqGetPort(s)
   #define ishare_Recv(s,p,b)		icqRecv(s,p,b)
   #define ishare_Send(s,p,b)		icqSend(s,p,b)
   #define ishare_SendTo(s,i,p,b,q)     icqSendTo(s,i,p,b,q)
   #define ishare_GetHostByName(n)      icqGetHostByName(n)
   #define ishare_GetHostID()		icqGetHostID()
   #define ishare_ConnectSock(s,i,p)    icqConnectSock(s,i,p)

#else

   #error Non ICQNETW version isnt defined

#endif

#ifndef MSG_NOSIGNAL
   #define MSG_NOSIGNAL 0
#endif

   #define walklist(V,L) for(V=(void *)L.first;V;V=(void *)V->l.down)

   #define DECLARE_UDP_PACKET_PROCESSOR(p)  int ishare_udp_##p(ISHARED_CONFIG *, long, int, int, ISHARE_UDP_##p *);
   #define BEGIN_UDP_PACKET_PROCESSOR(p)    int ishare_udp_##p(ISHARED_CONFIG *cfg, long fromIP, int fromPort, int sz, ISHARE_UDP_##p *pkt)
   #define END_UDP_PACKET_PROCESSOR(p)      return 0;
   #define USE_UDP_PACKET_PROCESSOR( x )    (void *)(ishare_udp_##x)

   #define DECLARE_TCP_PACKET_PROCESSOR(p)  int ishare_tcp_##p(ISHARED_ADMIN *, ISHARED_CONFIG *, char *);
   #define BEGIN_TCP_PACKET_PROCESSOR(p)    int ishare_tcp_##p(ISHARED_ADMIN *adm, ISHARED_CONFIG *cfg, char *buffer)
   #define END_TCP_PACKET_PROCESSOR(p)      return 0;
   #define USE_TCP_PACKET_PROCESSOR( l, p ) { l, ishare_tcp_##p }
   #define NO_TCP_PACKET_PROCESSOR          { 0, NULL }


   #define INITIALIZE_TCP_PACKET_POINTER(x,c) memset(x,0,sizeof(ISHARE_TCP_##c));x->h.marker = ISHARE_TCP_MARKER;x->h.id=ISHARETCP_##c
   #define ISHARE_DECLARE_TCP_PREFIX(v,p)  static const ISHARE_TCP_PREFIX v = { ISHARE_TCP_MARKER, ISHARETCP_##p }

/*---[ Debug macros ]------------------------------------------------------------------------------------*/

#ifndef DBGMessage

 #ifdef DEBUGSTREAM
    #define DEBUGFILE DEBUGSTREAM
 #endif

 #ifndef DEBUGFILE
    #define DEBUGFILE stderr
 #endif

 #ifdef DEBUG

    #define OPEN_DEBUG() FILE * DEBUGFILE = fopen("/tmp/" PROJECT ".log","a")
    #define CLOSE_DEBUG() fclose(DEBUGFILE)

    #define DBGOpen(x)    freopen(x ".DBG","w+",stderr);freopen(x ".OUT","w+",stdout);
    #define DBGMessage(x) fprintf(DEBUGFILE,"%s(%d):\t%s\n",__FILE__,__LINE__,x);fflush(DEBUGFILE);
    #define CHKPoint()    fprintf(DEBUGFILE,"%s(%d):\t%s\t\t(" __DATE__ " " __TIME__")\n",__FILE__,__LINE__,__FUNCTION__);fflush(DEBUGFILE);
    #define DBGTrace(x)   fprintf(DEBUGFILE,"%s(%d):\t%s = %ld\n",__FILE__,__LINE__,#x, (unsigned long) x);fflush(DEBUGFILE);
    #define DBGTracex(x)  fprintf(DEBUGFILE,"%s(%d):\t%s = %08lx\n",__FILE__,__LINE__,#x, (unsigned long) x);fflush(DEBUGFILE);
    #define DBGPrint(x,y) fprintf(DEBUGFILE,"%s(%d):\t" x "\n",__FILE__,__LINE__,y);fflush(DEBUGFILE);
    #define DBGString(x)  if(x == NULL) { fprintf(DEBUGFILE,"%s(%d):\t%s = NULL\n",__FILE__,__LINE__,#x);fflush(DEBUGFILE); } else { fprintf(DEBUGFILE,"%s(%d):\t%s(%08lx) = \"%s\"\n",__FILE__,__LINE__,#x,(ULONG) x, x);fflush(DEBUGFILE); }
    #define DBGPos(x,y)   fprintf(DEBUGFILE,"%s(%d):\t%s->%s = %ld\n",__FILE__,__LINE__,#x,#y, ((unsigned long) &x->y) - ((ULONG) x));fflush(DEBUGFILE)
    #define DBGMsg(x,y,z) fprintf(DEBUGFILE,"%s(%d):\t" x "\n",__FILE__,__LINE__,y,z);fflush(DEBUGFILE);

 #else
    #define OPEN_DEBUG() /* */
    #define CLOSE_DEBUG() /* */

    #define DBGOpen(x) /* x */
    #define DBGMessage(x) /* x */
    #define DBGTrace(x) /* x */
    #define DBGTracex(x) /* x */
    #define CHKPoint()  /* */
    #define DBGPrint(x,y) /* x y */
    #define DBGString(x) /* x */
    #define DBGPos(x,y)     /* x->y */
    #define DBGMsg(x,y,z) /* x,y,z */

 #endif

#endif

/*---[ Thread support ]----------------------------------------------------------------------------------*/

 #ifdef PWICQ
    #define DECLARE_THREAD(x)      	static void _System x(HTHREAD);
    #define THREAD_START(x)        	static void _System x(HTHREAD thread_handler)
    #define GET_THREAD_PARAMETER() 	thread_handler->parm
    #define THREAD_END()		return
    #define ishare_beginThread(c,s,e,p) icqCreateThread(c->icq,e,s,0,p,"ish")
 #else

    #ifdef __OS2__
       #define STDOS2  1
       #define DECLARE_THREAD(x)      	static void _System x(ULONG);
       #define THREAD_START(x)        	static void _System x(ULONG thread_parm)
       #define THREAD_END()		return
       #define GET_THREAD_PARAMETER() 	thread_parm
       THREAD_ID ICQAPI ishare_beginThread(ISHARED_CONFIG *, int, void (* _System)(ULONG), void *);
    #endif

    #ifdef WIN32
       #define DECLARE_THREAD(x)      	static DWORD WINAPI x(LPVOID);
       #define THREAD_START(x)        	static DWORD WINAPI x(LPVOID thread_parm)
       #define GET_THREAD_PARAMETER() 	thread_parm
       #define THREAD_END()		return 0
       #define ishare_beginThread(c,s,e,p) CreateThread(NULL,s,e,p,0,NULL)
    #endif

    #ifdef linux
       #define STDLINUX  1
       #define DECLARE_THREAD(x)      	static void _System x(ULONG);
       #define THREAD_START(x)        	static void _System x(ULONG thread_parm)
       #define THREAD_END()		return
       #define GET_THREAD_PARAMETER() 	thread_parm
       THREAD_ID ICQAPI ishare_beginThread(ISHARED_CONFIG *, int, void (* _System)(ULONG), void *);
    #endif

 #endif

 #ifdef PWICQ
    extern const MSGMGR pwICQMessageManager;
 #endif

/*---[ Data file formats ]-------------------------------------------------------------------------------*/

 typedef struct _control_file_record
 {
    USHORT sz;
    UCHAR  md5[16];
    long   user;
    long   time;
    ULONG  begin;
    ULONG  end;
 } ISHARE_CTL_RECORD;

 typedef struct _control_file_header
 {
    USHORT sz;
    USHORT status;	// 0 = Receiving, 1 = MD5 mismatch 2 = Complete
    UCHAR  md5[16];
    ULONG  fileSize;
    ULONG  fileKey;
    USHORT szFilename;
 } ISHARE_CTL_HEADER;


/*---[ Packet processors ]-------------------------------------------------------------------------------*/

   DECLARE_UDP_PACKET_PROCESSOR( HELLO         )
   DECLARE_UDP_PACKET_PROCESSOR( HELLOACK      )
   DECLARE_UDP_PACKET_PROCESSOR( KEEPALIVE     )
   DECLARE_UDP_PACKET_PROCESSOR( FOUND         )
   DECLARE_UDP_PACKET_PROCESSOR( SEARCH        )

#ifdef PWICQ
   DECLARE_UDP_PACKET_PROCESSOR( MESSAGE       )
#endif

   DECLARE_TCP_PACKET_PROCESSOR( LOGON         )
   DECLARE_TCP_PACKET_PROCESSOR( SEARCHTEXT    )
   DECLARE_TCP_PACKET_PROCESSOR( STOP          )
   DECLARE_TCP_PACKET_PROCESSOR( BEGINTRANSFER )
   DECLARE_TCP_PACKET_PROCESSOR( SENDINGFILE   )


/*---[ Prototipes ]--------------------------------------------------------------------------------------*/

   void 		  ishare_hello(ISHARED_CONFIG *,long);

   void 		  ICQAPI ishare_initialize(ISHARED_CONFIG *);

   void   		  ICQAPI ishare_main(ISHARED_CONFIG *);

   void   		  ICQAPI ishare_timer(ISHARED_CONFIG *);
   int    	 	  ICQAPI ishare_send(ISHARED_CONFIG *, long, BOOL, int, void *);
   int    		  ICQAPI ishare_sendTo(ISHARED_CONFIG *, long, USHORT, BOOL, int, void *);
   USHORT 		  ICQAPI ishare_crc16(int, const char *);

   void   		  ICQAPI ishare_insertItem(ISHARED_LIST *, ISHARED_LISTHEADER *);
   void   		  ICQAPI ishare_removeItem(ISHARED_LIST *, ISHARED_LISTHEADER *);
   void			  ICQAPI ishare_deleteList(ISHARED_LIST *);

   int    		  ICQAPI ishare_distribute(ISHARED_CONFIG *, int, ISHARE_UDP_PREFIX *, long, ISHARED_USER *);

   void	  		  ICQAPI ishare_loadFolder(ISHARED_CONFIG *, const char *);
   void 		  ICQAPI ishare_loadMD5(ISHARED_CONFIG *);

   ISHARED_SEARCHDESCR *  ICQAPI ishare_findSearch(ISHARED_CONFIG *, USHORT id);
   ISHARED_SEARCHDESCR *  ICQAPI ishare_search(ISHARED_CONFIG *, const char *, void *);

   ISHARED_SEARCHDESCR *  ICQAPI ishare_createSearch(ISHARED_CONFIG *,const char *);
   int                    ICQAPI ishare_destroySearch(ISHARED_CONFIG *, ISHARED_SEARCHDESCR *);

   ISHARED_USER *         ICQAPI ishare_cacheUser(ISHARED_CONFIG *, long, USHORT);
   ISHARED_USER *         ICQAPI ishare_getUser(ISHARED_CONFIG *, long);

   const char *           ICQAPI ishare_formatIP(long, char *);

   int                    ICQAPI ishare_MDFile(ISHARED_CONFIG *, const char *, char *);

   void 		  ICQAPI ishare_sendTCPStatus(ISHARED_ADMIN *, int);

   int                    ICQAPI ishare_createControlFile(ISHARED_TRANSFER *);

#ifdef CLIENT
   void 		          ICQAPI ishare_startTCPListener(ISHARED_CONFIG *);
   void                   ICQAPI ishare_stopTCPListener(ISHARED_CONFIG *);
   int                    ICQAPI ishare_requestFile(ISHARED_CONFIG *, long, USHORT, USHORT, const UCHAR *, const char *, ULONG);
   int                    ICQAPI ishare_updateControlFile(ISHARED_CONFIG *, char *, ULONG, ULONG, ULONG);
   char *                 ICQAPI ishare_makeControlFilePath(ISHARED_CONFIG *, char *);

#endif

#ifdef __OS2__
   int                    ICQAPI ishare_SetEA(const char *, const char *, const char *);
#endif

   int 		          ICQAPI ishare_RecBlock(int, int, void *, const USHORT *);
   int                    ICQAPI ishare_beginSession(ISHARED_ADMIN *);
   USHORT                 ICQAPI ishare_RecShort(ISHARED_ADMIN *);
   int                    ICQAPI ishare_RecString(ISHARED_ADMIN *, char *, int);
   int                    ICQAPI ishare_RecFilename(ISHARED_ADMIN *, char *, int);
   int                    ICQAPI ishare_SendFilename(ISHARED_ADMIN *, char *, int);
   int                    ICQAPI ishare_SendString(ISHARED_ADMIN *, char *, int);



#endif
