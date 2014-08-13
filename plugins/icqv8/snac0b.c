/*
 * SNAC15.C
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>

 #include "icqv8.h"

/*---[ Defines ]----------------------------------------------------------------------------------------------*/


/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static int _System srv_setinterval(HICQ, ICQV8 *, int, ULONG, const USHORT *);

/*---[ SNAC processors ]--------------------------------------------------------------------------------------*/

 const SNAC_CMD icqv8_snac0B_table[] =
 {
    { 0x02,   SNAC_CMD_PREFIX   srv_setinterval       },

    { 0x00,                     NULL                  }
 };


/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 static int _System srv_setinterval(HICQ icq, ICQV8 *cfg, int sz, ULONG req, const USHORT *vlr)
 {
    DBGMessage("SNAC 0B-02");
    DBGTracex(*vlr);

//    icqDumpPacket(icq, NULL, "SNAC 0B-02", sz, (unsigned char *) ptr);
    return 0;

/*

server sends          // Set minimum report interval? maybe related to the SNACs icq2001 sends periodically
 SNAC  B,02
 2 BYTE  04 B0

08/21/2002 13:40:32 icqkrnl    Unknown SNAC packet (0x000b/0x0002 - 11/2) (12 bytes)
0b 00 02 00 00 00 93 3d 7a 2d 04 b0               ......“=z-.°

*/

    return 0;

 }

