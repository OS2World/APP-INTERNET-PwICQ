/*
 * CRC16.C - Calculate the CRC-16 chksum
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif


 #include <stdio.h>
 #include <malloc.h>
 #include <string.h>

 #include "ishared.h"

/*---[ Implementation ]----------------------------------------------------------------------------------*/

 USHORT ICQAPI ishare_crc16(int size, const char *texto)
 {
    short crc     = 0;
    short count;
    short c;

    while(size--)
    {
       c = *texto;

       for (count = 8; --count >= 0; )
       {
          if (crc & 0x8000)
          {
             crc <<= 1;
             crc += (short) (((c <<= 1) & 0400) != 0);
             crc ^= 0x1021;
          }
          else
          {
             crc <<= 1;
             crc += (short) (((c <<= 1) & 0400) != 0);
          }
       }
       texto++;
    }

    return(crc);
 }


