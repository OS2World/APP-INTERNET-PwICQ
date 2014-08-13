/*
 * msg.c - Receive message
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>

 #include "peer.h"

/*---[ Tables ]-----------------------------------------------------------------------------------------------*/

 #pragma pack(1)

 struct action
 {
    USHORT   cmd;
    int    (*exec)(HICQ,PEERSESSION *, int, PEER_MSG_HEADER *);
 };

 static int sendMsgAck(HICQ, PEERSESSION *, PEER_MSG_HEADER *);

 static int normalMessage(HICQ,PEERSESSION *, int, PEER_MSG_HEADER *);
 static int messageAck(HICQ,PEERSESSION *, int, PEER_MSG_HEADER *);
 static int messageCancel(HICQ,PEERSESSION *, int, PEER_MSG_HEADER *);

 static const struct action actionTable[] =
 {
    { 0x07ee, normalMessage     }, // 0x07ee - normal message.
    { 0x07da, messageAck        }, // 0x07da - acknowledge message.
    { 0x07d0, messageCancel     }, // 0x07d0 - cancel given message.
    { 0x0000, NULL              }
 };


/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 int icqv8_recvMsg(HICQ icq, PEERSESSION *cfg, int sz, unsigned char *pkt)
 {
    PEER_MSG_HEADER      *h = (PEER_MSG_HEADER *) pkt;
    const struct action  *action;

    DBGTracex(h->checkCode);

    if(icqv8_decript( pkt+1, sz-1))
    {
       icqWriteSysLog(icq,PROJECT,"Invalid peer message received");
       return 0;
    }

    icqDumpPacket(icq, icqQueryUserHandle(icq,cfg->uin), "Received message decripted", sz, pkt);

    DBGTracex(h->action);
    DBGTracex(h->status);

    for(action = actionTable;action->cmd;action++)
    {
       if(action->cmd == h->action && action->exec)
          return action->exec(icq,cfg,sz,h);
    }

    icqDumpPacket(icq, NULL, "Unknown action code in peer message", sz, (unsigned char *) pkt);
    return 0;
 }

 static int normalMessage(HICQ icq,PEERSESSION *cfg, int sz, PEER_MSG_HEADER *pkt)
 {
#ifdef EXTENDED_LOG
    char  logBuffer[0x0100];
#endif
    char  *txt = (char *) (pkt+1);
    ULONG uin  = 0;

    if(cfg)
       uin = cfg->uin;
    else
       icqWriteSysLog(icq,PROJECT,"Unexpected message without session");

    DBGTracex(pkt->msgType);
    DBGTracex(pkt->flags);
    DBGTrace(pkt->msgSize);

#ifdef EXTENDED_LOG
    sprintf(logBuffer,"Peer from %s (%ld) Type:%04x Flag: %04x Status: %04x Sequence: %04x",
    				icqQueryUserNick(icqQueryUserHandle(icq,cfg->uin)),
    				cfg->uin,
    				pkt->msgType,
    				pkt->flags,
    				pkt->status,
    				(int) pkt->sequence );
    icqWriteSysLog(icq,PROJECT,logBuffer);    				
#endif

    if( (sz- sizeof(PEER_MSG_HEADER)) < pkt->msgSize)
    {
       icqDumpPacket(icq, NULL, "Invalid size in message received", sz, (unsigned char *) pkt);
       return -1;
    }

    if(pkt->msgType == MSG_GREET)
    {
       icqv8_rcvGreet(icq,cfg,sz,pkt);
    }
    else
    {
       *(txt + pkt->msgSize) = 0;

       icqDumpPacket(icq, NULL, "Text block", pkt->msgSize, (unsigned char *) txt);

       DBGMessage(txt);
       if(icqInsertRTFMessage(icq, uin, cfg->seqMasc|((ULONG) pkt->sequence), pkt->msgType, 0, (pkt->flags & 0x0040)  ? MSGF_URGENT : 0, txt))
       {
          if(cfg && pkt->msgType != MSG_FILE)
             return sendMsgAck(icq,cfg,pkt);
       }
    }
    CHKPoint();

    return 0;
 }

 static int messageAck(HICQ icq, PEERSESSION *cfg, int sz, PEER_MSG_HEADER *pkt)
 {
    ULONG sequence = cfg->seqMasc | ((ULONG) pkt->sequence);
    int   port     = 0;
    char  *ptr;
    int   strSize;

    char  buffer[0x0100];

    sprintf(buffer,"%s (ICQ#%ld) confirmed message %08lx",
    			icqQueryUserNick(icqQueryUserHandle(icq,cfg->uin)),
    			cfg->uin,
    			sequence );

#ifdef EXTENDED_LOG
    icqDumpPacket(icq, NULL, buffer, sz, (unsigned char *) pkt);
#else
    icqWriteSysLog(icq,PROJECT,buffer);    			
#endif

    icqRemoveMessageByID(icq, sequence);
/*
02 bc fc f1 c1 da 07 0e 00 ff ff 00 00 00 00 00   .¼üñÁÚ...ÿÿ.....
00 00 00 00 00 00 00 1a 00 00 00 00 00 01 00 00   ................
29 00 f0 2d 12 d9 30 91 d3 11 8d d7 00 10 4b 06   ).ð-.Ù0‘Ó.×..K.
46 2e 00 00 04 00 00 00 46 69 6c 65 00 00 01 00   F.......File....
00 01 00 00 00 00 00 00 00 00 00 15 00 00 00 01   ................
00 00 00 78 08 fd 00 00 02 00 78 00 0e 00 00 00   ...x.ý....x.....
fd 08 00 00                                       ý...

ypedef struct peer_msg_header

  UCHAR  cmd;       // BYTE  02  CMD  The command: send a message.
                    // Note: all following data is encrypted as in the protocol v5 and v6.
  ULONG  checkCode; // DWORD.L xx xx xx xx CHECKCODE The checksum of this packet.
  USHORT action;    // WORD.L xx xx COMMAND The command like sending a message or getting auto-response strings.
                    // 0x07d0 = 2000 - cancel given message.
                    // 0x07da = 2010 - acknowledge message.
                    // 0x07ee = 2030 - normal message.

  USHORT x1;        // WORD.L xx xx UNKNOWN Unknown: 0xe = 14.
  USHORT sequence;  // WORD.L xx xx SEQUENCE Our sequence number.
  ULONG  x2;        // DWORD.L 00 00 00 00 UNKNOWN Unknown: empty.
  ULONG  x3;        // DWORD.L 00 00 00 00 UNKNOWN Unknown: empty.
  ULONG  x4;        // DWORD.L 00 00 00 00 UNKNOWN Unknown: empty.
  USHORT msgType;   // WORD.L xx xx MSGTYPE The message type.

  USHORT status;    // WORD.L xx xx xx xx STATUS The current status of the user,
                    // or whether the message was accepted or not.
                    // 0x00 - user is online, message was receipt, file transfer accepted
                    // 0x01 - refused
                    // 0x04 - auto-refused, because of away
                    // 0x09 - auto-refused, because of occupied
                    // 0x0a - auto-refused, because of dnd
                    // 0x0e - auto-refused, because of na
                    // Please note that the value 0x01 e.g. also means away.
  USHORT flags;     // WORD.L xx xx FLAGS
                    // 0x0010 =   16 - real message (whatever that is)
                    // 0x0020 =   32 - list (flag) (whatever that means)
                    // 0x0040 =   64 - urgent (flag)
  USHORT msgSize;
 PEER_MSG_HEADER;

29 00 f0 2d 12 d9 30 91 d3 11 8d d7 00 10 4b 06
46 2e 00 00 04 00 00 00 46 69 6c 65 00 00 01 00
00 01 00 00 00 00 00 00 00 00 00 15 00 00 00 01
00 00 00 78 08 fd 00 00 02 00 78 00 0e 00 00 00
fd 08 00 00

29 00 f0 2d 12 d9 30 91 d3 11 8d d7 00 10 4b 06   ).ð-.Ù0‘Ó.×..K.
46 2e 00 00 04 00 00 00 46 69 6c 65 00 00 01 00   F.......File....
00 01 00 00 00 00 00 00 00 00 00 15 00 00 00 01   ................
00 00 00 78 08 fd 00 00 02 00 78 00 0e 00 00 00   ...x.ý....x.....
fd 08 00 00                                       ý...

*/

    if(pkt->msgType == MSG_GREET)
    {
       /* Extrai o numero da porta que esta em listen */
       ptr      = ((char *) (pkt+1)) + pkt->msgSize + 20;
       sz      -= (sizeof(PEER_MSG_HEADER)+pkt->msgSize + 20);

       strSize  = ((int) *( (USHORT *) ptr))+19;
       ptr     += strSize;
       sz      -= strSize;

       DBGTrace(sz);
       DBGTrace( *((ULONG *) ptr) );

       if( (*((ULONG *) ptr) + 4) != sz)
       {
          icqDumpPacket(icq, NULL, "Unexpected size indicator in Peer Message ack", sz, (unsigned char *) ptr);
       }
       else
       {
          ptr += 4;
          sz  -= 4;

          strSize  = ((int) *( (USHORT *) ptr))+4;
          ptr     += strSize;
          sz      -= strSize;

//          icqDumpPacket(icq, NULL, "Extra", sz, (unsigned char *) ptr);

          if(sz > 2)
             port = SHORT_VALUE( *((USHORT *)ptr) );

       }
    }

    if(pkt->status)
       icqRequestRefused(icq, sequence, port, pkt->msgSize, (const char *) (pkt+1));
    else
       icqRequestAccepted(icq, sequence, port, pkt->msgSize, (const char *) (pkt+1));

    /*
     * Pendencia:
     *
     * Caso exista um texto (indicador de motivo), enviar para o skin manager de forma
     * a abrir um popup-dialog informando esse motivo para o usuario
     *
     */


    /* Elimino qualquer request relacionado a essa mensagem */
    icqDestroyRequest(icq, sequence, TRUE);

    return 0;
 }

 static int messageCancel(HICQ icq, PEERSESSION *cfg, int sz, PEER_MSG_HEADER *pkt)
 {
    ULONG sequence = cfg->seqMasc | ((ULONG) pkt->sequence);
    char buffer[0x0100];

    sprintf(buffer,"%s (ICQ#%ld) canceled message %08lx",
    			icqQueryUserNick(icqQueryUserHandle(icq,cfg->uin)),
    			cfg->uin,
    			sequence );

    icqDestroyRequest(icq, sequence, FALSE);

#ifdef EXTENDED_LOG
    icqDumpPacket(icq, NULL, buffer, sz, (unsigned char *) pkt);
#else
    icqWriteSysLog(icq,PROJECT,buffer);    			
#endif

    return 0;
 }

 static int sendMsgAck(HICQ icq, PEERSESSION *cfg, PEER_MSG_HEADER *from)
 {
    #pragma pack(1)
    struct _pkt
    {
       PEER_MSG_HEADER h;
       char            txt;
       ULONG           x1;
       char            x2[3];
    } pkt;

    memset(&pkt,0,sizeof(pkt));

    pkt.h.cmd		= 0x02;
    pkt.h.action	= 0x07da;
    pkt.h.x1		= 0x0e;
    pkt.h.sequence	= from->sequence;
    pkt.h.msgType	= from->msgType;
    pkt.h.msgSize	= 0x01;
    pkt.x2[0] =
    pkt.x2[1] =
    pkt.x2[2] = 0xFF;

    icqDumpPacket(icq, 0, "Peer Ack unencripted", sizeof(pkt), (unsigned char *) &pkt);

    icqv8_encript( ((UCHAR *) &pkt)+1, sizeof(pkt)-1);

    if(icqv8_sendBlock(cfg->sock, sizeof(pkt), &pkt))
       icqWriteNetworkErrorLog(icq, PROJECT, "Error sending message Ack");

    return 0;
 }

 int icqv8_msgInit(HICQ icq, PEERSESSION *cfg, int sz, unsigned char *pkt)
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
    } *h = (struct _header *) pkt;

    char buffer[0x0100];

    if(sz < sizeof(struct _header))
    {
       sprintf(buffer,"Invalid size %d in msg init, expected %d)",sz,sizeof(struct _header));
       icqWriteSysLog(icq,PROJECT,buffer);
       return -1;
    }

/*
    DBGTracex(h->x1);
    DBGTracex(h->x2);
    DBGTracex(h->x3);
    DBGTracex(h->x4);
    DBGTracex(h->x5);
    DBGTracex(h->x6);
    DBGTracex(h->x7);
    DBGTracex(h->x8);
*/

#ifdef EXTENDED_LOG
    sprintf(buffer,"%08lx init Packet: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx",
    	           (ULONG) cfg,h->x1,h->x2,h->x3,h->x4,h->x5,h->x6,h->x7,h->x8 );

    icqDumpPacket(icq,icqQueryUserHandle(icq,cfg->uin),buffer,sizeof(struct _header),(unsigned char *) h);

//    icqWriteSysLog(icq,PROJECT,buffer);
#endif

    if(h->x2 != 0x00000001)
    {
       sprintf(buffer,"Invalid msg init packet in session %08lx, aborting",(ULONG) cfg);
       icqWriteSysLog(icq,PROJECT,buffer);
       cfg->flags &= ~PEERFLAG_ACTIVE;
       return -1;
    }

    cfg->flags |= PEERFLAG_MESSAGE;

    if(!(cfg->flags & PEERFLAG_ORIGINATE))
       icqv8_sendMsgInit(icq, cfg, FALSE);

    DBGTrace(cfg->uin);

    if(icqSetPeerSession(icq,cfg->uin, (ULONG) cfg, &icqv8_peerDescriptor, cfg))
    {
       DBGTrace(cfg->uin);
       icqWriteSysLog(icq,PROJECT,"Failure registering peer session");
       cfg->flags &= ~PEERFLAG_ACTIVE;
       return -1;
    }

    sprintf(buffer,"Session %08lx is ready for messaging",(ULONG) cfg);
    icqWriteSysLog(icq,PROJECT,buffer);


    return 0;
 }



