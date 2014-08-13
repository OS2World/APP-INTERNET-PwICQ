/*
 * listen.c - listener thread
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>

 #include "peer.h"



/*---[ Command processors ]-----------------------------------------------------------------------------------*/

 static int icqv8_peerInit(       HICQ, PEERSESSION *, int, unsigned char *);
 static int icqv8_peerInitAck(    HICQ, PEERSESSION *, int, unsigned char *);
 static int icqv8_peerFileInit(   HICQ, PEERSESSION *, int, unsigned char *);
 static int icqv8_peerMsgPacket(  HICQ, PEERSESSION *, int, unsigned char *);
 static int icqv8_peerInitPacket( HICQ, PEERSESSION *, int, unsigned char *);
 static int icqv8_peerFileData(   HICQ, PEERSESSION *, int, unsigned char *);

 #pragma pack(1)

 struct cmd
 {
    UCHAR   cmd;
    int    (*exec)(HICQ,PEERSESSION *, int, unsigned char *);
 };

 static const struct cmd cmdTable[] =
 {
    { 0x00, icqv8_peerFileInit       },
    { 0x01, icqv8_peerInitAck        },
    { 0x02, icqv8_peerMsgPacket      },
    { 0x03, icqv8_peerInitPacket     },
    { 0x06, icqv8_peerFileData       },
    { 0xFF, icqv8_peerInit           },
    { 0x00, NULL                     }
 };


/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/


/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 void icqv8_peerSession(HICQ icq, PEERSESSION *cfg)
 {
    char       *pkt = NULL;
    int        sz;

    DBGMessage("--------Sessao peer iniciada------------------------");
    DBGTracex(cfg->flags & PEERFLAG_ACTIVE);

    icqv8_sendPeerHello(icq,cfg);

    cfg->idle = 0;
    DBGTracex(cfg->flags & PEERFLAG_ACTIVE);

    while( (cfg->flags & PEERFLAG_ACTIVE) && icqIsActive(icq) && (sz = icqv8_recvPeer(icq,cfg->peer,cfg->sock,(void **) &pkt)) > 0)
    {
       cfg->idle = 0;
       DBGTrace(*pkt);

       if(icqv8_execPeer(icq,cfg,sz,pkt))
       {
          DBGMessage("********* ERRO EM EXEC PEER");
          DBGTracex(*pkt);

#ifdef EXTENDED_LOG
          icqWriteSysLog(icq,PROJECT,"Failure in packet processing, stopping session");
#endif
          cfg->flags &= ~PEERFLAG_ACTIVE;
       }

       free((void *) pkt);
       pkt = NULL;
    }

    if(cfg->flags & PEERFLAG_ACTIVE)
       icqWriteNetworkErrorLog(icq, PROJECT, "Closing session");

    DBGTracex(cfg->flags & PEERFLAG_ACTIVE);
    cfg->idle = 0;

    if(pkt)
       free((void *) pkt);

    icqv8_terminateSession(icq,cfg);
    icqRemoveElement(cfg->peer->sessions, cfg);

 }

 int icqv8_execPeer(HICQ icq, PEERSESSION *cfg, int sz, unsigned char *pkt)
 {
    const struct cmd *tbl;

    for(tbl = cmdTable;tbl->exec;tbl++)
    {
       if(tbl->cmd == *pkt)
          return tbl->exec(icq,cfg,sz,pkt);
    }

    icqDumpPacket(icq, NULL, "Unknown Peer packet received", sz, (unsigned char *) pkt);
    return 0;
 }

 void icqv8_terminateSession(HICQ icq, PEERSESSION *cfg)
 {
    char buffer[0x0100];

    sprintf(buffer,"Closing session %08lx, tid = %d",(ULONG) cfg, cfg->thread);
    icqWriteSysLog(icq,PROJECT,buffer);

    icqDestroySessionRequests(icq,(ULONG) cfg);

    if(cfg->arq)
    {
       fclose(cfg->arq);
       cfg->arq = 0;
    }

    if(cfg->flags & PEERFLAG_MESSAGE)
    {
       DBGMessage("Registering session end");
       icqSetPeerSession(icq, cfg->uin, 0, &icqv8_peerDescriptor, NULL);
    }

    cfg->uin  = 0;

    if(cfg->sock > 0)
    {
       icqCloseSocket(cfg->sock);
       cfg->sock = 0;
    }

 }

 int icqv8_sendPeerHello(HICQ icq, PEERSESSION *cfg)
 {
    #pragma pack(1)

    struct _hello
    {
       UCHAR  cmd;      // BYTE  ff  CMD  The command: connect.
       USHORT tcpVer;   // WORD.L 08 00 TCPVER The peer-to-peer version this packet uses.
       USHORT szBlock;  // WORD.L 2b 00 LENGTH The length of the following data in bytes.
       ULONG  to;       // DWORD.L xx xx xx xx DESTUIN The UIN this packet is sent to.
       USHORT x1;       // WORD.L 00 00 UNKNOWN Unknown: empty.
       ULONG  port1;    // DWORD.L xx xx xx xx OURPORT The port the sender listens on.
       ULONG  from;     // DWORD.L xx xx xx xx OURUIN The UIN of the sender.
       ULONG  gateway;  // DWORD xx xx xx xx OURREMIP The IP of the sender as the server sees it.
       ULONG  ip;       // DWORD xx xx xx xx OURINTIP The local IP of the sender.
       UCHAR  tcpFlags; // BYTE 04 TCPFLAGS TCP connection flags.
       ULONG  port2;    // DWORD.L xx xx xx xx OURPORT2 The sender's "other" port.
       ULONG  cookie;   // DWORD.L xx xx xx xx COOKIE The connection cookie the server gave for this pair of UINs.
       ULONG  x2;       // DWORD.L 50 00 00 00 UNKNOWN Unknown: 0x50 = 80.
       ULONG  x3;       // DWORD.L 03 00 00 00 UNKNOWN Unknown: 0x3 = 3.
       ULONG  x5;       // DWORD.L 00 00 00 00 UNKNOWN Unknown: empty.
    } h;

    HUSER usr = icqQueryUserHandle(icq,cfg->uin);

    if(!usr) // || !usr->peerCookie)
    {
       DBGMessage("Cookie invalido!");
       icqWriteSysLog(icq,PROJECT,"Invalid user in hello request");
       cfg->flags &= ~PEERFLAG_ACTIVE;
       return -1;
    }

    icqRegisterUserAction(usr);
    usr->idle  = 0;

    if(usr->flags & USRF_HIDEONLIST)
       icqUserEvent(icq,usr,ICQEVENT_SHOW);

    icqAjustUserIcon(icq, usr, TRUE);

    memset(&h,0,sizeof(h));
    h.cmd      = 0xFF;
    h.tcpVer   = 0x08;
    h.szBlock  = 0x2B;
    h.to       = cfg->uin;
//    h.x1       =
    h.port1    = icqv8_getPort(icq, cfg->peer);
    h.from     = icqQueryUIN(icq);
    h.gateway  = icqQueryLocalIP(icq);
    h.ip       = icqGetHostID();
    h.tcpFlags = 0x04;
    h.port2    = h.port1;
    h.cookie   = usr->peerCookie;
    h.x2       = 0x50;
    h.x3       = 0x03;
//    h.x5       =

    icqv8_sendBlock(cfg->sock, sizeof(h), (const void *) &h);

    return 0;
 }

 static int icqv8_peerInit(HICQ icq, PEERSESSION *cfg, int sz, unsigned char *ptr)
 {
    #pragma pack(1)

    static const char peer_initAck[]  = { 0x01, 0x00, 0x00, 0x00 };

    struct _hello
    {
       UCHAR cmd;       // BYTE  ff  CMD  The command: connect.
       USHORT tcpVer;   // WORD.L 08 00 TCPVER The peer-to-peer version this packet uses.
       USHORT szBlock;  // WORD.L 2b 00 LENGTH The length of the following data in bytes.
       ULONG  to;       // DWORD.L xx xx xx xx DESTUIN The UIN this packet is sent to.
       USHORT x1;       // WORD.L 00 00 UNKNOWN Unknown: empty.
       ULONG  port1;    // DWORD.L xx xx xx xx OURPORT The port the sender listens on.
       ULONG  from;     // DWORD.L xx xx xx xx OURUIN The UIN of the sender.
       ULONG  gateway;  // DWORD xx xx xx xx OURREMIP The IP of the sender as the server sees it.
       ULONG  ip;       // DWORD xx xx xx xx OURINTIP The local IP of the sender.
       UCHAR  tcpFlags; // BYTE 04 TCPFLAGS TCP connection flags.
       ULONG  port2;    // DWORD.L xx xx xx xx OURPORT2 The sender's "other" port.
       ULONG  cookie;   // DWORD.L xx xx xx xx COOKIE The connection cookie the server gave for this pair of UINs.
       ULONG  x2;       // DWORD.L 50 00 00 00 UNKNOWN Unknown: 0x50 = 80.
       ULONG  x3;       // DWORD.L 03 00 00 00 UNKNOWN Unknown: 0x3 = 3.
       ULONG  x5;       // DWORD.L 00 00 00 00 UNKNOWN Unknown: empty.
    } *pkt = (struct _hello *) ptr;

    HUSER       usr;
    ULONG	uin;
    char        buffer[0x0100];

    if(sz < sizeof(struct _hello))
    {
       icqDumpPacket(icq, NULL, "Unexpected size in hello packet", sz, (unsigned char *) pkt);
       return 1;
    }
    else if(pkt->tcpVer != 0x08)
    {
       DBGTrace(pkt->tcpVer);
       icqDumpPacket(icq, NULL, "Unexpected version in hello packet", sz, (unsigned char *) pkt);
       return 2;
    }
    else if(pkt->to != icqQueryUIN(icq))
    {
       icqDumpPacket(icq, NULL, "Unexpected version in hello packet", sz, (unsigned char *) pkt);
       return 3;
    }
    else
    {
       uin = pkt->from;

       if(icqIsOnline(icq) || !(cfg->flags & PEERFLAG_ORIGINATE) )
          usr = icqSetUserIPandPort(icq, uin, pkt->gateway, pkt->ip, pkt->port1);
       else
          usr = icqQueryUserHandle(icq,uin);
    }

    if(usr)
    {
       if(!usr->peerCookie)
       {
          usr->peerCookie = pkt->cookie;
       }
       else if(usr->peerCookie != pkt->cookie)
       {
          sprintf(buffer,"WARNING! Cookie mismatch (Expected:%08lx Received:%08lx)",usr->peerCookie,pkt->cookie);
          icqWriteSysLog(icq,PROJECT,buffer);
       }
    }

#ifdef EXTENDED_LOG
    sprintf(buffer,"%s (ICQ#%ld) Cookie=%08lx",icqQueryUserNick(usr),uin,pkt->cookie);
    icqWriteSysLog(icq,PROJECT,buffer);
#endif

    icqv8_sendBlock(cfg->sock, 4, peer_initAck);

    if( (cfg->flags & (PEERFLAG_MESSAGE|PEERFLAG_ORIGINATE)) == (PEERFLAG_MESSAGE|PEERFLAG_ORIGINATE))
       return icqv8_sendMsgInit(icq,cfg,TRUE);

    return 0;
 }

 int icqv8_sendMsgInit(HICQ icq, PEERSESSION *cfg, BOOL origin)
 {
    #pragma pack(1)

    struct _header
    {
       UCHAR cmd;	// BYTE  03  CMD  The command: the last connect package.
       ULONG x1;        // DWORD.L 0a 00 00 00 UNKNOWN Unknown: 0xa = 10.
       ULONG x2;        // DWORD.L 01 00 00 00 UNKNOWN Unknown: 0x1 = 1. Note: sometimes Windows ICQ sends a packet with rather strange values from here on; this seems to be to test the connection. If this value isn't 1, disconnect.
       ULONG x3;        // DWORD.L xx xx xx xx UNKNOWN Unknown. Use 01 00 00 00 = 0x1 = 1 for incoming, 0 for outgoing connections.
       ULONG x4;        // DWORD.L 00 00 00 00 UNKNOWN Unknown: empty.
       ULONG x5;        // DWORD.L 00 00 00 00 UNKNOWN Unknown: empty.
       ULONG x6;        // DWORD.L xx xx xx xx UNKNOWN Unknown. Use 01 00 04 00 = 0x40001 for incoming and 0 for outgoing connections.
       ULONG x7;        // DWORD.L 00 00 00 00 UNKNOWN Unknown: empty.
       ULONG x8;        // DWORD.L xx xx xx xx UNKNOWN Unknown. Use 0 on incoming, but 01 00 04 00 = 0x4001 for outgoing connections.
    } h = { 0x03, 0x0A,  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00040001 };

#ifdef EXTENDED_LOG
    char buffer[0x0100];
#endif


/*
 Origem:   0000000a
           00000001
           00000000
           00000000
           00000000
           00000000
           00000000
           00040001
  							
 Resposta: 0000000a
           00000001
           00000001 <X3
           00000000
           00000000
           00040001 <x6
           00000000
           00000000 <x8 							

*/  							

   if(!origin)
   {
      h.x3 = 0x00000001;
      h.x6 = 0x00040001;
      h.x8 = 0x00000000;
   }

#ifdef EXTENDED_LOG
    sprintf(buffer,"%s (ICQ#%ld): Sending msg_init for session %08lx",icqQueryUserNick(icqQueryUserHandle(icq,cfg->uin)),cfg->uin,(ULONG) cfg);
    icqWriteSysLog(icq,PROJECT,buffer);

    sprintf(buffer,"%08lx Send init: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx",
    	           (ULONG) cfg,h.x1,h.x2,h.x3,h.x4,h.x5,h.x6,h.x7,h.x8 );
//    icqWriteSysLog(icq,PROJECT,buffer);
    icqDumpPacket(icq,icqQueryUserHandle(icq,cfg->uin),buffer,sizeof(h),(unsigned char *) &h);
#endif

    return icqv8_sendBlock(cfg->sock, sizeof(h), &h);

 }

 static int icqv8_peerInitAck(HICQ icq, PEERSESSION *cfg, int sz, unsigned char *ptr)
 {
    char  buffer[0x0100];

    if((cfg->flags & PEERFLAG_INIT) && (sz > 5))
    {
       icqv8_setPeerSpeed(icq,cfg,*((ULONG *) ptr+1));

       sprintf(buffer,"Peer file-init-ack in session %08lx (Speed: %ld)",(ULONG) cfg, cfg->speed);
#ifdef EXTENDED_LOG
       icqDumpPacket(icq, icqQueryUserHandle(icq,cfg->uin), buffer, sz, (unsigned char *) ptr);
#else
       icqWriteSysLog(icq,PROJECT,buffer);
#endif
       DBGMessage(buffer);

       icqv8_peerFileStart(icq,cfg,(const char *) (cfg+1));
    }
    else if(sz != 4)
    {
       sprintf(buffer,"Unexpected peer-init ack in session %08lx",(ULONG) cfg);
       icqDumpPacket(icq, icqQueryUserHandle(icq,cfg->uin), buffer, sz, (unsigned char *) ptr);
    }

    return 0;
 }

 int icqv8_initSession(HICQ icq, HPEER cfg, PEERSESSION *session, ULONG uin)
 {
#ifdef EXTENDED_LOG
    char buffer[0x0100];
#endif

    session->sz        = sizeof(PEERSESSION);
    session->uin       = uin;
    session->peer      = cfg;

    session->sequence  = 0xFFFF;
    session->seqMasc   = (ULONG) icqQueryShortSequence(icq);
    session->seqMasc <<= 16;

#ifdef EXTENDED_LOG
    sprintf(buffer,"Session %08lx using sequence ID %08lx",(ULONG) session, session->seqMasc);
    icqWriteSysLog(icq,PROJECT,buffer);
#endif

    return 0;
 }

 static int icqv8_peerFileInit(HICQ icq, PEERSESSION *cfg, int sz, unsigned char *ptr)
 {
    #pragma pack(1)

    char     buffer[0x0100];
    char     *req;
    ULONG    id	      = 0;

    struct _ack
    {
       UCHAR cmd;	// BYTE  01  CMD  The command: PEER_FILE_INITACK
       ULONG speed;	// DWORD.L xx xx xx xx SPEED The receiver's speed. See PEER_FILE_INIT.
       USHORT nickSize; // LNTS xx xx ... NICK The receiver's nick.
       char   nick[1];
    } *ack = (struct _ack *) buffer;

    struct _header
    {
       UCHAR  cmd;      // UBYTE 00 CMD The command: PEER_FILE_INIT.
       ULONG  x1;       // DWORD 00 00 00 00 EMPTY Unknown: empty.
       ULONG  count;    // DWORD.L xx xx xx xx COUNT Total number of files to be sent.
       ULONG  bytes;    // DWORD.L xx xx xx xx BYTES Total bytes of all files to sent.
       ULONG  speed;    // DWORD.L xx xx xx xx SPEED The sender's speed: 0 = PAUSE, 64 = no delay between packets, 0 < n < 64 = (64-n) * 0.05s delay.
       USHORT nickSize; // LNTS xx xx ... NICK Sender's nick.
       char   nick[1];
    } *h = (struct _header *) ptr;

#ifdef EXTENDED_LOG
    sprintf(buffer,"Session %08lx file init for %ld file(s) with %ld bytes (speed=%ld)",
    				(ULONG) cfg,
    				h->count,
    				h->bytes,
    				h->speed);
    icqWriteSysLog(icq,PROJECT,buffer);
    DBGMessage(buffer);
#endif

#ifdef DEBUG

    DBGTracex(icqQueryNextRequest(icq, 0, &id));
    id = 0;

    DBGTracex(icqQueryNextRequest(icq, (ULONG) cfg->peer, &id));
    id = 0;

#endif

    while( (req = icqQueryNextRequest(icq, (ULONG) cfg->peer, &id)) != NULL)
    {
       DBGTrace(id);

       DBGTracex( *( (USHORT *) req)     );
       DBGTrace( *( (USHORT *) (req+2)) );
       DBGTrace( *( (ULONG  *) (req+4)) );

       if( *( (USHORT *) req) == MSG_GREET_FILE && *( (ULONG  *) (req+4)) == h->bytes )
       {       						
#ifdef EXTENDED_LOG
          sprintf(buffer,"Session %08lx has accepted file init",(ULONG) cfg);
          DBGMessage(buffer);
          icqWriteSysLog(icq,PROJECT,buffer);
#endif
          cfg->counter  = h->count;
          cfg->bytes    = h->bytes;
          cfg->flags   |= PEERFLAG_FILE;

          icqv8_setPeerSpeed(icq, cfg, h->speed);

          memset(buffer,0,0xFF);

          ack->cmd   = 0x01;
          ack->speed = cfg->speed;

          icqLoadString(icq, "NickName", "pwICQ user", ack->nick, 0xFF - sizeof(struct _ack));
          ack->nickSize = strlen(ack->nick)+1;

          icqv8_sendBlock(cfg->sock, sizeof(struct _ack)+ack->nickSize-1, (const void *) ack);

          return 0;
       }

    }

#ifdef EXTENDED_LOG
    sprintf(buffer,"Session %08lx has rejected file init",(ULONG) cfg);
    DBGMessage(buffer);
    icqWriteSysLog(icq,PROJECT,buffer);
#endif

    cfg->flags &= ~PEERFLAG_ACTIVE;

    return 0;
 }

 static int icqv8_peerMsgPacket(HICQ icq, PEERSESSION *cfg, int sz, unsigned char *pkt)
 {
    if(cfg->flags & PEERFLAG_FILE)
       return icqv8_recvFileStart(icq, cfg, sz, pkt);
    return icqv8_recvMsg(icq, cfg, sz, pkt);
 }


 static int icqv8_peerInitPacket(HICQ icq, PEERSESSION *cfg, int sz, unsigned char *pkt)
 {
    if(cfg->flags & PEERFLAG_FILE)
       return icqv8_recvFileInit(icq, cfg, sz, pkt);
    return icqv8_msgInit(icq, cfg, sz, pkt);
 }

 static int icqv8_peerFileData(HICQ icq, PEERSESSION *cfg, int sz, unsigned char *ptr)
 {
/*
BYTE  06  CMD  The command: PEER_FILE_DATA.
DATA ... DATA The data to transfer. Unless the end of the file is reached, this should always be 2048 bytes.
*/
    sz--;
    ptr++;

    if(!(cfg->flags & PEERFLAG_FILE))
    {
       icqWriteSysLog(icq,PROJECT,"Peer file data received in a non file session");
       return 1;
    }

    if(cfg->flags & PEERFLAG_ORIGINATE)
    {
       icqWriteSysLog(icq,PROJECT,"Peer file data received in a originate session");
       return 2;
    }

    if(!cfg->arq)
    {
       icqWriteSysLog(icq,PROJECT,"Peer file data received without file prefix");
       return 2;
    }

    if(fwrite(ptr, sz,  1, cfg->arq) != 1)
    {
       icqWriteSysRC(icq, PROJECT, -1, "Can't write output file");
       return 3;
    }

    cfg->transfer += sz;

    if(cfg->transfer >= cfg->bytes)
    {
       icqWriteSysLog(icq,PROJECT,"File was received");
       fclose(cfg->arq);
       cfg->arq    = NULL;
       cfg->flags &= PEERFLAG_READY;
       cfg->flags |= PEERFLAG_COMPLETE;
    }

    return 0;
 }
