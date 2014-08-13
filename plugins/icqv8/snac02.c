/*
 * SNAC2.C
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>

 #include "icqv8.h"

/*---[ Defines ]----------------------------------------------------------------------------------------------*/


/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static int _System srv_replylocation(HICQ, ICQV8 *, int, ULONG, void *);


/*---[ SNAC processors ]--------------------------------------------------------------------------------------*/


 const SNAC_CMD icqv8_snac02_table[] =
 {
    { 0x03,   SNAC_CMD_PREFIX   srv_replylocation     },

    { 0x00,                     NULL                  }
 };


/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 static int _System srv_replylocation(HICQ icq, ICQV8 *cfg, int sz, ULONG req, void * ptr)
 {
    return 0;
 }



