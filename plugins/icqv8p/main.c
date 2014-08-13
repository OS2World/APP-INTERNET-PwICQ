/*
 * main.c - ICQV8 Peer Protocol manager
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

 #include <stdio.h>
 #include <time.h>
 #include <stdlib.h>
 #include <string.h>

#ifdef linux
 #include <unistd.h>
#endif

 #define ICQV8P_TYPES_DEFINED 1
 #define HPEER                void *
 #define PEERDATA             void

 #include "peer.h"

/*---[ Statics ]----------------------------------------------------------------------------------------------*/


 HMODULE module = NULLHANDLE;

 #define REQUEST_PREFIX     (int ( * _System )(HICQ, ULONG, void *, ULONG, void *, const char *))
 #define REQUEST_ACK_PREFIX (int ( * _System )(HICQ, ULONG, void *, ULONG, int, void *))

 const REQUESTMGR  icqv8p_requestManager =
 {
    sizeof(REQUESTMGR),

    /* Request accepted/refused HICQ, ID, DataBlock, SESSION, PORT, PARAM */
    REQUEST_ACK_PREFIX icqv8_accepted,      // int (* _System accepted)(HICQ, ULONG, PEERDATA *, ULONG, void *);
    NULL,                                   // int (* _System refused )(HICQ, ULONG, PEERDATA *, ULONG, void *);
    REQUEST_PREFIX     icqv8_accept,        // int (* _System accept  )(HICQ, ULONG, PEERDATA *, ULONG, void *);
    REQUEST_PREFIX     icqv8_refuse,        // int (* _System refuse  )(HICQ, ULONG, PEERDATA *, ULONG, void *);

 };

 const PEERPROTMGR icqv8_peerDescriptor =
 {
      sizeof(PEERPROTMGR),                    // Structure size
      8,                                      // Protocol version
      XBUILD,                                 // Module build
#ifdef DEBUG
    "ICQ Version 8 peer protocol manager Build " BUILD " (Debug Version)",
#else
    "ICQ Version 8 peer protocol manager Build " BUILD,
#endif
      2048,                                   // Maximum message size
      0,                                      // Peer protocol flags

      /* Session management */
      NULL,                                   // USHORT (* _System queryStatus)(HICQ, HUSER, PEERDATA *);
      icqv8_startListener,                    // int    (* _System startListener)(HICQ, PEERDATA *);
      NULL,

      /* Send message to one user */
      icqv8_sendMessage,		      // USHORT (* _System sendMessage)(HICQ, HUSER, USHORT, UCHAR *, ULONG *, PEERDATA *)

      icqv8_sendFileReq,                      // USHORT (* _System sendFileReq)(HICQ, HUSER, const char *, int, const char *, ULONG );
      NULL,                                   // USHORT (* _System sendChatReq)(HICQ, HUSER, const char *);

      /* Peer cookie */
      icqv8_setCookie,                        // ULONG  (* _System setCookie)(HICQ,void *, ULONG)
      icqv8_getCookie,                        // ULONG  (* _System getCookie)(HICQ,void *)

      /* Network actions */
      icqv8_getPort,                          // long   (* _System getPort)(HICQ,HPEER);
      icqv8_getAddress,                       // long   (* _System getAddress)(HICQ,HPEER);

      /* Session management */
      icqv8_connect,                          // int    (* _System beginSession)(HICQ, void *, HUSER);
      NULL                                    // int	(* _System closeAll)(HICQ,ULONG);

 };


/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 int EXPENTRY icqv8p_Configure(HICQ icq, HMODULE mod)
 {
    char buffer[0x0100];

    strcpy(buffer,"Loading ");
    strncat(buffer,icqv8_peerDescriptor.descr,0xFF);
    icqWriteSysLog(icq,PROJECT,buffer);

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"**** Extensive logging enabled ****");
#endif

    module = mod;

    if(icqQueryUserDataBlockLength() != sizeof(ICQUSER))
    {
       icqWriteSysLog(icq,PROJECT,"Can't start protocol manager due to invalid pwICQ core version");
       return -1;
    }

    return sizeof(PEERCONF);

 }

 int EXPENTRY icqv8p_Start(HICQ icq, HPLUGIN p, HPEER cfg)
 {
    char buffer[0x0100];

    sprintf(buffer,"Client<->Client V8 protocol manager starting session manager %08lx",(ULONG) cfg);
    DBGMessage(buffer);
    icqWriteSysLog(icq,PROJECT,buffer);

    if(icqLoadValue(icq, "betatester", 0))
       icqSetCaps(icq,ICQMF_FILE);

    if(!icqRegisterPeerProtocolManager(icq, &icqv8_peerDescriptor, cfg))
    {
       icqWriteSysLog(icq,PROJECT,"Can't register peer protocol manager");
       return -1;
    }

    return 0;
 }

 int EXPENTRY icqv8p_Stop(HICQ icq, HPLUGIN p, HPEER cfg)
 {
    int f;

    ((PEERCONF *) cfg)->active = FALSE;

    icqWriteSysLog(icq,PROJECT,"Client<->Client V8 protocol manager stopping");

    icqDeRegisterPeerProtocolManager(icq, &icqv8_peerDescriptor);

    DBGTrace(((PEERCONF *) cfg)->sock);

    if( ((PEERCONF *) cfg)->sock > 0)
       icqShutdownSocket(((PEERCONF *) cfg)->sock);

#ifdef __OS2__
    for(f=0;f < 60 && ((PEERCONF *) cfg)->thread > 0;f++)
       DosSleep(100);
#else
    for(f=0;f < 3000 && ((PEERCONF *) cfg)->thread > 0;f++)
       usleep(100);
#endif

    DBGMessage("Peer protocol manager stopped");

    return 0;
 }

 int EXPENTRY icqv8p_Timer(HICQ icq, HPEER cfg)
 {
    icqv8_peerTimer(icq,cfg);
    return 0;
 }

