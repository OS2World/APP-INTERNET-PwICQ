/*
 * init.c - Initialization
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

 #include <stdio.h>
 #include <malloc.h>
 #include <string.h>

#ifdef WINICQ
 #include <ICQAPINotifications.h>
 #include <ICQAPICalls.h>
 #include <ICQAPIData.h>
 #include <ICQAPIInterface.h>
#endif

 #include "ishared.h"

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/

 DECLARE_THREAD(loadFilelist)
 DECLARE_THREAD(loadIPlist)

/*---[ Constants ]---------------------------------------------------------------------------------------*/

#ifdef WINICQ

 static const char *pszName     = "XPTO";
 static const char *pszPassword = "olecramsomel";
 static const char *pszLicense  = "B01A1B856DE5BAE8";

#endif

/*---[ Implementation ]----------------------------------------------------------------------------------*/

 void ICQAPI ishare_initialize(ISHARED_CONFIG *cfg)
 {
    ishare_beginThread(cfg,16384,loadFilelist,cfg);

#ifdef CLIENT
    ishare_startTCPListener(cfg);
#endif

#ifdef WINICQ
    cfg->winICQEnabled = ICQAPICall_SetLicenseKey((char *)pszName, (char *)pszPassword, (char *)pszLicense);
    if(!cfg->winICQEnabled)
       log(cfg,"Failure in ICQ API initialization");
#endif

    ishare_beginThread(cfg,16384,loadIPlist,cfg);

 }

 THREAD_START(loadFilelist)
 {
    ISHARED_CONFIG *cfg = (ISHARED_CONFIG *) GET_THREAD_PARAMETER();
    FILE	   *fl;
    char	   *ptr;
    char	   buffer[0x0100];
/*	
#ifdef DEBUG
    ISHARED_FILEDESCR	*entry;
#endif
*/	

    DBGMessage("Shared list load thread started");

    log(cfg,"Loading shared folders");

#ifdef __OS2__
    DosSetPriority(PRTYS_THREAD, PRTYC_IDLETIME, 0, 0);
#endif

#ifdef linux
    queryPath(cfg,"ishare.folder",buffer,0xFF);
#else	
    queryPath(cfg,"ishare.fld",buffer,0xFF);
#endif	
    DBGMessage(buffer);

    fl = fopen(buffer,"r");

    if(!fl)
    {
       log(cfg,"Error opening shared folders list");
    }
    else
    {
       *buffer = 0;
       fgets(buffer,0xFF,fl);
       while(!feof(fl))
       {
          for(ptr=buffer;*ptr && *ptr >= ' ';ptr++);
          *ptr = 0;
          if(*buffer)
             ishare_loadFolder(cfg,buffer);
          *buffer = 0;
          fgets(buffer,0xFF,fl);
       }
       fclose(fl);
       log(cfg,"Shared folders loading complete");
    }

    ishare_loadMD5(cfg);

/*
#ifdef DEBUG
    entry = (ISHARED_FILEDESCR *) cfg->sharedFiles.first;
    if(entry)
       ishare_requestFile(cfg, IPADDR_LOOPBACK, ISHARE_UDP_PORT, entry->fileID, entry->md5, entry->filePath + entry->offset, entry->fileSize);
#endif
*/
    DBGMessage("Shared list load thread terminated");

    THREAD_END();
 }

 THREAD_START(loadIPlist)
 {
    ISHARED_CONFIG *cfg = (ISHARED_CONFIG *) GET_THREAD_PARAMETER();
    FILE	   *fl;
    char	   *ptr;
    char	   buffer[0x0100];
    long	   ip;

    DBGMessage("IP list load thread started");

    log(cfg,"Loading static list");

#ifdef __OS2__
    DosSetPriority(PRTYS_THREAD, PRTYC_IDLETIME, 0, 0);
#endif

    queryPath(cfg,"ishare.lst",buffer,0xFF);

    DBGMessage(buffer);

    fl = fopen(buffer,"r");

    if(fl)
    {
       *buffer = 0;
       fgets(buffer,0xFF,fl);
       while(!feof(fl) && isActive(cfg))
       {
          for(ptr=buffer;*ptr && *ptr >= ' ';ptr++);

          *ptr = 0;
          if(*buffer)
          {
             DBGMessage(buffer);
             ip = ishare_GetHostByName(buffer);
             DBGTracex(ip);
             if(ip)
             {
                ishare_cacheUser(cfg, ip, ISHARE_UDP_PORT);
                ishare_hello(cfg, ip);
                INTER_PACKET_DELAY
             }
          }

          fgets(buffer,0xFF,fl);
       }
       fclose(fl);
    }

    DBGMessage("Static list load thread terminated");

    THREAD_END();
 }

