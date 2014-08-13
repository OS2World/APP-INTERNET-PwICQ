/*
 * ENCRIPT.C - Calcula checkcode, faz a encriptacao de um pacote
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdio.h>
 #include <stdlib.h>

 #include "peer.h"

/*---[ Implementacao ]---------------------------------------------------------------------------*/

 static const unsigned char table[] = {
    "As part of this software beta version Mirabilis is "
    "granting a limited access to the ICQ network, "
    "servers, directories, listings, information and databases (\""
    "ICQ Services and Information\"). The "
    "ICQ Service and Information may databases (\""
    "ICQ Services and Information\"). The "
    "ICQ Service and Information may\0"
 };

/*---[ Implementacao ]---------------------------------------------------------------------------*/

 int icqv8_encript(unsigned char *pkt, int sz)
 {
    ULONG	  chkPkt1;
//    ULONG         chkPkt2;	

    ULONG	  chkByte1;
    ULONG	  chkByte2;
    ULONG	  chkByte3;
    ULONG         checkCode;

    ULONG         key;
    ULONG         verify;
    int           f         = 4;

    ULONG         masc;

    /* Calcula o bloco de verificacao */
    verify   = (rand() % ((sz < 255 ? sz : 255) - 10)) + 10;
    chkByte1 = pkt[verify] ^ 0xff;
    chkByte2 = rand() % 220;
    chkByte3 = table[chkByte2] ^ 0xff;
    chkPkt1  = (pkt[4] << 24) | (pkt[6] << 16) | (pkt[4] << 8) | pkt[6];

    /* Calcula o checkcode */
    checkCode  = (verify << 24) | (chkByte1 << 16) | (chkByte2 << 8) | chkByte3;
    checkCode ^= chkPkt1;
    key        = 0x67657268 * sz + checkCode;

    while(f < (sz + 3) / 4)
    {
       masc    = key + table[f & 0xff];
       pkt[f++] ^=  masc        & 0xff;
       pkt[f++] ^= (masc >>  8) & 0xff;
       pkt[f++] ^= (masc >> 16) & 0xff;
       pkt[f++] ^= (masc >> 24) & 0xff;
    }

    *((ULONG *)pkt) = checkCode;

    return 0;
 }

 int icqv8_decript(unsigned char *pkt, int sz)
 {
    ULONG         masc;
    ULONG         key;
    ULONG         verify;

    ULONG	  chkPkt1;
    ULONG         chkPkt2;	

    ULONG	  chkByte1;
    ULONG	  chkByte2;
    ULONG	  chkByte3;

    ULONG         checkCode = *((ULONG *)pkt);
    int           f         = 4;

    DBGTrace(sz);
    DBGTracex(checkCode);

    key = 0x67657268 * sz + checkCode;

    while(f < (sz + 3) / 4)
    {
        masc    = key + table[f & 0xff];
        pkt[f++] ^=  masc        & 0xff;
        pkt[f++] ^= (masc >>  8) & 0xff;
        pkt[f++] ^= (masc >> 16) & 0xff;
        pkt[f++] ^= (masc >> 24) & 0xff;
    }

    verify  = (pkt[4]<<24) | (pkt[6]<<16) | (pkt[4]<<8) | (pkt[6]<<0);
    verify ^= checkCode;

    /* Valida o pacote */
    chkPkt1  = (pkt[4]<<24) | (pkt[6]<<16) | (pkt[4]<<8) | (pkt[6]<<0);
    chkPkt1 ^= checkCode;

    chkPkt2 = (chkPkt1 >> 24) & 0xFF;
    if (chkPkt2 < 10 || chkPkt2 >= sz)
        return 1;

    chkByte1 = pkt[chkPkt2] ^ 0xFF;
    if (((chkPkt1 >> 16) & 0xFF) != chkByte1)
        return 2;

    chkByte2 = ((chkPkt1 >> 8) & 0xFF);
    if (chkByte2 < 220)
    {
        chkByte3 = table[chkByte2] ^ 0xFF;
        if((chkPkt1 & 0xFF) != chkByte3)
            return 3;
    }

    DBGMessage("Checksum ok");
    return 0;
 }




