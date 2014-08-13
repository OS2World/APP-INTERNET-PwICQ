/*
 * SNAC1.C
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>

 #include "icqv8.h"

/*---[ Defines ]----------------------------------------------------------------------------------------------*/


/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/


/*---[ SNAC processors ]--------------------------------------------------------------------------------------*/

 static int _System srv_families(HICQ, ICQV8 *, int, ULONG, USHORT *);
 static int _System srv_families2(HICQ, ICQV8 *, int, ULONG, USHORT *);
 static int _System srv_motd(HICQ, ICQV8 *, int, ULONG, USHORT *);
 static int _System srv_rates(HICQ, ICQV8 *, int, ULONG, void *);
 static int _System srv_replyinfo(HICQ, ICQV8 *, int, ULONG, UCHAR *);

 const SNAC_CMD icqv8_snac01_table[] =
 {
    { 0x03,   SNAC_CMD_PREFIX   srv_families          },
    { 0x18,   SNAC_CMD_PREFIX   srv_families2         },
    { 0x13,   SNAC_CMD_PREFIX   srv_motd              },
    { 0x07,   SNAC_CMD_PREFIX   srv_rates             },
    { 0x0F,   SNAC_CMD_PREFIX   srv_replyinfo         },


    { 0x00,                     NULL                  }
 };


/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 static int _System srv_families(HICQ icq, ICQV8 *cfg, int sz, ULONG req, USHORT *in)
 {
    static const ULONG cli_families[] = {   0x00010003,
                                            0x00130002,
                                            0x00020001,
                                            0x00030001,
                                            0x00150001,
                                            0x00040001,
                                            0x00060001,
                                            0x00090001,
                                            0x000A0001,
                                            0x000B0001
                                        };

    PACKET      pkt     = icqv8_allocatePacket(icq, cfg, 45);
    int         f;

    for(f=0;f<10;f++)
       icqv8_insertLong(pkt, 0, cli_families[f]);

    icqv8_sendSnac(icq, cfg, 0x01, 0x17, 0, pkt);

    return 0;
 }

 static int _System srv_families2(HICQ icq, ICQV8 *cfg, int sz, ULONG req, USHORT *in)
 {
    return 0;
 }

 static int _System srv_motd(HICQ icq, ICQV8 *cfg, int sz, ULONG req, USHORT *in)
 {
    icqv8_sendSingleSnac(icq, cfg, 0x01, 0x06, 0);
    return 0;
 }

 static void _System auxThread(ICQTHREAD *thd)
 {
    #pragma pack(1)
    struct reqroaster
    {
       ULONG    time;
       USHORT   size;
    } req;

    static const USHORT ack_rates[] = { 0x0001,
                                        0x0002,
                                        0x0003,
                                        0x0004,
                                        0x0005
                                      };


    PACKET      pkt;
    int         f;

    DBGMessage("Thread auxiliar de login iniciada");

    /*--- CLI_ACKRATES --------------------*/
    pkt = icqv8_allocatePacket(thd->icq, thd->parm, 45);

    for(f=0;f<5;f++)
       icqv8_insertShort(pkt, 0, ack_rates[f]);

    icqv8_sendSnac(thd->icq, thd->parm, 0x01, 0x08, 0, pkt);

    /*--- CLI_REQINFO ---------------------*/
    icqv8_sendSingleSnac(thd->icq, thd->parm, 0x01, 0x0E, 0);

    /*---- CLI_REQUNKNOWN -----------------*/
    icqv8_sendSingleSnac(thd->icq, thd->parm, 0x13, 0x02, 0);

    /*---- CLI_REQROSTER ------------------*/
    pkt = icqv8_allocatePacket(thd->icq, thd->parm, 45);

    memset(&req,0,sizeof(struct reqroaster));

    if(icqQueryFirstUser(thd->icq))
       req.time = LONG_VALUE(icqLoadValue(thd->icq, "SSUL.Update", 0x3C36D708));
    else
       req.time = 0x3C36D708;

    req.size = SHORT_VALUE(icqLoadValue(thd->icq, "v8.usercount", 0));

    icqv8_insertBlock(pkt, 0, sizeof(struct reqroaster), (UCHAR *) &req);
    icqv8_sendSnac(thd->icq, thd->parm, 0x13, 0x05, 0x00010005, pkt);

    /*---- CLI_REQLOCATION ----------------*/
    icqv8_sendSingleSnac(thd->icq, thd->parm, 0x02, 0x02, 0);

    /*---- CLI_REQBUDDY -------------------*/
    icqv8_sendSingleSnac(thd->icq, thd->parm, 0x03, 0x02, 0);

    /*---- CLI_REQICBM --------------------*/
    icqv8_sendSingleSnac(thd->icq, thd->parm, 0x04, 0x04, 0);

    /*---- CLI_REQBOS ---------------------*/
    icqv8_sendSingleSnac(thd->icq, thd->parm, 0x09, 0x02, 0);


    DBGMessage("Thread auxiliar de login terminada");
 }


 static int _System srv_rates(HICQ icq, ICQV8 *cfg, int sz, ULONG req, void *data)
 {
    icqCreateThread(icq, auxThread, 16384, 0, cfg, "pkt");
    return 0;
 }

 static int _System srv_replyinfo(HICQ icq, ICQV8 *cfg, int sz, ULONG req, UCHAR *pkt)
 {
/*
BUIN  xx ..  UIN  The UIN this information is about.
WORD 00 00 WARNING Probably a warning level left over from OSCAR.
WORD 00 xx COUNT Total number of TLVs to follow.
TLV(1) 00 01 00 02 00 80 UNKNOWN Unknown.
TLV(12) 00 0c 00 25 ... CLI2CLI Direct connection info, see CLI_SETSTATUS SNAC(1,30).
TLV(15) 00 0f 00 04 xx xx xx xx UNKNOWN Number of seconds that user has been online?
TLV(2) 00 02 00 04 TIME MEMBERTIME The member since time.
TLV(3) 00 03 00 04 TIME ONLINETIME The online since time.
TLV(5) 00 05 00 04 TIME UNKNOWN Some unknown time.
TLV(6) 00 06 00 04 xx xx xx xx STATUS The current online status.
TLV(30) 00 1e 00 04 00 00 00 00 UNKNOWN Unknown: empty.
*/
    char buffer[0x0100];
    int  count;
    TLV  tlv;

    icqDumpPacket(icq, NULL, "Reply-Info packet", sz, (char *) pkt);

    count = *pkt;
    pkt++;

    strncpy(buffer,pkt,count+1);
    *(buffer+count+1) = 0;

    DBGMessage(buffer);

    if(atoi(buffer) != icqQueryUIN(icq))
    {
       icqWriteSysLog(icq,PROJECT,"WARNING:Unexpected UIN received in reply info packet");
       return -1;
    }

    sz  -= count;
    pkt += count;

    DBGTracex( *((USHORT *) pkt) );
    pkt += 2;

    DBGTracex( *((USHORT *) pkt) );
    count = SHORT_VALUE(*((USHORT *) pkt));
    pkt += 2;
    sz  -= 4;

    DBGTrace(count);
    DBGTracex(count);
    DBGTrace(sz);

    pkt = (UCHAR *) icqv8_getTLV(icq, cfg, &tlv, &sz, pkt);

    DBGTracex(pkt);

    while(pkt)
    {
       count--;

       DBGTrace(tlv.id);

       switch(tlv.id)
       {
       case 0x0001:	// TLV(1) 00 01 00 02 00 80 UNKNOWN Unknown.
       case 0x001e:	// TLV(30) 00 1e 00 04 00 00 00 00 UNKNOWN Unknown: empty.
       case 0x000c:	// TLV(12) 00 0c 00 25 ... CLI2CLI Direct connection info, see CLI_SETSTATUS SNAC(1,30).
       case 0x000f:	// TLV(15) 00 0f 00 04 xx xx xx xx UNKNOWN Number of seconds that user has been online?
//       case 0x0002:	// TLV(2) 00 02 00 04 TIME MEMBERTIME The member since time.
       case 0x0003:	// TLV(3) 00 03 00 04 TIME ONLINETIME The online since time.
       case 0x0005:	// TLV(5) 00 05 00 04 TIME UNKNOWN Some unknown time.
       case 0x0006:	// TLV(6) 00 06 00 04 xx xx xx xx STATUS The current online status.
          break;

       case 0x000A:
          DBGMessage(icqFormatIP(*((long *) tlv.data)) );
          icqSetLocalIP(icq,*((long *) tlv.data));
          break;

       default:
          icqSetConnectTimer(icq,20);
          icqv8_dumptlv(icq, "Unexpected TLV %d received in reply info", 0, &tlv);

       }


       pkt = (UCHAR *) icqv8_getTLV(icq, cfg, &tlv, &sz, pkt);
    }

    DBGTrace(count);
    DBGTrace(sz);

    return 0;
 }


