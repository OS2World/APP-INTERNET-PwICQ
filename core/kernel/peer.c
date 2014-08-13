/*
 * PEER.C - Gerenciamento de protocolos para peer
 */

 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>

 #include <icqkernel.h>

/*---[ Implementacao ]------------------------------------------------------------------------------------------*/

 long EXPENTRY icqQueryPeerPort(HICQ icq)
 {
    long ret = 0;

    if(icq->peer && icq->peer->getPort)
       ret = icq->peer->getPort(icq,icq->peerData);

    if(ret > 0)
       return ret;

    return 9091;
 }

 long EXPENTRY icqQueryPeerAddress(HICQ icq)
 {
    long ret = 0;

    if(icq->peer && icq->peer->getAddress)
       ret = icq->peer->getAddress(icq,icq->peerData);

    if(ret > 0)
       return ret;

    return icqGetHostID();
 }
 
 const PEERPROTMGR * EXPENTRY icqQueryPeerProtocolManager(HICQ icq, USHORT version)
 {
    if(icq->peer && (icq->peer->version == version || !version))
       return icq->peer;
    return NULL;
 }

 void * EXPENTRY icqQueryPeerProtocolData(HICQ icq, USHORT version)
 {
    if(icq->peer && (icq->peer->version == version || !version))
       return icq->peerData;
    return NULL;
 }


 ULONG EXPENTRY icqSetPeerCookie(HICQ icq, USHORT version, ULONG cookie)
 {
    if(icq->peer && icq->peer->setCookie)
       return icq->peer->setCookie(icq,icq->peerData,cookie);
    return 0;
 }

 ULONG EXPENTRY icqGetPeerCookie(HICQ icq, USHORT version)
 {
    if(icq->peer && icq->peer->getCookie)
       return icq->peer->getCookie(icq,icq->peerData);
    return 0;
 }

 PEERDATA * EXPENTRY icqGetPeerSession(HICQ icq, ULONG uin, const PEERPROTMGR *peer)
 {
    HUSER usr = icqQueryUserHandle(icq,uin);

    if(!usr)
       return NULL;

    if(!usr->peer)
       return NULL;

    if(peer && peer != usr->peer)
       return NULL;

    return usr->peerData;
 }

 int EXPENTRY icqSetPeerSession(HICQ icq, ULONG uin, ULONG session, const PEERPROTMGR *peer, PEERDATA *peerData)
 {
#ifdef EXTENDED_LOG
    int     reqCount = 0;
#endif
    char    buffer[0x0100];
    HUSER   usr = icqQueryUserHandle(icq,uin);
    REQUEST *req;
    REQUEST *next;

    if(!usr)
    {
       sprintf(buffer,"Invalid user (ICQ#%ld) when registering peer protocol session",uin);
       icqWriteSysLog(icq,PROJECT,buffer);
       return 1;
    }

    usr->u.lastAction = time(NULL);

    if(peer && peerData)
    {
       if(peer->sz != sizeof(PEERPROTMGR))
       {
          icqWriteSysLog(icq,PROJECT,"Invalid peer protocol manager, can't register it in the user");
          return 2;
       }

       if(peer->sendFileReq && (icqGetCaps(icq)&ICQMF_FILE))
          usr->u.flags |= USRF_CANRECEIVE;

       if(peer->sendChatReq)
          usr->u.flags |= USRF_CANCHAT;

       usr->peer     = peer;
       usr->peerData = peerData;

       sprintf(buffer,"V%d Peer message session %08lx registered for user %s (ICQ#%ld)",peer->version, (ULONG) peerData, icqQueryUserNick(usr), uin);
       icqWriteSysLog(icq,PROJECT,buffer);

       icqUserEvent(icq, usr, ICQEVENT_PEERBEGIN);

       if( usr->u.mode  == icqQueryOfflineMode(icq))
       {
#ifdef EXTENDED_LOG
          icqWriteSysLog(icq,PROJECT,"Remote session for offline user, icon needs adjust");
#endif
          usr->u.modeIcon  = 14;
          usr->u.flags    |= USRF_ONLINE;

          icqAjustUserIcon(icq, usr, TRUE);
       }

    }
    else if(usr->peer == peer)
    {

       sprintf(buffer,"Peer message session %08lx unregistered for user %s (ICQ#%ld)",(ULONG) usr->peerData, icqQueryUserNick(usr), uin);
       icqWriteSysLog(icq,PROJECT,buffer);

       usr->u.flags &= ~(USRF_CANRECEIVE|USRF_CANCHAT);

       usr->peer     =
       usr->peerData = NULL;

       icqUserEvent(icq, usr, ICQEVENT_PEEREND);

       next = icqQueryFirstElement(icq->requests);

       while(next)
       {
          req  = next;
          next = icqQueryNextElement(icq->requests,req);

          if(req->session == session)
          {
             sprintf(buffer,"Abnormal ending for request %08lx (Messaged ID %08lx)",(ULONG) req, req->id);
             icqWriteSysLog(icq,PROJECT,buffer);
             icqRemoveElement(icq->requests,req);
          }
#ifdef EXTENDED_LOG
          else
          {
             reqCount++;
          }
#endif
       }

       if( usr->u.mode  == icqQueryOfflineMode(icq))
       {
#ifdef EXTENDED_LOG
          icqWriteSysLog(icq,PROJECT,"Remote session for offline user, icon needs adjust");
#endif
          usr->u.flags    &= ~USRF_ONLINE;

          icqAjustUserIcon(icq, usr, TRUE);
       }
    }

#ifdef EXTENDED_LOG
    sprintf(buffer,"Pending requests: %d",reqCount);
    icqWriteSysLog(icq,PROJECT,buffer);
#endif
    return 0;
 }

 int EXPENTRY icqSendFile(HICQ icq, ULONG uin, const char *fileName, const char *message)
 {
    HUSER usr       = icqQueryUserHandle(icq,uin);
    long  fileSize  = 0;
    FILE  *arq;

    if(!usr)
       return 1;

    if(!usr->peer)
    {
       icqWriteSysLog(icq,PROJECT,"No peer protocol manager for the user");
       return 2;
    }

    if(!((PEERPROTMGR *) usr->peer)->sendFileReq)
    {
       icqWriteSysLog(icq,PROJECT,"The active peer protocol manager can't send files");
       return 3;
    }

    arq = fopen(fileName,"r");

    if(!arq)
    {
       icqWriteSysRC(icq, PROJECT, -1, fileName);
       return 4;
    }
    else
    {
       if(fseek(arq, 0, SEEK_END))
       {
          icqWriteSysRC(icq, PROJECT, -1, "Can't get file size");
          fclose(arq);
          return 5;
       }
       fileSize = ftell(arq);
       fclose(arq);

       if(fileSize == -1)
          return 6;

    }

    return (int) ((PEERPROTMGR *) usr->peer)->sendFileReq(icq,usr,usr->peerData,fileName,1,message,fileSize);

 }

 int EXPENTRY icqBeginPeerSession(HICQ icq, HUSER usr)
 {
#ifdef EXTENDED_LOG
    char         logBuffer[0x0100];
#endif

    const PEERPROTMGR *peer;

    DBGTracex(usr);

    if(!usr)
       return 1;

    if(!icqQueryUserIP(icq, usr))
    {
#ifdef EXTENDED_LOG
       sprintf(logBuffer,"%s (ICQ#%ld) Doesn't have IP address defined",icqQueryUserNick(usr),usr->u.uin);
       icqWriteSysLog(icq,PROJECT,logBuffer);       				
#endif
       return 5;
    }


    if(!usr->port)
    {
#ifdef EXTENDED_LOG
       sprintf(logBuffer,"%s (ICQ#%ld) Doesn't have peer port defined",icqQueryUserNick(usr),usr->u.uin);
       icqWriteSysLog(icq,PROJECT,logBuffer);       				
#endif
       usr->port = 2301;
    }

    if(usr->peer)
    {
       peer = usr->peer;
    }
    else if(!usr->u.peerVersion)
    {
       peer = icqQueryPeerProtocolManager(icq, 0);
    }
    else
    {
       peer = icqQueryPeerProtocolManager(icq, usr->u.peerVersion);
       if(!peer)
       {
          if(icq->peer && (icq->peer->version < usr->u.peerVersion))
             peer = icq->peer;
       }
    }

    DBGTracex(peer);
    DBGTracex(icqQueryPeerProtocolData(icq,usr->u.peerVersion));

    if(peer && peer->beginSession)
    {
#ifdef EXTENDED_LOG
       sprintf(logBuffer,"%s (ICQ#%ld) PeerVersion: %d, Manager: %08lx (Vrs %d)",icqQueryUserNick(usr),usr->u.uin,(int) usr->u.peerVersion,(ULONG) peer, (int) peer->version);
       icqWriteSysLog(icq,PROJECT,logBuffer);       				
#endif
       DBGMessage(peer->descr);
       return peer->beginSession(icq,icqQueryPeerProtocolData(icq,usr->u.peerVersion),usr,usr->port);
    }

#ifdef EXTENDED_LOG
    sprintf(logBuffer,"%s (ICQ#%ld) PeerVersion: %d, Manager: %08lx",icqQueryUserNick(usr),usr->u.uin,(int) usr->u.peerVersion,(ULONG) peer);
    icqWriteSysLog(icq,PROJECT,logBuffer);       				
#endif

    return 4;

 }


