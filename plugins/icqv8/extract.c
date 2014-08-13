/*
 * EXTRACT.C - Extract information from packets
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>

 #include "icqv8.h"

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/


/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 const void * icqv8_getTLV(HICQ icq, ICQV8 *cfg, TLV *tlv, int *sz, const void *pkt)
 {
    USHORT *ptr = (USHORT *) pkt;

    if(*sz < 4)
       return NULL;

    *sz -= 4;

    tlv->id   = SHORT_VALUE( *(ptr++));
    tlv->sz   = SHORT_VALUE( *(ptr++));

    if(tlv->sz > *sz)
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected TLV size received");
       return NULL;
    }

    tlv->data = ptr;

    *sz -= tlv->sz;

    return ((UCHAR *) tlv->data ) + tlv->sz;
 }


 void icqv8_dumptlv(HICQ icq, const char *text, ULONG uin, TLV *tlv)
 {
    char linha[132];
    sprintf(linha,text,tlv->id,uin);
#ifdef EXTENDED_LOG
    icqDumpPacket(icq, icqQueryUserHandle(icq, uin), linha, tlv->sz, tlv->data);
#else
    icqWriteSysLog(icq,PROJECT,linha);
#endif
 }


