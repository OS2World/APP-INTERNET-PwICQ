/*
 * listen.c - listener thread
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

#ifdef linux
 #include <unistd.h>
#endif

 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>

 #include "peer.h"

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static void         execute(HICQ, HPEER);
 static int  	     validate(HICQ,HPEER,int);

 static void _System beginSession(ICQTHREAD *);
 static void _System peerThread(ICQTHREAD *);

 static void         sendAck(HICQ, PEERSESSION *);

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 int _System icqv8_startListener(HICQ icq, HPEER cfg)
 {
    cfg->thread   = icqCreateThread(icq, peerThread, 16384, 0, cfg, "c2c");
    return 0;
 }

 static void  _System peerThread(ICQTHREAD *thd)
 {
    PEERCONF *cfg = thd->parm;

    DBGMessage("Listener thread started");

    execute(thd->icq,cfg);

    if(cfg->sock > 0)
    {
       icqCloseSocket(cfg->sock);
       cfg->sock = 0;
    }
	
    cfg->active = FALSE;
    cfg->thread = -1;
    DBGMessage("Listener thread stopped");
 }

 static void execute(HICQ icq, HPEER cfg)
 {
    char        buffer[0x0100];
    PEERSESSION *session;
    long        ip;
    int         port;
    int         sock;
    int         f;

    cfg->sock = icqOpenTCP(icqLoadValue(icq,"peer:listen",2301));

    DBGTrace(cfg->sock)

    if(cfg->sock < 1)
    {
       icqWriteNetworkErrorLog(icq, PROJECT, "Error creating peer socket");
       return;
    }

    if(icqListen(cfg->sock))
    {
       icqWriteNetworkErrorLog(icq, PROJECT, "Error configuring peer socket");
       return;
    }

	DBGTracex(icqQueryPeerAddress(icq));

    cfg->sessions = icqCreateList();
    cfg->active   = TRUE;
    cfg->sz       = sizeof(PEERSESSION);

    sprintf(buffer,"Peer session manager %08lx started",(ULONG) cfg);
    DBGMessage(buffer);
    icqWriteSysLog(icq,PROJECT,buffer);

    sock = icqAcceptConnct(cfg->sock, &ip, &port);
    DBGTrace(sock);

    while(icqIsActive(icq) && cfg->active && sock > 0)
    {
       DBGTrace(sock);

       strcpy(buffer,"Connection received from ");
       icqIP2String(ip,buffer+strlen(buffer));
       sprintf(buffer+strlen(buffer),":%d",port);
       icqWriteSysLog(icq,PROJECT,buffer);

       if(icqCheckBlacklist(icq, ip))
       {
          icqWriteSysLog(icq,PROJECT,"Blacklisted IP address, connection rejected");
          icqSystemEvent(icq,ICQEVENT_CONNCTNOK);
          icqCloseSocket(sock);
       }
       else if(validate(icq,cfg,sock))
       {
          icqWriteSysLog(icq,PROJECT,"Peer session rejected");
          icqSystemEvent(icq,ICQEVENT_CONNCTNOK);
          icqCloseSocket(sock);
       }

       sock = icqAcceptConnct(cfg->sock, &ip, &port);
    }

    icqWriteSysLog(icq,PROJECT,"Closing all sessions");

    for(session = icqQueryFirstElement(cfg->sessions);session;session = icqQueryNextElement(cfg->sessions,session))
    {
       DBGTrace(session->uin);
       session->flags &= ~PEERFLAG_ACTIVE;
       if(session->sock > 0)
          icqShutdownSocket(session->sock);
    }
    cfg->active = FALSE;

    DBGTracex(icqQueryFirstElement(cfg->sessions));

#ifdef __OS2__
    for(f=0;f < 30 && icqQueryFirstElement(cfg->sessions);f++)
       DosSleep(100);
#else
    for(f=0;f < 1500 && icqQueryFirstElement(cfg->sessions);f++)
       usleep(100);
#endif

    DBGTracex(icqQueryFirstElement(cfg->sessions));

    if(icqQueryFirstElement(cfg->sessions))
       icqWriteSysLog(icq,PROJECT,"Failure closing sessions");

    CHKPoint();
    icqDestroyList(cfg->sessions);

    CHKPoint();
    icqWriteSysLog(icq,PROJECT,"Sessions closed");

    CHKPoint();
    cfg->thread = 0;

 }

 static int validate(HICQ icq, HPEER cfg, int sock)
 {
    #pragma pack(1)

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
    } *pkt;

    int         sz           = icqv8_recvPeer(icq,cfg,sock,(void **) &pkt);
    ULONG       uin          = 0;
    HUSER       usr          = NULL;
    PEERSESSION *session;
    char        buffer[0x0100];

    if(!sz)
       return -1;

    if(sz != sizeof(struct _hello))
    {
       icqDumpPacket(icq, NULL, "Unexpected size in hello packet", sz, (unsigned char *) pkt);
    }
    else if(pkt->tcpVer != 0x08)
    {
       sprintf(buffer,"Unexpected version %d in hello packet",pkt->tcpVer);
       icqDumpPacket(icq, NULL, buffer, sz, (unsigned char *) pkt);
    }
    else if(pkt->to != icqQueryUIN(icq))
    {
       sprintf(buffer,"Unexpected ICQ# %ld in hello packet",pkt->to);
       icqDumpPacket(icq, NULL, buffer, sz, (unsigned char *) pkt);
    }
    else
    {
       uin = pkt->from;
       usr = icqSetUserIPandPort(icq, pkt->from, pkt->gateway, pkt->ip, pkt->port1);
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
    sprintf(buffer,"%s (ICQ#%lx) Cookie=%08lx",icqQueryUserNick(usr),uin,pkt->cookie);
    icqWriteSysLog(icq,PROJECT,buffer);
#endif

    free(pkt);

    if(uin)
    {
       session = icqAddElement(cfg->sessions, sizeof(PEERSESSION));
       if(session)
       {
          icqSystemEvent(icq,ICQEVENT_CONNCTOK);
          icqv8_initSession(icq,cfg, session, uin);
          session->flags  = PEERFLAG_ACTIVE;
          session->sock   = sock;
          session->thread = icqCreateThread(icq, beginSession, 16384, 0, session, "per");
          sprintf(buffer,"Peer session %08lx started with %s (ICQ#%ld),tid=%d",(ULONG) session, icqQueryUserNick(usr), uin, (int) session->thread);
          icqWriteSysLog(icq,PROJECT,buffer);
       }
       else
       {
          uin = 0;
       }

    }

    return uin == 0;
 }

 static void sendAck(HICQ icq, PEERSESSION *cfg)
 {
    #pragma pack(1)

    static const struct ackPkt
    {
       UCHAR cmd;   //   BYTE  01  CMD  The command: acknowlegde the PEER_INIT.
       UCHAR x1[3]; //   3 00 00 00 UNKNOWN Unknown: empty.
    } pkt = { 0x01, { 0x00, 0x00, 0x00 } };

    icqv8_sendBlock(cfg->sock,sizeof(struct ackPkt),&pkt);

 }

 static void _System beginSession(ICQTHREAD *thd)
 {
    while( ! ((PEERSESSION *) thd->parm)->thread )
    {
#ifdef __OS2__
       DosSleep(100);
#else
       usleep(100);
#endif
    }

    DBGMessage("Thread de controle de sessao iniciada");
    sendAck(thd->icq, (PEERSESSION *) thd->parm);
    icqv8_peerSession(thd->icq,(PEERSESSION *) thd->parm);
    DBGMessage("Thread de controle de sessao terminada");
 }

 void icqv8_peerTimer(HICQ icq, HPEER cfg)
 {
    char        buffer[0x0100];
    PEERSESSION *session        = NULL;
    PEERSESSION *next		= NULL;

    if(cfg->active)
    {
       for(session = icqQueryFirstElement(cfg->sessions);session;session = next)
       {
          next = icqQueryNextElement(cfg->sessions,session);
          if(session->idle++ > 600)
          {
             sprintf(buffer,"Session %08lx idle for %d seconds",(ULONG) session, (int) session->idle);
             icqWriteSysLog(icq,PROJECT,buffer);
             session->flags &= ~PEERFLAG_ACTIVE;
             if(session->sock > 0)
                icqShutdownSocket(session->sock);
          }
          else if(session->thread < 1 && session->idle > 5)
          {
             sprintf(buffer,"Cleaning session %08lx",(ULONG) session);
             DBGMessage(buffer);
             icqWriteSysLog(icq,PROJECT,buffer);
             icqRemoveElement(cfg->sessions, session);
          }
       }
    }
 }


