/*
 * advanced.c - Send an advanced message
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <time.h>

 #include "icqv8.h"

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static void type1(HICQ, ICQV8 *, PACKET, const char *);
 static void type2(HICQ, ICQV8 *, UCHAR, PACKET, const char *, ULONG, ULONG);
 static void type4(HICQ, ICQV8 *, UCHAR, PACKET, const char *);
 static void messageTLV(HICQ, ICQV8 *, PACKET, const char *, UCHAR, UCHAR);

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 USHORT icqv8_sendSystemMessage(HICQ icq, ULONG uin, USHORT type, USHORT subType, const char *txt, ULONG *id)
 {
#ifdef EXTENDED_LOG
    char                extLog[0x0100];
#endif

    ICQV8               *cfg = icqGetPluginDataBlock(icq, module);
    #pragma pack(1)

    struct _header
    {
       ULONG    msgTime;        // DWORD xx xx xx xx TIME The time part of the message ID.
       ULONG    msgID;          // DWORD xx xx xx xx MID The random part of the message ID.
       USHORT   msgType;        // The message type
    }                   h;

    PACKET              pkt     = icqv8_allocatePacket(icq,cfg,0x0100);

    memset(&h,0,sizeof(h));

    DBGTracex(subType);

    h.msgTime = time(NULL);
    h.msgID   = (cfg->seqMasc | (icqv8_querySequence(icq) & 0x0000FFFF))& 0x3FFFFFFF;
    h.msgType = subType;

#ifdef EXTENDED_LOG
    sprintf(extLog,"Sending msg %08lx:%08lx to %s (ICQ#%ld)",h.msgTime,h.msgID,icqQueryUserNick(icqQueryUserHandle(icq,uin)),uin);
    icqWriteSysLog(icq,PROJECT,extLog);
#endif

    DBGTracex(h.msgID);

    if(!txt)
       txt = "";

    icqv8_insertBlock(pkt, 0, sizeof(h), (const unsigned char *) &h);
    icqv8_insertBUIN(pkt, 0, uin);

    switch(h.msgType)
    {
    case 0x0100:        // Normal messages
       type1(icq,cfg,pkt,txt);
       break;

    case 0x0200:        // Advanced messages
       type2(icq,cfg,(UCHAR) type,pkt,txt,h.msgTime,h.msgID);
       if(id)
          *id = h.msgID;
       break;

    case 0x0400:        // URL messages
       type4(icq,cfg,(UCHAR) type,pkt,txt);
       break;

    }
    icqv8_insertBlock(pkt, 6, 0, 0);

#ifdef LOGUSER
    if(uin == LOGUSER)
       icqv8_dumpPacket(pkt, icq, "Advanced message sent");
#endif

    return icqv8_sendSnac(icq, cfg, 0x04, 0x06, LONG_VALUE(h.msgID), pkt);

 }

 static void type1(HICQ icq, ICQV8 *cfg, PACKET pkt, const char *txt)
 {
    PACKET              block;
    PACKET              msg;

    DBGMessage("MSG Type 1");

    block = icqv8_allocatePacket(icq,cfg,0x0100);

    icqv8_insertBlock(block,1281,1,"\x01");

    msg = icqv8_allocatePacket(icq,cfg,0x0100);
    icqv8_insertLong(msg,0,0);
    icqv8_insertString(msg, 0, txt);

    icqv8_insertPacket(icq, cfg, block, 257, msg);
    icqv8_insertPacket(icq, cfg, pkt,     2, block);
 }

 static void type4(HICQ icq, ICQV8 *cfg, UCHAR type, PACKET pkt, const char *txt)
 {
    PACKET              block;
    ULONG               lTemp;

    DBGMessage("MSG Type 4");

    block = icqv8_allocatePacket(icq,cfg,0x0100);

    lTemp  = icqQueryUIN(icq);
    icqv8_insertBlock(block,0,4,(const char *) &lTemp);

    icqv8_insertBlock(block,0,1,(const char *) &type);

    icqv8_insertBlock(block,0,1,(const char *) "" );

    icqv8_insertLNTS(block, (const UCHAR *) txt);

    icqv8_insertPacket(icq, cfg, pkt, 5, block);

 }

 static void type2(HICQ icq, ICQV8 *cfg, UCHAR type, PACKET base, const char *txt, ULONG msgTime, ULONG msgID)
 {
    PACKET pkt = icqv8_allocatePacket(icq,cfg,0x0100);

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

    } header    = {     0x0000,
                        msgTime,
                        msgID,
                        { 0x09, 0x46, 0x13, 0x49, 0x4C, 0x7F, 0x11, 0xD1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 }
                   };


    DBGMessage("MSG Type 2");
    icqv8_insertBlock(pkt,  0,  sizeof(header),(char *) &header);

    icqv8_insertLong(pkt,  6,   icqQueryOnlineMode(icq) );

    icqv8_insertShort(pkt, 10,  2);    // ACKTYPE2 0x0001 - normal message 0x0002 - file ack or file ok

    icqv8_insertBlock(pkt, 15,  0, 0); // UNKNOWN Unknown, emtpy.

    messageTLV(icq,cfg,pkt,txt, 0x01, 0x00);

//    icqv8_dumpPacket(pkt, icq, "TLV-5");

    icqv8_insertPacket(icq, cfg, base, 5, pkt);
 }

 static void messageTLV(HICQ icq, ICQV8 *cfg, PACKET base, const char *txt, UCHAR msgType, UCHAR msgFlags)
 {
    static const UCHAR footer[] = { 0x10, 0x18, 0x06, 0x70, 0x54, 0x71, 0xd3, 0x11, 0x8d, 0xd2,
                                    0x00, 0x10, 0x4b, 0x06, 0x46, 0x2e, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00 };

    PACKET pkt = icqv8_allocatePacket(icq,cfg,0x0100);
//    ULONG  lTemp;
    USHORT seq = 0;

    #pragma pack(1)

    struct _header
    {
       USHORT   len;            //  WORD.L 1b 00 LEN The length till the end of SEQ1.
       USHORT   tcpVer;         //  WORD.L xx xx TCPVER This is the version of the TCP protocol that the sending client uses.
       UCHAR    caps[16];       //  16 ... CAPABILITY Seen:
       UCHAR    x1[3];          //  3 00 00 03 UNKNOWN Unknown.
       ULONG    x2;             //  DWORD 00 00 00 xx UNKNOWN 0 = normal message, 4 = file ok or file request.
       USHORT   seq;            //  WORD xx xx SEQ1 A sequence that starts at 0xffff. I'm not sure yet when this decrements, most likely on each message send.
       USHORT   x5;             //  WORD.L xx xx LEN2 Unknown: 0x0e. Some people consider it the length till the end of the unknown 12 bytes array; however, that does not fit for the type-2 messages ICQ2001/2002 sends when it sees you logging in.
       USHORT   seq2;           //  WORD xx xx SEQ2 This is a copy of SEQ1.
       UCHAR    x3[12];         //  12 00 ... 00 UNKNOWN Unknown, alwasy zero.
       UCHAR    msgType;        //  BYTE xx MSGTYPE What type of message this is:
       UCHAR    msgFlags;       //  BYTE xx MSGFLAGS 00 - normal message 80 - multiple recipients 03 - auto reply message request.
       USHORT   x4;             //  WORD xx xx UNKNOWN Usually 0, seen 0x2000.
       USHORT   priority;       //  WORD xx xx PRIORITY Usually 0, seen 0x0002 in information request messages.
    } header = {        0x001b,
                        0x0008,
                        { 0x10, 0xcf, 0x40, 0xd1, 0x4f, 0xe9, 0xd3, 0x11, 0xbc, 0xd2, 0x00, 0x04, 0xac, 0x96, 0xdd, 0x96 },
//                        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                        { 0x00, 0x00, 0x03 },
                        0,
                        0,
                        0x12,
                        0,
                        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                        0,
                        0,
                        0,
                        0 };

    header.msgType  = msgType;
    header.msgFlags = msgFlags;

    LOCK(cfg);
    header.seq      =
    header.seq2     = cfg->downSequence--;
    UNLOCK(cfg);

    DBGTracex(seq);
    DBGTracex(header.seq);

    icqv8_insertBlock(pkt,  0,  sizeof(header),(char *) &header);

    icqv8_insertLNTS(pkt, txt);                         // LNTS xx xx ... MESSAGE The message!

    icqv8_insertBlock(pkt,  0,  sizeof(footer),(char *) &footer);

//    icqv8_dumpPacket(pkt, icq, "TLV-0x2711");

    icqv8_insertPacket(icq, cfg, base, 10001, pkt);
 }

/*
    TLV(5) 00 05 xx xx ... MESSAGE2 Used for type-2. TLV(05) contains many seperate fields including the message itself, as follows:
        WORD xx xx ACKTYPE 0x0000 - normal message
0x0001 - abort request
0x0002 - file ack
        TIME xx xx xx xx TIME A copy of the very first time in this packet.
        DWORD xx xx xx xx ID The random message ID again.
        16 ... CAPABILITY One of the capabilities sent in CLI_SETUSERINFO: 09 46 13 49 4C 7F 11 D1 82 22 44 45 53 54 00 00

        TLV(10) 00 0a 00 02 xx xx ACKTYPE2 0x0001 - normal message 0x0002 - file ack or file ok
        TLV(15) 00 0f 00 00 UNKNOWN Unknown, emtpy.

        TLV(10001) 27 11 xx xx ... MESSAGE This is yet another message TLV that adds yet another layer of complexity as follows:
            WORD.L 1b 00 LEN The length till the end of SEQ1.
            WORD.L xx xx TCPVER This is the version of the TCP protocol that the sending client uses.
            16 ... CAPABILITY Seen:
a0 e9 3f 37 4f e9 d3 11 bc d2 00 04 ac 96 dd 96
10 cf 40 d1 4f e9 d3 11 bc d2 00 04 ac 96 dd 96
            3 00 00 03 UNKNOWN Unknown.
            DWORD 00 00 00 xx UNKNOWN 0 = normal message, 4 = file ok or file request.
            WORD xx xx SEQ1 A sequence that starts at 0xffff. I'm not sure yet when this decrements, most likely on each message send.
            WORD.L xx xx LEN2 Unknown: 0x0e. Some people consider it the length till the end of the unknown 12 bytes array; however, that does not fit for the type-2 messages ICQ2001/2002 sends when it sees you logging in.
            WORD xx xx SEQ2 This is a copy of SEQ1.
            12 00 ... 00 UNKNOWN Unknown, alwasy zero.
            BYTE xx MSGTYPE What type of message this is:
0x01 - normal message
0x02 - chat request
0x03 - file request
0x04 - URL
0x05 - Unknown
0x06 - Authorisation Request
0x07 - Authorisation Rejected
0x08 - Authorisation Accepted
0x0c - User Added You
0x0d - Web Pager message
0x0e - Email Express message (eg. [uin]@pager.icq.com)
0x13 - A Contact List
0x1a - SMS message
0xe8-0xec - Request away messages:
0xe8 Away, 0xe9 Occupied, 0xea NA, 0xeb DND, 0xec Free for chat
            BYTE xx MSGFLAGS 00 - normal message 80 - multiple recipients 03 - auto reply message request.
            WORD xx xx UNKNOWN Usually 0, seen 0x2000.
            WORD xx xx PRIORITY Usually 0, seen 0x0002 in information request messages.
            LNTS xx xx ... MESSAGE The message!
            COLOR xx xx xx xx FOREGROUND Only present in actual real messages, this will be the text color of the message in RGB0 format.
            COLOR xx xx xx xx BACKGROUND Only present in actual real messages, this will be the background color of the text box in RGB0 format.
            DWORD xx xx xx xx
            DATA ...
GUID This GUID seems to indicate that the client is capable of handling Multibyte Wide Character Strings as messages. Only present in real messages sent by build 3659 2001b clients.
*/

/*

11/15/2002 23:52:49 icqkrnl    System message received (172 bytes)
4d 6e db 04 aa 1a 00 00 00 02 07 35 34 39 36 37   Mn€.™......54967
31 32 00 00 00 04 00 01 00 02 00 50 00 06 00 04   12.........P....
20 02 00 00 00 0f 00 04 00 00 04 9f 00 03 00 04    ..........ü....
3d d5 a0 53 00 05 00 74 00 00 4d 6e db 04 aa 1a   =’†S...t..Mn€.™.
00 00 09 46 13 49 4c 7f 11 d1 82 22 44 45 53 54   ...F.IL.—Ç"DEST
00 00 00 0a 00 02 00 01 00 0f 00 00 27 11 00 4c   ............'..L
1b 00 08 00 10 cf 40 d1 4f e9 d3 11 bc d2 00 04   .....œ@—OÈ”.º“..
ac 96 dd 96 00 00 03 00 00 00 00 fd ff 12 00 fd   ¨ñ›ñ.......˝ˇ..˝
ff 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00   ˇ...............
00 00 00 01 00 00 10 18 06 70 54 71 d3 11 8d d2   .........pTq”.ç“
00 10 4b 06 46 2e 00 00 00 00 00 00               ..K.F.......

        4d 6e db 04             MSGTime
        aa 1a 00 00             MSGID

        00 02                   MSGFormat

        07                      From Slaughter
        35 34 39 36
        37 31 32

        00 00                   Warning level
        00 04                   4 TLVs

        00 01                   01) TLV-1 Unknown
        00 02
        00 50

        00 06                   02) TLV-6 User online status
        00 04
        20 02 00 00

        00 0f                   03) TLV-15
        00 04
        00 00 04 9f

        00 03                   04) TLV-3
        00 04
        3d d5 a0 53

        00 05                   05) TLV-5
        00 74

        00 00 4d 6e db 04 aa 1a
00 00 09 46 13 49 4c 7f 11 d1 82 22 44 45 53 54
00 00 00 0a 00 02 00 01 00 0f 00 00 27 11 00 4c
1b 00 08 00 10 cf 40 d1 4f e9 d3 11 bc d2 00 04
ac 96 dd 96 00 00 03 00 00 00 00 fd ff 12 00 fd
ff 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00
00 00 00 01 00 00 10 18 06 70 54 71 d3 11 8d d2
00 10 4b 06 46 2e 00 00 00 00 00 00


11/15/2002 23:52:49 icqkrnl    Message type 2 (90 bytes)
00 0a 00 02 00 01 00 0f 00 00 27 11 00 4c 1b 00   ..........'..L..
08 00 10 cf 40 d1 4f e9 d3 11 bc d2 00 04 ac 96   ...œ@—OÈ”.º“..¨ñ
dd 96 00 00 03 00 00 00 00 fd ff 12 00 fd ff 00   ›ñ.......˝ˇ..˝ˇ.
00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00   ................
00 01 00 00 10 18 06 70 54 71 d3 11 8d d2 00 10   .......pTq”.ç“..
4b 06 46 2e 00 00 00 00 00 00                     K.F.......
11/15/2002 23:52:49 icqv8      ICQ#5496712 Status:0000 Subtype:0001 Size:0001 Flags:00
11/15/2002 23:52:49 icqv8      Message Flag: Normal
11/15/2002 23:52:49 icqkrnl    Sending confirmation (82 bytes)
4d 6e db 04 aa 1a 00 00 00 02 07 35 34 39 36 37   Mn€.™......54967
31 32 00 03 1b 00 08 00 00 00 00 00 00 00 00 00   12..............
00 00 00 00 00 00 00 00 00 00 03 00 00 00 00 fd   ...............˝
ff 0e 00 fd ff 00 00 00 00 00 00 00 00 00 00 00   ˇ..˝ˇ...........
00 01 00 04 00 00 00 01 00 00 00 00 00 00 ff ff   ..............ˇˇ
ff 00                                             ˇ.

*/



