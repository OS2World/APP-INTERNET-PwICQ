/*
 * CHANNEL1.C
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <malloc.h>

 #include "icqv8.h"

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static void sendLoginPacket(HICQ, ICQV8 *);

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 void icqv8_processChannel1(HICQ icq, ICQV8 *cfg, int sz, void *buffer)
 {
    CHKPoint();

    if( *((ULONG *) buffer) != 0x01000000)
    {
       icqDumpPacket(icq, NULL, "Packet Channel 1 received", sz, (unsigned char *) buffer);
       return;
    }

    if(cfg->status == ICQV8_STATUS_LOGIN)
    {
       sendLoginPacket(icq, cfg);
    }
    else
    {
       icqv8_sendFlap(icq, cfg, CHANNEL_NEW, cfg->wrkPacket);
       cfg->wrkPacket = 0xFFFF;
    }

 }

 static void sendLoginPacket(HICQ icq, ICQV8 *cfg)
 {
    static const UCHAR pwdKey[] =       { 0xF3, 0x26, 0x81, 0xC4, 0x39, 0x86,
                                          0xDB, 0x92, 0x71, 0xA3, 0xB9, 0xE6,
                                          0x53, 0x7A, 0x95, 0x7C
                                        };


    UCHAR       buffer[0x0100];
    int         f,p;
    PACKET      login                   = icqv8_allocatePacket(icq,cfg,0x0100);

    icqv8_insertLong(login, 0, 1);

    sprintf(buffer,"%ld",icqQueryUIN(icq));
    icqv8_insertString(login,1,buffer);

    icqQueryPassword(icq,buffer);
    DBGMessage(buffer);										

    p = strlen(buffer);

    for(f=0;f<p;f++)
       buffer[f] ^= pwdKey[f];

    icqv8_insertBlock(login, 2, p, buffer);

    icqLoadString(icq,  "v8.ClientVersion",
                        "ICQ Inc. - Product of ICQ (TM).2001b.5.15.1.3638",
                        buffer,
                        0xFF);

    icqv8_insertString(login,3,buffer);

    icqv8_insertShort(login, 0x16, 0x010A);

    icqv8_insertShort(login, 0x17, VER_MAJOR);
    icqv8_insertShort(login, 0x18, VER_MINOR);
    icqv8_insertShort(login, 0x19, VER_LESSER);
    icqv8_insertShort(login, 0x1A, VER_BUILD);
    icqv8_insertLong( login, 0x14, VER_SUBBUILD);

    icqLoadString(icq,  "v8.Language", "en", buffer, 0xFF);
    icqv8_insertString(login,0x0F,buffer);

    icqLoadString(icq,  "v8.Country",  "us", buffer, 0xFF);
    icqv8_insertString(login,0x0E,buffer);

    icqv8_sendFlap(icq, cfg, CHANNEL_NEW, login);

 }



