/*
 * send.c - I-Share search manager
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

 #include <stdio.h>
 #include <malloc.h>
 #include <string.h>
 #include <time.h>

 #include "ishared.h"

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/


/*---[ Implementation ]----------------------------------------------------------------------------------*/

 ISHARED_SEARCHDESCR * ICQAPI ishare_createSearch(ISHARED_CONFIG *cfg, const char *key)
 {
    ISHARED_SEARCHDESCR *sr = malloc(sizeof(ISHARED_SEARCHDESCR)+strlen(key));

    if(!sr)
    {
       log(cfg,"Memory allocation error when creating search");
       return NULL;
    }

    memset(sr,0,sizeof(ISHARED_SEARCHDESCR));

#ifdef __OS2__
    DosEnterCritSec();
#endif
    sr->id = cfg->searchID++;
#ifdef __OS2__
    DosExitCritSec();
#endif

    strcpy(sr->key,key);

    ishare_insertItem(&cfg->mySearches,&sr->l);

    DBGTrace(sr->id);
    	
    return sr;
 }

 ISHARED_SEARCHDESCR * ICQAPI ishare_findSearch(ISHARED_CONFIG *cfg, USHORT id)
 {
    ISHARED_SEARCHDESCR *ret = NULL;

    walklist(ret,cfg->mySearches)
    {
       if(ret->id == id)
          return ret;
    }

    return ret;
 }

 int ICQAPI ishare_destroySearch(ISHARED_CONFIG *cfg, ISHARED_SEARCHDESCR *sr)
 {
#ifdef ADMIN
    ISHARE_TCP_SEARCHSTATUS p = { { ISHARE_TCP_MARKER, ISHARETCP_SEARCHSTATUS }, sr->id, 1 };
#endif

    CHKPoint();
    DBGTracex(cfg->mySearches.first);
    DBGTracex(sr);

    ishare_removeItem(&cfg->mySearches,&sr->l);

    CHKPoint();
    ishare_deleteList(&sr->result);
    CHKPoint();

#ifdef ADMIN
    if(sr->adm)
    {
       memset(&p,0,sizeof(ISHARE_TCP_SEARCHSTATUS));
       ishare_Send(sr->adm->sock,&p,sizeof(ISHARE_TCP_SEARCHSTATUS));
    }
#endif

    CHKPoint();
    free(sr);

    return 0;
 }

 BEGIN_UDP_PACKET_PROCESSOR( SEARCH  )
 {
#ifdef CLIENT
    /*
     * Search received, verify the file list (Only valid in client builds)
     */
    ISHARE_UDP_FOUND		*rsp;
    const ISHARED_FILEDESCR 	*fl;
    USHORT			fCount	= 0;
    int 			f;
    char			buffer[0x0100];
    ISHARED_USER		*user		= NULL;

    /* If first hop let's correct the packet information */

    if(pkt->userIP == IPADDR_LOOPBACK || !pkt->userIP)
       pkt->userIP = fromIP;

    if(!pkt->userPort)
       pkt->userPort = fromPort;

    /* Check for the packet origin */
    if(pkt->userIP == cfg->myIP)
       return 0;

    if(pkt->userIP == localIP(cfg))
       return 0;

    /* Check for circular searchs */

    DBGTrace(pkt->id);

    for(f = 0;f < CACHE_SEARCHS;f++)
    {
       if(cfg->cached[f].userIP == pkt->userIP && cfg->cached[f].userPort == pkt->userPort
       				 	       && cfg->cached[f].id       == pkt->id )
       {
          DBGMessage("Cached search, ignoring-it");
	  cfg->cached[f].idle = 0;
          return 0;
       }
    }

    /* It's ok, cache and do search */

    f = cfg->cachePtr++;
    if(cfg->cachePtr >= CACHE_SEARCHS)
       cfg->cachePtr = 0;

    (cfg->cached+f)->userIP   = pkt->userIP;
    (cfg->cached+f)->userPort = pkt->userPort;
    (cfg->cached+f)->id       = pkt->id;
    (cfg->cached+f)->idle     = 0;

    user = ishare_cacheUser(cfg, pkt->userIP, pkt->userPort);

    strlwr(pkt->filename);

#ifdef LOGSEARCHS
    ishare_formatIP(pkt->userIP,buffer);
    sprintf(buffer+strlen(buffer),":%d asking for ",pkt->userPort);
    strncat(buffer,pkt->filename,0xFF);
    log(cfg,buffer);
#endif

    walklist(fl,cfg->sharedFiles)
    {
       strncpy(buffer,(fl->filePath+fl->offset),0xFF);
       strlwr(buffer);

       if(strstr(buffer, pkt->filename))
       {
          DBGMessage((fl->filePath+fl->offset));
          fCount++;

          rsp = malloc(sizeof(ISHARE_UDP_FOUND)+strlen(fl->filePath+fl->offset));

          if(rsp)
          {
             DBGMessage("Sending search response");
             memset(rsp,0,sizeof(ISHARE_UDP_FOUND));
             strcpy(rsp->filename,fl->filePath+fl->offset);
             rsp->h.type = ISHAREUDP_FOUND;
             rsp->id     = pkt->id;	        /* Search ID					*/
             rsp->size   = fl->fileSize;	/* File size					*/
             rsp->uin    = queryUIN(cfg);	/* The user UIN (Can be 0)		        */
             rsp->fileID = fl->fileID;		/* File sequence				*/

             for(f=0;f<16;f++)
                rsp->md5[f] = fl->md5[f];

             if(ishare_sendTo(cfg, pkt->userIP, pkt->userPort, 0, sizeof(ISHARE_UDP_FOUND)+strlen(rsp->filename)-1, rsp))
                netlog(cfg,"Unable to send response");

             INTER_PACKET_DELAY
             free(rsp);
          }
       }
    }

#ifdef LOGSEARCHS
    sprintf(buffer,"Found %d file(s)",fCount);
    log(cfg,buffer);
#endif

    DBGTrace(fCount);
    DBGTrace(pkt->hops);

    if(fCount || pkt->hops < 2)	/* File found or no more hops, stop searching */
       return 0;

    CHKPoint();

    /* Redistribute search */
    pkt->hops--;
    ishare_distribute(cfg, sz, (ISHARE_UDP_PREFIX *) pkt, pkt->userIP, user);

#else  /* CLIENT */

    ISHARE_ERROR	rsp;

    memset(&rsp,0,sizeof(rsp));
    rsp.h.type = ISHAREUDP_ERROR;
    rsp.id     = pkt->id;
    rsp.error  = ISHARE_STATUS_CANTSEARCH;

    ishare_sendTo(cfg, fromIP, fromPort, 0, sizeof(rsp), &rsp);

#endif /* CLIENT */

    END_UDP_PACKET_PROCESSOR( SEARCH );
 }

 BEGIN_UDP_PACKET_PROCESSOR( FOUND )
 {
#ifdef LOGSEARCHS
    char buffer[0x0100];
#endif

    #pragma pack(1)
	
    struct header
    {
       ISHARE_TCP_PREFIX h;
       USHORT		 searchID;
       long		 userIP;
       USHORT		 userPort;
       USHORT		 fileID;
       UCHAR		 md5[16];
       ULONG		 uin;
       ULONG		 size;
    } *p = (struct header *) buffer;

    ISHARED_SEARCHDESCR *sr;
    ISHARED_RESULT	*result;
    int			f;

    if(cfg->myIP && fromIP == cfg->myIP)
       return 0;

    if(fromIP == localIP(cfg))
       return 0;

    sr = ishare_findSearch(cfg, pkt->id);

    if(!sr)
       return 0;

    sr->idle = 0;
    sr->resultCount++;

#ifdef LOGSEARCHS
    ishare_formatIP(fromIP,buffer);
    sprintf(buffer+strlen(buffer),":%d ICQ# %ld Size: %ld ID:%d File: ",fromPort,pkt->uin,pkt->size,(int)pkt->fileID);
    strncat(buffer,pkt->filename,0xFF);
    log(cfg,buffer);
#endif

    result = malloc(sizeof(ISHARED_RESULT) + strlen(pkt->filename));

    if(!result)
    {
       log(cfg,"Memory allocation error when storing search");
    }
    else
    {
       memset(result,0,sizeof(ISHARED_RESULT));
       result->userIP	= fromIP;
       result->userPort  = fromPort;
       result->size      = pkt->size;
       result->uin       = pkt->uin;
       result->fileID    = pkt->fileID;
       for(f=0;f<16;f++)
          result->md5[f] = pkt->md5[f];
       strcpy(result->filename,pkt->filename);
       ishare_insertItem(&sr->result,&result->l);
    }

#ifdef ADMIN
    if(sr->adm)
    {
       /* It was requested by ADM interface, send result */
       p->h.marker = ISHARE_TCP_MARKER;
       p->h.id     = ISHARETCP_FOUND;
       	
       p->searchID = sr->id;
       p->userIP   = fromIP;
       p->userPort = fromPort;
       p->fileID   = pkt->fileID;
       p->uin      = pkt->uin;
       p->size     = pkt->size;

       for(f=0;f<16;f++)
          p->md5[f] = pkt->md5[f];

       ishare_Send(sr->adm->sock,p,sizeof(struct header));
       ishare_SendFilename(sr->adm, pkt->filename, -1);
       ishare_SendString(sr->adm,"",-1);
    }
#endif
	
    return 0;	
 }


