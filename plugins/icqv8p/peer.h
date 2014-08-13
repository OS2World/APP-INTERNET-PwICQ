/*
 * peer.c
 */

 #include <icqtlkt.h>

/*---[ Structures ]----------------------------------------------------------------------------------------*/

 #pragma pack(1)

 typedef struct _peerconf
 {
    USHORT	sz;
    UCHAR	active;
    ULONG       cookie;
    HLIST       sessions;

    int		thread;
    int         sock;

 } PEERCONF;

#ifndef HPEER
 #define HPEER PEERCONF *
#endif

 typedef struct _peersession
 {
    USHORT  sz;
    ULONG   uin;
    int     sock;
    int     thread;
    HPEER   peer;
    USHORT  idle;
    ULONG   seqMasc;
    USHORT  sequence;
    int     port;

    USHORT  counter;		// On file transfer: Number of files
    USHORT  number;		// On file transfer: File number
    ULONG   bytes;              // On file transfer: Total bytes to send
    ULONG   transfer;		// On file transfer: Bytes received/send
    ULONG   speed;
    ULONG   delay;		// File transfer delay
    FILE    *arq;		// On file transfer: The handle of open file

    ULONG   flags;
    #define PEERFLAG_ACTIVE	0x00000001
    #define PEERFLAG_MESSAGE	0x00000002
    #define PEERFLAG_ORIGINATE  0x00000004
    #define PEERFLAG_FILE       0x00000008
    #define PEERFLAG_READY      0x00000010
    #define PEERFLAG_INIT       0x00000020
    #define PEERFLAG_COMPLETE   0x00000040

 } PEERSESSION;


 #ifndef ICQV8P_TYPES_DEFINED
    #ifdef PEERDATA
       #undef PEERDATA
    #endif
    #define PEERDATA PEERSESSION
 #endif

 #ifndef NULLHANDLE
    #define NULLHANDLE 0
 #endif


/*---[ Packet format ]----------------------------------------------------------------------------------------*/

 #pragma pack(1)

 typedef struct peer_msg_header
 {
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
 } PEER_MSG_HEADER;



/*---[ Defines ]-------------------------------------------------------------------------------------------*/

 #define MSG_GREET_FILE          0x0029
 #define MSG_GREET_CHAT          0x002d
 #define MSG_GREET_STATUS        0x0032
 #define MSG_GREET_BEGIN         0x003a
 #define MSG_GREET_URL           0x0040

 #define MSG_GREET_FILE_REJECTED 0x8029
 #define MSG_GREET_FILE_ACCEPTED 0x4029

 #define ICQV8_PEER_AUTOREPLY    0x0000 //    0 - auto reply
 #define ICQV8_PEER_NORMAL       0x0001 //    1 - message
 #define ICQV8_PEER_CHAT         0x0002 //    2 - chat
 #define ICQV8_PEER_FILE         0x0003 //    3 - file
 #define ICQV8_PEER_URL          0x0004 //    4 - url
 #define ICQV8_PEER_REQUEST      0x0006 //    6 - request auth
 #define ICQV8_PEER_REFUSED      0x0007 //    7 - deny auth
 #define ICQV8_PEER_ACCEPTED     0x0008 //    8 - grant auth
 #define ICQV8_PEER_ADD          0x000c //   12 - added to contact list
 #define ICQV8_PEER_PAGER        0x000d //   13 - web pager
 #define ICQV8_PEER_MAIL         0x000e //   14 - email pager
 #define ICQV8_PEER_CONTACT      0x0013 //   19 - add UIN
 #define ICQV8_PEER_GREETING     0x001a //   26 - greeting
 #define ICQV8_PEER_REQAWAY      0x03e8 // 1000 - request "away" message.
 #define ICQV8_PEER_REQBUSY      0x03e9 // 1001 - request "occupied" message.
 #define ICQV8_PEER_REQNA        0x03ea // 1002 - request "n/a" message.
 #define ICQV8_PEER_REQDND       0x03eb // 1003 - request "dnd" message.
 #define ICQV8_PEER_REQFFC       0x03ec // 1004 - request "ffc" message.


/*---[ Macros ]--------------------------------------------------------------------------------------------*/

 #define FORWARD(x,s) ptr += x; sz -= (x+s);

/*---[ Publics ]-------------------------------------------------------------------------------------------*/

 extern HMODULE                        module;
 extern const PEERPROTMGR              icqv8_peerDescriptor;
 extern const REQUESTMGR               icqv8p_requestManager;

/*---[ Entry points ]--------------------------------------------------------------------------------------*/

 int EXPENTRY    icqv8_sendBlock(int, USHORT, const void *);
 int EXPENTRY    icqv8_recBlock(HPEER, int, int, void *);

 ULONG   _System icqv8_setCookie(HICQ,HPEER,ULONG);
 ULONG   _System icqv8_getCookie(HICQ,HPEER);
 long    _System icqv8_getPort(HICQ,HPEER);
 long    _System icqv8_getAddress(HICQ, HPEER);

 int     _System icqv8_connect(HICQ,HPEER,HUSER,int);
 USHORT  _System icqv8_sendMessage(HICQ, HUSER, USHORT, const char *, ULONG *, PEERDATA *);
 USHORT  _System icqv8_sendFileReq(HICQ, HUSER, PEERDATA *, const char *, int, const char *, ULONG);

 int     _System icqv8_accept(HICQ, ULONG, PEERDATA *, ULONG, char *, const char *);
 int     _System icqv8_refuse(HICQ, ULONG, PEERDATA *, ULONG, char *, const char *);
 int     _System icqv8_accepted(HICQ, ULONG, PEERDATA *, ULONG, int, char *);

 int     _System icqv8_startListener(HICQ, HPEER);

 int             icqv8_decript(unsigned char *, int);
 int             icqv8_encript(unsigned char *, int);

 int             icqv8_execPeer(HICQ, PEERSESSION *, int, unsigned char *);
 int             icqv8_sendPeerHello(HICQ, PEERSESSION *);
 void            icqv8_terminateSession(HICQ, PEERSESSION *);
 int             icqv8_setPeerSpeed(HICQ, PEERSESSION *, ULONG);

 int             icqv8_sendFile(HICQ, PEERDATA *, int, char *);
 int             icqv8_recvFileStart(HICQ, PEERSESSION *, int, unsigned char *);
 int             icqv8_peerFileStart(HICQ, PEERSESSION *, const char *);
 int             icqv8_fileStop(HICQ, PEERSESSION *);
 char *          icqv8_mountFilePath(HICQ, PEERSESSION *, char *);
 int             icqv8_recvFileInit(HICQ, PEERSESSION *, int, unsigned char *);

 int             icqv8_recvPeer(HICQ, HPEER, int, void **);
 void            icqv8_peerSession(HICQ,PEERSESSION *);
 void	         icqv8_peerTimer(HICQ,HPEER);


 int             icqv8_sendMsgInit(HICQ, PEERSESSION *, BOOL);
 int             icqv8_recvMsg(HICQ,PEERSESSION *, int, unsigned char *);
 int             icqv8_msgInit(HICQ,PEERSESSION *, int, unsigned char *);

 int             icqv8_initSession(HICQ, HPEER, PEERSESSION *, ULONG);

 int             icqv8_rcvGreet(HICQ, PEERSESSION *, int, PEER_MSG_HEADER *);

 int             icqPeer_sndGreet(HICQ, PEERSESSION *, USHORT, ULONG, USHORT, const char *, const char *, int, const char *, int, const char *, int, ULONG);

 int             icqv8_connectPeer(HICQ, PEERSESSION *, BOOL);

#ifdef DEBUG
 void            debugUrlAck(HICQ, PEERSESSION *, USHORT);
#endif

 #define SHORT_VALUE(x)  (  ((x & 0xFF00) >> 8) | ( (x & 0x00FF) << 8 ) )


