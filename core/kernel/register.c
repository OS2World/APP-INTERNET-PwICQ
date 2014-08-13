/*
 * REGISTER.C - Registra plugins de controle
 */

 #include <string.h>
 #include <icqkernel.h>
 #include <stdio.h>

 #define sysLog(i,t) icqWriteSysLog(i,PROJECT,t)

/*---[ Implementacao ]------------------------------------------------------------------------*/

 void EXPENTRY icqSetServiceStatus(HICQ icq,int id, BOOL status)
 {
    if(status)
       icq->srvcStatus |= id;
    else
       icq->srvcStatus &= ~id;
 }

 BOOL EXPENTRY icqRegisterServerProtocolManager(HICQ icq, const C2SPROTMGR *mgr)
 {
    const ICQMODETABLE *md;

    if(icq->c2s)
    {
       sysLog(icq,"WARNING: More than one server protocol manager installed");
       return FALSE;
    }

    if(mgr->sz != sizeof(C2SPROTMGR))
    {
       sysLog(icq,"Invalid server protocol manager loaded");
       return FALSE;
    }

    if(!mgr->modeButton)
    {
       sysLog(icq,"No mode table defined by protocol manager");
       return FALSE;
    }

    md = mgr->modeButton;

    for(md=mgr->modeButton;md->descr;md++)
       icq->offline = md;

    icq->currentMode    = icqQueryOfflineMode(icq);
    icq->c2s            = mgr;

    return TRUE;
 }

 BOOL EXPENTRY icqDeRegisterServerProtocolManager(HICQ icq, const C2SPROTMGR *mgr)
 {
    if(mgr != icq->c2s)
       return FALSE;
    icq->c2s = NULL;
    return TRUE;
 }


 BOOL ICQAPI icqRegisterPeerProtocolManager(HICQ icq, const PEERPROTMGR *mgr, void *data)
 {
    char buffer[0x0100];

    if(!mgr)
    {
       sysLog(icq,"Invalid call to icqRegisterPeerProtocolManager");
       return FALSE;
    }

    if(icq->peer)
    {
       sysLog(icq,"WARNING: More than one peer protocol manager installed");
       return FALSE;
    }

    if(mgr->sz != sizeof(PEERPROTMGR))
    {
       sysLog(icq,"Invalid peer protocol manager loaded");
       return FALSE;
    }

    icq->peer     = mgr;
    icq->peerData = data;

    if(mgr->startListener)
    {
       sprintf(buffer,"Starting peer protocol manager version %d as default",mgr->version);
       mgr->startListener(icq,data);
    }
    else
    {
       sprintf(buffer,"Peer protocol manager version %d installed as default",mgr->version);
    }
    sysLog(icq,buffer);


    return TRUE;
 }

 BOOL ICQAPI icqDeRegisterPeerProtocolManager(HICQ icq, const PEERPROTMGR *mgr)
 {
    if(mgr != icq->peer)
       return FALSE;

    icq->peer     = NULL;
    icq->peerData = NULL;

    return TRUE;
 }

 BOOL EXPENTRY icqRegisterSkinManager(HICQ icq, const SKINMGR *mgr)
 {
    DBGTracex(mgr);

    if(!icq || icq->sz != sizeof(ICQ))
    {
       sysLog(icq,"Unexpected ICQ handler received on icqRegisterSkinManager");
       return FALSE;
    }

    if(icq->skin)
    {
       sysLog(icq,"WARNING: More than one skin manager installed");
       return FALSE;
    }

    if(mgr->sz != sizeof(SKINMGR))
    {
       sysLog(icq,"Invalid skin manager loaded");
       return FALSE;
    }

    if(icq->skinRequired && stricmp(icq->skinRequired,mgr->id))
    {
#ifdef EXTENDED_LOG
       icqSysLog(icq,"Skin Manager rejected by configuration");
#endif
       return FALSE;
    }

#ifdef EXTENDED_LOG
    icqSysLog(icq,"Skin Manager loaded");
#endif

    icq->skin     = mgr;

    DBGMessage("Skin manager loaded");

    return TRUE;
 }

 BOOL EXPENTRY icqRegisterUserDBManager(HICQ icq, const USERDBMGR *mgr)
 {
    if(mgr->sz != sizeof(USERDBMGR))
    {
       sysLog(icq,"Invalid user database manager loaded");
       return FALSE;
    }
    icq->userDB = mgr;
    return TRUE;
 }

 BOOL EXPENTRY icqDeRegisterUserDBManager(HICQ icq, const USERDBMGR *mgr)
 {
    if(mgr != icq->userDB)
       return FALSE;
    icq->userDB = NULL;
    return TRUE;
 }

 int EXPENTRY icqRegisterDefaultMessageProcessor(HICQ icq, USHORT sz, const MSGMGR **msgMgr)
 {
    const MSGMGR **mgr = msgMgr;

    if(sz != sizeof(MSGMGR))
       return 1;

    DBGTracex(icq->msgMgr);
    if(icq->msgMgr)
    {
       icqWriteSysLog(icq,PROJECT,"Default message processor already installed");
       return 2;
    }

    /* Pesquisa no processador default pelo principal */

    while(*mgr)
    {
       if( (*mgr)->sz != sizeof(MSGMGR) )
       {
          icqWriteSysLog(icq,PROJECT,"Invalid message processor detected");
          return 3;
       }
       mgr++;
    }

    icq->msgMgr = msgMgr;

    return 0;
 }

