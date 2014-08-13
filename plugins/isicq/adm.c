/*
 * adm.c - Administration port listener
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>

 #include "ishared.h"

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/


/*---[ Implementation ]----------------------------------------------------------------------------------*/

 BEGIN_TCP_PACKET_PROCESSOR( LOGON )
 {
    USHORT	clientVersion	= ishare_RecShort(adm);
    char	clientName[0x080];
    char	userName[40];
    char	password[40];

    ishare_RecString(adm,clientName,0x7F);
    ishare_RecString(adm,userName,39);
    ishare_RecString(adm,password,39);

    if(adm->rc)
       return -1;

    sprintf(buffer,"Session 0x%08lx username: %s",(ULONG) adm,userName);
    log(cfg,buffer);           	

    sprintf(buffer,"Session 0x%08lx client: %s (Version 0x%04x)",(ULONG) adm, clientName, (int) clientVersion);
    log(cfg,buffer);           	

    DBGTracex(clientVersion);
    DBGMessage(clientName);
    DBGMessage(userName);
    DBGMessage(password);

    #ifdef PWICQ

    if(cfg->icq)
    {
       icqQueryPassword(cfg->icq, buffer);
       if(atoi(userName) == icqQueryUIN(cfg->icq) && !strcmp(password,buffer))
       {
          adm->level = adm->level == 1 ? 3 : 4;
  	  ishare_sendTCPStatus(adm,ISHARE_OK);
          return 0;
       }
    }

    #endif
	
    #ifdef linux
    if(!strcmp(userName,"anonymous") && !strcmp(password,"password"))
    {
       log(cfg,"Anonymous session started");
       adm->level = adm->level == 1 ? 3 : 4;
       ishare_sendTCPStatus(adm,ISHARE_OK);
       return 0;
    }
	
    #endif

    adm->rc = ISHARE_STATUS_DENIED;

    END_TCP_PACKET_PROCESSOR( LOGON );
 }

 BEGIN_TCP_PACKET_PROCESSOR( STOP )
 {
    USHORT reason = ishare_RecShort(adm);

    sprintf(buffer,"Session 0x%08lx Stop request (Reason: 0x%04x)",(ULONG) adm, reason);
    log(cfg,buffer);           	

    cfg->active = FALSE;

    END_TCP_PACKET_PROCESSOR( SEARCHTEXT );
 }

 BEGIN_TCP_PACKET_PROCESSOR( SEARCHTEXT )
 {
#ifdef CLIENT

    char 		    key[0x0200];
    ISHARED_SEARCHDESCR	    *sr;
    ISHARE_TCP_SEARCHSTATUS *p = (ISHARE_TCP_SEARCHSTATUS *) buffer;
	
    ishare_RecFilename(adm,key,0x01FF);

    DBGMessage(key);

    if(!*key)
    {
       ishare_sendTCPStatus(adm,ISHARE_STATUS_BADSEARCH);
       return 0;
    }

    CHKPoint();

    sr = ishare_search(cfg, key, adm);
	
    if(!sr)
    {
       ishare_sendTCPStatus(adm,ISHARE_STATUS_SEARCHFAILED);
       return 0;
    }

    INITIALIZE_TCP_PACKET_POINTER( p, SEARCHSTATUS);
	
    p->id       = sr->id;

    DBGTracex(sr->adm);
    DBGTrace(p->id);
	
    ishare_Send(adm->sock,p,sizeof(ISHARE_TCP_SEARCHSTATUS));

#else

    ishare_sendTCPStatus(adm,ISHARE_STATUS_CANTSEARCH);

#endif

    END_TCP_PACKET_PROCESSOR( SEARCHTEXT );
 }



