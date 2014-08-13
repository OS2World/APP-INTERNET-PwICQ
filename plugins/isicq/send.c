/*
 * send.c - I-Share send packets
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
 #include <types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
#endif

#ifdef WINICQ
 #include <windows.h>
 #include <ICQAPINotifications.h>
 #include <ICQAPICalls.h>
 #include <ICQAPIData.h>
 #include <ICQAPIInterface.h>
#endif


 #include <stdio.h>
 #include <malloc.h>
 #include <string.h>

 #include "ishared.h"

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/


/*---[ Implementation ]----------------------------------------------------------------------------------*/

 int ICQAPI ishare_send(ISHARED_CONFIG *cfg, long ip, BOOL ack, int sz, void *pkt)
 {
    return ishare_sendTo(cfg, ip, ISHARE_UDP_PORT, ack, sz, pkt);
 }

 int ICQAPI ishare_sendTo(ISHARED_CONFIG *cfg, long ip, USHORT port, BOOL ack, int sz, void *pkt)
 {
#ifndef USEICQNETW
    struct sockaddr_in   to;
#endif

    if(sz < 2)
    {
       log(cfg,"*** INTERNAL ERROR Bad packet size ***");
       return -1;
    }

    if(ack)
    {
       ((ISHARE_UDP_PREFIX *) pkt)->sequence = ++cfg->packetSequence;
       DBGTrace(((ISHARE_UDP_PREFIX *) pkt)->sequence);
    }

    ((ISHARE_UDP_PREFIX *) pkt)->crc = ishare_crc16(sz-2, ((const char *)pkt)+2 );

#ifdef USEICQNETW

    return icqSendTo(cfg->sock,ip,port,pkt,sz);

#else

    to.sin_family      = AF_INET;
    to.sin_addr.s_addr = ip;
    to.sin_port        = htons(port);

    if(sendto(cfg->sock,(char *) pkt,sz,MSG_NOSIGNAL,(struct sockaddr *) &to, sizeof(to)) == -1)
       return -1;

    return 0;

#endif

 }

 void ishare_hello(ISHARED_CONFIG *cfg, long ip)
 {
    char 		buffer[0x0100];
    ISHARE_UDP_HELLO	*pkt			= (ISHARE_UDP_HELLO *) buffer;
#ifdef PWICQ
    const PEERPROTMGR   *peer                   = icqQueryPeerProtocolManager(cfg->icq, 0);
#endif

    memset(pkt,0,sizeof(ISHARE_UDP_HELLO));

    pkt->h.type = ISHAREUDP_HELLO;
    pkt->uin    = queryUIN(cfg);

#ifdef XBUILD
    pkt->build  = XBUILD;
#endif

#ifdef PWICQ
    pkt->flags       |= ISHARE_FLAG_ICQ;
    pkt->peerPort     = icqQueryPeerPort(cfg->icq);

    DBGTracex(peer);

    if(peer)
       pkt->peerVersion  = peer->version;

    DBGTrace(pkt->peerPort);
    DBGTrace(pkt->peerVersion);
#endif

#ifdef WINICQ
    pkt->flags |= ISHARE_FLAG_ICQ;
#endif

    loadNickname(cfg,pkt->nickname,0xF0 - sizeof(ISHAREUDP_HELLO));

//    DBGMessage(pkt->nickname);

    ishare_send(cfg,ip,0,sizeof(ISHARE_UDP_HELLO)+strlen(pkt->nickname)-1,pkt);

 }

 ISHARED_SEARCHDESCR * ICQAPI ishare_search(ISHARED_CONFIG *cfg, const char *key, void *adm)
 {
    int			sz      = sizeof(ISHARE_UDP_SEARCH)+strlen(key)-1;
    ISHARE_UDP_SEARCH	*pkt	= malloc(sz+3);
    ISHARED_SEARCHDESCR *sr;
#ifdef LOGSEARCHS
    char		buffer[0x0100];
#endif

    DBGTracex(cfg);

    if(!pkt)
    {
       log(cfg,"Memory allocation error when searching");
       return NULL;
    }

#ifdef LOGSEARCHS
    strcpy(buffer,"Searching for ");
    strncat(buffer,key,0xFF);
    log(cfg,buffer);
    DBGMessage(buffer);
#endif

    sr = ishare_createSearch(cfg,key);
	
    if(!sr)
       return NULL;

#ifdef ADMIN
	sr->adm = adm;
#endif	
	
    memset(pkt,0,sizeof(ISHARE_UDP_SEARCH));
    pkt->h.type   = ISHAREUDP_SEARCH;
    pkt->id       = sr->id;
	
    DBGTrace(pkt->id);
    DBGTrace(pkt->userPort);

    strcpy(pkt->filename,key);

    if(loadValue(cfg,"broadcast",1))
    {
       pkt->hops     = 4;   /* Broadcast searchs aren't distributed */
       ishare_send(cfg,IPADDR_BROADCAST,0,sz,pkt);
    }

    pkt->hops = loadValue(cfg,"hops",65);

    ishare_distribute(cfg, sz, (ISHARE_UDP_PREFIX *) pkt, pkt->userIP, NULL);

    free(pkt);
	
    return sr;
	
 }

 int ICQAPI ishare_distribute(ISHARED_CONFIG *cfg, int sz, ISHARE_UDP_PREFIX *pkt, long userIP, ISHARED_USER *cached)
 {	
#ifdef PWICQ
    HUSER	usr;
#endif

//#ifdef WINICQ
//    int           user_count    = 0;
//    BSICQAPI_User **user_list   = NULL;
//#endif

#ifdef EXTENDED_LOG
    char          logBuffer[0x0100];
#endif

    long          ip;
    int           f;		

   if(sz < 2)
   {
      log(cfg,"*** INTERNAL ERROR Bad packet size ***");
      return -1;
   }

   DBGMessage("------------- ISHARE_DISTRIBUTE BEGIN----------------------------------------");
#ifdef DEBUG
    ishare_formatIP(userIP,logBuffer);
    DBGMessage(logBuffer);
#endif

   pkt->crc = ishare_crc16(sz-2, ((const char *)pkt)+2 );
	
#ifdef PWICQ
    /*
     * Distribute to pwICQ users
     */
    if(cfg->icq)
    {
       for(usr=icqQueryFirstUser(cfg->icq);usr;usr=icqQueryNextUser(cfg->icq,usr))
       {
          ip = icqQueryUserGateway(cfg->icq, usr);
          if(ip && ip != IPADDR_LOOPBACK && ip != userIP)
          {
#ifdef EXTENDED_LOG
             sprintf(logBuffer,"Sending to %s ",icqQueryUserNick(usr));
             ishare_formatIP(ip,logBuffer+strlen(logBuffer));
             sprintf(logBuffer+strlen(logBuffer),":%d",ISHARE_UDP_PORT);
             log(cfg,logBuffer);
#endif	
             ishare_SendTo(cfg->sock, ip, ISHARE_UDP_PORT, pkt, sz);
             INTER_PACKET_DELAY
          }
          if(cached && cached->ip == ip)
          {
             cached->ip      = 0;
             cached->port    = 0;
          }
       }
    }
#endif

#ifdef WINICQ
    /*
     * Distribute to Windows ICQ users
     */

    if(cfg->winICQEnabled)
    {
       for(f=0; f < MAX_WINICQ_USER; f++)
       {
          ip = cfg->winICQUsers[f].ip;
          if(ip)
          {
             sprintf(logBuffer,"Sending to ICQ#%ld at ",cfg->winICQUsers[f].uin);
             ishare_formatIP(ip,logBuffer+strlen(logBuffer));
             log(cfg,logBuffer);
             ishare_SendTo(cfg->sock, ip, ISHARE_UDP_PORT, pkt, sz);
             INTER_PACKET_DELAY
          }
          if(cached && cached->ip == ip)
          {
             cached->ip      = 0;
             cached->port    = 0;
          }
       }
    }

/*

    if(cfg->winICQEnabled)
    {
       yield();
       if(ICQAPICall_GetOnlineListDetails(&user_count, &user_list))
       {
          sleep(1);
          for(f=0; f < user_count; f++)
          {
             sleep(1);
             if(ICQAPICall_GetFullUserData(user_list[f], WINICQAPI_VERSION))
             {
                ip = user_list[f]->m_iIP;
                if(ip && ip != IPADDR_LOOPBACK && ip != userIP)
                {

                   strcpy(logBuffer,"Sending to ICQ User in ");
                   ishare_formatIP(ip,logBuffer+strlen(logBuffer));
	           log(cfg,logBuffer);

                   ishare_SendTo(cfg->sock, ip, ISHARE_UDP_PORT, pkt, sz);
                   INTER_PACKET_DELAY
                }
                if(cached && cached->ip == ip)
                {
                   cached->ip      = 0;
                   cached->port    = 0;
                }
             }
          }
          ICQAPIUtil_FreeUsers(user_count, user_list);
       }
       else
       {
          log(cfg,"Failure getting user list");
       }
    }
*/

#endif

    for(f=0;f<CACHE_USERS;f++)
    {
       ip = cfg->users[f].ip;
       if(ip && ip != userIP)
       {
#ifdef EXTENDED_LOG
	  strcpy(logBuffer,"Sending to cached ");
	  ishare_formatIP(ip,logBuffer+strlen(logBuffer));
	  sprintf(logBuffer+strlen(logBuffer),":%d",cfg->users[f].port);
	  log(cfg,logBuffer);
#endif	
          ishare_SendTo(cfg->sock, ip, cfg->users[f].port, pkt, sz);
          INTER_PACKET_DELAY
       }
    }

    DBGMessage("------------- ISHARE_DISTRIBUTE END -----------------------------------------");

    return 0;
 }


