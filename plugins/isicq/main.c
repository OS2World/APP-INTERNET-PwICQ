/*
 * main.c - I-Search client main loop
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #include <types.h>
 #include <sys\socket.h>
 #define INCL_DOSPROCESS
#endif

 #include <stdio.h>
 #include <stdlib.h>
 #include <malloc.h>
 #include <time.h>

 #include <string.h>

#ifdef WINICQ
 #include <windows.h>
 #include <ICQAPINotifications.h>
 #include <ICQAPICalls.h>
 #include <ICQAPIData.h>
 #include <ICQAPIInterface.h>
#endif

 #include "ishared.h"

/*---[ Structures and Macros ]---------------------------------------------------------------------------*/


/*---[ Function processors ]-----------------------------------------------------------------------------*/


 static const void *udp_packets[ISHAREUDP_UNKNOWN] =
 {
      NULL,                                      /* ISHAREUDP_NOP       Nop	            */
      USE_UDP_PACKET_PROCESSOR( HELLO ),         /* ISHAREUDP_HELLO     Hello to server     */
      USE_UDP_PACKET_PROCESSOR( HELLOACK ),      /* ISHAREUDP_HELLOACK  Server ack          */
      NULL,                                      /* ISHAREUDP_ERROR     Message error       */
      USE_UDP_PACKET_PROCESSOR( KEEPALIVE ),     /* ISHAREUDP_KEEPALIVE Keep alive packet   */
      NULL,                                      /* ISHAREUDP_LISTREQ   IP List request     */
      NULL,                                      /* ISHAREUDP_REQACK    List request ack    */
      USE_UDP_PACKET_PROCESSOR( SEARCH ),        /* ISHAREUDP_SEARCH    Search request      */
      USE_UDP_PACKET_PROCESSOR( FOUND),          /* ISHAREUDP_FOUND     Search response     */
#ifdef PWICQ
      USE_UDP_PACKET_PROCESSOR( MESSAGE ),       /* ISHAREUDP_MESSAGE   Instant message     */
#else
      NULL,
#endif
      NULL,                                      /* ISHAREUDP_MSGACK    Instant message ack */
      NULL,                                      /* ISHAREUDP_ACK       Packet Ack          */
      NULL                                       /* ISHAREUDP_SEARCHMD5 Search by MD5       */
 };

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/

 static int validPkt(ISHARED_CONFIG *, ULONG, int, ISHARE_UDP_PREFIX *);

#ifdef WINICQ
  static int loadWinICQUsers(ISHARED_CONFIG *);
#endif

/*---[ Implementation ]----------------------------------------------------------------------------------*/

 void ICQAPI ishare_main(ISHARED_CONFIG *cfg)
 {
    ISHARE_UDP_PREFIX  *buffer;
    int		   (*exec)(ISHARED_CONFIG *, long, int, int, char *);
    long 	   fromIP;
    int  	   fromPort;
    int  	   bytes;
    ISHARE_STATUS  *st           = NULL;
    USHORT	   sequence;

    DBGMessage("Thread de servico iniciada");

    srand(time(NULL));

    CHKPoint();
    if(cfg->sz != sizeof(ISHARED_CONFIG))
    {
       log(cfg,"Invalid configuration block");
       return;
    }
	
    CHKPoint();
    cfg->kplTimer   = ISHARE_KEEPALIVE_TIMER;
    cfg->active     = TRUE;

    CHKPoint();
    cfg->searchID   = loadValue(cfg,"sid",rand());
    DBGTrace(cfg->searchID);
	
    buffer = malloc(ISHARE_MAX_PACKET_SIZE+1);

    CHKPoint();
	
    if(!buffer)
    {
       log(cfg,"Memory allocation error when creating work buffer");
       return;
    }

    if(!cfg->st)
    {
       cfg->st = st = malloc(sizeof(ISHARE_STATUS));

       if(cfg->st)
       {
          cfg->st->sz = sizeof(ISHARE_STATUS);
       }
       else
       {
          log(cfg,"Memory allocation error when creating status block");
          free(buffer);
          return;
       }
    }

    cfg->sock = ishare_OpenUDP(loadValue(cfg,"listen",ISHARE_UDP_PORT));

    if(cfg->sock < 0)
    {
       netlog(cfg,"Can't create listener on requested port; retrying in any port");
       cfg->sock = ishare_OpenUDP(0);
    }

    DBGTrace(cfg->sock);
    if(cfg->sock < 0)
    {
       netlog(cfg,"Can't create listener sock");
    }
    else
    {

#ifdef __OS2__
       DosSetPriority(PRTYS_THREAD, PRTYC_FOREGROUNDSERVER, 0, 0);
#endif

       cfg->myIP = ishare_GetHostID();

       if(loadValue(cfg,"broadcast",1))
       {
          CHKPoint();
          ishare_EnableBroadcast(cfg->sock);
          ishare_hello(cfg,IPADDR_BROADCAST);
       }

       ishare_initialize(cfg);
#ifdef WINICQ
       loadWinICQUsers(cfg);
#endif

       bytes = ishare_RecvFrom(cfg->sock, &fromIP, &fromPort, buffer, ISHARE_MAX_PACKET_SIZE);
       while(isActive(cfg) && bytes >= 0)
       {

          if(cfg->packetCounter++ > PPS_LIMIT)
             yield();

          if(bytes > 0 && validPkt(cfg,fromIP,bytes,buffer))
          {
             *( ((char *) buffer) + bytes) = 0; // To finish the strings (the \0 isn't sent thru the network)
	
	     if(buffer->type >= ISHAREUDP_UNKNOWN)
	     {
                sprintf((char *) buffer,"Unknown packet %d (0x%02d) received",(int) buffer->type,(int) buffer->type);
		log(cfg, (char *) buffer);
	     }
     	     else if(buffer->type == ISHAREUDP_ACK)
	     {
                ishare_cacheUser(cfg, fromIP, fromPort);
	     }	
	     else if(!udp_packets[buffer->type])
	     {
                sprintf((char *) buffer,"Unsuported packet %d (0x%02d) received",(int) buffer->type,(int) buffer->type);
		log(cfg,(char *) buffer);
	     }
	     else
	     {
                sequence = buffer->sequence;
		exec     = (int (*)(ISHARED_CONFIG *, long, int, int, char *)) udp_packets[buffer->type];

                exec(cfg, fromIP, fromPort, bytes, (char *) buffer);
		
                if(sequence)
                {
                   DBGMessage("Enviar Ack!");
                   memset(buffer,0,sizeof(ISHARE_UDP_PREFIX));
                   buffer->type     = ISHAREUDP_ACK;
                   buffer->sequence = sequence;
                   ishare_sendTo(cfg, fromIP, fromPort, FALSE, sizeof(ISHARE_UDP_PREFIX), buffer);
                }
	     }
          }
#ifdef WINICQ
          else
          {
             if(cfg->winICQTimer > 30)
             {
                loadWinICQUsers(cfg);
                cfg->winICQTimer = 0;
             }
          }
#endif
          bytes = ishare_RecvFrom(cfg->sock, &fromIP, &fromPort, buffer, ISHARE_MAX_PACKET_SIZE);
       }

       ishare_CloseSocket(cfg->sock);

#ifdef __OS2__
       DosSetPriority(PRTYS_THREAD, PRTYC_REGULAR, 0, 0);
#endif
    }

#ifdef CLIENT
    ishare_stopTCPListener(cfg);
    ishare_deleteList(&cfg->admUsers);

    CHKPoint();
    ishare_deleteList(&cfg->sharedFiles);

    CHKPoint();
    while(cfg->mySearches.first)
       ishare_destroySearch(cfg, (ISHARED_SEARCHDESCR *) cfg->mySearches.first);

    CHKPoint();
    ishare_deleteList(&cfg->mySearches);
#endif

    if(st)
    {
       cfg->st = NULL;
       free(st);
    }

    CHKPoint();
    free(buffer);
    DBGMessage("Thread de servico terminada");
 }

// static int hello(ISHARED_CONFIG *cfg, long fromIP, int fromPort, int bytes, ISHARE_HELLO *pkt)
 BEGIN_UDP_PACKET_PROCESSOR( HELLO     )
 {
    char 		buffer[0x0100];
    ISHARE_UDP_HELLOACK	*rsp			= (ISHARE_UDP_HELLOACK *) buffer;
#ifdef PWICQ
    HUSER		usr;
    const PEERPROTMGR   *peer                   = icqQueryPeerProtocolManager(cfg->icq, 0);
#endif

    DBGMessage("Hello!");
    DBGTrace(pkt->uin);

    strcpy(buffer,"Hello from client ");
    sprintf(buffer+strlen(buffer),"%08lx at ",pkt->build);
    ishare_formatIP(fromIP,buffer+strlen(buffer));
    sprintf(buffer+strlen(buffer),":%d (ICQ#%ld)",fromPort,pkt->uin);
    log(cfg,buffer);

    if(pkt->uin && pkt->uin == queryUIN(cfg))
       return 0;

    memset(rsp,0,sizeof(ISHARE_UDP_HELLOACK));
    rsp->h.type = ISHAREUDP_HELLOACK;
    rsp->uin    = queryUIN(cfg);

#ifdef XBUILD
    rsp->build  = XBUILD;
#endif

#ifdef PWICQ
    rsp->flags     |= ISHARE_FLAG_ICQ;
    rsp->peerPort   = icqQueryPeerPort(cfg->icq);
	rsp->localIP    = icqQueryPeerAddress(cfg->icq);

    DBGTracex(peer);

    if(peer)
       rsp->peerVersion  = peer->version;

    DBGTrace(rsp->peerPort);
    DBGTrace(rsp->peerVersion);

#endif

    loadNickname(cfg,pkt->nickname,0xF0 - sizeof(ISHARE_UDP_HELLO));

    ishare_sendTo(cfg, fromIP, fromPort, 0, sizeof(ISHARE_UDP_HELLOACK)+strlen(rsp->nickname)-1, rsp);

#ifdef PWICQ
    if(cfg->icq && pkt->uin)
    {
       if(pkt->uin == queryUIN(cfg))
       {
          cfg->myIP = fromIP;
       }
       else
       {
          usr = icqQueryUserHandle(cfg->icq,pkt->uin);
	
          if(usr)
          {
             usr->idle = 0;
             if(!(usr->flags & USRF_ONLINE))
             {
  	            if(pkt->build >= 0x20030728)
			    {
                   log(cfg,"User is online");
				   CHKPoint();
                   icqUserOnline(cfg->icq, pkt->uin, 0x00000100, pkt->localIP, fromIP, pkt->peerPort, pkt->peerVersion, ISHARE_MAGICNUMBER);
			    }
			    else
			    {
                   log(cfg,"User is online with outdated version");
				   CHKPoint();
                   icqUserOnline(cfg->icq, pkt->uin, 0x00000100, fromIP, fromIP, pkt->peerPort, pkt->peerVersion, ISHARE_MAGICNUMBER);
			    }
             }
          }
       }
    }
#endif

    ishare_cacheUser(cfg, fromIP, fromPort);

    return 0;
 }

 BEGIN_UDP_PACKET_PROCESSOR( HELLOACK  )
 {
#ifdef PWICQ
    HUSER     usr;
#endif	
    char      buffer[0x0100];

    strcpy(buffer,"Ack from client ");
    sprintf(buffer+strlen(buffer),"%08lx at ",pkt->build);
    ishare_formatIP(fromIP,buffer+strlen(buffer));
    sprintf(buffer+strlen(buffer),":%d (ICQ#%ld)",fromPort,pkt->uin);
    log(cfg,buffer);

#ifdef DEBUG
    DBGMessage("Hello ACK");
	DBGTrace( (pkt->build >= 0x20030728) );
	
	if(pkt->build >= 0x20030728)
       DBGMessage(pkt->nickname);
    DBGTrace(pkt->uin);
#endif
	
#ifdef PWICQ
	
    DBGTrace(pkt->peerPort);
    DBGTrace(pkt->peerVersion);
	
	if(pkt->uin)
	{
	   usr = icqQueryUserHandle(cfg->icq,pkt->uin);
	   DBGTrace(usr);
	   
	   if(usr)
	   {
          DBGTracex(usr->flags & USRF_ONLINE);
		  
		  usr->idle = 0;
          if(!(usr->flags & USRF_ONLINE))
          {
  	         if(pkt->build >= 0x20030728)
			 {
                log(cfg,"User is online");
				CHKPoint();
                icqUserOnline(cfg->icq, pkt->uin, 0x00000100, pkt->localIP, fromIP, pkt->peerPort, pkt->peerVersion, ISHARE_MAGICNUMBER);
			 }
			 else
			 {
                log(cfg,"User is online with outdated version");
				CHKPoint();
                icqUserOnline(cfg->icq, pkt->uin, 0x00000100, fromIP, fromIP, pkt->peerPort, pkt->peerVersion, ISHARE_MAGICNUMBER);
			 }
          }
	   }
	}

/*	
    if(pkt->uin && setUserInfo(cfg,pkt->uin,fromIP,pkt->peerPort,pkt->peerVersion))
       return 0;
*/	
#endif
    ishare_cacheUser(cfg, fromIP, fromPort);
    return 0;
 }

 static int validPkt(ISHARED_CONFIG *cfg, ULONG fromIP, int sz, ISHARE_UDP_PREFIX *pkt)
 {
/*
   if(cfg->myIP && fromIP == cfg->myIP)
      return 0;

   if(fromIP == localIP(cfg) && localIP(cfg))
      return 0;
*/
   if(ishare_crc16(sz-2,((const char *) pkt)+2) != pkt->crc)
   {
       DBGMessage("****** CRC invalido");
       DBGTracex(ishare_crc16(sz-2,((const char *) pkt)+2));
       DBGTracex(pkt->crc);
       DBGTrace(pkt->type);
       return 0;
    }
	
    return 1;	
 }


 BEGIN_UDP_PACKET_PROCESSOR( KEEPALIVE )
 {
#ifdef EXTENDED_LOG
    char buffer[0x0100];
#endif

    if(pkt->uin && pkt->uin == queryUIN(cfg))
       return 0;

#ifdef EXTENDED_LOG
    ishare_formatIP(fromIP,buffer);
    sprintf(buffer+strlen(buffer),":%d is Alive ",fromPort);
    log(cfg,buffer);
#endif

    ishare_cacheUser(cfg, fromIP, fromPort);

    return 0;
 }

#ifdef WINICQ

 static int loadWinICQUsers(ISHARED_CONFIG *cfg)
 {
    int           f;
    int           user_count    = 0;
    int		  pos		= 0;
    BSICQAPI_User **user_list   = NULL;
    long          ip;

    if(!cfg->winICQEnabled)
       return -1;

    DBGMessage("Atualizando lista de usuarios ICQ");

    yield();
    if(!ICQAPICall_GetOnlineListDetails(&user_count, &user_list))
       return 1;

    Sleep(1);
    for(f=0; f < user_count && pos < MAX_WINICQ_USER; f++)
    {
       Sleep(1);
       if(ICQAPICall_GetFullUserData(user_list[f], WINICQAPI_VERSION))
       {
          ip = user_list[f]->m_iIP;
          if(ip && ip != IPADDR_LOOPBACK)
          {
             cfg->winICQUsers[pos].uin = user_list[f]->m_iUIN;
             cfg->winICQUsers[pos].ip  = user_list[f]->m_iIP;
             pos++;
          }
       }
    }
    ICQAPIUtil_FreeUsers(user_count, user_list);

    while(pos < MAX_WINICQ_USER)
    {
       cfg->winICQUsers[pos].uin = 0;
       cfg->winICQUsers[pos].ip  = 0;
       pos++;
    }

    DBGMessage("Lista de usuarios ICQ atualizada");

    return 0;

 }

#endif
