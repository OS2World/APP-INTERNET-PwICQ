/*
 * connect.c - Begin connection
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

#ifdef linux
 #include <unistd.h>
#endif

 #include <string.h>
 #include <stdio.h>
 #include <stdlib.h>

 #include "peer.h"

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static void _System beginSession(ICQTHREAD *);

 static void execute(HICQ,PEERSESSION *);
// static int  initialize(HICQ, PEERSESSION *);

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 int _System icqv8_connect(HICQ icq, HPEER cfg, HUSER usr, int port)
 {
    char        buffer[0x0100];
    PEERSESSION *session;

    if(cfg->sz != sizeof(PEERSESSION))
    {
       sprintf(buffer,"Unexpected session manager %08lx in connect request",(ULONG) cfg);
       DBGMessage(buffer);
       icqWriteSysLog(icq,PROJECT,buffer);
       return 5;
    }

    if(!cfg->active)
    {
       sprintf(buffer,"Session manager %08lx isn't active",(ULONG) cfg);
       DBGMessage(buffer);
       icqWriteSysLog(icq,PROJECT,buffer);
       return 6;
    }

    DBGTracex(usr);

    if(!usr)
    {
       icqWriteSysLog(icq,PROJECT,"Connect request without user handle, rejected");
       return 1;
    }

    DBGTracex(usr);

#ifndef DEBUG
    if(usr->peerVersion < 0x08)
    {
       icqWriteSysLog(icq,PROJECT,"Warning: Unexpected peer version in connect request");
    }

    CHKPoint();

    if(!icqQueryUserIP(icq, usr))
    {
       icqWriteSysLog(icq,PROJECT,"Invalid IP address in connect request");
       return 3;
    }
#endif

    CHKPoint();

    for(session = icqQueryFirstElement(cfg->sessions);session;session = icqQueryNextElement(cfg->sessions,session))
    {
       if(session->sz != sizeof(PEERSESSION))
       {
          sprintf(buffer,"Session %08lx with unexpected header",(ULONG) session);
          icqWriteSysLog(icq,PROJECT,buffer);
          return 5;
       }
       if(session->uin == usr->uin)
          return 4;
    }

    CHKPoint();

    session = icqAddElement(cfg->sessions, sizeof(PEERSESSION));
    if(session)
    {
       icqv8_initSession(icq,cfg,session,usr->uin);
       session->flags  = PEERFLAG_ACTIVE|PEERFLAG_MESSAGE|PEERFLAG_ORIGINATE;
       session->port   = port;
       session->thread = icqCreateThread(icq, beginSession, 16384, 0, session, "pes");
       sprintf(buffer,"Starting peer session %08lx with %s (ICQ#%ld),tid=%d",(ULONG) session, icqQueryUserNick(usr), usr->uin, (int) session->thread);
       icqWriteSysLog(icq,PROJECT,buffer);
    }

    CHKPoint();

    return 0;

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

    DBGMessage("Thread de conexao iniciada");
    execute(thd->icq,(PEERSESSION *) thd->parm);
    DBGMessage("Thread de conexao terminada");
 }

 static void execute(HICQ icq, PEERSESSION *cfg)
 {
    DBGTracex(cfg->flags & PEERFLAG_ACTIVE);
    if(icqv8_connectPeer(icq,cfg,TRUE))
    {
       icqRemoveElement(cfg->peer->sessions, cfg);
       return;
    }
    DBGTracex(cfg->flags & PEERFLAG_ACTIVE);
    icqv8_peerSession(icq, cfg);
 }

/*
 static int initialize(HICQ icq, PEERSESSION *cfg)
 {
    HUSER usr = icqQueryUserHandle(icq,cfg->uin);


    char  buffer[0x0100];

    if(!usr)
    {
       sprintf(buffer,"Invalid user %ld in connect request",cfg->uin);
       return 1;
    }

    if(!usr->peerCookie)
    {
       icqWriteSysLog(icq,PROJECT,"Invalid cookie in connect request");
       return 2;
    }


    sprintf(buffer,"Session %08lx contacting %s (ICQ#%ld)",(ULONG) cfg,icqQueryUserNick(usr),usr->uin);
    icqWriteSysLog(icq,PROJECT,buffer);

    cfg->sock = icqOpenTCP(-1);
    DBGTrace(cfg->sock);

    if(cfg->sock < 1)
       return -1;

    cfg->sock = icqConnectUser(icq,cfg->uin,cfg->sock);
    DBGTrace(cfg->sock);

    if(cfg->sock < 1)
       return -1;

    icqUserEvent(icq, usr, ICQEVENT_CONTACT);
    sprintf(buffer,"Session %08lx contacted %s (ICQ#%ld)",(ULONG) cfg,icqQueryUserNick(usr),usr->uin);

    return 0;

 }
*/

 int icqv8_connectPeer(HICQ icq, PEERSESSION *cfg, BOOL event)
 {
    char   buffer[0x0100];
    USHORT eventCode         = ICQEVENT_CANTCONNECT;
    HUSER  usr               = icqQueryUserHandle(icq,cfg->uin);
    int    rc                = 0;
    long   ip;

    if(!usr)
    {
       sprintf(buffer,"Invalid user %ld in connect request",cfg->uin);
       return 1;
    }

/*
    if(!usr->peerCookie)
    {
       icqWriteSysLog(icq,PROJECT,"Invalid cookie in connect request");
       return 2;
    }
*/

    ip = icqQueryUserIP(icq,usr);

    sprintf(buffer,"Session %08lx contacting %s (ICQ#%ld) at ",(ULONG) cfg,icqQueryUserNick(usr),cfg->uin);
    icqIP2String(ip,buffer+strlen(buffer));
    sprintf(buffer+strlen(buffer),":%d",cfg->port);
    icqWriteSysLog(icq,PROJECT,buffer);

    DBGMessage(buffer);

    cfg->sock = icqOpenTCP(-1);
    DBGTrace(cfg->sock);

    if(cfg->sock < 1)
       return 3;

    DBGTrace(cfg->port);

    cfg->sock = icqConnectSock(cfg->sock,ip,cfg->port);

    DBGTrace(cfg->sock);

    if(cfg->sock < 1)
    {
       sprintf(buffer,"Session %08lx can't contact %s (ICQ#%ld)",(ULONG) cfg,icqQueryUserNick(usr),usr->uin);
       icqWriteSysRC(icq, PROJECT, -1, buffer);
       cfg->flags &= ~PEERFLAG_ACTIVE;
       rc          = 4;
    }
    else
    {
       eventCode = ICQEVENT_CONTACT;
       sprintf(buffer,"Session %08lx stablished with %s (ICQ#%ld)",(ULONG) cfg,icqQueryUserNick(usr),usr->uin);
       icqWriteSysLog(icq,PROJECT,buffer);
    }

    DBGMessage(buffer);

    if(event)
       icqUserEvent(icq, usr, eventCode);

    return rc;

 }


