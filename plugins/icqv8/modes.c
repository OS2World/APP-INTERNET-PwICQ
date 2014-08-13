/*
 * MODES.C - Administracao de modos
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>

 #include "icqv8.h"

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 #pragma pack(1)
 struct userMode
 {
    USHORT      mode;
    USHORT      icon;
 };

 USHORT _System icqv8_queryModeIcon(HICQ icq, ULONG mode)
 {
    const struct userMode        *ptr;
    const static struct userMode tbl[]          = {     { 0x0000,  5 },     // Online
                                                        { 0x0008, 41 },     // Aska.ru
                                                        { 0x0001,  6 },     // Away
                                                        { 0x0005, 40 },     // N/A
                                                        { 0x0004, 40 },     // NA99A
                                                        { 0x0010, 12 },     // Occupied (MAC)
                                                        { 0x0011, 12 },     // Occupied
                                                        { 0x0013, 42 },     // DND
                                                        { 0x0002, 42 },     // DND (LICQ)
                                                        { 0x0020, 13 },     // Free for chat
                                                        { 0x0100, 14 },     // Invisible
                                                        { 0xFFFF,  7 },     // Offline
                                                        { 0x0000,  0 }
                                                 };

    mode &= MODE_MASC;
    for(ptr = tbl; ptr->icon && ptr->mode != mode; ptr++);
    return ptr->icon ? ptr->icon : 5;
 }

 ULONG  _System icqv8_setMode(HICQ icq, ULONG current, ULONG last)
 {
    ICQV8 *cfg = icqGetPluginDataBlock(icq, module);
    HUSER usr;

    current &= MODE_MASC;
    last    &= MODE_MASC;

    DBGTracex(current);

    icqWalkUsers(icq,usr)
    {
       usr->c2sFlags &= ~USRV8_STATUSSENT;
    }

    /* Ajusta as flags de acordo com o arquivo de configuracao */
    if(current == ICQ_OFFLINE)
    {
       cfg->status = 0;
    }
    else if(cfg->thread < 1)
    {
      DBGTracex(cfg);
      cfg->thread = icqCreateThread(icq, icqv8_mainThread, 16384, 0, cfg, "c2s");
    }
    else if(cfg->sock < 0)
    {
       icqWriteSysLog(icq,PROJECT,"Stopping service thread");
       cfg->status = 0;
    }
    else
    {
       icqv8_sendModePacket(icq, cfg, current);
    }
    return current;

 }

/*

CLI_SETSTATUS

stricq@owlnet.net
CLI_SETSTATUS
Channel: 2 SNAC(1,1E)
  This sets the clients online status code and some other direct client to client information as well.

Parameters
Length/TLV Content (If Fixed) Designations Description
TLV06 00 06 00 04 xx xx xx xx STATUS This sets the client's online status and some flags:
00xxxxxx = allow direct connection with everyone
10xxxxxx = allow direct connection upon authorization
20xxxxxx = allow direct connection with users in contact list
xx02xxxx = show IP
xx03xxxx = webaware
xxxx0000 = online
xxxx0100 = invisible
xxxx0001 = away
xxxx0005 = NA
xxxx0011 = Occupied
xxxx0013 = DND
xxxx0020 = Free for chat
TLV08 00 08 00 02 00 00 ERROR Error code?
TLV0C 00 0C 00 25 CLI2CLI The TLV contains several sections broken out as follows.
4 xx xx xx xx IPINTERNAL The client computer's local IP address.
4 00 00 xx xx PORT This is the port to connect with when making client to client connections.
1 xx TCPFLAG 01 = Firewall (or HTTPS proxy)
02 = SOCKS4/5 proxy
04 = 'normal' connection
2 00 08 TCPVERSION The highest client to client protocol version this client uses.
4 xx xx xx xx COOKIE Probably a direct client to client connection cookie.
2 00 00 Unknown 0, Unknown
2 00 50 Unknown 80, Unknown
2 00 00 Unknown 0, Unknown
2 00 03 COUNT Three items follow.
4 xx xx xx xx TIME A time(NULL) value.
4 xx xx xx xx TIME A time(NULL) value.
4 xx xx xx xx TIME A time(NULL) value.
2 00 00 Unknown 0, Unknown


2A 02 14 BF 00 37                   Flap: Channel 2, Sequence 5311, Length 55
   00 01 00 1E 00 00 00 00 00 00    Snac: Type x01/x1E, ID x0000, Flags 0
      00 06 00 04 20 02 00 00       Status: Direct connect=List, Show IP, Online
      00 08 00 02 00 00             Error code
      00 0C 00 25 C0 A8 01 02       192.168.1.2, Internal IP
                  00 00 2E 44       11844, Port
                  01                 1, TCP Flag: Firewall
                  00 08              8, Protocol version: ICQ2001=v8
                  89 9F CD A0       Direct connect cookie
                  00 00              0, Unknown
                  00 50             80, Unknown
                  00 00              0, Unknown
                  00 03             Count: 3
                     3B A8 DB AF    time(NULL): Wed Sep 19 13:53:51 2001
                     3B EB 53 73    time(NULL): Thu Nov 08 22:54:27 2001
                     3B EB 52 62    time(NULL): Thu Nov 08 22:49:54 2001
                  00 00              0, Unknown

*/


 void icqv8_sendModePacket(HICQ icq, ICQV8 *cfg, ULONG mode)
 {
#ifdef EXTENDED_LOG
    char        logLine[0x0100];
#endif

    C2CINFO	info;
    ULONG       cookie  = (rand() & 0xFFFF);
    PACKET      pkt;

    cookie <<= 16;
    cookie |= (rand() & 0xFFFF);

    icqSetPeerCookie(icq,8,cookie);

    mode &= MODE_MASC;

    DBGTracex(mode);

    if(mode == ICQ_OFFLINE)
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected set mode request");
       icqAbend(icq,0);
       return;
    }

    pkt = icqv8_allocatePacket(icq,cfg,70);

    DBGTracex(mode);

    if(icqLoadValue(icq,"ip:request",0))
       mode |= ICQ_DIRECTWITHAUT;

    if(icqLoadValue(icq,"ip:contact",1))
       mode |= ICQ_DIRECTINCONTACT;

    if(icqLoadValue(icq,"ip:show",1))
       mode |= ICQ_SHOWIPADDRESS;

    if(icqLoadValue(icq,"webAware",0))
       mode |= ICQ_WEBAWARE;

    DBGTracex(mode);

    icqv8_insertLong(pkt, 0x06, mode);
    icqv8_insertShort(pkt, 0x08, 0);

    memset(&info,0,sizeof(info));
    info.ipAddress      = icqGetHostID();
    info.c2cPort        = LONG_VALUE(icqQueryPeerPort(icq));
    info.tcpFlag        = 0x04;
    info.tcpVersion     = 0x0800;
    info.cookie         = LONG_VALUE(cookie);
//    info.x1           =;
    info.x2             = 0x5000;
//    info.x3           =;
    info.counter        = 0x0300;
    info.t1             = LONG_VALUE(0x3BA8DBAF);
    info.t2             = LONG_VALUE(0x3BEB5373);
    info.t3             = LONG_VALUE(0x3BEB5262);
//    info.x4           =;

#ifdef EXTENDED_LOG
    sprintf(logLine,"Online mode changed to %08lx (listening on %d)", mode,(int) LONG_VALUE(info.c2cPort));
    icqWriteSysLog(icq,PROJECT,logLine);
#endif

    DBGTracex(info.c2cPort);

    icqv8_insertBlock(pkt, 0x0c, sizeof(info), (const unsigned char *) &info);

    icqv8_sendSnac(icq, cfg, 0x01, 0x1E, 0, pkt);

    DBGTracex(cookie);

 }



