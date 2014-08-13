/*
 * MAIN.C -
 */


#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>
 #include <time.h>
 #include <malloc.h>

 #include "icqv8.h"

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static void start(HICQ, ICQV8 *);
 static void stop(HICQ, ICQV8 *);
 static void execute(HICQ, ICQV8 *);
 static void recFlap(HICQ,ICQV8 *);
 static void recPacket(HICQ, ICQV8 *, UCHAR, int, char *);
 static void sendConfirmations(HICQ, ICQV8 *);
 static void sendStatusMessage(HICQ, ICQV8 *);

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 void _System icqv8_mainThread(ICQTHREAD *thd)
 {
    ICQV8 *cfg = thd->parm;

    DBGMessage("Thread principal iniciada");

    start(thd->icq,cfg);

    if(cfg->sock > 0)
       execute(thd->icq,cfg);

    stop(thd->icq,cfg);

    DBGMessage("Thread principal terminada");
 }

 static void start(HICQ icq, ICQV8 *cfg)
 {
    long addr;
    char buffer[0x0100];
    char logLine[0x0200];

    CHKPoint();

    setStatus(icq,cfg,ICQV8_STATUS_FINDING);

    cfg->downSequence = 0xFFFF;
    cfg->wrkPacket    = 0xFFFF;

    icqSystemEvent(icq, ICQEVENT_FINDINGSERVER);

    icqLoadString(icq,"loginServer","login.icq.com",buffer,0xFF);

    sprintf(logLine,"Using server %s:%d",buffer,(int) LOGIN_PORT);
    icqWriteSysLog(icq,PROJECT,logLine);

    addr = icqGetHostByName(buffer);

    if(!addr || addr == 0xFFFFFFFF)
    {
       icqWriteSysLog(icq,PROJECT,"Unable to resolve the server name.");
       addr = icqLoadValue(icq,"v8.server",0);

       if(!addr)
       {
          setStatus(icq,cfg,ICQV8_STATUS_OFFLINE);
          return;
       }
       strcpy(logLine,"Trying cached address ");
       icqIP2String(addr,logLine+strlen(logLine));
       icqWriteSysLog(icq,PROJECT,logLine);
    }

    icqSystemEvent(icq, ICQEVENT_CONNECTING);

    cfg->packetSeq = random();
    cfg->sock      = icqConnect(addr, LOGIN_PORT);

    DBGTrace(cfg->sock);

    if(cfg->sock > 0)
       icqSaveValue(icq,"v8.server",addr);

 }

 static void stop(HICQ icq, ICQV8 *cfg)
 {
    cfg->status = 0;

    DBGMessage("*** Conexao encerrada ***");

    DBGTrace(cfg->sock);
    DBGTrace(icqIsActive(icq));

    if(cfg->wrkPacket != 0xFFFF)
       icqv8_releasePacket(icq, cfg, cfg->wrkPacket);

    if(cfg->sock > 0)
    {
       icqWriteSysLog(icq,PROJECT,"Sending Disconnect request");
	   CHKPoint();
       icqv8_goodbye(icq,cfg);
    }

	CHKPoint();
	
    if(icqLoadValue(icq,"hideWhenOffline",0))
       icqSystemEvent(icq,ICQEVENT_HIDE);

	CHKPoint();
    icqSetServiceStatus(icq,ICQSRVC_C2S, FALSE);

	CHKPoint();
    icqSystemEvent(icq, ICQEVENT_OFFLINE);
	
	CHKPoint();
    setStatus(icq,cfg,ICQV8_STATUS_OFFLINE);

    cfg->thread = -1;
 }

 static void execute(HICQ icq, ICQV8 *cfg)
 {
    UCHAR       byte = 0;
    int         sz;

    if(cfg->sock < 0)
       return;

    setStatus(icq,cfg,ICQV8_STATUS_LOGIN);

    if(icqLoadValue(icq,"hideWhenOffline",0))
       icqSystemEvent(icq,ICQEVENT_SHOW);

    cfg->watchDogCounter = 0;

    sz = icqRecv(cfg->sock,&byte,1);
    while(sz >= 0 && cfg->sock > 0 && cfg->status && icqIsActive(icq))
    {
       if(sz)
       {
          cfg->watchDogCounter++;
       }
       else
       {
          cfg->watchDogCounter = 0;
       }

//       DBGTrace(cfg->watchDogCounter);

       if(sz)
       {
          if(byte == PACKET_START)
          {
             recFlap(icq,cfg);
          }
          else
          {
             icqWriteSysLog(icq,PROJECT,"Invalid prefix received");
             icqv8_goodbye(icq,cfg);
          }
       }
       else if(!icqClearToSend(icq))
       {
#ifdef EXTENDED_LOG
          icqSysLog(icq,"C2S thread is IDLE");
#endif
          icqSetServerBusy(icq,FALSE);
       }
       else if(cfg->flags & ICQV8_FLAG_SEND_CONFIRMATIONS)
       {
          cfg->flags           &= ~ICQV8_FLAG_SEND_CONFIRMATIONS;
          sendConfirmations(icq,cfg);
       }
       else if(cfg->flags & ICQV8_FLAG_SENDSTATUSMSG)
       {
          sendStatusMessage(icq,cfg);
       }

       byte = 0;
       sz   = icqRecv(cfg->sock,&byte,1);
    }
	
	DBGTrace(sz);
	DBGTrace(cfg->sock);

    if(sz < 0 && cfg->sock > 0)
    {	   
	   DBGMessage("**** Abnormal disconnect, closing socket");
	   CLOSE_SOCKET(cfg);
       icqWriteNetworkErrorLog(icq, PROJECT, "Disconnected");
	}
	else
	{
	   DBGMessage("Normal disconnection");
       icqWriteNetworkErrorLog(icq, PROJECT, "Disconnecting");
	}

 }


 int icqv8_receiveBlock(HICQ icq, ICQV8 *cfg, int size, void *buffer)
 {
    int  bytes;
    char *ptr    = (char *) buffer;

    do
    {
       bytes = icqRecv(cfg->sock, ptr, size);

       while(bytes == 0 && icqIsActive(icq) && cfg->sock > 0)
          bytes = icqRecv(cfg->sock, ptr, size);

       if(bytes < 0)
       {
#ifdef EXTENDED_LOG
          icqWriteSysLog(icq,PROJECT,"Error receiving block");
#endif
          icqCloseSocket(cfg->sock);
          cfg->sock = -1;
          return -1;
       }

       ptr  += bytes;
       size -= bytes;
    } while(size > 0);

    return 0;
 }

 static void recFlap(HICQ icq,ICQV8 *cfg)
 {
    FLAP        flap;
    int         sz;
    char        work[0x0100];
    char        *buffer;

    if(icqv8_receiveBlock(icq,cfg,sizeof(FLAP),&flap))
       return;

    if(flap.channel > CHANNEL_CLOSE)
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected channel packet, disconnecting");
       CLOSE_SOCKET(cfg);
       return;
    }

    sz = SHORT_VALUE(flap.size);

//    DBGTrace(sz);

    if(sz > 0xFFFF)
    {
       icqWriteSysLog(icq,PROJECT,"Packet size greather than 0xFFFF, disconnecting");
       CLOSE_SOCKET(cfg);
       return;
    }

    if(sz > 0xFF)
    {
       DBGTrace(sz);

#ifdef OS2DEBUG
       if(_heapchk() != _HEAPOK)
           DBGPrint("************************ ERRO %d NO HEAP ************************",_heapchk());
#endif

       buffer = malloc(sz + 5); // 5 bytes of protection (Some message processing functions add a /0 code at the ending
       if(!buffer)
       {
          icqWriteSysLog(icq,PROJECT,"Memory allocation error when receiving packet");
          CLOSE_SOCKET(cfg);
          icqAbend(icq,0);
          return;
       }

       recPacket(icq,cfg,flap.channel,sz,buffer);

       free(buffer);
    }
    else
    {
       recPacket(icq,cfg,flap.channel,sz,work);
    }

 }

 void icqv8_processChannel3(HICQ icq, ICQV8 *cfg, int sz, void *buffer)
 {
    CHKPoint();
    icqDumpPacket(icq, NULL, "Packet Channel 3 received", sz, (unsigned char *) buffer);

 }


 static void recPacket(HICQ icq, ICQV8 *cfg, UCHAR channel, int sz, char *buffer)
 {
    static CHANNELPROCESSOR *proc[] = {                      icqv8_processChannel1,
                                        (CHANNELPROCESSOR *) icqv8_processChannel2,
                                                             icqv8_processChannel3,
                                                             icqv8_processChannel4
                                      };

    if(icqv8_receiveBlock(icq,cfg,sz,buffer))
       return;

    proc[channel-1](icq,cfg,sz,buffer);
 }

 ULONG icqv8_queryCurrentID(HICQ icq)
 {
    ICQV8 *cfg = icqGetPluginDataBlock(icq, module);

    ULONG ret = time(NULL);

    LOCK(cfg);

    while(ret == cfg->msgID)
       ret++;

    cfg->msgID = ret;

    UNLOCK(cfg);

    return ret;
 }

 ULONG icqv8_querySequence(HICQ icq)
 {
    ICQV8 *cfg = icqGetPluginDataBlock(icq, module);
    ULONG ret;

    LOCK(cfg);
    ret = cfg->sequence++;
    UNLOCK(cfg);

    return ret;
 }

 static void sendConfirmations(HICQ icq, ICQV8 *cfg)
 {

    DBGMessage("Confirmando modo online");

#ifdef DEBUG
    DBGTrace(cfg->watchDogCounter);
    icqWriteSysLog(icq,PROJECT,"ICQEVENT_LOGONCOMPLETE");
#endif
    icqSystemEvent(icq, ICQEVENT_LOGONCOMPLETE);

#ifdef DEBUG
    DBGTrace(cfg->watchDogCounter);
    icqWriteSysLog(icq,PROJECT,"ICQEVENT_ONLINE");
#endif
    icqSystemEvent(icq, ICQEVENT_ONLINE);

#ifdef DEBUG
    DBGTrace(cfg->watchDogCounter);
    icqWriteSysLog(icq,PROJECT,"ICQV8_STATUS_ONLINE");
#endif
    setStatus(icq, cfg, ICQV8_STATUS_ONLINE);

#ifdef DEBUG
    DBGTrace(cfg->watchDogCounter);
    icqWriteSysLog(icq,PROJECT,"Ready for status messages");
#endif
 }

 static void sendStatusMessage(HICQ icq, ICQV8 *cfg)
 {
#ifdef EXTENDED_LOG
    char  buffer[0x0100];
#endif
    HUSER usr;

    for(usr=icqQueryFirstUser(icq);usr;usr=icqQueryNextUser(icq,usr))
    {
       if(usr->c2sFlags & USRV8_SENDSTATUSMSG)
       {
#ifdef EXTENDED_LOG
          sprintf(buffer,"Sending status message to %s (ICQ#%ld)",icqQueryUserNick(usr),usr->uin);
          icqWriteSysLog(icq,PROJECT,buffer);
#endif
          icqSetServerBusy(icq,TRUE);
          icqv8_sendSystemMessage(icq, usr->uin, 1, 0x0200, "", 0);
          usr->c2sFlags &= ~USRV8_SENDSTATUSMSG;
          usr->c2sFlags |= USRV8_STATUSSENT;
          return;
       }
    }
    cfg->flags &= ~ICQV8_FLAG_SENDSTATUSMSG;
 }

