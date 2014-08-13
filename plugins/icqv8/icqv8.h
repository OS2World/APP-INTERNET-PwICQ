/*
 * ICQV8.H
 */

#ifdef __OS2__
 #define INCL_DOSSEMAPHORES
 #define INCL_DOSPROCESS
#endif

 #include <stdio.h>
 #include <pwMacros.h>
 #include <icqtlkt.h>
 #include <icqreserved.h>

#ifdef __OS2__
 #define random rand
#endif

/*---[ Protocol defines ]-------------------------------------------------------------------------------*/

 #define LOGIN_PORT             5190
 #define PACKET_START           0x2a
 #define DEF_PACKET_SIZE        10

 #define VER_MAJOR              0x0005
 #define VER_MINOR              0x000F
 #define VER_LESSER             0x0001
 #define VER_BUILD              0x0E36
 #define VER_SUBBUILD           0x00000055

/*---[ Flags for usr->c2sFlags ]------------------------------------------------------------------------*/

 #define USRV8_NOTCONFIRMED     0x00000001
 #define USRV8_SENDSTATUSMSG    0x00000002
 #define USRV8_STATUSSENT       0x00000004

/*---[ Packet formats ]---------------------------------------------------------------------------------*/

 #pragma pack(1)

 typedef struct flap
 {
    UCHAR       channel;                // * Channel ID (byte)
    #define CHANNEL_NEW         0x01    // * 0x01 - New Connection Negotiation
    #define CHANNEL_SNAC        0x02    // * 0x02 - SNAC data (non connection-oriented data)
    #define CHANNEL_ERROR       0x03    // * 0x03 - FLAP-level Error
    #define CHANNEL_CLOSE       0x04    // * 0x04 - Close Connection Negotiation
    #define CHANNEL_KEEPALIVE   0x05    // * 0x05 - Keep alive

    USHORT      sequence;               // * Sequence Number (word)
    USHORT      size;                   // * Data Field Length (word)

 } FLAP;

 typedef struct snac
 {
    USHORT      family;                 // 1 word Family ID
    USHORT      subType;                // 3 word SubType ID
    UCHAR       flags[2];               // 5 byte Flags[0]
    ULONG       request;                // 7 dword Request ID
                                        // 11 variable SNAC Data
 } SNAC;

 typedef struct tlv
 {
    USHORT      id;
    USHORT      sz;
    void        *data;
 } TLV;

 typedef unsigned short PACKET;

/*---[ Structures ]-------------------------------------------------------------------------------------*/

 #pragma pack(1)

 typedef struct _icqv8
 {
    USHORT           sz;
    int              sock;
    int              thread;
    UCHAR            status;
    #define ICQV8_STATUS_OFFLINE                0x00
    #define ICQV8_STATUS_FINDING                0x01
    #define ICQV8_STATUS_LOGIN                  0x02
    #define ICQV8_STATUS_CONNECTING             0x03
    #define ICQV8_STATUS_ONLINE                 0x04

    UCHAR            flags;
    #define ICQV8_FLAG_SEND_CONFIRMATIONS       0x01
    #define ICQV8_FLAG_SENDSTATUSMSG		0x02
    #define ICQV8_FLAG_CONFIRMOFFLINEMSG	0x04

    int              packets;
    USHORT           packetSeq;
    USHORT           toICQSrvSeq;
    ULONG            seqMasc;

    int              kplTimer;

    PACKET           wrkPacket;

    ULONG            msgID;
    ULONG            sequence;
    USHORT           downSequence;              // * Down Sequence
    int 	     watchDogCounter;


#ifdef __OS2__
    HMTX             sendFlag;
#endif

#ifdef linux
   pthread_mutex_t   sendFlag;
#endif

 } ICQV8;

 #define SNAC_CMD_PARMS HICQ, ICQV8 *, int, ULONG, void *
 #define SNAC_CMD_PREFIX (int (* _System)(SNAC_CMD_PARMS))

 typedef struct snaccmd
 {
    USHORT subType;
    int ( * _System exec)(SNAC_CMD_PARMS);
 } SNAC_CMD;


#ifdef EXTENDED_LOG
 #define setStatus(i,x,s) x->status = s;icqWriteSysLog(i,PROJECT,icqv8_statusLog[s])
#else
 #define setStatus(i,x,s) x->status = s
#endif


 struct xmlRequest
 {
    ULONG       id;
    const char  *key;
 };

/*---[ Types ]---------------------------------------------------------------------------------------------*/


 #define        ICQ_ALLOWDIRECT     0x00000000 // allow direct connection with everyone
 #define        ICQ_DIRECTWITHAUT   0x10000000 // allow direct connection upon authorization
 #define        ICQ_DIRECTINCONTACT 0x20000000 // allow direct connection with users in contact list

 #define        ICQ_SHOWIPADDRESS   0x00020000 // show IP
 #define        ICQ_WEBAWARE        0x00010000 // webaware

 #define        ICQ_BIRTHDAY        0x00080000 // User's birthday

 #define        ICQ_ONLINE          0x00000000 // online
 #define        ICQ_INVISIBLE       0x00000100 // invisible
 #define        ICQ_AWAY            0x00000001 // away
 #define        ICQ_NA              0x00000005 // NA
 #define        ICQ_OCCUPIED        0x00000011 // Occupied
 #define        ICQ_DND             0x00000013 // DND
 #define        ICQ_FREEFORCHAT     0x00000020 // Free for chat

 #define        ICQ_OFFLINE         0x0000FFFF

 #define        MODE_MASC           0x0000FFFF

/*
 #define ICQ_ONLINE             0x0000  //     0000  online
 #define ICQ_FREEFORCHAT        0x0020  //     0020  free4chat
 #define ICQ_AWAY               0x0001  //     0001  away
 #define ICQ_NA                 0x0004  //     0004  n/a
 #define ICQ_NA2                0x0005  //     0005  n/a
 #define ICQ_OCCUPIED           0x0010  //     0010  occupied
 #define ICQ_OCCUPIED2          0x0011  //     0011  occupied
 #define ICQ_DND                0x0013  //     0013  dnd
 #define ICQ_INVISIBLE          0x0100  //     0100  invisible
*/


/*---[ Typedefs ]---------------------------------------------------------------------------------------*/

 typedef void (CHANNELPROCESSOR)(HICQ, ICQV8 *, int, void *);

/*---[ Constants ]--------------------------------------------------------------------------------------*/

 extern const C2SPROTMGR        icqv8_ftable;
 extern HMODULE                 module;
 extern const char              *icqv8_statusLog[];
 extern const struct xmlRequest icqv8_xmltable[];

 extern const SNAC_CMD icqv8_snac01_table[];
 extern const SNAC_CMD icqv8_snac02_table[];
 extern const SNAC_CMD icqv8_snac03_table[];
 extern const SNAC_CMD icqv8_snac04_table[];
 extern const SNAC_CMD icqv8_snac09_table[];
 extern const SNAC_CMD icqv8_snac0B_table[];
 extern const SNAC_CMD icqv8_snac13_table[];
 extern const SNAC_CMD icqv8_snac15_table[];

 extern const ICQMODETABLE icqv8_modes[];

/*---[ Macros ]-----------------------------------------------------------------------------------------*/

 #define SHORT_VALUE(x)  icqv8_convertShort(x)
 #define LONG_VALUE(x)   icqv8_convertLong(x)

 #define CLOSE_SOCKET(c) icqCloseSocket(cfg->sock);cfg->sock = -1;

 #define LOCK(x)         icqv8_lock(x)
 #define UNLOCK(x)       icqv8_unlock(x)

 #define FORWARD(x,s) ptr += x; sz -= (x+s);

/*---[ Packets ]----------------------------------------------------------------------------------------*/

 typedef struct _c2cinfo
 {
    ULONG    ipAddress;      // 4 xx xx xx xx IPINTERNAL The client computer's local IP address.
    ULONG    c2cPort;        // 4 00 00 xx xx PORT This is the port to connect with when making client to client connections.
    UCHAR    tcpFlag;        // 1 xx TCPFLAG 01 = Firewall (or HTTPS proxy)
                             // 02 = SOCKS4/5 proxy
                             // 04 = 'normal' connection
    USHORT   tcpVersion;     // 2 00 08 TCPVERSION The highest client to client protocol version this client uses.
    ULONG    cookie;         // 4 xx xx xx xx COOKIE Probably a direct client to client connection cookie.
    USHORT   x1;             // 2 00 00 Unknown 0, Unknown
    USHORT   x2;             // 2 00 50 Unknown 80, Unknown
    USHORT   x3;             // 2 00 00 Unknown 0, Unknown
    USHORT   counter;        // 2 00 03 COUNT Three items follow.
    ULONG    t1;             // A timestamp. This one is abused for version information:
                             // 7d xx xx xx - licq; reminder wxxy decimal is version w.x.y.
                             //               Note: if & 0x0000ffff is > 1000, mICQ.
                             // 00 80 00 00 - licq SSL flag
                             // ff ff ff ff - Miranda
                             // ff ff ff 8f - StrICQ
                             // ff ff ff 42 - mICQ
                             // ff ff ff 7f - &RQ
                             // ff ff ff be - alicq
                             // ff ff ff ab - YSM (does not send version)
                             //
                             // Otherwise unknown client.
    ULONG    t2;             // A timestamp. This one is also abused for version information:
                             // 0x80000000 Miranda flag: client is an alpha version
                             // aa bb cc dd Version a.b.c.d, if not determined by TIME1.
    ULONG    t3;             // Another timestamp.

    USHORT   x4;             // 2 00 00 Unknown 0, Unknown
 } C2CINFO;

 struct tlv2711
 {
    USHORT msgFlag;	// WORD.L  1b 00  LEN  The length till the end of SEQ1.
    USHORT tcpVer;	// WORD.L  xx xx  TCPVER  This is the version of the TCP protocol that the sending client uses.
    UCHAR  caps[16];	// 16  ...  CAPABILITY  Seen:	a0 e9 3f 37 4f e9 d3 11 bc d2 00 04 ac 96 dd 96
			//				10 cf 40 d1 4f e9 d3 11 bc d2 00 04 ac 96 dd 96
    UCHAR  x1[3];	// 3 00 00 03 UNKNOWN Unknown.
    ULONG  x2;		// DWORD 00 00 00 xx UNKNOWN 0 = normal message, 4 = file ok or file request.
    USHORT seq1;	// WORD xx xx SEQ1 A sequence that starts at 0xffff.
    USHORT x3;		// WORD.L xx xx LEN2 Unknown: 0x0e.
    USHORT seq2;	// WORD xx xx SEQ2 This is a copy of SEQ1.
    UCHAR  x4[12];	// 12 00 ... 00 UNKNOWN Unknown, alwasy zero.
    UCHAR  msgType;	// BYTE xx MSGTYPE What type of message this is:
    UCHAR  flags;	// BYTE xx MSGFLAGS 00 - normal message 80 - multiple recipients 03 - auto reply message request.
    USHORT x5;		// WORD xx xx UNKNOWN Usually 0, seen 0x2000.
    USHORT x6;		// WORD xx xx PRIORITY Usually 0, seen 0x0002 in information request messages.
    USHORT msgSize;	// LNTS xx xx ... MESSAGE The message!
 };

 struct searchResponse
 {
    USHORT  seq;
    USHORT  command;
    UCHAR   result;
 };


/*---[ Prototipos ]-------------------------------------------------------------------------------------*/

 USHORT  _System icqv8_queryModeIcon(HICQ, ULONG);
 ULONG   _System icqv8_setMode(HICQ,ULONG,ULONG);
 void    _System icqv8_mainThread(ICQTHREAD *);
 USHORT  _System icqv8_sendMessage(HICQ, ULONG, USHORT, const char *, ULONG *);

// USHORT _System icqv8_sendAdvancedMessage(HICQ, ULONG, USHORT, UCHAR *, ULONG *);

 int     _System icqv8_isOnline(HICQ);
 int     _System icqv8_addUser(HICQ, HUSER);
 int     _System icqv8_delUser(HICQ, HUSER);
 int     _System icqv8_srvRecGreet(HICQ, ULONG, USHORT, struct tlv2711 *);

 /* Searchs */
 HSEARCH _System icqv8_searchByUIN(HICQ, ULONG, int);
 int     _System icqv8_requestUserInfo(HICQ, ULONG, int);

 int     _System icqv8_searchByICQ(HICQ, ULONG, HWND, SEARCHCALLBACK);
 int 	 _System icqv8_searchByMail(HICQ, const char *, HWND, SEARCHCALLBACK);
 int 	 _System icqv8_searchRandom(HICQ, USHORT, HWND, SEARCHCALLBACK);
 int 	 _System icqv8_searchByInformation(HICQ,HWND,const char *, const char *,const char *, const char *,BOOL, SEARCHCALLBACK );

 int     _System icqv8_setPassword(HICQ,const char *, const char *);

 int     _System icqv8_searchResult(HICQ, ICQV8 *, int, ULONG, struct searchResponse *);

 void           icqv8_processChannel1(HICQ, ICQV8 *, int, void *);
 void           icqv8_processChannel2(HICQ, ICQV8 *, int, SNAC *);
 void           icqv8_processChannel3(HICQ, ICQV8 *, int, void *);
 void           icqv8_processChannel4(HICQ, ICQV8 *, int, void *);

 void           icqv8_lock(ICQV8 *);
 void           icqv8_unlock(ICQV8 *);

 int            icqv8_receiveBlock(HICQ, ICQV8 *, int, void *);

 USHORT         icqv8_convertShort(USHORT);
 ULONG          icqv8_convertLong(ULONG);
 ULONG          icqv8_queryCurrentID(HICQ);
 ULONG          icqv8_querySequence(HICQ);

 void           icqv8_initPacketList(void);
 PACKET         icqv8_allocatePacket(HICQ, ICQV8 *, int);
 void           icqv8_releasePacket(HICQ, ICQV8 *, PACKET);
 void           icqv8_insertLong(PACKET, USHORT, ULONG);
 void           icqv8_insertShort(PACKET, USHORT, USHORT);
 void           icqv8_dumpPacket(PACKET, HICQ, const char *);
 void           icqv8_insertString(PACKET, USHORT, const UCHAR *);
 void           icqv8_insertLNTS(PACKET, const UCHAR *);
 void           icqv8_insertBlock(PACKET, USHORT, USHORT, const unsigned char *);
 void           icqv8_toICQServer(HICQ, ICQV8 *, USHORT, ULONG, int, void *);
 void           icqv8_insertBUIN(PACKET, USHORT, ULONG);
 int            icqv8_getPacketSize(PACKET);
 void           icqv8_insertPacket(HICQ,ICQV8 *,PACKET,USHORT,PACKET);

 int            icqv8_sendFlap(HICQ, ICQV8 *, UCHAR, PACKET);
 int            icqv8_sendSnac(HICQ, ICQV8 *, USHORT, USHORT, ULONG, PACKET);
 int            icqv8_sendSingleSnac(HICQ, ICQV8 *, USHORT, USHORT, ULONG);
 void           icqv8_sendModePacket(HICQ, ICQV8 *, ULONG);

 USHORT         icqv8_sendSystemMessage(HICQ, ULONG, USHORT, USHORT, const char *, ULONG *);

 const void *   icqv8_getTLV(HICQ, ICQV8 *, TLV *, int *, const void *);
 void           icqv8_dumptlv(HICQ, const char *, ULONG, TLV *);

 void           icqv8_goodbye(HICQ, ICQV8 *);

 int            icqv8_downloadUserList(HICQ, ICQV8 *, int, const UCHAR *);

 void           icqv8_completeLogin(HICQ, ICQV8 *);

 void           icqv8_startUserUpdate(HICQ, ICQV8 *,ULONG);

 int 		icqv8_procTLV2711(HICQ, ULONG, USHORT, struct tlv2711 *, char *);
 void           icqv8_sendKeepAlive(HICQ, ICQV8 *);

#ifdef DEBUG
 int debug_SearchResponse(HICQ, ICQV8 *, int, BOOL, char *);
 int debug_offlineMessage(HICQ, ICQV8 *, int, char *);
#endif

#ifdef DEBUG_DL
 void debug_dlUserList(HICQ);
#endif
