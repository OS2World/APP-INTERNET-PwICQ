/*
 * SNAC3.C
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>

 #include "icqv8.h"

/*---[ Defines ]----------------------------------------------------------------------------------------------*/


/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static int _System srv_replybuddy(HICQ, ICQV8 *, int, ULONG, void *);
 static int _System srv_useronline(HICQ, ICQV8 *, int, ULONG, const UCHAR *);
 static int _System srv_useroffline(HICQ, ICQV8 *, int, ULONG, const UCHAR *);


/*---[ SNAC processors ]--------------------------------------------------------------------------------------*/

 const SNAC_CMD icqv8_snac03_table[] =
 {
    { 0x03,   SNAC_CMD_PREFIX   srv_replybuddy        },
    { 0x0B,   SNAC_CMD_PREFIX   srv_useronline        },
    { 0x0C,   SNAC_CMD_PREFIX   srv_useroffline        },

    { 0x00,                     NULL                  }
 };


/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 static int _System srv_replybuddy(HICQ icq, ICQV8 *cfg, int sz, ULONG req, void * ptr)
 {
    return 0;
 }

 static int _System srv_useroffline(HICQ icq, ICQV8 *cfg, int sz, ULONG req, const UCHAR * pkt)
 {
    char                buffer[0x0101];

    strncpy(buffer,pkt+1,*pkt);
    *(buffer+*pkt) = 0;

/*
1  xx  STRLEN  Length of the uin in ASCII.    Varies  xx ..  UIN  The uin that is going offline.    10  00 00 00 01 00 01 00 02 00 00  UNKNOWN  Seems to be fixed.
08/21/2002 13:33:02 icqkrnl    Unknown SNAC packet (0x0003/0x000c - 3/12) (29 bytes)
03 00 0c 00 00 00 93 82 b6 25 08 32 37 32 34 31   ......“‚¶%.27241
32 33 34 00 00 00 01 00 01 00 02 00 00            234..........
*/

    DBGMessage(buffer);
    icqSetUserOnlineMode(icq, atol(buffer), ICQ_OFFLINE);

    return 0;

 }

 static int _System srv_useronline(HICQ icq, ICQV8 *cfg, int sz, ULONG req, const UCHAR * pkt)
 {
    char                buffer[0x0100];
    short               qtd;
    TLV                 tlv;
    const C2CINFO       *info           = NULL;
    ULONG               userMode        = 0;
    long                ipAddress	    = 0;
    long      	        memberSince     = 0;
    long                onlineSince     = 0;
    HUSER				usr		        = NULL;
    ULONG				uin;

//    icqDumpPacket(icq, NULL, "User is online packet received", sz, (unsigned char *) pkt);

    strncpy(buffer,(pkt+1),*pkt);
    *(buffer+*pkt) = 0;

    DBGMessage(buffer);

    uin = atoi(buffer);

    DBGPrint("Usuario %ld online",uin);

    if(!uin)
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected UIN in user online packet");
       return -1;
    }

    sz  -= *pkt;
    pkt += *pkt;

    pkt += 3;
    sz  -= 3;

    qtd  = SHORT_VALUE(*((USHORT *) pkt ));
    pkt += 2;
    sz  -= 2;

    DBGTrace(qtd);

    while( qtd-- && (pkt = icqv8_getTLV(icq, cfg, &tlv, &sz, pkt)) != NULL)
    {
       switch(tlv.id)
       {
       case 0x01: // TLV01 00 01 00 02 xx xx UNKNOWN Unknown - appears to be 00 50 if the user is 'online' and 00 70 for 'away'. I haven't (yet) seen any other states.
          break;

       case 0x0C: // TLV0C 00 0C 00 25 CLI2CLI Direct connection info, the contents of this TLV are as follows.
          info = (const C2CINFO *) tlv.data;
          break;

       case 0x0A: // TLV0A 00 0A 00 04 xx xx xx xx IPEXTERNAL The LAN external IP address of this contact.
          ipAddress = *( (long *) tlv.data );
          break;

       case 0x06: // TLV06 00 06 00 04 xx xx xx xx STATUS The online status of this contact.
          userMode = LONG_VALUE(*( (ULONG *) tlv.data ));
          break;

       case 0x0D: // TLV0D 00 0D xx xx .. CAPABILITIES The capabilities of this client.
          break;

       case 0x0F: // TLV0F 00 0F 00 04 xx xx xx xx UNKNOWN Unknown
          break;

       case 0x02: // TLV02 00 02 00 04 xx xx xx xx TIME A time(NULL) value. Member of ICQ since this time.
          memberSince     = LONG_VALUE(*( (ULONG *) tlv.data ));
          break;

       case 0x03: // TLV03 00 03 00 04 xx xx xx xx TIME A time(NULL) value. Online since this time.
          onlineSince     = LONG_VALUE(*( (ULONG *) tlv.data ));
          break;

       case 0x04: // TLV04 00 04 00 02 01 d1 UNKNOWN Unknown.
          break;

       case 0x11: // TLV11 00 11 00 22 xx xx ... SECURITYSETTINGS List of numbers; nonsensical at first glance. Occured when user altered his 'let other people see my mobile phone number' etc. settings.
          break;

       default:
          icqWriteSysLog(icq,PROJECT,"Unexpected TLV in user online packet");
       }
    }

    DBGTrace(qtd);
    DBGTrace(sz);
    DBGTracex(userMode);
    DBGTracex(info->tcpVersion);
    DBGTracex(info->t1);
    DBGTracex(info->t2);
    DBGTracex(info->t3);

    if(info)
    {
#ifdef EXTENDED_LOG
       sprintf(buffer,"%s (ICQ#%ld) T1:%08lx T2:%08lx T3:%08lx P:%08lx TFLAG:%02x Cookie: %08lx",icqQueryUserNick(icqQueryUserHandle(icq,uin)),uin,info->t1,info->t2,info->t3,LONG_VALUE(info->c2cPort),(int) info->tcpFlag,info->cookie);
       icqWriteSysLog(icq,PROJECT,buffer);
#endif
       usr = icqUserOnline(     icq,
                                uin,
                                userMode,
                                info->ipAddress,
                                ipAddress,
                                LONG_VALUE(info->c2cPort),
                                SHORT_VALUE(info->tcpVersion), // TCP Version
                                info->t1 );                    // Client Version
       if(usr)
          usr->peerCookie = LONG_VALUE(info->cookie);
    }
    else
    {
       usr = icqUserOnline(     icq,
                                uin,
                                userMode,
                                0,
                                ipAddress,
                                0,
                                0,
                                0 );
    	
    }

    if(usr)
    {
       if(memberSince)
          usr->memberSince = memberSince;
       if(onlineSince)
          usr->onlineSince = onlineSince;
       else
          usr->onlineSince = time(NULL);

       /* Check for Hello message */

       if( !(usr->flags & (USRF_TEMPORARY|USRF_CACHED)) && (usr->peerVersion > 7) && !(usr->c2sFlags&USRV8_STATUSSENT))
       {
          usr->c2sFlags |= USRV8_SENDSTATUSMSG;
          cfg->flags    |= ICQV8_FLAG_SENDSTATUSMSG;
       }

    }

/*
SRV_USERONLINE
Channel: 2 SNAC(3,B)
  This SNAC is sent whenever a contact in your contact list goes online.
Parameters
Length/TLV Content (If Fixed) Designations Description
1 xx STRLEN The length of the following UIN.
Varies xx .. UIN The UIN of the contact that is coming online in ASCII format.
2 00 00 UNKNOWN Unknown.

2 xx xx COUNT The total number of following TLVs.
TLV01 00 01 00 02 xx xx UNKNOWN Unknown - appears to be 00 50 if the user is 'online' and 00 70 for 'away'. I haven't (yet) seen any other states.

TLV0C 00 0C 00 25 CLI2CLI Direct connection info, the contents of this TLV are as follows.

   C2CINFO!
4 xx xx xx xx IPINTERNAL The LAN internal IP address of the contact.
4 00 00 xx xx PORT The port to use in order to establish a direct connection.
1 xx TCPFLAG 01 = Firewall (or HTTPS proxy)
02 = SOCKS4/5 proxy
04 = 'normal' connection
2 xx xx TCPVERSION The highest client to client protocol version this client uses.
4 xx xx xx xx COOKIE This cookie is needed when establishing a direct client to client connection.
4 00 00 00 50 UNKNOWN Unknown
4 00 00 00 03 UNKNOWN Unknown
4 xx xx xx xx TIME A time(NULL) value. Usually a fairly recent time value.
4 xx xx xx xx TIME A time(NULL) value.
4 xx xx xx xx TIME A time(NULL) value.
2 00 00 UNKNOWN Unknown


TLV0A 00 0A 00 04 xx xx xx xx IPEXTERNAL The LAN external IP address of this contact.
TLV06 00 06 00 04 xx xx xx xx STATUS The online status of this contact.
TLV0D 00 0D xx xx .. CAPABILITIES The capabilities of this client.
TLV0F 00 0F 00 04 xx xx xx xx UNKNOWN Unknown
TLV02 00 02 00 04 xx xx xx xx TIME A time(NULL) value. Member of ICQ since this time.
TLV03 00 03 00 04 xx xx xx xx TIME A time(NULL) value. Online since this time.
TLV04 00 04 00 02 01 d1 UNKNOWN Unknown.
TLV11 00 11 00 22 xx xx ... SECURITYSETTINGS List of numbers; nonsensical at first glance. Occured when user altered his 'let other people see my mobile phone number' etc. settings.


*/

    return 0;
 }

/*

19/07/2002 17:09:26 icqkrnl    User is online packet received (128 bytes)
08 32 37 32 34 31 32 33 34 00 00 00 07 00 01 00   .27241234.......
02 00 70 00 0c 00 25 00 00 00 00 00 00 00 00 04   ..p...%.........
00 07 71 7a 59 30 00 00 00 50 00 00 00 03 3b 17   ..qzY0...P....;.
6b 57 3b 18 ac e9 3b 17 6b 4e 00 00 00 0a 00 04   kW;.¬é;.kN......
00 00 00 00 00 06 00 04 30 01 00 13 00 0d 00 20   ........0......
09 46 13 49 4c 7f 11 d1 82 22 44 45 53 54 00 00   .F.IL.Ñ‚"DEST..
09 46 13 44 4c 7f 11 d1 82 22 44 45 53 54 00 00   .F.DL.Ñ‚"DEST..
00 0f 00 04 00 00 68 ff 00 03 00 04 3d 38 08 f7   ......hÿ....=8.÷

        08
        32 37 32 34
        31 32 33 34

        00 00                   Unknown
        00 07                   07 TLVs

        00 01                   TLV-01
        00 02
        00 70

        00 0c                   TLV-0C
        00 25
        00 00 00 00 00
        00 00 00 04 00
        07 71 7a 59 30
        00 00 00 50 00
        00 00 03 3b 17
        6b 57 3b 18 ac
        e9 3b 17 6b 4e
        00 00

        00 0a                   TLV-0A
        00 04
        00 00 00 00

        00 06                   TLV-06
        00 04
        30 01 00 13
                                TLV-0D
        00 0d
        00 20
        09 46 13 49 4c
        7f 11 d1 82 22
        44 45 53 54 00
        00 09 46 13 44
        4c 7f 11 d1 82
        22 44 45 53 54
        00 00

        00 0f                   TLV-0F
        00 04
        00 00 68 ff

        00 03                   TLV-03
        00 04
        3d 38 08 f7



*/

