/*
 * tcptools.c -
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

 #include <stdio.h>
 #include <malloc.h>
 #include <string.h>

 #include "peer.h"

/*---[ Structures and Macros ]---------------------------------------------------------------------------*/


/*---[ Function processors ]-----------------------------------------------------------------------------*/


/*---[ Prototipes ]--------------------------------------------------------------------------------------*/


/*---[ Implementation ]----------------------------------------------------------------------------------*/

 int EXPENTRY icqv8_recBlock(HPEER cfg, int sock, int sz, void *buffer)
 {
    int  toRec  = sz;
    int  bytes  = 0;
    char *ptr   = buffer;

    while( toRec > 0 && (bytes = icqRecv(sock,ptr,toRec)) >= 0 && cfg->active)
    {
       ptr   += bytes;
       toRec -= bytes;
    }
    return toRec ? 0 : sz;
 }

 int icqv8_recvPeer(HICQ icq, HPEER cfg, int sock, void **pkt)
 {
    USHORT	sz;
    int	        rc;

    if(!icqv8_recBlock(cfg, sock, sizeof(sz), (void *) &sz))
       return 0;

    DBGTrace(sz);

    *pkt = malloc(sz+5);

    if(!*pkt)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when receiving packet");
       return 0;
    }

    rc = icqv8_recBlock(cfg, sock, sz, *pkt);

    if(rc)
       icqDumpPacket(NULL, 0, "Received block", sz, (unsigned char *) *pkt);

    return rc;

 }

 int EXPENTRY icqv8_sendBlock(int sock, USHORT sz, const void *buffer)
 {
    icqDumpPacket(NULL, 0, "Sending block", sz, (unsigned char *) buffer);

    if(icqSend(sock, &sz, 2))
       return -1;
    return icqSend(sock, buffer, sz);
 }



