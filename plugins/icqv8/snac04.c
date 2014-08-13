/*
 * SNAC4.C
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>

 #include "icqv8.h"

/*---[ Some information ]-------------------------------------------------------------------------------------

Hello icq-devel,

I found new info about SNAC(04,06) and SNAC(04,07) channel 2 (type-2)
messages. They both contain TLV(05). And I found that format for this
TLV is depend on capability it contain:

-------------------------------------------------------------------------
00 05           // TLV 0x05
00 D0           // TLV size

00 00           // abort flag

7C 75 F9 29     // message cookie
02 49 00 00

xx xx xx xx     // capability - I think it describe
xx xx xx xx     // following data tlv chain
xx xx xx xx
xx xx xx xx

xx ..           // tlv_chain depends on capability
-------------------------------------------------------------------------

We all known TLV(05) format for {09461349-4C7F-11D1-8222-444553540000}
capability (it contain tlv(2711) in data chain), but I found that you
can send your own messages with your data - just replace default CLSID
by yours. AOL server on unknown CLSID just copy tlv chain from
SNAC(04,06) to SNAC(04,07) and send it to user. WARN: Server will
return error 0x09 if recipient doesn't have such capability.

-- With respect, Alexandr V. Shutko mailto:AVShutko@mail.khstu.ru

--------------------------------------------------------------------------------------------------------------*/


/*---[ Structs ]----------------------------------------------------------------------------------------------*/

 struct srvAckOfflineHeader
 {
    ULONG  time;        // WTIME  xx xx xx xx  TIME  Time as 1/5000th second number as in SNAC(4,6) you sent.
    ULONG  id;          // DWORD xx xx xx xx RANDOM The random token you gave in SNAC(4,6).
    USHORT msgType;     // WORD 00 02 TYPE The message type: 2.
    UCHAR  uinSz;       // BUIN xx ... UIN The UIN the message was sent to.
 };

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/


 static int _System srv_replyicbm(HICQ, ICQV8 *, int, ULONG, void *);
 static int _System srv_recvmsg1(HICQ, ICQV8 *, int, ULONG, const char *);
 static int _System srv_ackmsg(HICQ, ICQV8 *, int, ULONG, const char *);
 static int _System srv_icbmerr(HICQ, ICQV8 *, int, ULONG, const USHORT *);
 static int _System srv_ackOffline(HICQ, ICQV8 *, int, ULONG, const struct srvAckOfflineHeader *);

/*---[ SNAC processors ]--------------------------------------------------------------------------------------*/


 const SNAC_CMD icqv8_snac04_table[] =
 {
    { 0x01,   SNAC_CMD_PREFIX   srv_icbmerr           },
    { 0x05,   SNAC_CMD_PREFIX   srv_replyicbm         },
    { 0x07,   SNAC_CMD_PREFIX   srv_recvmsg1          },
    { 0x0b,   SNAC_CMD_PREFIX   srv_ackmsg            },
    { 0x0c,   SNAC_CMD_PREFIX   srv_ackOffline        },

    { 0x00,                     NULL                  }
 };

/*---[ Static functions ]-------------------------------------------------------------------------------------*/

 static int  messageType2(HICQ, ICQV8 *, ULONG, ULONG, int, const char *, char *);

 static void advancedMessage(HICQ, ICQV8 *, ULONG, int, const char *, char *);
 static void sendConfirmation(HICQ, ICQV8 *, ULONG, ULONG, ULONG, ULONG, USHORT, struct tlv2711 *, char *);

 static int  urlMessage(HICQ, ICQV8 *, ULONG, int, const char *, char *);

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 static int _System srv_replyicbm(HICQ icq, ICQV8 *cfg, int sz, ULONG req, void * ptr)
 {
    return 0;
 }

 static int _System srv_icbmerr(HICQ icq, ICQV8 *cfg, int sz, ULONG req, const USHORT *rc)
 {
    char 	buffer[0x0100];
    USHORT	err	= SHORT_VALUE(*rc);

    sprintf(buffer,"Error %d (%04x) when sending type-2 message (Request: %08lx)",err,err,req);

#ifdef EXTENDED_LOG
    icqDumpPacket(icq, NULL, buffer, sz, (unsigned char *) rc);
#else
    icqWriteSysLog(icq,PROJECT,buffer);
#endif
    return 0;
 }

 static int _System srv_recvmsg1(HICQ icq, ICQV8 *cfg, int sz, ULONG req, const char *ptr)
 {
    #pragma pack(1)
    struct _prefix
    {
       ULONG  msgTime;
       ULONG  msgID;
       USHORT msgFormat;
    }           *prefix;

    struct _header
    {
       USHORT warning;
       short  tlvs;
    } *header;

#ifdef EXTENDED_LOG
    char        logBuffer[0x0100];
#endif

    char        tempBuffer[80];
    ULONG       uin;
    TLV         tlv;
    HUSER       usr;
    int         rc      = 0;

//    icqDumpPacket(icq, NULL, "System type 1 message", sz, (unsigned char *) ptr);

    DBGTracex(req);

    prefix = (struct _prefix *) ptr;
    ptr += sizeof(struct _prefix);


    prefix->msgTime   = LONG_VALUE(prefix->msgTime);
    prefix->msgID     = LONG_VALUE(prefix->msgID);
    prefix->msgFormat = SHORT_VALUE(prefix->msgFormat);

    DBGTracex(prefix->msgTime);
    DBGTracex(prefix->msgID);
    DBGTracex(prefix->msgFormat);

    DBGTrace(*ptr);

    if(*ptr > 79)
    {
       icqWriteSysLog(icq,PROJECT,"ICQ# is too large for the supplied buffer");
       icqAbend(icq,0);
    }

    strncpy(tempBuffer,ptr+1,*ptr);
    *(tempBuffer+*ptr) = 0;

    DBGMessage(tempBuffer);

    uin = atoi(tempBuffer);
    usr = icqQueryUserHandle(icq,uin);

    DBGTrace(uin);

    sz  -= (*ptr)+1;
    ptr += (*ptr)+1;
    sz  -= sizeof(struct _prefix);

    header = (struct _header *) ptr;

    ptr += sizeof(struct _header);
    sz  -= sizeof(struct _header);

    header->warning = SHORT_VALUE(header->warning);
    header->tlvs    = SHORT_VALUE(header->tlvs);

#ifdef EXTENDED_LOG

    sprintf(tempBuffer,"MT:%08lx MI:%08lx MF:%04x TLVS:%04x", prefix->msgTime, prefix->msgID, (int) prefix->msgFormat,(int) header->tlvs);
    DBGMessage(tempBuffer);
    icqWriteSysLog(icq,PROJECT,tempBuffer);

    strcpy(logBuffer,"TLVs: ");

#endif

    while( header->tlvs-- >= 0 && (ptr = icqv8_getTLV(icq, cfg, &tlv, &sz, ptr)) != NULL)
    {
#ifdef EXTENDED_LOG
       if(strlen(logBuffer) < 0xF0)
          sprintf(logBuffer+strlen(logBuffer),"%04x ",tlv.id);
#endif
       DBGTracex(tlv.id);
       switch(tlv.id)
       {
       case 0x01 : // TLV(1) 00 01 00 02 00 50 UNKNOWN Unknown: 0x50 = 80.
          break;

       case 0x02:
          if(tlv.sz > 4)
             advancedMessage(icq,cfg,uin,tlv.sz,tlv.data, tempBuffer);
          else
             icqDumpPacket(icq,usr,"Unexpected size in TLV2 Message T1",tlv.sz,(unsigned char *) tlv.data);
          break;

       case 0x04 : // TLV(4) 00 04 00 02 00 00 UNKNOWN Unknown: 0.
          break;

       case 0x05:

          switch(prefix->msgFormat)
          {
          case 0x04:
             rc = urlMessage(icq,cfg,uin,tlv.sz,tlv.data,tempBuffer);
             break;

          case 0x02:
             rc = messageType2(icq, cfg, req, uin, tlv.sz, tlv.data, tempBuffer);
             break;

          default:
             sprintf(tempBuffer,"Unexpected TLV5 msg format 0x%04x (%ld) [%ld]",prefix->msgFormat,(ULONG) prefix->msgFormat,(ULONG) __LINE__);
             icqDumpPacket(icq,icqQueryUserHandle(icq,uin),tempBuffer,tlv.sz,(unsigned char *) tlv.data);
             rc = -1;
          }
          break;

       case 0x06 : // TLV(6) 00 06 00 04 xx xx xx xx STATUS The sender's online status.
          usr = icqSetUserOnlineMode(icq, uin, LONG_VALUE( *((ULONG *) tlv.data)));
          break;

       case 0x0f : // TLV(15) 00 0f 00 04 TIME TIME A timestamp.
          break;

       case 0x03 : // TLV(3) 00 03 00 04 TIME TIME The online since time.
          if(usr)
             usr->onlineSince = LONG_VALUE( *((ULONG *) tlv.data));
          break;

       default:
          sprintf(tempBuffer,"Unexpected TLV %d in Message T1",tlv.id);
          icqDumpPacket(icq,usr,tempBuffer,tlv.sz,(unsigned char *) tlv.data);
       }
    }

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,logBuffer);
    DBGMessage(logBuffer);
#endif

    if(sz > 0)
       icqDumpPacket(icq, NULL, "Extra bytes in advanced message", sz, (unsigned char *) ptr);

    return rc;
 }

 static void advancedMessage(HICQ icq, ICQV8 *cfg, ULONG uin, int sz, const char *ptr, char *tempBuffer)
 {
    TLV                 tlv;
    unsigned char       *txt;

    DBGMessage("Processando advanced-message");

    while( (ptr = icqv8_getTLV(icq, cfg, &tlv, &sz, ptr)) != NULL)
    {
       DBGTracex(tlv.id);

       switch(tlv.id)
       {
       case 1281:       //    TLV(1281) 05 01 xx xx ... UNKNOWN Unknown.
          break;

       case 257:        //    TLV(257) 01 01 xx xx ... MESSAGE This TLV contains the message.
                        // DWORD xx xx xx xx UNKNOWN Unknown.
                        // DATA ... MESSAGE The actual message. Not null terminated.
          DBGTracex( *( (ULONG *) tlv.data ) );
          txt       = tlv.data;
          txt      += 4;
          tlv.sz   -= 4;
          DBGTrace(tlv.sz);
          DBGMessage(txt);
          icqInsertMessage(icq, uin, 0, MSG_NORMAL, 0, 0, tlv.sz, txt);
          CHKPoint();
          break;

       default:
          icqv8_dumptlv(icq,"Unexpected TLV %d received in advanced message from ICQ#%lu (A)",uin,&tlv);

 //         sprintf(tempBuffer,"Unexpected TLV %d in advanced message",tlv.id);
 //         icqDumpPacket(icq,icqQueryUserHandle(icq,uin),tempBuffer,tlv.sz,(unsigned char *) tlv.data);
       }
    }

    DBGMessage("Processamento de advanced-message terminado");
 }


 static int urlMessage(HICQ icq, ICQV8 *cfg, ULONG uin, int sz, const char *pkt, char *tempBuffer)
 {
/*
        12 ab 9f 01
        04 00
        14 00

                    54 65 73 74 65 20 64 65          .«Ÿ.....Teste de
        20 55 52 4c 2e fe 54 65 73 74 65 00                URL.þTeste.

*/

    #pragma pack(1)

    struct _header
    {
       ULONG    uin;
       USHORT   type;
       USHORT   sz;
    } * header = (struct _header *) pkt;

    DBGTrace(header->uin);
    DBGTrace(header->type);
    DBGTrace(header->sz);

    icqInsertMessage(icq,header->uin,0,header->type,0,0,header->sz,(char *)(header+1));

    return 0;
 }

 static int messageType2(HICQ icq, ICQV8 *cfg, ULONG req, ULONG uin, int sz, const char *pkt, char *tempBuffer)
 {
    #pragma pack(1)

    struct _header
    {
       USHORT   ackType;        // ACKTYPE  0x0000 - normal message
                                // 0x0001 - abort request
                                // 0x0002 - file ack

       ULONG    msgTime;        // TIME  A copy of the very first time in this packet.
       ULONG    msgID;          // ID  The random message ID again.

       char     caps[16];       // CAPABILITY  One of the capabilities sent in CLI_SETUSERINFO:
                                // 09 46 13 49 4C 7F 11 D1 82 22 44 45 53 54 00 00

    } * header = (struct _header *) pkt;

#ifdef EXTENDED_LOG
    char	buffer[0x0100];
#endif

    TLV         tlv;
    HUSER       usr;
    ULONG       onlineSince     = 0;
    ULONG       onlineMode      = ICQ_OFFLINE;
//    int         tlv2            = 0;

    pkt += sizeof(struct _header);
    sz  -= sizeof(struct _header);


    DBGMessage("---------------------------------- Mensagem Recebida -------------------------------");
    DBGTracex(header->ackType);
    DBGTracex(header->msgTime);
    DBGTracex(header->msgID);

    while( (pkt = icqv8_getTLV(icq, cfg, &tlv, &sz, pkt)) != NULL)
    {
       DBGTrace(tlv.id);

       switch(tlv.id)
       {
       case 0x01: // TLV(1)  00 01 00 02 00 50  UNKNOWN  Unknown: 0x50 = 80.
       case 0x04: // TLV(4)  00 04 00 02 00 00  UNKNOWN  Unknown: 0.
       case 0x0f: // TLV(15)  00 0f 00 04 TIME  TIME  A timestamp.
          break;

       case 0x0a: // TLV(10)  00 0a 00 02 xx xx  ACKTYPE2
                  // 0x0001 - normal message
                  // 0x0002 - file ack or file ok
          break;

       case 0x06: // TLV(6)  00 06 00 04 xx xx xx xx  STATUS  The sender's online status.
	  onlineMode = LONG_VALUE(*( (ULONG *) tlv.data));
#ifdef EXTENDED_LOG
          sprintf(buffer,"ICQ#%ld (%s) changed to online status %08lx by message",uin,icqQueryUserNick(icqQueryUserHandle(icq,uin)),onlineMode);
          icqWriteSysLog(icq,PROJECT,buffer);
#endif
          break;

       case 0x02: // TLV(2)  00 02 00 04 TIME  TIME  The member since time.
//          tlv2++;
          break;

       case 0x2711: // TLV(10001)  27 11 xx xx ...  MESSAGE
                    // This is yet another message TLV that adds yet another layer of complexity

          if(!icqv8_procTLV2711(icq, uin, tlv.sz, (struct tlv2711 *) tlv.data, tempBuffer))
             sendConfirmation(icq, cfg, req, uin, header->msgTime, header->msgID, 0x02, (struct tlv2711 *) tlv.data, tempBuffer);
          break;

       case 0x03: // TLV(3)  00 03 00 04 TIME  TIME  The online since time.
          onlineSince = LONG_VALUE( * ((ULONG *) tlv.data) );
          break;

       default:
          icqv8_dumptlv(icq,"Unexpected TLV %d received in T2 message from ICQ#%lu",uin,&tlv);
       }


    }

    DBGTrace(sz);

    usr = icqQueryUserHandle(icq,uin);

    if(usr)
    {
       if(onlineMode != ICQ_OFFLINE)
          icqSetUserOnlineMode(icq, uin, onlineMode);
       if(onlineSince)
          usr->onlineSince = onlineSince;
    }

    return 0;
 }


 static void sendConfirmation(HICQ icq, ICQV8 *cfg, ULONG req, ULONG uin, ULONG msgTime, ULONG msgID, USHORT type, struct tlv2711 *hdr, char *buffer)
 {
    /* Envia confirmacao de mensagem recebida */

/*

Data type  Content  Name  Description
DWORD xx xx xx xx TIME The time part of the message ID.
DWORD xx xx xx xx MID The random part of the message ID.
WORD.B 00 02 TYPE The message type - a type-2 message acknowledge.

BUIN xx ... UIN The UIN the message was sent to.

WORD.B 00 03 UNKNOWN Unknown: 0x03 = 3. No idea.

WORD.L 1b 00 LEN The length of the following sub chunk:
    WORD.L xx xx TCPVER The version for direct connections used.
    CAP xx xx ... xx CAPABILITY The capability send with the message.
    WORD.L xx xx UNKNOWN Unknown.
    DWORD.L xx xx xx xx UNKNOWN Unknown.
    BYTE xx UNKNOWN Unknown.
    WORD.L xx xx SEQ The sequence of the acknowledged packet.
WORD.L 0e 00 UNKNOWN Unkown: 0x0e = 14. No idea.
WORD.L xx xx SEQ2 The sequence number of the packet.
12 00 ... 00 UNKNOWN Unknown: always zero.
WORD.L xx xx MSGTYPE The message type and its flags.
WORD.L xx xx STATUS The response status; same as for peer to peer acknowledges.
WORD.L xx xx PRIORITY Unknown.
LNTS xx xx ... TEXT The text with this acknowledge. Non-empty for away messages.
DATA ... DATA More data depending on the MSGTYPE.
VOID

*/

    #pragma pack(1)
    struct _header
    {
       ULONG    msgTime;        // DWORD xx xx xx xx TIME The time part of the message ID.
       ULONG    msgID;          // DWORD xx xx xx xx MID The random part of the message ID.
       USHORT   msgType;        // WORD.B 00 02 TYPE The message type - a type-2 message acknowledge.
    } *h        = (struct _header *) buffer;

    struct _midPacket
    {
       USHORT   x1;             // WORD.B 00 03 UNKNOWN Unknown: 0x03 = 3. No idea.
       USHORT   len;            // WORD.L 1b 00 LEN The length of the following sub chunk:
       USHORT   tcpVer;         // WORD.L xx xx TCPVER The version for direct connections used.
       UCHAR    caps[16];       // CAP xx xx ... xx CAPABILITY The capability send with the message.
       USHORT   x2;             // WORD.L xx xx UNKNOWN Unknown.
       ULONG    x3;             // DWORD.L xx xx xx xx UNKNOWN Unknown.
       UCHAR    x4;             // BYTE xx UNKNOWN Unknown.
       USHORT   ackSeq;         // WORD.L xx xx SEQ The sequence of the acknowledged packet.
       USHORT   x5;             // WORD.L 0e 00 UNKNOWN Unkown: 0x0e = 14. No idea.
       USHORT   Sequence;       // WORD.L xx xx SEQ2 The sequence number of the packet.
       UCHAR    x6[12];         // 12 00 ... 00 UNKNOWN Unknown: always zero.
       UCHAR    subType;        // WORD.L xx xx MSGTYPE The message type and its flags.
       UCHAR    flags;
       USHORT   status;         // WORD.L xx xx STATUS The response status; same as for peer to peer acknowledges.
       USHORT   priority;       // WORD.L xx xx PRIORITY Unknown.
    } *m        = (struct _midPacket *) buffer;

    ULONG       temp;
    PACKET      pkt             = icqv8_allocatePacket(icq, cfg, sizeof(struct _header) + sizeof(struct _midPacket) + 0x0100);

    memset(h,0,sizeof(struct _header));

    h->msgTime  = msgTime;
    h->msgID    = msgID;
    h->msgType  = SHORT_VALUE(type);

    DBGTracex(h->msgTime);
    DBGTracex(h->msgID);
    DBGTracex(h->msgType);

    icqv8_insertBlock(pkt, 0, sizeof(struct _header), (const unsigned char *) h);
    icqv8_insertBUIN(pkt, 0, uin);

    memset(m,0,sizeof(struct _midPacket));

    m->x1       = 0x0300;
    m->len      = 0x001b;
    m->tcpVer   = 0x0008;
    m->x2       = 0x0000;
    m->x3       = 0x00000003;
    m->x4       = 0;

    m->Sequence =
    m->ackSeq   = hdr->seq1;

    m->x5       = 0x000e;

    m->subType  = hdr->msgType;
    m->flags    = hdr->flags;
    m->status   = 0x0004;
    m->priority = 0;

    icqv8_insertBlock(pkt, 0, sizeof(struct _midPacket), (const unsigned char *) m);

    icqv8_insertLNTS(pkt, icqQueryAwayMessage(icq));

    temp = 0;
    icqv8_insertBlock(pkt, 0, sizeof(temp), (const unsigned char *) &temp);

    temp = LONG_VALUE(0xffffff00);
    icqv8_insertBlock(pkt, 0, sizeof(temp), (const unsigned char *) &temp);

    DBGTrace(sizeof(struct _header));
    DBGTrace(sizeof(struct _midPacket));

//#ifdef EXTENDED_LOG
//    icqv8_dumpPacket(pkt, icq, "Sending confirmation");
//#endif

    DBGTracex(req);
    icqv8_sendSnac(icq, cfg, 0x04, 0x0B, 0x04000000, pkt);

 }

/*

00:42:46 Eingehendes Server-Paket (Version 8):

  2a 02 e7 a1 00 7b               FLAP  ch 2 seq 0000e7a1 length 007b
  00 04 00 0b 00 00 8a e9  ef 62  SNAC     (4,b) [SRV/CLI_ACKMSG] flags 0 ref 8ae9ef62

  00 00 32 ff                     DWORD.B  0x000032ff = 0000013055
  00 00 3e c2                     DWORD.B  0x00003ec2 = 0000016066
  00 02                           WORD.L   0x0200 = 00512
  08 33 31 32 37 33 31 36  32     UIN      31273162
  00 03                           WORD.B   0x0003 = 00003
  1b 00                           DWORD.L  "(WCWDbW)WWDDDWWWLDD"
    08 00                           WORD.B   0x0800 = 02048
    00 00 00 00 00 00 00 00
    00 00 00 00 00 00 00 00         CAP_NONE
    00 00                           WORD.B   0x0000 = 00000
    03 00 00 00                     DWORD.B  0x03000000 = 0050331648
    00                              BYTE     0x00 = 000
    ff ff                           WORD.B   0xffff = 65535
  0e 00                           WORD.B   0x0e00 = 03584
  ff ff                           WORD.B   0xffff = 65535
  00 00 00 00                     DWORD.B  0x00000000 = 0000000000
  00 00 00 00                     DWORD.B  0x00000000 = 0000000000
  00 00 00 00                     DWORD.B  0x00000000 = 0000000000
  01 00                           WORD.B   0x0100 = 00256
  04 00                           WORD.B   0x0400 = 01024
  00 00                           WORD.B   0x0000 = 00000
  1f 00 41 62 77 65 73 65  6e 64 21 0d 0a 2a 45 78\
  74 72 61 20 66 fc 72 20  52 fc 64 69 67 65 72 2a\
  00                              LNTS     'Abwesend!
                                            *Extra für Rüdiger*'
  00 00 00 00                     DWORD.B  0x00000000 = 0000000000
  ff ff ff 00                     DWORD.B  0xffffff00 = 4294967040



        ac b6 e6 05
        2f 5e 00 00

        00 02

        08
        37 30 34 32
        31 33 37 38

        00 03
        1b 00
        08 00
        00 00 00 00 00 00 00 00
        00 00 00 00 00 00 00 00
        00 00
        03 00 00 00
        00
        ef ff
        0e 00
        fd ff
        00 00 00 00
        00 00 00 00
        00 00 00 00
        01 00
        04 00
        00 00
        01 00
        00
        00 00 00 00
        ff ff ff 00


DWORD xx xx xx xx TIME The time part of the message ID.
DWORD xx xx xx xx MID The random part of the message ID.
WORD.B 00 02 TYPE The message type - a type-2 message acknowledge.

BUIN xx ... UIN The UIN the message was sent to.

WORD.B 00 03 UNKNOWN Unknown: 0x03 = 3. No idea.

WORD.L 1b 00 LEN The length of the following sub chunk:
    WORD.L xx xx TCPVER The version for direct connections used.
    CAP xx xx ... xx CAPABILITY The capability send with the message.
    WORD.L xx xx UNKNOWN Unknown.
    DWORD.L xx xx xx xx UNKNOWN Unknown.
    BYTE xx UNKNOWN Unknown.
    WORD.L xx xx SEQ The sequence of the acknowledged packet.
WORD.L 0e 00 UNKNOWN Unkown: 0x0e = 14. No idea.
WORD.L xx xx SEQ2 The sequence number of the packet.
12 00 ... 00 UNKNOWN Unknown: always zero.
WORD.L xx xx MSGTYPE The message type and its flags.
WORD.L xx xx STATUS The response status; same as for peer to peer acknowledges.
WORD.L xx xx PRIORITY Unknown.
LNTS xx xx ... TEXT The text with this acknowledge. Non-empty for away messages.
DATA ... DATA More data depending on the MSGTYPE.
VOID

*/

 static int _System srv_ackmsg(HICQ icq, ICQV8 *cfg, int sz, ULONG req, const char *pkt)
 {
    char         buffer[0x0100];
    HUSER       usr;

    #pragma pack(1)

    struct header
    {
       ULONG     msgTime;        // DWORD  xx xx xx xx  TIME  The time part of the message ID.
       ULONG     msgID;          // DWORD xx xx xx xx MID The random part of the message ID.
       USHORT    msgType;        // WORD.B 00 02 TYPE The message type - a type-2 message acknowledge.
       UCHAR     szUIN;
    }                            *header         = (struct header *) pkt;

    struct midPacket
    {
       USHORT    x1;             // WORD.B 00 03 UNKNOWN Unknown: 0x03 = 3. No idea.
       USHORT    size;           // WORD.L 1b 00 LEN The length of the following sub chunk:
       USHORT    tcpVer;         // WORD.L xx xx TCPVER The version for direct connections used.
       UCHAR     caps[16];       // CAP xx xx ... xx CAPABILITY The capability send with the message.
       USHORT    x2;             // WORD.L xx xx UNKNOWN Unknown.
       ULONG     x3;             // DWORD.L xx xx xx xx UNKNOWN Unknown.
       UCHAR     x4;             // BYTE xx UNKNOWN Unknown.
       USHORT    ackSeq;         // WORD.L xx xx SEQ The sequence of the acknowledged packet.
       USHORT    x5;             // WORD.L 0e 00 UNKNOWN Unkown: 0x0e = 14. No idea.
       USHORT    Sequence;       // WORD.L xx xx SEQ2 The sequence number of the packet.
       UCHAR     x6[12];         // 12 00 ... 00 UNKNOWN Unknown: always zero.
       USHORT    msgType;        // WORD.L xx xx MSGTYPE The message type and its flags.
       USHORT    status;         // WORD.L xx xx STATUS The response status; same as for peer to peer acknowledges.
       USHORT    priority;       // WORD.L xx xx PRIORITY Unknown.
    }                    *mid;

    ULONG                uin;

    CHKPoint();

    strncpy(buffer,(char *)(header+1),header->szUIN);
    *(buffer+header->szUIN) = 0;

    DBGMessage(buffer);
    uin = atoi(buffer);

    mid = (struct midPacket *)  ( ((char *) (header+1)) + header->szUIN );

    usr = icqQueryUserHandle(icq,uin);

    if(usr)
       usr->peerVersion = mid->tcpVer;

    DBGTracex(header->msgTime);
    DBGTracex(header->msgID);
    DBGTracex(header->msgType);
    DBGTracex(mid->x1);
    DBGTracex(mid->size);
    DBGTracex(mid->tcpVer);

#ifdef EXTENDED_LOG
    sprintf(buffer,"Message %08lx:%08lx:%04x was confirmed",header->msgTime,header->msgID,mid->Sequence);
    icqWriteSysLog(icq,PROJECT,buffer);
#endif

    icqProcessServerConfirmation(icq, uin, header->msgID);
    return 0;
 }


/*

11/16/2002 00:32:43 icqkrnl    Unknown SNAC packet (0x0004/0x000b - 4/11) (93 bytes)
04 00 0b 00 00 00 84 21 64 7b 7a d8 d5 3d 00 00   ......„!d{zØÕ=..
00 00 00 02 08 32 37 32 34 31 32 33 34 00 03 1b   .....27241234...
00 08 00 10 cf 40 d1 4f e9 d3 11 bc d2 00 04 ac   ....Ï@ÑOéÓ.¼Ò..¬
96 dd 96 00 00 03 00 00 00 00 ff ff 12 00 ff ff   –Ý–.......ÿÿ..ÿÿ
00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 d5   ...............Õ
3d dc 01 00 00 00 00 00 00 ff ff ff ff            =Ü.......ÿÿÿÿ

        04 00
        0b 00
        00 00
        84 21 64 7b

        7a d8 d5 3d
        00 00 00 00

        00 02

        08
        32 37 32 34
        31 32 33 34

        00 03           Unknown
        1b 00           Size of the next block
        08 00           TCP Protocol

        10 cf 40 d1     capability
        4f e9 d3 11
        bc d2 00 04
        ac 96 dd 96

        00 00           Unknown
        03 00 00 00     Unknown

        00              Unknown
        ff ff           Sequence
        12 00           No idea why it's 12 00 should be 0x0e
        ff ff           Sequence (Again)

        00 00 00 00     12 zeros
        00 00 00 00
        00 00 00 00

        01 00           Message type
        00 d5           Receiver status
        3d dc           Priority

        01 00           No Text
        00

        00 00 00 00
        ff ff ff ff

11/20/2002 22:16:25 icqkrnl    Unknown SNAC packet (0x0004/0x000c - 4/12) (28 bytes)
04 00 0c 00 00 00 00 00 00 00 08 50 dc 3d 00 00   ...........PÜ=..
00 00 00 04 07 35 34 39 36 37 31 32               .....5496712

*/

 static int _System srv_ackOffline(HICQ icq, ICQV8 *cfg, int sz, ULONG req, const struct srvAckOfflineHeader *h)
 {
    char   buffer[0x0100];
    ULONG  uin                  = atoi((char *)(h+1));

    sprintf(buffer,"MSG #%08lx (%d) to ICQ#%ld confirmed by server",h->id,(int) h->msgType,uin);
    icqWriteSysLog(icq,PROJECT,buffer);

    icqProcessServerConfirmation(icq, uin, h->id);

    return 0;
 }

