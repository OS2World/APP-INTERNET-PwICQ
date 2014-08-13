/*
 * SNAC13.C
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>
 #include <ctype.h>

 #include "icqv8.h"

/*---[ Defines ]----------------------------------------------------------------------------------------------*/


/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static int _System srv_replyunknown(HICQ, ICQV8 *, int, ULONG, void *);
 static int _System srv_replyroster(HICQ, ICQV8 *, int, ULONG, UCHAR *);
 static int _System srv_replyrosterok(HICQ, ICQV8 *, int, ULONG, UCHAR *);
 static int _System srv_addedyou(HICQ, ICQV8 *, int, ULONG, UCHAR *);
 static int _System srv_authorize(HICQ, ICQV8 *, int, ULONG, UCHAR *);
 static int _System srv_authorized(HICQ, ICQV8 *, int, ULONG, UCHAR *);
 static int _System srv_updateAck(HICQ, ICQV8 *, int, ULONG, UCHAR *);
 static int _System srv_unavailable(HICQ, ICQV8 *, int, ULONG, UCHAR *);


/*---[ SNAC processors ]--------------------------------------------------------------------------------------*/

 const SNAC_CMD icqv8_snac13_table[] =
 {
    { 0x01,   SNAC_CMD_PREFIX   srv_unavailable       },
    { 0x03,   SNAC_CMD_PREFIX   srv_replyunknown      },
    { 0x06,   SNAC_CMD_PREFIX   srv_replyroster       },
    { 0x0E,   SNAC_CMD_PREFIX   srv_updateAck         },
    { 0x0F,   SNAC_CMD_PREFIX   srv_replyrosterok     },
    { 0x1B,   SNAC_CMD_PREFIX   srv_authorized        },
    { 0x1C,   SNAC_CMD_PREFIX   srv_addedyou          },
    { 0x19,   SNAC_CMD_PREFIX   srv_authorize         },

    { 0x00,                     NULL                  }
 };


/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 static int _System srv_replyunknown(HICQ icq, ICQV8 *cfg, int sz, ULONG req, void * ptr)
 {
    return 0;
 }

 static int _System srv_replyroster(HICQ icq, ICQV8 *cfg, int sz, ULONG req, UCHAR *pkt)
 {
    icqv8_downloadUserList(icq,cfg,sz,pkt);
    icqv8_completeLogin(icq,cfg);
    return 0;
 }

 static int _System srv_replyrosterok(HICQ icq, ICQV8 *cfg, int sz, ULONG req, UCHAR *pkt)
 {
    ULONG  lastUpdate   = LONG_VALUE( *((ULONG *)pkt) );
//    USHORT clSize       = SHORT_VALUE( *((USHORT *)(pkt+4)) );

    DBGTracex(lastUpdate);
//    DBGTrace(clSize);

    icqWriteSysLog(icq,PROJECT,"Contact-list confirmed by server");
//    icqSaveValue(icq, "v8.modified", lastUpdate);
    icqSaveValue(icq, "SSUL.Update", LONG_VALUE( *( (ULONG *) pkt) ));

    icqv8_completeLogin(icq,cfg);
    return 0;
 }


 static int _System srv_addedyou(HICQ icq, ICQV8 *cfg, int sz, ULONG req, UCHAR * ptr)
 {

#ifdef EXTENDED_LOG
    icqDumpPacket(icq, NULL, "You were added message received", sz, (unsigned char *) ptr);
#endif

/*
SRV_ADDEDYOU
Channel: 2 SNAC(13,1C)
  This SNAC informs the client that some other client has added the local client's UIN to it's contact list
Parameters
Length/TLV Content (If Fixed) Designations Description
2 xx xx BYTELEN Guess: A count of the following bytes.
TLV(01) 00 01 00 02 xx xx UNKNOWN Guess: Unknown, seen in several SNACs.
1 xx STRLEN The length of the following UIN in ASCII format.
Varies xx .. UIN The user's UIN in ASCII format.
*/

    return 0;
 }

 static int _System srv_authorize(HICQ icq, ICQV8 *cfg, int sz, ULONG req, UCHAR * ptr)
 {
    #pragma pack(1)

    char        buffer[0x0100];
    ULONG       uin;

    struct _header
    {
       ULONG x1;        // 8 00 06 00 01
       ULONG x2;        //   00 02 00 02 UNKNOWN Unknown. Seen in several buddy list related packets from server
       UCHAR szNick;    // 1 xx STRLEN The length of the following UIN in ASCII format
    } *header   = (struct _header *) ptr;

#ifdef EXTENDED_LOG
    icqDumpPacket(icq, NULL, "Authorization request received", sz, (unsigned char *) ptr);
#endif

    ptr += sizeof(struct _header);


//    DBGMessage(ptr);

    strncpy(buffer,ptr,header->szNick);
    *(buffer+header->szNick) = 0;

//    DBGMessage(buffer);

    ptr += (header->szNick+2);

    sz -= (sizeof(struct _header) + header->szNick + 5);

//    DBGTrace(sz);
//    DBGTrace(strlen(ptr));

    DBGTracex(header->x1);
    DBGTracex(header->x2);
    DBGMessage(ptr);

    uin = atol(buffer);

    icqAddUserInCache(icq, uin);
    icqInsertMessage(icq, uin, 0, MSG_REQUEST, 0, 0, sz, ptr);

    return 0;
 }

/*

Length/TLV  Content (If Fixed)  Designations  Description

8 00 06 00 01 00 02 00 02 UNKNOWN Unknown. Seen in several buddy list related packets from server
1 xx STRLEN The length of the following UIN in ASCII format
Varies xx UIN The UIN of the user requesting authorization in ASCII format
2 xx xx STRLEN The length of the following reason in ASCII format.
Varies xx .. REASON The reason given by the user for requesting authorization.

20/08/2002 13:55:12 icqkrnl    Unknown SNAC packet (0x0013/0x0019 - 19/25) (112 bytes)

13 00 19 00 80 00 91 d8 29 fd 00 06 00 01 00 02   ....€.‘Ø)ý......
00 02 08 32 36 30 37 35 34 32 39 00 51 50 6c 65   ...26075429.QPle
61 73 65 20 61 75 74 68 6f 72 69 7a 65 20 6d 79   ase authorize my
20 72 65 71 75 65 73 74 20 61 6e 64 20 61 64 64    request and add
20 6d 65 20 74 6f 20 79 6f 75 72 20 43 6f 6e 74    me to your Cont
61 63 74 20 4c 69 73 74 2e 20 4d 61 72 63 65 6c   act List. Marcel
6f 20 49 43 51 23 32 36 30 37 35 34 32 39 00 00   o ICQ#26075429..

*/

 static int _System srv_authorized(HICQ icq, ICQV8 *cfg, int sz, ULONG req, UCHAR * ptr)
 {
    char        buffer[0x0100];
    ULONG       uin;
    USHORT      msgSize;
    UCHAR       status;

    #pragma pack(1)

    struct _header
    {

       USHORT   x1;             // WORD 00 06 UNKNOWN Unknown: 6.
       USHORT   tlv1;           // TLV(1) 00 01 00 02 xx xx UNKNOWN Unknown.
       USHORT   tlv1_sz;
       USHORT   tlv1_data;
       UCHAR    szNick;         // BUIN xx ... UIN The UIN that granted authorization.
    } *header = (struct _header *) ptr;


#ifdef EXTENDED_LOG
    icqDumpPacket(icq, NULL, "Authorization response received", sz, (unsigned char *) ptr);
#endif

    ptr += sizeof(struct _header);

    DBGMessage(ptr);

    strncpy(buffer,ptr,header->szNick);
    *(buffer+header->szNick) = 0;

    DBGTrace(header->szNick);
    DBGMessage(buffer);

    uin = atol(buffer);

    if(!uin)
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected UIN received in authorization packet");
       return 0;
    }

    ptr += (header->szNick);
    sz -= (sizeof(struct _header) + header->szNick);

    DBGTrace(sz);
    DBGTrace(*ptr);

    status = *(ptr++);

    msgSize = SHORT_VALUE( *((USHORT *)ptr));

    DBGTrace(msgSize);

    sz -= 3;
    DBGTrace(sz);

    ptr += 2;

    DBGTrace(status);
    DBGMessage(ptr);

    if(status)
    {
       if(msgSize)
       {
          icqInsertMessage(icq, uin, 0, MSG_ACCEPTED, 0, 0, msgSize, ptr);
       }
       else
       {
          strcpy(buffer,"You were authorized");
          icqInsertMessage(icq, uin, 0, MSG_ACCEPTED, 0, 0, strlen(buffer), buffer);
       }
    }
    else
    {
       icqInsertMessage(icq, uin, 0, MSG_REFUSED, 0, 0, msgSize, ptr);
    }


 /*

     08/26/2002 13:24:15 icqkrnl    Unknown SNAC packet (0x0013/0x001b - 19/27) (36 bytes)
     13 00 1b 00 80 00 84 e9 a7 48 00 06 00 01 00 02   ....€.„é§H......
     00 02 08 32 36 30 37 35 34 32 39 00 00 04 4e 6f   ...26075429...No
     70 65 00 00                                       pe..

Parameters
Data type Content Name Description
WORD 00 06 UNKNOWN Unknown: 6.
TLV(1) 00 01 00 02 xx xx UNKNOWN Unknown.
BUIN xx ... UIN The UIN that granted authorization.
BYTE xx AUTHORIZE 00 - Rejected, 01 - Granted
WORD xx xx REASON.LEN Length of reason (when rejected), always empty if granted.
    STRING xx ... REASON Reason, can be null.
WORD 00 00 UNKNOWN Always empty.

 */

    return 0;

 }

 static int _System srv_updateAck(HICQ icq, ICQV8 *cfg, int sz, ULONG req, UCHAR * ptr)
 {
/*
WORD  00 06  LEN  Guess: The length in bytes of the following data.
TLV(1) 00 01 00 02 xx xx UNKNOWN Unknown.
WORD 00 00 ERRCODE Error code:

00 - Success
0a - Failed?
0e - Failed; Authorization required to add buddy

13 00 0e 00 80 00 00 04 00 0a 00 06 00 01 00 02   ....€...........
00 02 00 00                                       ....


13 00 0e 00 80 00 00 04 00 0a 00 06

00 01
00 02
00 02 00 00

*/


    #pragma pack(1)

    struct _header
    {
       USHORT   x1;
       USHORT   tlv1_id;
       USHORT   tlv1_sz;
       USHORT   x2;
       USHORT   errCode;
    }           *header         = (struct _header *) ptr;
    char buffer[0x0100];

    CHKPoint();
    if( (header->tlv1_id != 0x0100) || (header->tlv1_sz != 0x0200))
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected TLV in srv_updateAck");
       return -1;
    }

    DBGTrace(req);

    DBGTracex(req);
    DBGTracex(header->tlv1_id);
    DBGTracex(header->tlv1_sz);

/*
snac13.c(271):  req = 27241234
snac13.c(273):  header->tlv1_id = 00000100
snac13.c(274):  header->tlv1_sz = 00000200
snac13.c(275):  header->errCode = 00000200
*/
    header->errCode = SHORT_VALUE(header->errCode);
    DBGTrace(header->errCode);

    switch(header->errCode)
    {
    case 0x00:  // Success
       icqWriteSysLog(icq,PROJECT,"User update ok");
       break;

    case 0x0a: // Failed?
       icqWriteSysLog(icq,PROJECT,"User update failed");
       break;

    case 0x0e: // Failed; Authorization required to add buddy
       icqWriteSysLog(icq,PROJECT,"Update failed, authorization needed");
       break;

    default:
       sprintf(buffer,"Error code %d in srv_updateAck()", (int) header->errCode);
       icqWriteSysLog(icq,PROJECT,buffer);
    }

    return 0;
 }

 static int _System srv_unavailable(HICQ icq, ICQV8 *cfg, int sz, ULONG req, UCHAR * ptr)
 {
    icqWarning(icq, 0, PROJECT, 0, "The service is temporally unavailable");
    icqSetConnectTimer(icq,20);
    return 0;
 }


