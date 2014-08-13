/*
 * TLV2711.C
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>

 #include "icqv8.h"

/*---[ Structs ]----------------------------------------------------------------------------------------------*/

 #pragma pack(1)
 struct cmd
 {
    UCHAR msgFlag;
    UCHAR msgType;
    int   ( * _System proc)(HICQ, ULONG, USHORT, struct tlv2711 *);
 };

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

/*---[ Static functions ]-------------------------------------------------------------------------------------*/

 static int _System 	regular(HICQ, ULONG, USHORT, struct tlv2711 *);
// static int _System 	sms(HICQ, ULONG, USHORT, struct tlv2711 *);
 static int _System 	reqAway(HICQ, ULONG, USHORT, struct tlv2711 *);

 static int		ipMsg(HICQ,char *);

/*---[ Message processors ]-----------------------------------------------------------------------------------*/

 static const struct cmd proc[] = {	{ 0x00, MSG_NORMAL,	regular	                },
 					{ 0x00, MSG_GREET,	icqv8_srvRecGreet	},
 					{ 0x03, 0xe8,	        reqAway      	        },
 					{ 0x03, 0xe9,	        reqAway	                },
 					{ 0x03, 0xea,	        reqAway	                },
 					{ 0x03, 0xeb,	        reqAway	                },
 					{ 0x03, 0xec,	        reqAway	                },
 					{ 0,    0,		NULL	                }
 				  };



/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 int icqv8_procTLV2711(HICQ icq, ULONG uin, USHORT sz, struct tlv2711 *h, char *linha)
 {
    HUSER 		usr   = icqQueryUserHandle(icq,uin);
    int   		bytes = sz;
    const struct cmd 	*cmd;

    DBGTrace(uin);

    if(!usr && icqLoadValue(icq,"cacheOnSpecial",0))
    {
#ifdef EXTENDED_LOG
       icqWriteSysLog(icq,PROJECT,"Can't find user, caching it");
#endif
       usr = icqAddUserInCache(icq, uin);
    }

    if(sz < sizeof(struct tlv2711))
    {
       if(sz == 27 && *( (ULONG *) h) == uin)
          return ipMsg(icq,(char *) h);

#ifdef EXTENDED_LOG
       icqDumpPacket(icq, usr, "Unexpected size in TLV 0x2711, rejected", sz, (unsigned char *) h);
#else
       icqWriteSysLog(icq,PROJECT,"Unexpected message size, rejected");
#endif
       return -1;
    }

    DBGTracex(h->msgFlag);

    if(h->msgFlag != 0x1b)
    {
#ifdef EXTENDED_LOG
       icqDumpPacket(icq, usr, "Unexpected TLV 0x2711 (Unexpected flag)", sz, (unsigned char *) h);
#else
       icqWriteSysLog(icq,PROJECT,"Unexpected TLV 0x2711 (Unexpected flag)");
#endif
       return -1;
    }

    if(usr)
    {
       DBGTrace(usr->peerVersion);
       usr->peerVersion = h->tcpVer;
       icqRegisterUserAction(usr);
    }

    bytes  -= sizeof(struct tlv2711);

#ifdef EXTENDED_LOG

    sprintf(linha,"ICQ#%lu Sub:%04x Sz:%04x Fl:%02x Tcp:%02x",uin,h->msgType,h->msgSize,h->flags,h->tcpVer);
    icqWriteSysLog(icq,PROJECT,linha);

    switch(h->flags)
    {
    case 0x00:  // = normal
       icqWriteSysLog(icq,PROJECT,"Message Flag: Normal");
       break;

    case 0x80:  // = multiple
       icqWriteSysLog(icq,PROJECT,"Message Flag: Multiple");
       break;

    case 0x03:  //  = special (used for auto-msg-req)
       icqWriteSysLog(icq,PROJECT,"Message Flag: Special");
       break;

    default:
       icqWriteSysLog(icq,PROJECT,"Message Flag: Unknown");
    }

#endif

    for(cmd = proc;cmd->proc;cmd++)
    {
       if(cmd->msgType == h->msgType && cmd->msgFlag == h->flags)
          return cmd->proc(icq, uin, bytes, h);
    }

    sprintf(linha,"Unexpected message 0x%02x - 0x%02x in TLV2711, rejected", (int) h->flags, (int) h->msgType);
    icqDumpPacket(icq, usr, linha, sz, (unsigned char *) h);

    return -1;
 }

 static int _System regular(HICQ icq, ULONG uin, USHORT bytes, struct tlv2711 *h)
 {
    char *pkt  = (char *) (h+1);

    DBGTracex(h->msgType);
    DBGTrace(h->msgSize);
    DBGTrace(h->seq1);
    DBGTrace(h->seq2);

    DBGTrace(bytes);

    if(h->msgSize > bytes)
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected message size, rejected");
       return -1;
    }

    DBGTrace(h->msgSize);
    DBGTrace(bytes);

    if(h->msgSize > 1)
    {
       *(pkt+h->msgSize) = 0;
       DBGMessage(pkt);
       icqInsertRTFMessage(icq, uin, 0, h->msgType, 0, 0, pkt);
    }
#ifdef EXTENDED_LOG
    else
    {
       icqWriteSysLog(icq,PROJECT,"Message without text");
    }
#endif

    CHKPoint();

    return 0;

 }

/*
 static int _System sms(HICQ icq, ULONG uin, USHORT bytes, struct tlv2711 *pkt)
 {
    int    sz 		= bytes;
//    USHORT type;

    #pragma pack(1)

    struct _header
    {
       UCHAR x1;	// 00
       ULONG x2;	// 40 00 37 1c
       ULONG x3;	// 58 72 e9 87
       ULONG x4;	// d4 11 a4 c1
       ULONG x5;	// 00 d0 b7 59
       ULONG x6;	// b1 d9 00 00
       ULONG x7;	// 1b 00
       ULONG x8;	// 00 00
    }	*h	= (struct _header *) (pkt+1);

    struct _prefix
    {
       UCHAR  x1;	// 00
       ULONG  x2;	// 00 00 00 00
       USHORT x3;	// 01 00
       ULONG  x4;	// 00 00 00 00
       ULONG  x5;	// 00 00 00 00
       USHORT szBase;   // 78 00
       USHORT x6;	// 00 00
       USHORT msgSize;  // 74 00
       USHORT x7;       // 00 00
    }  *p	= (struct _prefix *) (((char *) (h+1)) + 23 );

    sz -= sizeof(struct header *);

    DBGTrace(50-sizeof(struct _header));

    if(sz < 28)
    {
       icqDumpPacket(icq, 0, "Unexpected header size in SMS Message", bytes, (unsigned char *) (pkt+1));
       return -1;
    }

    if( h->x1 != 00000000 || h->x2 != 0x1c370040
    			  || h->x3 != 0x87e97258
    			  || h->x4 != 0xc1a411d4
    			  || h->x5 != 0x59b7d000
    			  || h->x6 != 0x0000d9b1
    			  || h->x7 != 0x001b
    			  || h->x8 != 0x646e6553 )
    {    			
       icqDumpPacket(icq, 0, "Unexpected header values in SMS Message", bytes, (unsigned char *) (pkt+1));
    }			

    sz -= 23;

    if(sz < sizeof(struct _prefix))
    {
       icqDumpPacket(icq, 0, "Unexpected prefix size in SMS message", bytes, (unsigned char *) (pkt+1));
       return -1;
    }

    sz -= sizeof(struct _prefix);

    if(sz < 0 || sz < p->msgSize)
    {
       icqDumpPacket(icq, 0, "Unexpected message size in SMS Message", bytes, (unsigned char *) (pkt+1));
       return -1;
    }

    DBGTrace(sz);
    DBGTrace(p->msgSize);

    return icqInsertMessage(icq, uin, 0, MSG_URL, 0, 0, p->msgSize, (unsigned char *) (p+1));
 }
*/

 static int ipMsg(HICQ icq, char *pkt)
 {
    #pragma pack(1)
    struct _header
    {
       ULONG 	uin;
       ULONG	ip;
       USHORT	port;
       USHORT	x1;
       ULONG	x2;
       UCHAR	x3;
       USHORT   port2;
       ULONG	x4;
       ULONG	sequence;	// Sequence
    } *h = (struct _header *) pkt;

#ifdef EXTENDED_LOG

    char buffer[0x0100];

    sprintf(buffer,"IP from %s X1=%04x x2=%08lx X3=%02x X4=%08lx Sq=%08lx", icqQueryUserNick(icqQueryUserHandle(icq,h->uin)),(int) h->x1,h->x2,(int) h->x3,h->x4,h->sequence);
    icqWriteSysLog(icq,PROJECT,buffer);
//    icqDumpPacket(icq, icqQueryUserHandle(icq,h->uin), buffer, sizeof(struct _header), (unsigned char *) pkt);

#endif

    icqSetUserIPandPort(icq,h->uin,0,h->ip,h->port);

    return 0;
 }

 static int _System reqAway(HICQ icq, ULONG uin, USHORT bytes, struct tlv2711 *h)
 {
/*
   0xe8 Away,
   0xe9 Occupied,
   0xea NA,
   0xeb DND,
   0xec Free for chat
*/
#ifdef EXTENDED_LOG
    char logBuffer[0x0100];
    sprintf(logBuffer,"%s (ICQ#%ld) is requesting away message 0xg%02x",icqQueryUserNick(icqQueryUserHandle(icq,uin)),uin,(int) h->msgType);
    icqWriteSysLog(icq,PROJECT,logBuffer);
#endif

    /* Nesse ponto: Responder ao pedido */

    return 0;
 }

