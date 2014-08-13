/*
 * login.c - Finaliza processo de login
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>

 #include "icqv8.h"

/*---[ Capabilities ]-----------------------------------------------------------------------------------------*/

/*---[ Constants ]--------------------------------------------------------------------------------------------*/

 static const char ICQ2001[]  =  {      0x09, 0x46, 0x13, 0x49, 0x4C, 0x7F, 0x11, 0xD1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00,
                                        0x97, 0xB1, 0x27, 0x51, 0x24, 0x3C, 0x43, 0x34, 0xAD, 0x22, 0xD6, 0xAB, 0xF7, 0x3F, 0x14, 0x92,
                                        0x2E, 0x7A, 0x64, 0x75, 0xFA, 0xDF, 0x4D, 0xC8, 0x88, 0x6F, 0xEA, 0x35, 0x95, 0xFD, 0xB6, 0xDF,
                                        0x09, 0x46, 0x13, 0x44, 0x4C, 0x7F, 0x11, 0xD1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00
                                 };


 static const char ICQ2002a[] =  {      0x09, 0x46, 0x13, 0x49, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x55, 0x53, 0x54, 0x00, 0x00,
                                        0x09, 0x46, 0x13, 0x4e, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00,
                                        0x97, 0xb1, 0x27, 0x51, 0x24, 0x3c, 0x43, 0x34, 0xad, 0x22, 0xd6, 0xab, 0xf7, 0x3f, 0x14, 0x92,
                                        0x09, 0x46, 0x13, 0x44, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00
                                 };


 static const char ICQ2002b[] =  {      0x09, 0x46, 0x13, 0x49, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00,
                                        0x09, 0x46, 0x13, 0x4e, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00,
                                        0x97, 0xb1, 0x27, 0x51, 0x24, 0x3c, 0x43, 0x34, 0xad, 0x22, 0xd6, 0xab, 0xf7, 0x3f, 0x14, 0x92,
                                        0x09, 0x46, 0x13, 0x44, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00
                                 };


 static const char setICBM[] =   {      0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1F, 0x40,
                                        0x03, 0xE7, 0x03, 0xE7, 0x00, 0x00, 0x00, 0x00
                                 };


 static const char CLI_READY[] = {      0x00, 0x01, 0x00, 0x03, 0x01, 0x10, 0x04, 0x7B,
                                        0x00, 0x13, 0x00, 0x02, 0x01, 0x10, 0x04, 0x7B,
                                        0x00, 0x02, 0x00, 0x01, 0x01, 0x01, 0x04, 0x7B,
                                        0x00, 0x03, 0x00, 0x01, 0x01, 0x10, 0x04, 0x7B,
                                        0x00, 0x15, 0x00, 0x01, 0x01, 0x10, 0x04, 0x7B,
                                        0x00, 0x04, 0x00, 0x01, 0x01, 0x10, 0x04, 0x7B,
                                        0x00, 0x06, 0x00, 0x01, 0x01, 0x10, 0x04, 0x7B,
                                        0x00, 0x09, 0x00, 0x01, 0x01, 0x10, 0x04, 0x7B,
                                        0x00, 0x0A, 0x00, 0x01, 0x01, 0x10, 0x04, 0x7B,
                                        0x00, 0x0B, 0x00, 0x01, 0x01, 0x10, 0x04, 0x7B
                                };


/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static void _System    auxThread(ICQTHREAD *);
 static void            completeLogin(HICQ, ICQV8 *);

/*---[ Implementation ]---------------------------------------------------------------------------------------*/

/*
2A 02 14 BC 00 00                   Flap: Channel 2, Sequence 5308, Length 0
   00 13 00 07 00 00 00 00 00 00
*/

 void icqv8_completeLogin(HICQ icq, ICQV8 *cfg)
 {
    icqCreateThread(icq, auxThread, 16384, 0, cfg, "lgn");
 }

 static void sendBlock(HICQ icq, ICQV8 *cfg, USHORT family, USHORT subType, ULONG request, USHORT tlv, int sz, const char *block)
 {
    PACKET pkt = icqv8_allocatePacket(icq, cfg, sz+sizeof(TLV)+sizeof(SNAC));
    icqv8_insertBlock(pkt, tlv, sz, block);
    icqv8_sendSnac(icq, cfg, family, subType, request, pkt);
 }

 static void _System auxThread(ICQTHREAD *thd)
 {
    DBGMessage("Thread secundaria de login iniciada");
    icqWriteSysLog(thd->icq,PROJECT,"Starting secondary login thread");
    completeLogin(thd->icq,(ICQV8 *) thd->parm);
    icqWriteSysLog(thd->icq,PROJECT,"Secondary login thread finished");
    DBGMessage("Thread secundaria de login terminada");

    ( (ICQV8 *) thd->parm)->flags |= (ICQV8_FLAG_SEND_CONFIRMATIONS);

 }

 static void completeLogin(HICQ icq, ICQV8 *cfg)
 {
    char                        buffer[0x0100];
    const struct xmlRequest     *reqxml = icqv8_xmltable;

    int                 sz;

    #pragma pack(1)

    struct client_id
    {
       USHORT   cmd;            //   BE 0A                      2750, Type: Client identification?
       USHORT   majorVersion;   //   05 00                      Client major version
       USHORT   minorVersion;   //   0F 00                      Client minor version
       USHORT   lesserVersion;  //   01 00                      Client lesser version
       USHORT   clientBuild;    //   36 0E                      Client build
       ULONG    subBuild;       //   55 00 00 00                Client sub-build
       ULONG    x1;             //   90 7C 21 2C                Unknowns
       ULONG    x2;             //   91 4D D3 11
       ULONG    x3;             //   AD EB 00 04
       ULONG    x4;             //   AC 96 AA B2
       USHORT   x5;             //   00 00
       ULONG    x6;             //   03 00 00 00
       USHORT   x7;             //   2C 00
       USHORT   x8;             //   01 00
       USHORT   x9;             //   00 01
       ULONG    x10;            //   00 00 00 00
       ULONG    x11;            //   01 00 00 00
       ULONG    x12;            //   00 00 00 00
       ULONG    x13;            //   00 00 00 00
       ULONG    x14;            //   00 00 00 00
       ULONG    x15;            //   00 00 00 00
       ULONG    x16;            //   00 00 00 00
       ULONG    x17;            //   00 00 00 00
       ULONG    x18;            //   00 00 00 00
       ULONG    x19;            //   00 00 00 00
    } *cli_id   = (struct client_id *) buffer;

    DBGMessage("Terminando processo de login");

    icqv8_sendSingleSnac(icq, cfg, 0x13, 0x07, 0);

    sendBlock(icq,cfg,0x02,0x04,0,5,0x40,ICQ2002b);
    sendBlock(icq,cfg,0x04,0x02,0,0,0x10,setICBM);

    icqv8_sendModePacket(icq,cfg,icqQueryOnlineMode(icq));

    sendBlock(icq,cfg,0x01,0x02,0,0,0x50,CLI_READY);

    cfg->toICQSrvSeq = 2;       // Inicia sequencia em 2

    icqv8_toICQServer(icq, cfg, 0x3c, 0, 0, NULL);

    while( reqxml->key )
    {
       strncpy(buffer+4,"<key>",0xF0);
       strncat(buffer+4,reqxml->key,0xF0);
       strncat(buffer+4,"</key>",0xF0);

//       DBGMessage(buffer+4);
       sz = strlen(buffer+4)+1;

       *( (USHORT *) buffer )     = 2200;
       *( (USHORT *) (buffer+2) ) = sz;

//       DBGMessage(buffer+4);

//       DBGTracex(reqxml->id);
       icqv8_toICQServer(icq, cfg, 2000, reqxml->id, sz+4, buffer);

       reqxml++;
    }

    memset(cli_id,0,sizeof(struct client_id));


    cli_id->cmd                 = 0x0ABE;       //   BE 0A                      2750, Type: Client identification?
    cli_id->majorVersion        = 0x0005;       //   05 00                      Client major version
    cli_id->minorVersion        = 0x000F;       //   0F 00                      Client minor version
    cli_id->lesserVersion       = 0x0001;       //   01 00                      Client lesser version
    cli_id->clientBuild         = 0x0e36;       //   36 0E                      Client build
    cli_id->subBuild            = 0x00000055;   //   55 00 00 00                Client sub-build
    cli_id->x1                  = 0x2c217c90;   //   90 7C 21 2C                Unknowns
    cli_id->x2                  = 0x11d34d91;   //   91 4D D3 11
    cli_id->x3                  = 0x0400EBAD;   //   AD EB 00 04
    cli_id->x4                  = 0xB2AA96AC;   //   AC 96 AA B2
    cli_id->x6                  = 0x00000003;   //   03 00 00 00
    cli_id->x7                  = 0x002C;       //   2C 00
    cli_id->x8                  = 0x0001;       //   01 00
    cli_id->x9                  = 0x0100;       //   00 01
    cli_id->x11                 = 0x00000001;   //   01 00 00 00

    icqv8_toICQServer(icq, cfg, 2000, 0, sizeof(struct client_id), cli_id);

 }



