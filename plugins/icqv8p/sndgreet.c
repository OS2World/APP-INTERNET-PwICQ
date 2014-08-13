/*
 * sndgreet.c - Send Greetings message
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>

 #include "peer.h"


/*
WORD.L  xx xx   GCMD  The command in this packet. Valid values are:

0x0029 =  41 - file request
0x002d =  45 - chat request
0x0032 =  50 - file request granted/refused
0x003a =  58 - Send / Start ICQ Chat

16 ... ID Those 16 bytes seem to be fixed for different commands. Seen:

bf f7 20 b2 37 8e d4 11 bd 28 00 04 ac 96 d9 05 - chat request
f0 2d 12 d9 30 91 d3 11 8d d7 00 10 4b 06 46 2e - file request or file request granted/refused

WORD.L 00 00 EMPTY Unknown: empty.
DLStr xx xx xx xx ... TEXT 	"ICQ Chat" or
				"Send / Start ICQ Chat" for chat request,
				"File" for file request,
				"File Transfer" for file request grant/refusal.
				This text is displayed in the requester opened by Windows.
15 ... UNKNOWN Unknown. Seen:

00 00 01 00 00 01 00 00 00 00 00 00 00 00 00 on chat/file requests,
00 00 01 01 00 00 00 00 00 00 00 00 00 00 00 on grants/refusals.

DWORD.L xx xx xx xx LENGTH The length of the remaining data. No, this is not a TLV.
DLStr xx xx xx xx ... REASON The reason for the chat request/file request/file request gran/refusal.

WORD.B xx xx PORT The port to connect to in bigendian.
WORD.B xx xx PAD Empty or unknown, probably padding.
BSTR xx xx ... NAME The file name for the chat request/grant/refusal, empty for chat.
DWORD.L xx xx xx xx FILELEN The length of the file in bytes.
DWORD.L xx xx xx xx PORT2 Again the port, but as double word and in long endian. Missing for chat requests.

93 f7 f1 f3 ee 07 0e 00 fc ff 00 00 00 00 00 00
00 00 00 00 00 00 1a 00 00 00 01 00 01 00 00 29
00 f0 2d 12 d9 30 91 d3 11 8d d7 00 10 4b 06 46
2e 00 00 04 00 00 00 46 69 6c 65 00 00 01 00 00
01 00 00 00 00 00 00 00 00 00 23 00 00 00 05 00
00 00 74 65 73 74 65 a8 43 5c 01 0c 00 69 72 65
61 6c 69 7a 65 2e 78 6d 00 c1 3b 2b 00 43 a8 00
00

02/13/2003 19:42:00 icqkrnl    Greetings message (113 bytes)
00 00 00 00 ee 07 0e 00 00 00 00 00 00 00 00 00   ....Ó...........
00 00 00 00 00 00 1a 00 00 00 21 00 01 00 00 29   ..........!....)
00 f0 2d 12 d9 30 91 d3 11 8d d7 00 10 4b 06 46   .-.Ÿ0ë”.ç◊..K.F
2e 00 00 04 00 00 00 46 69 6c 65 00 00 01 00 00   .......File.....
01 00 00 00 00 00 00 00 00 00 17 00 00 00 05 00   ................
00 00 74 65 73 74 65 32 db 00 00 0c 00 69 72 65   ..teste2€....ire
61 6c 69 7a 65 2e 78 6d 00 c1 3b 2b 00 db 32 00   alize.xm.¡;+.€2.
00                                                .

 29 00
 f0 2d 12 d9
 30 91 d3 11
 8d d7 00 10
 4b 06 46 2e

 00 00 04 00
 00 00
 46 69 6c 65

 00 00 01 00
 00 01 00 00
 00 00 00 00
 00 00 00

 17 00 00 00 <----------- ERA PARA SER 23!!!

 05 00 00 00
 74 65 73 74
 65 32 db 00
 00 0c 00 69
 72 65 61 6c
 69 7a 65 2e
 78 6d 00 c1
 3b 2b 00 db
 32 00 00


*/

/*
    static const char chatReq[]  = { 0xbf, 0xf7, 0x20, 0xb2, 0x37, 0x8e, 0xd4, 0x11, 0xbd, 0x28, 0x00, 0x04, 0xac, 0x96, 0xd9, 0x05 };
    static const char fileReq[]  = { 0xf0, 0x2d, 0x12, 0xd9, 0x30, 0x91, 0xd3, 0x11, 0x8d, 0xd7, 0x00, 0x10, 0x4b, 0x06, 0x46, 0x2e };

    static const char reqBlock[] = { 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static const char rspBlock[] = { 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

   URL:
   0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29

*/


/*---[ Constants ]--------------------------------------------------------------------------------------------*/

 #pragma pack(1)

 struct greeting_msg_data
 {
    USHORT type;
    USHORT msgType;
    USHORT action;
    USHORT msgFlags;
    USHORT szExtra;
    UCHAR  flags;
    UCHAR  x1[16];
    UCHAR  x2[15];
 };

 static const struct greeting_msg_data icqv8_greetings_data[] =
 {
    {
       MSG_GREET_URL,
       MSG_GREET_URL,
       0x07ee,
       0x0021,
       0,
       0x00,
       { 0x37, 0x1c, 0x58, 0x72, 0xe9, 0x87, 0xd4, 0x11, 0xa4, 0xc1, 0x00, 0xd0, 0xb7, 0x59, 0xb1, 0xd9 },
       { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00       }
    },
    {
       MSG_GREET_FILE,
       MSG_GREET_FILE,
       0x07ee,
       0x0021,
       14,
       0x03,
       { 0xf0, 0x2d, 0x12, 0xd9, 0x30, 0x91, 0xd3, 0x11, 0x8d, 0xd7, 0x00, 0x10, 0x4b, 0x06, 0x46, 0x2e },
       { 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00       }
    },
    {
       MSG_GREET_FILE_REJECTED,
       MSG_GREET_FILE,
       0x07da,
       0x0000,
       14,
       0x03,
       { 0xf0, 0x2d, 0x12, 0xd9, 0x30, 0x91, 0xd3, 0x11, 0x8d, 0xd7, 0x00, 0x10, 0x4b, 0x06, 0x46, 0x2e },
       { 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00       }
    },
    {
       MSG_GREET_FILE_ACCEPTED,
       MSG_GREET_FILE,
       0x07da,
       0x0000,
       14,
       0x03,
       { 0xf0, 0x2d, 0x12, 0xd9, 0x30, 0x91, 0xd3, 0x11, 0x8d, 0xd7, 0x00, 0x10, 0x4b, 0x06, 0x46, 0x2e },
       { 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
    }

 };


/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 static const struct greeting_msg_data * icqPeer_queryGreetingsData(USHORT type)
 {
    int                             f;
    int                             sz            = (sizeof(icqv8_greetings_data)/sizeof(struct greeting_msg_data));

    DBGTrace(sz);

    for(f=0;f < sz && icqv8_greetings_data[f].type != type; f++);

    if(f >= sz)
       return NULL;

    return icqv8_greetings_data+f;

 }


 int icqPeer_sndGreet(HICQ icq, PEERSESSION *cfg, USHORT type, ULONG sequence, USHORT status, const char *text, const char *title, int szTitle, const char *reason, int szReason, const char *fileName, int szFileName, ULONG fileSize)
 {
    const struct greeting_msg_data  *msgData     = icqPeer_queryGreetingsData(type);
    int  	                    sz;
    int                             rc           = 0;

    #pragma pack(1)

    PEER_MSG_HEADER		    *h;

    struct _pkt
    {
       USHORT          type;         // WORD.L  xx xx   GCMD  The command in this packet. Valid values are:
       UCHAR           x1[16];
       USHORT          x2;           // EMPTY  Unknown: empty.
       USHORT          szTitle;
       USHORT          x3;
    } *prefix;


    char   *ptr;
    USHORT port = icqQueryPeerPort(icq);
    int    f;

    if(!msgData)
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected greetings message type");
       return -1;
    }

    if(szTitle < 0)
        szTitle = strlen(title);

    if(szReason < 0)
        szReason = strlen(reason);

    if(szFileName < 0)
        szFileName = strlen(fileName)+1;

    sz  = szTitle+szReason+szFileName+sizeof(struct _pkt)+sizeof(PEER_MSG_HEADER)+strlen(text)+1+msgData->szExtra+23;
    h   = malloc(sz+5);

    if(!h)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when sending greetings message");
       return rc;
    }

    memset(h,0,sz);

    DBGTrace(sz);

    h->cmd      = 0x02;
    h->action   = msgData->action;
    h->x1       = 0x0e;
    h->msgType  = MSG_GREET;
    h->flags    = msgData->msgFlags;
    h->msgSize  = strlen(text)+1;
    h->status   = status;

    ptr = (char *) (h+1);

    strncpy(ptr,text,h->msgSize);
    ptr += h->msgSize;

    prefix       = (struct _pkt *) ptr;
    prefix->type = msgData->msgType;

    ptr = (char *) (prefix+1);

    if(sequence)
    {
       h->sequence = (USHORT) (sequence & 0x0000FFFF);
       rc          = sequence;
    }
    else
    {
       h->sequence = cfg->sequence--;
       rc = cfg->seqMasc|((ULONG) h->sequence);
    }


    CHKPoint();

    for(f=0;f<16;f++)
       prefix->x1[f] = msgData->x1[f];

    CHKPoint();

    prefix->szTitle = szTitle;
    strncpy(ptr,title,szTitle);
    ptr += szTitle;

    for(f=0;f<15;f++)
       *(ptr++) = msgData->x2[f];


    if(msgData->flags & 1)
       *( (ULONG *) ptr ) = szReason+szFileName+msgData->szExtra+4;
    else
       *( (ULONG *) ptr ) = szReason+msgData->szExtra+4;

    DBGTracex(*(ULONG *) ptr);

    ptr += 4;

    *( (USHORT *) ptr ) = szReason;
    ptr += 4;

    strncpy(ptr,reason,szReason);

    for(f=0;f<szReason;f++)
    {
       if( *(ptr+f) == ICQ_FIELD_SEPARATOR )
          *(ptr+f) = 0xFE;
    }

    ptr += szReason;


    if(msgData->flags & 1)
    {

       CHKPoint();
       * ((USHORT *) ptr ) = SHORT_VALUE(port);
       ptr += 4;

       *( (USHORT *) ptr ) = szFileName;
       ptr += 2;

       strncpy(ptr,fileName,szFileName);
       ptr += szFileName;

       *( (ULONG *) ptr ) = fileSize;
       ptr += 4;
    }

    if(msgData->flags & 2)
    {
       CHKPoint();
       *( (ULONG *) ptr ) = port;
       ptr += 4;
    }

    /* Finalizacao */

    DBGTrace( ((ULONG) ptr) - ((ULONG)h));
    DBGTrace(sz);
    icqDumpPacket(icq, NULL, "Greetings message", sz-1, ((unsigned char *) h)+1);

    ptr = (UCHAR *) h;
    icqv8_encript(ptr+1, sz-1);
    icqv8_sendBlock(cfg->sock, sz, h);

    free(h);
    return rc;
 }



