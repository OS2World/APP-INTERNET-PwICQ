/*
 * tcp.c - TCP connection manager
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

 #include <stdio.h>
 #include <malloc.h>
 #include <string.h>
 #include <stdlib.h>

 #include "ishared.h"

/*---[ Structures and Macros ]---------------------------------------------------------------------------*/


/*---[ Function processors ]-----------------------------------------------------------------------------*/

 #pragma pack(1)

 struct tcp_commands
 {
    int		level;
    int		(*exec)(ISHARED_ADMIN *, ISHARED_CONFIG *, char *);
 };

 /*
  * Acess levels:
  *
  * 0 - No logon required
  * 1 - No logon if connected thru loopback
  * 3 - Logon required
  * 4 - Logon and loopback connection is required
  *
  */

 static const struct tcp_commands tcp_cmd[ISHARETCP_UNKNOWN] =
 {
    NO_TCP_PACKET_PROCESSOR,                     /*  0 ISHARETCP_ERROR          Error                  */
    USE_TCP_PACKET_PROCESSOR( 0, LOGON ),        /*  1 ISHARETCP_LOGON          Logon                  */
    USE_TCP_PACKET_PROCESSOR( 0, BEGINTRANSFER), /*  2 ISHARETCP_BEGINTRANSFER  Transfer request       */
    USE_TCP_PACKET_PROCESSOR( 3, SEARCHTEXT ),   /*  3 ISHARETCP_SEARCHTEXT     Search for string      */
    NO_TCP_PACKET_PROCESSOR,                     /*  4 ISHARETCP_SEARCHMD5      Search for MD5         */
    NO_TCP_PACKET_PROCESSOR,                     /*  5 ISHARETCP_SEARCHSTATUS   Search status          */
    NO_TCP_PACKET_PROCESSOR,                     /*  6 ISHARETCP_FOUND          File found             */
    NO_TCP_PACKET_PROCESSOR,                     /*  7                                                 */
    NO_TCP_PACKET_PROCESSOR,                     /*  8 ISHARETCP_REQUESTFILE    Begin download         */
    NO_TCP_PACKET_PROCESSOR,                     /*  9                                                 */
    NO_TCP_PACKET_PROCESSOR,                     /* 10                                                 */
    USE_TCP_PACKET_PROCESSOR( 1, STOP       ),   /* 11                                                 */
    USE_TCP_PACKET_PROCESSOR( 0, SENDINGFILE),   /* 12 Sending file                                    */

 };


/*---[ Prototipes ]--------------------------------------------------------------------------------------*/

 DECLARE_THREAD(admMain)
 DECLARE_THREAD(admThread)

/*---[ Implementation ]----------------------------------------------------------------------------------*/

 void ICQAPI ishare_startTCPListener(ISHARED_CONFIG *cfg)
 {
    if(cfg->sz != sizeof(ISHARED_CONFIG))
    {
       log(cfg,"Invalid configuration block");
       return;
    }
    CHKPoint();
    ishare_beginThread(cfg,16384,admMain,cfg);
    CHKPoint();

 }

 void ICQAPI ishare_stopTCPListener(ISHARED_CONFIG *cfg)
 {
    ISHARED_ADMIN *adm;
    int		  f;

    if(cfg->admSock > 0)
       ishare_ShutdownSocket(cfg->admSock);

    walklist(adm,cfg->admUsers)
       adm->rc = ISHARE_STATUS_SHUTDOWN;

    DBGTracex(cfg);
    DBGTracex(cfg->admUsers.first);

    delayloop(f, 300, cfg->admUsers.first);

    CHKPoint();

 }

 THREAD_START(admMain)
 {
    ISHARED_CONFIG      *cfg = (ISHARED_CONFIG *) GET_THREAD_PARAMETER();
    int  		sock;
    long 		ip;
    int  		port;
    ISHARED_ADMIN	*adm;
    char		buffer[0x0100];

    DBGMessage("Administration service started");
    DBGTracex(cfg);

    cfg->admSock = ishare_OpenTCP(loadValue(cfg,"admin",ISHARE_ADM_PORT));

    if(cfg->admSock < 0)
    {
       log(cfg,"Can't create administration port");
#ifdef WIN32
       return 0;
#else
       return;
#endif
    }

    ishare_Listen(cfg->admSock);

    sock = ishare_Accept(cfg->admSock,&ip,&port);

    DBGTrace(sock);
    while(isActive(cfg) && sock > 0)
    {

       adm = malloc(sizeof(ISHARED_ADMIN));
       memset(adm,0,sizeof(ISHARED_ADMIN));

       adm->sock = sock;
       adm->cfg  = cfg;
       ishare_insertItem(&cfg->admUsers,&adm->l);

       if(ip == IPADDR_LOOPBACK)
          adm->level = 1;

       sprintf(buffer,"Session 0x%08lx (%d) starting from ",(ULONG) adm, (int) adm->level);
       ishare_formatIP(ip,buffer+strlen(buffer));
       log(cfg,buffer);

       DBGTracex(adm->cfg);
       ishare_beginSession(adm);

       sock = ishare_Accept(cfg->admSock,&ip,&port);
    }

    if(cfg->admSock > 0)
       ishare_CloseSocket(cfg->admSock);

    DBGMessage("Administration service stopped");

    THREAD_END();
 }

 int ICQAPI ishare_beginSession(ISHARED_ADMIN *adm)
 {
    DBGTrace(adm->type);
    adm->thread = ishare_beginThread(adm->cfg,16384,admThread,adm);
    return 0;
 }

 void ICQAPI ishare_sendTCPStatus(ISHARED_ADMIN *adm, int rc)
 {
    ISHARE_TCP_ERROR p = { { ISHARE_TCP_MARKER, ISHARETCP_ERROR }, rc };
    ishare_Send(adm->sock,&p,sizeof(p));
 }

 static void disconnect(ISHARED_ADMIN *adm)
 {
    if(adm->rc)
       ishare_sendTCPStatus(adm,adm->rc);
    ishare_CloseSocket(adm->sock);
    adm->sock = -1;
 }

 THREAD_START(admThread)
 {
    ISHARED_ADMIN       *adm = (ISHARED_ADMIN *) GET_THREAD_PARAMETER();
    ISHARED_CONFIG      *cfg = adm->cfg;
    ISHARED_SEARCHDESCR *sr;

    char		buffer[0x0100];
    ISHARE_TCP_PREFIX	h;

    DBGTracex(cfg);
    DBGTrace(sizeof(h));
    	
    sprintf(buffer,"TCP session 0x%08lx started",(ULONG) adm);
    log(cfg,buffer);

    DBGTracex(cfg);

    while(adm->sock > 0 && !adm->rc && ishare_RecBlock(adm->sock,sizeof(h),&h,&adm->rc))
    {
       DBGTracex(h.id);

       if(h.marker != ISHARE_TCP_MARKER)
       {
          adm->rc = ISHARE_STATUS_MALFORMED;
       }
       else if(h.id >= ISHARETCP_UNKNOWN)
       {
	  ishare_sendTCPStatus(adm,ISHARE_STATUS_UNKNOWN);
       }
       else if(tcp_cmd[h.id].level > adm->level)
       {
          adm->rc = ISHARE_STATUS_DENIED;
       }
       else if(!tcp_cmd[h.id].exec)
       {
	  ishare_sendTCPStatus(adm,ISHARE_STATUS_UNAVAILABLE);
       }
       else
       {
	  tcp_cmd[h.id].exec(adm,cfg,buffer);
       }

    }

    if(adm->sock > 0)
       disconnect(adm);

    walklist(sr,cfg->mySearches)
    {
       if(sr->adm == adm)
       {
          sr->adm = NULL;
          ishare_destroySearch(cfg, sr);
       }
    }

    sprintf(buffer,"TCP session 0x%08lx terminated",(ULONG) adm);
    log(cfg,buffer);

    ishare_removeItem(&cfg->admUsers,&adm->l);

    free(adm);

    THREAD_END();
 }



