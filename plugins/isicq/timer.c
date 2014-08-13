/*
 * timer.c - I-Share timer manager
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdio.h>
 #include <malloc.h>
 #include <string.h>

 #include "ishared.h"

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/

 static void ishare_keepalive(ISHARED_CONFIG *, BOOL, long, USHORT);

/*---[ Implementation ]----------------------------------------------------------------------------------*/

 void ICQAPI ishare_timer(ISHARED_CONFIG *cfg)
 {
    int 		f;
    ISHARED_USER	*usr = NULL;  	
    ISHARED_SEARCHDESCR *sr;
    ISHARED_SEARCHDESCR *srNext;
    char                logBuffer[0x0100];

#ifdef WINICQ
    cfg->winICQTimer++;
#endif

    sr = (ISHARED_SEARCHDESCR *) cfg->mySearches.first;
    while(sr)
    {
       srNext = (ISHARED_SEARCHDESCR *) sr->l.down;
       if(sr->idle++ > 240)
          ishare_destroySearch(cfg, sr);
       sr = srNext;
    }

    for(f=0;f<CACHE_USERS;f++)
    {
       usr = cfg->users+f;
       if(usr->ip && usr->port)
       {
          usr->idle++;
	  if(usr->idle > (ISHARE_KEEPALIVE_TIMER+usr->seed))
	  {
	     if(usr->kpl > ISHARE_MAXKEEPALIVE)
	     {
                strcpy(logBuffer,"User at ");
                ishare_formatIP(usr->ip,logBuffer+strlen(logBuffer));
                strncat(logBuffer," is offline",0xFF);
                log(cfg,logBuffer);

                usr->ip   = 0;
                usr->port = 0;
                usr->idle =
                usr->kpl  = 0;
                usr->seed = 0;
	     }
	     else if(!usr->kpl || (usr->idle & 0x01))
             {
                DBGMessage("Sending keep-alive");
                usr->kpl++;
                DBGTrace(usr->kpl);
                ishare_keepalive(cfg,TRUE,usr->ip,usr->port);
             }
          }
       }
    }

    for(f = 0;f < CACHE_SEARCHS; f++)
    {
       if(cfg->cached[f].idle++ > 240)
       {
	  cfg->cached[f].userIP   = 0;
	  cfg->cached[f].userPort = 0;
	  cfg->cached[f].idle     = 0;
	  cfg->cached[f].id       = 0;
       }
    }

    if(--cfg->kplTimer < 0)
    {
       if(loadValue(cfg,"broadcast",1))
          ishare_keepalive(cfg,FALSE,IPADDR_BROADCAST,ISHARE_UDP_PORT);
       cfg->kplTimer = ISHARE_KEEPALIVE_TIMER;
    }
    cfg->packetCounter = 0;

 }

 static void ishare_keepalive(ISHARED_CONFIG *cfg, BOOL ack, long ip, USHORT port)
 {
    ISHARE_UDP_KEEPALIVE pkt;
    memset(&pkt,0,sizeof(pkt));
    pkt.h.type     = ISHAREUDP_KEEPALIVE;
    pkt.uin        = queryUIN(cfg);

    ishare_sendTo(cfg,ip,port,ack,sizeof(pkt), &pkt);
    INTER_PACKET_DELAY

    DBGTrace(pkt.h.sequence);
 }

