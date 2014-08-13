/*
 * MESSAGE.C - Processamento e envio de mensagem
 */


#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <malloc.h>

 #include "icqv8.h"

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static USHORT advancedMsg(HICQ, ICQV8 *, USHORT, ULONG, const char *, ULONG);
 static USHORT sendAuthorization(HICQ, ICQV8 *, UCHAR, ULONG, const char *);
 static USHORT requestAuthorization(HICQ, ICQV8 *, ULONG, const char *);

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 USHORT _System icqv8_sendMessage(HICQ icq, ULONG uin, USHORT type, const char *txt, ULONG *msgID)
 {
    ICQV8   *cfg        = icqGetPluginDataBlock(icq, module);
    HUSER   usr         = icqQueryUserHandle(icq,uin);
    ULONG   id          = 0;
    USHORT  rc          = 0;
    char    delimiter   = 0xFE;
    char    *ptr;
    char    *workBuffer = NULL;

#ifdef EXTENDED_LOG
    char    extLog[0x0100];
    sprintf(extLog,"Sending message type %04x to ICQ#%ld",type,uin);
    icqWriteSysLog(icq,PROJECT,extLog);
#endif

    if(!txt)
       txt = "";

    DBGTrace(type);

    if(type == MSG_URL)
    {
       workBuffer = strdup(txt);
       if(workBuffer)
       {
          if(icqLoadValue(icq,"URLAsText",0))
          {
             type      = MSG_NORMAL;
             delimiter = '\n';
          }

          for(ptr = workBuffer;*ptr;ptr++)
          {
             if(*ptr == ICQ_FIELD_SEPARATOR)
                *ptr = delimiter;
          }
          txt = workBuffer;
       }
    }

    switch(type)
    {
    case MSG_REQUEST:
       rc = requestAuthorization(icq,cfg,uin,txt);
       if(usr)
       {
          usr->flags |= USRF_WAITING;
          icqv8_addUser(icq, usr);
       }
       break;

    case MSG_NORMAL:
    case MSG_URL:
       if(usr && usr->peerVersion >= 0x08 && icqLoadValue(icq,"newMsgFormat",1))
       {
#ifdef EXTENDED_LOG
          icqWriteSysLog(icq,PROJECT,"Using ICQV8 extended format");
#endif
          rc = icqv8_sendSystemMessage(icq, uin, type, type == MSG_NORMAL ? 0x0100 : 0x0400, txt, &id);
       }
       else
       {
#ifdef EXTENDED_LOG
          icqWriteSysLog(icq,PROJECT,"Using ICQV7 format");
#endif
          id = icqv8_queryCurrentID(icq);
          rc = advancedMsg(icq,cfg,type,uin,txt,id);
       }
       break;

    case MSG_REFUSED:
       rc = sendAuthorization(icq,cfg,0,uin,txt);
       break;

    case MSG_AUTHORIZED:
       rc = sendAuthorization(icq,cfg,1,uin,txt);
       break;

    default:
       icqWriteSysLog(icq,PROJECT,"Unexpected message type when sending");
       rc = 1;

    }

    if(workBuffer)
       free(workBuffer);

    return rc;

 }

 static USHORT sendAuthorization(HICQ icq, ICQV8 *cfg, UCHAR flag, ULONG uin, const char *txt)
 {
    PACKET pkt = icqv8_allocatePacket(icq,cfg,strlen(txt)+20);

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"Sending authorization response");
#endif

    icqv8_insertBUIN(pkt, 0, uin);
    icqv8_insertBlock(pkt, 0, 1, &flag);
    icqv8_insertShort(pkt, 0, strlen(txt));
    icqv8_insertString(pkt, 0, txt);
    icqv8_insertShort(pkt, 0, 0);

#ifdef LOGPACKETS
    icqv8_dumpPacket(pkt, icq, "Authorization response");
#endif

    return icqv8_sendSnac(icq, cfg, 0x13, 0x1A, 0, pkt);
 }

 static USHORT advancedMsg(HICQ icq, ICQV8 *cfg, USHORT type, ULONG uin, const char *txt, ULONG id)
 {
    PACKET pkt       = icqv8_allocatePacket(icq,cfg,strlen(txt)+50);
    int    szString  = strlen(txt)+1;

    #pragma pack(1)

    struct _header
    {
       ULONG    id1;
       ULONG    id2;
       USHORT   format;
    } header;

    struct _type1
    {
       USHORT   tlv2;
       USHORT   sz;

       USHORT   hdr_1281;
       USHORT   sz_1281;
       UCHAR    byte_1281;

       USHORT   header_257;
       USHORT   sz_257;
       ULONG    x1_257;

    } type1;

    struct _type4
    {
       USHORT   tlv5;
       USHORT   sz;
       ULONG    myUIN;
       UCHAR    type;
       UCHAR    flags;
       USHORT   msgSize;
    } type4;


    header.id1 = id;
    header.id2 = icqv8_querySequence(icq);

    switch(type)
    {
    case MSG_NORMAL:

       header.format = 0x0100;
       icqv8_insertBlock(pkt, 0, sizeof(struct _header), (const unsigned char *) &header);
       icqv8_insertBUIN(pkt, 0, uin);

/*
TLV(2) 00 02 xx xx ... MESSAGE1 The message to send; only present for type 1.
    TLV(1281) 05 01 00 01 01 UNKNOWN Unknown: 0x1 = 1.
    TLV(257) 01 01 xx xx ... MESSAGE This TLV encapsulates the message.
        DWORD 00 00 00 00 UNKNOWN Unknown: empty. vICQ uses 00 00 ff ff.
        STR ... MESSAGE Finally, the message to send.
*/


       type1.tlv2       = 0x0200;
       type1.sz         = SHORT_VALUE(szString+13);

       type1.hdr_1281   = 0x0105;
       type1.sz_1281    = 0x0100;
       type1.byte_1281  = 0x01;

       type1.header_257 = 0x0101;
       type1.sz_257     = SHORT_VALUE(szString+4);
       type1.x1_257     = 0;

       icqv8_insertBlock(pkt, 0, sizeof(struct _type1), (const unsigned char *) &type1);
       icqv8_insertBlock(pkt, 0, szString, txt);

       break;

    case MSG_URL:
       DBGMessage("Enviar URL");

       header.format = 0x0400;
       icqv8_insertBlock(pkt, 0, sizeof(struct _header), (const unsigned char *) &header);
       icqv8_insertBUIN(pkt, 0, uin);

/*
TLV(5) 00 05 xx xx ... MESSAGE4 The message to send for type 4.
    DWORD.L xx xx xx xx UIN My UIN.

    BYTE xx TYPE The message type as in the old protocol.
    BYTE xx FLAGS Unknown flags; possibly the message flags.
    LNTS xx xx ... MESSAGE Finally the message to send.
*/

       type4.tlv5       = 0x0500;
       type4.sz         = SHORT_VALUE(szString+8);
       type4.myUIN      = icqQueryUIN(icq);
       type4.type       = type;
       type4.flags      = 0;
       type4.msgSize    = szString;

       icqv8_insertBlock(pkt, 0, sizeof(struct _type4), (const unsigned char *) &type4);
       icqv8_insertBlock(pkt, 0, szString, txt);

//       icqv8_dumpPacket(pkt, icq, "Message packet prepared");
//       icqv8_releasePacket(icq, cfg, pkt);
//       return -1;
       break;

    default:
       icqWriteSysLog(icq,PROJECT,"Unexpected message type when sending");
       icqv8_releasePacket(icq, cfg, pkt);
       return -1;
    }

    /* TLV(6) 00 06 00 00 UNKNOWN Always present empty TLV. */
    icqv8_insertBlock(pkt, 6, 0, "");

//    icqv8_dumpPacket(pkt, icq, "Message packet prepared");

    return icqv8_sendSnac(icq, cfg, 0x04, 0x06, 0, pkt);
 }


/*

Send a message.
Parameters
Data type Content Name Description
8 xx xx xx xx xx xx xx xx MID The message ID.
WORD xx xx FORMAT The message format this message is in.
1 - text messages
2 - special messages
4 - url etc messages
BUIN xx .. UIN The UIN to sent the message to.

TLV(2) 00 02 xx xx ... MESSAGE1 The message to send; only present for type 1.
    TLV(1281) 05 01 00 01 01 UNKNOWN Unknown: 0x1 = 1.
    TLV(257) 01 01 xx xx ... MESSAGE This TLV encapsulates the message.
        DWORD 00 00 00 00 UNKNOWN Unknown: empty. vICQ uses 00 00 ff ff.
        STR ... MESSAGE Finally, the message to send.

TLV(5) 00 05 xx xx ... MESSAGE4 The message to send for type 4.
    DWORD.L xx xx xx xx UIN My UIN.

    BYTE xx TYPE The message type as in the old protocol.
    BYTE xx FLAGS Unknown flags; possibly the message flags.
    LNTS xx xx ... MESSAGE Finally the message to send.
TLV(5) 00 05 xx xx ... MESSAGE2 The message to send for type 2. See SRV_RECVMSG for details.
TLV(6) 00 06 00 00 UNKNOWN Always present empty TLV.
Examples

0000  2a 02 ?? ?? 00 73 00 04  00 06 ?? ?? ?? ?? ?? ??   *.O³..*. h6.]....
0010  00 00 00 00 00 b9 00 00  00 00 00 00 00 00 00 01   .....¹.. ........
0020  07 34 37 32 30 32 38 35  00 02 00 39 05 01 00 01   .4720285 ...9....
0030  01 01 01 00 30 00 00 00  00 48 6d 6d 6d 2e 2e 2e   ....0... .Hmmm...
0040  2e 20 73 74 69 6d 6d 74  2e 20 45 72 20 73 63 68   . stimmt . Er sch
0050  69 63 6b 74 20 65 69 6e  65 6e 20 53 4e 41 43 28   ickt ein en SNAC(
0060  34 2c 36 29 2e 00 06 00  00                        4,6).... .

*/


 static USHORT requestAuthorization(HICQ icq, ICQV8 *cfg, ULONG uin, const char *txt)
 {
    PACKET pkt = icqv8_allocatePacket(icq,cfg,strlen(txt)+20);

/*
BUIN  xx ..  UIN  The UIN of the user authorization is requested from.
BSTR xx xx ... MESSAGE Message sent to user in the authorization request.
WORD 00 00 UNKNOWN Unknown: empty.
*/

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"Sending authorization request");
#endif

    icqv8_insertBUIN(pkt, 0, uin);
    icqv8_insertShort(pkt, 0, strlen(txt));
    icqv8_insertString(pkt, 0, txt);
    icqv8_insertShort(pkt, 0, 0);

    return icqv8_sendSnac(icq, cfg, 0x13, 0x18, 0x18, pkt);
 }


