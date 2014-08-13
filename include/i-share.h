/*
 * I-SHARE.H - Instant-sharing
 */

#ifndef ISHARE_INCLUDED

   #define ISHARE_INCLUDED 1

   #pragma pack(1)

/*---[ Defines ]-----------------------------------------------------------------------------------------*/

   #define ISHARE_UDP_PORT		6502
   #define ISHARE_TCP_PORT		6502
   #define ISHARE_ADM_PORT		6502

   #define ISHARE_MAX_PACKET_SIZE	1024

   #define ISHARE_TCP_MARKER		0x08FD

   #define ISHARE_KEEPALIVE_TIMER	120
   #define ISHARE_MAXKEEPALIVE		10

   #define IPADDR_LOOPBACK		0x0100007f
   #define IPADDR_BROADCAST		0xffffffff
   #define ISHARE_MAGICNUMBER           0x20020211

   enum ISHARE_UDP_PACKET_TYPES
   {
      ISHAREUDP_NOP,			/*  0 Nop                                       */

      ISHAREUDP_HELLO,			/*  1 Hello to server				            */
      ISHAREUDP_HELLOACK,		/*  2 Server ack                                */
      ISHAREUDP_ERROR,   		/*  3 Message error				                */
      ISHAREUDP_KEEPALIVE,		/*  4 Keep alive packet				            */
      ISHAREUDP_LISTREQ,		/*  5 IP List request				            */
      ISHAREUDP_REQACK,			/*  6 List request ack				            */
      ISHAREUDP_SEARCH,			/*  7 Search request				            */
      ISHAREUDP_FOUND,			/*  8 Search response				            */
      ISHAREUDP_MESSAGE,		/*  9 Instant message				            */
      ISHAREUDP_MSGACK,			/* 10 Instant message ack                       */
      ISHAREUDP_ACK,			/* 11 Packet Ack                                */
      ISHAREUDP_SEARCHMD5,		/* 12 Search by MD5                             */ 			


      ISHAREUDP_UNKNOWN
   };

   enum ISHARE_TCP_PACKET_TYPES
   {
      ISHARETCP_ERROR,			/*  0 Error					    */
      ISHARETCP_LOGON,                  /*  1 Logon					    */
      ISHARETCP_BEGINTRANSFER,          /*  2 Begin file transfer                           */
      ISHARETCP_SEARCHTEXT,       	/*  3 Search for string		                    */
      ISHARETCP_SEARCHMD5,	        /*  4 Search for MD5			            */
      ISHARETCP_SEARCHSTATUS,		/*  5 Search status                                 */
      ISHARETCP_FOUND,	                /*  6 File found                                    */
      ISHARETCP_RESERVED_2,             /*  7                                               */
      ISHARETCP_REQUESTFILE,  		/*  8 Download request                              */
      ISHARETCP_RESERVED_4,  		/*  9                                               */
      ISHARETCP_RESERVED_5,             /* 10                                               */
      ISHARETCP_STOP,                   /* 11                                               */
      ISHARETCP_SENDINGFILE,            /* 12 Sending file                                  */

      ISHARETCP_UNKNOWN
   };

   enum ISHARE_STATUS_CODES
   {
      ISHARE_OK,		        /* 0  Ok                                            */
      ISHARE_STATUS_CANTSEARCH,		/* 1  Sorry, I can't search files                   */
      ISHARE_STATUS_BANNED,	        /* 2  You're banned		                    */
      ISHARE_STATUS_MALFORMED,		/* 3  Malformed packet		                    */
      ISHARE_STATUS_DENIED,	        /* 4  Acess denied		                    */
      ISHARE_STATUS_UNKNOWN,	        /* 5  Unknown request		                    */
      ISHARE_STATUS_SHUTDOWN,		/* 6  Shuting down		                    */
      ISHARE_STATUS_BADVERSION,		/* 7  Version mismatch		                    */
      ISHARE_STATUS_ALREADYON,		/* 8  Already logged		                    */
      ISHARE_STATUS_BUSY,	        /* 9  I'm busy, try again later	                    */
      ISHARE_STATUS_UNAVAILABLE,        /* 10 Unavailable		                    */
      ISHARE_STATUS_BADSEARCH,		/* 11 Invalid search                                */
      ISHARE_STATUS_SEARCHFAILED,       /* 12 Search failed                                 */
      ISHARE_STATUS_TOOMANYSENDS,       /* 13 Too many uploads, try again later             */
      ISHARE_STATUS_INVALIDREQUEST,     /* 14 Invalid file transfer request                 */
      ISHARE_STATUS_INVALIDMD5,         /* 15 Invalid file transfer request (MD5 mismatch)  */
      ISHARE_STATUS_FILEERROR,          /* 16 File acess error                              */

      ISHARE_STATUS_UNDEFINED
   };


   #define ISHARE_FLAG_ICQ	0x00000001
   #define ISHARE_FLAG_SERVER	0x00000002

/*---[ Common descriptors ]------------------------------------------------------------------------------*/

   typedef struct _ishare_status
   {
      USHORT            sz;
      USHORT		sendCounter;
      USHORT		recvCounter;

   } ISHARE_STATUS;

   typedef struct _isharecommon_filereq
   {
      USHORT		fileID;			/* File sequence				*/
      UCHAR		md5[16];		/* File MD5 checksum				*/
   } ISHARE_COMMON_FILEREQUEST;

/*---[ UDP Protocol descriptors ]------------------------------------------------------------------------*/

   typedef struct _ishareudp_prefix
   {
      USHORT		crc;			/* Packet CRC					*/
      UCHAR		type;			/* Packet type					*/
      USHORT		sequence;		/* Packet sequence (0 means No Ack)		*/
   } ISHARE_UDP_PREFIX;

   typedef struct _ishare_hello
   {
      ISHARE_UDP_PREFIX	h;
      ULONG             build;			/* My Build level           */
      ULONG		        flags;			/* My flags					*/
      ULONG		        uin;			/* My UIN					*/
      USHORT	        peerPort;		/* My Peer port             */
      USHORT            peerVersion;    /* My Peer Version          */
	  long				localIP;        /* My Local IP Address      */
      UCHAR		        nickname[1];	/* My nickname				*/
   } ISHARE_UDP_HELLO;

   typedef struct _ishare_helloack
   {
      ISHARE_UDP_PREFIX	h;
      ULONG             build;		    /* My Build level                               */
      ULONG				flags;          /* My flags					*/
      ULONG				uin;	        /* My UIN					*/
      USHORT			peerPort;	    /* My Peer port             */
      USHORT            peerVersion;    /* My Peer Version          */
	  long				localIP;        /* My Local IP Address      */
      UCHAR				nickname[1];    /* My nickname				*/
   } ISHARE_UDP_HELLOACK;

   typedef struct _ishare_keepalive
   {
      ISHARE_UDP_PREFIX	h;
      ULONG		uin;			/* My UIN					*/
   } ISHARE_UDP_KEEPALIVE;

   typedef struct _ishare_listreq
   {
      ISHARE_UDP_PREFIX	h;
      USHORT		listRequest;		/* How much ips I'm requesting			*/
   } ISHARE_UDP_LISTREQ;


   typedef struct _ishare_reqack
   {
      ISHARE_UDP_PREFIX	h;
      USHORT		ipCount;		/* How many IP's in this response packet	*/
      long		ip[1];			/* IP List					*/
   } ISHARE_UDP_REQACK;

   typedef struct _ishare_search
   {
      ISHARE_UDP_PREFIX	h;
      USHORT		id;			/* Search ID					*/
      USHORT		hops;			/* Maximum hops					*/
      long		userIP;			/* IP from person who's searching the file	*/
      USHORT		userPort;		/* Port from person who's searching the file	*/	
      UCHAR		filename[1];		/* File name requested				*/
   } ISHARE_UDP_SEARCH;

   typedef struct _ishare_searchmd5
   {
      ISHARE_UDP_PREFIX	h;
      USHORT		id;			/* Search ID					*/
      USHORT		hops;			/* Maximum hops					*/
      long		userIP;			/* IP from person who's searching the file	*/
      USHORT		userPort;		/* Port from person who's searching the file	*/	
      UCHAR		md5[16];		/* MD5 requested                                */
   } ISHARE_UDP_SEARCHMD5;

   typedef struct _ishare_found
   {
      ISHARE_UDP_PREFIX	h;
      USHORT		id;			/* Search ID					*/
      ULONG		size;			/* File size					*/
      UCHAR		md5[16];		/* File MD5 checksum				*/
      ULONG		uin;			/* The user UIN (Can be 0)			*/
      USHORT		fileID;			/* File sequence				*/
      UCHAR		filename[1];		/* The filename found				*/
   } ISHARE_UDP_FOUND;

   typedef struct _ishare_error
   {
      ISHARE_UDP_PREFIX	h;
      USHORT		id;			/* Search ID (If necessary)			*/
      USHORT		error;			/* Error code					*/
   } ISHARE_UDP_ERROR;

   typedef struct _ishare_message
   {
      ISHARE_UDP_PREFIX	h;
      ULONG		        id;			/* Message id					*/
      ULONG		        from;		/* Sender UIN					*/
      USHORT		    msgType;	/* Message type					*/
      UCHAR		        text[1];	/* Message text					*/
   } ISHARE_UDP_MESSAGE;

   typedef struct _ishare_msgack
   {
      ISHARE_UDP_PREFIX	h;
      ULONG		        id;			/* Message id					*/
   } ISHARE_UDP_MSGACK;

/*---[ TCP protocol descriptors ]------------------------------------------------------------------------*/

   typedef struct _isharetcp_prefix
   {
      USHORT   marker;
      UCHAR	id;
   } ISHARE_TCP_PREFIX;
	
   typedef struct _isharetcp_error
   {
      ISHARE_TCP_PREFIX  h;
      USHORT             rc;
   } ISHARE_TCP_ERROR;
   	
   typedef struct _isharetcp_searchstatus
   {
      ISHARE_TCP_PREFIX  h;
      USHORT	         id;
      USHORT             status; // 0 = Begin 1 = End
   } ISHARE_TCP_SEARCHSTATUS;


   typedef struct _isharetcp_sending
   {
      ULONG              identifier;
      USHORT             speed;
      ULONG              offset;
      ULONG              bytes;
      UCHAR		 md5[16];
   } ISHARE_TCP_SENDINGFILE;

   typedef struct _isharetcp_filereq
   {
      ISHARE_COMMON_FILEREQUEST  r;
      USHORT		 speed;
      USHORT		 mtu;
      ULONG		 start;
      ULONG		 end;
   } ISHARE_TCP_FILEREQ;


   #pragma pack()

#endif

