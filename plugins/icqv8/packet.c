/*
 * PACKET.C - Packet management
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <malloc.h>

 #include "icqv8.h"

/*---[ Defines ]----------------------------------------------------------------------------------------------*/

 #define LIST_SIZE      20

/*---[ Structs ]----------------------------------------------------------------------------------------------*/

 #pragma pack(1)

 struct prefix
 {
    USHORT      sz;
    USHORT      used;
 };


/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static struct prefix * getPrefix(PACKET,int);

/*---[ Statics ]----------------------------------------------------------------------------------------------*/

 static const char      packet_prefix        = PACKET_START;

 static struct prefix * packetList[LIST_SIZE];

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 void icqv8_initPacketList(void)
 {
    int f;
    for(f=0;f<LIST_SIZE;f++)
       packetList[f] = NULL;
 }

 PACKET icqv8_allocatePacket(HICQ icq, ICQV8 *cfg, int sz)
 {
#ifdef DEBUG
    UCHAR          *CHK;
#endif
    int            f;

#ifdef OS2DEBUG
    if(_heapchk() != _HEAPOK)
        DBGPrint("************************ ERRO %d NO HEAP ************************",_heapchk());
#endif

    if(!sz)
       sz = DEF_PACKET_SIZE;

    LOCK(cfg);

    for(f=0;f<LIST_SIZE && packetList[f];f++);

    if(f >= LIST_SIZE)
    {
       UNLOCK(cfg);
       icqWriteSysLog(icq,PROJECT,"Not enough space in packet-list");
       icqAbend(icq,0);
       return -1;
    }

//    DBGTrace(f);

#ifdef DEBUG
    packetList[f] = malloc(sz+sizeof(struct prefix)+4);
#else
    packetList[f] = malloc(sz+sizeof(struct prefix));
#endif

    if(!packetList[f])
    {
       UNLOCK(cfg);
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when creating packet");
       icqAbend(icq,0);
       return -1;
    }

#ifdef DEBUG
    CHK = (UCHAR *) packetList[f];
    strcpy(CHK+sz+sizeof(struct prefix),"APW");
#endif

    cfg->packets++;

    UNLOCK(cfg);

//    DBGTrace(sz);

    packetList[f]->sz   = sz;
    packetList[f]->used = 0;

//    DBGTrace(f);
//    DBGTracex(packetList[f]);

    return f;
 }


 void icqv8_releasePacket(HICQ icq, ICQV8 *cfg, PACKET ptr)
 {
#ifdef DEBUG
    UCHAR          *CHK;
#endif

#ifdef OS2DEBUG
    if(_heapchk() != _HEAPOK)
        DBGPrint("************************ ERRO %d NO HEAP ************************",_heapchk());
#endif

    if(!packetList[ptr] || ptr >= LIST_SIZE)
    {
       icqWriteSysLog(icq,PROJECT,"Invalid packet request");
       icqAbend(icq,0);
       return;
    }

#ifdef DEBUG
    CHK = (UCHAR *) packetList[ptr];
    if(strcmp(CHK+packetList[ptr]->sz+sizeof(struct prefix),"APW"))
       DBGMessage(">>>>>>>>>>>>>>>>>>>>>>>>>>>>> ERRO NA MANIPULACAO DO PACOTE <<<<<<<<<<<<<<<<<<<<<<<<<<<<");
#endif

    if(packetList[ptr])
    {
       LOCK(cfg);
       free(packetList[ptr]);
       packetList[ptr] = NULL;
       cfg->packets--;
       UNLOCK(cfg);
    }

//    CHKPoint();
 }

 static struct prefix * getPrefix(PACKET ptr, int sz)
 {
#ifdef DEBUG
    UCHAR          *CHK;
#endif
    struct prefix *ret = packetList[ptr];

    if(!ret)
       return ret;

    if(ret->sz > (ret->used+sz))
       return ret;

    DBGMessage("Reajustar tamanho do pacote");
    DBGTrace(ret->sz);
    DBGTrace(ret->used+sz);

    sz += (ret->used+1);

    ret = packetList[ptr] = realloc(packetList[ptr],sz+sizeof(struct prefix));

    if(ret)
       ret->sz = sz;

#ifdef DEBUG
    CHK = (UCHAR *) ret;
    strcpy(CHK+sz+sizeof(struct prefix),"APW");
#endif

    return ret;
 }

 void icqv8_insertLong(PACKET pkt, USHORT tlv, ULONG vlr)
 {
    struct prefix *hdr = getPrefix(pkt,sizeof(ULONG));
    char          *ptr = (char *) (hdr+1);

    ptr += hdr->used;

    if(tlv)
    {
       *( (USHORT *) ptr ) = SHORT_VALUE(tlv);
       ptr += 2;
       *( (USHORT *) ptr ) = 0x0400;
       ptr += 2;
       hdr->used += 4;
    }

    *( (ULONG *) ptr ) = LONG_VALUE(vlr);
    hdr->used += sizeof(ULONG);

 }

 void icqv8_insertShort(PACKET pkt, USHORT tlv, USHORT vlr)
 {
    struct prefix *hdr = getPrefix(pkt,sizeof(ULONG));
    char          *ptr = (char *) (hdr+1);

    ptr += hdr->used;

    if(tlv)
    {
       *( (USHORT *) ptr ) = SHORT_VALUE(tlv);
       ptr       += 2;
       *( (USHORT *) ptr ) = 0x0200;
       ptr       += 2;
       hdr->used += 4;
    }

    *( (USHORT *) ptr ) = SHORT_VALUE(vlr);
    hdr->used += sizeof(USHORT);

 }

 void icqv8_dumpPacket(PACKET ptr, HICQ icq, const char *txt)
 {
    struct prefix *pkt = getPrefix(ptr,0);

    if(pkt)
       icqDumpPacket(icq, NULL, txt, pkt->used, (unsigned char *) (pkt+1));
    else
       icqWriteSysLog(icq,PROJECT,"Invalid dump packet request");
 }

 void icqv8_insertString(PACKET pkt, USHORT tlv, const UCHAR *string)
 {
    icqv8_insertBlock(pkt, tlv, strlen(string), string);
 }

 void icqv8_insertLNTS(PACKET pkt, const UCHAR *string)
 {
    USHORT sz = strlen(string)+1;
    icqv8_insertBlock(pkt, 0, 2, (const unsigned char *) &sz);
    icqv8_insertBlock(pkt, 0, sz, string);
 }

 void icqv8_insertBUIN(PACKET pkt, USHORT tlv, ULONG uin)
 {
    char buffer[0x0100];
    char *ptr = (buffer+1);

    sprintf(ptr,"%ld",uin);
    *buffer = strlen(ptr);

    icqv8_insertBlock(pkt,tlv,(*buffer)+1, buffer);

 }

 void icqv8_insertBlock(PACKET pkt, USHORT tlv, USHORT sz, const unsigned char *string)
 {
    struct prefix *hdr = getPrefix(pkt,sz+4);
    char          *ptr = (char *) (hdr+1);

    ptr += hdr->used;

    if(tlv)
    {
       *( (USHORT *) ptr ) = SHORT_VALUE(tlv);
       ptr += 2;
       *( (USHORT *) ptr ) = SHORT_VALUE(sz);
       ptr += 2;
       hdr->used += 4;
    }

    if(sz)
    {
       hdr->used += sz;

#ifdef __OS2__
       while(sz--)
          *(ptr++) = *(string++);
#else
       memcpy(ptr,string,sz);
#endif
    }

 }

 int icqv8_sendSingleSnac(HICQ icq, ICQV8 *cfg, USHORT family, USHORT subType, ULONG request)
 {
    FLAP          flap;
    SNAC          snac;
    int           rc            = 0;

#ifdef OS2DEBUG
    if(_heapchk() != _HEAPOK)
        DBGPrint("************************ ERRO %d NO HEAP ************************",_heapchk());
#endif

    if(cfg->sock < 1)
    {
       icqWriteSysLog(icq,PROJECT,"Invalid socket when sending");
       return -1;
    }

    LOCK(cfg);

    icqSetServerBusy(icq,TRUE);
    rc = icqSend(cfg->sock, &packet_prefix, 1);

    if(!rc)
    {
       /* Envia o FLAP */

       flap.channel     = CHANNEL_SNAC;
       flap.size        = SHORT_VALUE(sizeof(SNAC)) ;
       flap.sequence    = SHORT_VALUE(cfg->packetSeq++);
       rc = icqSend(cfg->sock, &flap, sizeof(FLAP));
    }

    if(!rc)
    {
       /* Envia o SNAC */
       snac.family      = SHORT_VALUE(family);
       snac.subType     = SHORT_VALUE(subType);
       snac.flags[0]    =
       snac.flags[1]    = 0;
       snac.request     = LONG_VALUE(request);
       rc = icqSend(cfg->sock, &snac, sizeof(SNAC));
    }

    UNLOCK(cfg);

    return rc;
 }

 int icqv8_sendSnac(HICQ icq, ICQV8 *cfg, USHORT family, USHORT subType, ULONG request, PACKET pkt)
 {
    struct prefix *hdr          = getPrefix(pkt,0);
    int           rc            = 0;
    FLAP          flap;
    SNAC          snac;

//#ifdef DEBUG
//    icqv8_dumpPacket(pkt, icq, "SNAC sent");
//#endif

#ifdef OS2DEBUG
    if(_heapchk() != _HEAPOK)
        DBGPrint("************************ ERRO %d NO HEAP ************************",_heapchk());
#endif

    if(cfg->sock < 1)
    {
       icqWriteSysLog(icq,PROJECT,"Invalid socket when sending");
       icqv8_releasePacket(icq,cfg,pkt);
       return -1;
    }

    LOCK(cfg);
    icqSetServerBusy(icq,TRUE);

    rc = icqSend(cfg->sock, &packet_prefix, 1);

    if(!rc)
    {
       /* Envia o FLAP */

       flap.channel     = CHANNEL_SNAC;
       flap.size        = SHORT_VALUE(hdr->used+sizeof(SNAC)) ;
       flap.sequence    = SHORT_VALUE(cfg->packetSeq++);
       rc = icqSend(cfg->sock, &flap, sizeof(FLAP));
    }

    if(!rc)
    {
       /* Envia o SNAC */
       snac.family      = SHORT_VALUE(family);
       snac.subType     = SHORT_VALUE(subType);
       snac.flags[0]    =
       snac.flags[1]    = 0;
       snac.request     = LONG_VALUE(request);
       rc = icqSend(cfg->sock, &snac, sizeof(SNAC));
    }

    if(!rc)
       rc = icqSend(cfg->sock, (hdr+1), hdr->used);

    UNLOCK(cfg);

    if(rc)
    {
#ifdef EXTENDED_LOG
       icqWriteSysLog(icq,PROJECT,"Error sending packet");
#endif
       CLOSE_SOCKET(cfg);
    }

    icqv8_releasePacket(icq,cfg,pkt);

    return rc;
 }


 int icqv8_sendFlap(HICQ icq, ICQV8 *cfg, UCHAR channel, PACKET pkt)
 {
    struct prefix *hdr          = getPrefix(pkt,0);
    int           rc            = 0;
    FLAP          flap;
//    static char   prefix        = PACKET_START;
    USHORT        sequence;

//#ifdef DEBUG
//    icqv8_dumpPacket(pkt, icq, "Flap sent");
//#endif

#ifdef OS2DEBUG
    if(_heapchk() != _HEAPOK)
        DBGPrint("************************ ERRO %d NO HEAP ************************",_heapchk());
#endif

    if(cfg->sock < 1)
    {
       icqWriteSysLog(icq,PROJECT,"Invalid socket when sending");
       icqv8_releasePacket(icq,cfg,pkt);
       return -1;
    }

    LOCK(cfg);
    icqSetServerBusy(icq,TRUE);

    rc = icqSend(cfg->sock, &packet_prefix, 1);

    if(!rc)
    {
       flap.channel     = channel;
       flap.size        = SHORT_VALUE(hdr->used);

       DBGTracex(cfg);
       DBGTrace(cfg->sz);

       sequence = cfg->packetSeq++;

       flap.sequence = SHORT_VALUE(sequence);

       rc = icqSend(cfg->sock, &flap, sizeof(FLAP));

    }

    if(!rc && hdr)
       rc = icqSend(cfg->sock, (hdr+1), hdr->used);

    UNLOCK(cfg);

    if(rc)
    {
#ifdef EXTENDED_LOG
       icqWriteSysLog(icq,PROJECT,"Error sending packet");
#endif
       CLOSE_SOCKET(cfg);
    }

    icqv8_releasePacket(icq,cfg,pkt);

    return rc;
 }

 void icqv8_toICQServer(HICQ icq, ICQV8 *cfg, USHORT cmd, ULONG req, int sz, void *block)
 {
    struct _2icqsrvprefix
    {
       USHORT   tlv_id; // Always TLV-01
       USHORT   tlv_sz; // TLV-Size

       USHORT   sz;     // 2 xx xx LENGTH The remaining number of bytes in the packet.
       ULONG    uin;    // 4 xx xx xx xx UIN The user's UIN.
       USHORT   cmd;    // 2 xx xx COMMAND The command for the ICQ server:
                        // 60 = request offline messages
                        // 2000 = request information
       USHORT   seq;    // 2 xx xx SEQUENCE A one-up sequence starting at 2. It should be noted that this value minus one is passed in the upper word of the SNAC header reference value.
    } prefix;

    PACKET pkt;

/*
   CLI_TOICQSRV
   Channel: 2 SNAC(15,2)
   This packet seems to act as an interface between the AIM OSCAR-based server and the old original ICQ server database.
   Parameters
   Length/TLV Content (If Fixed) Designations Description
   TLV01 00 01 xx xx ICQDATA The data in this TLV is seperated into several sections broken out as follows. The data is passed to the old original ICQ server as all numbers are little-endian and the data generally follows the format as was laid out in the ICQ protocol versions 2-5.
   2 xx xx LENGTH The remaining number of bytes in the packet.
   4 xx xx xx xx UIN The user's UIN.
   2 xx xx COMMAND The command for the ICQ server:
   60 = request offline messages
   2000 = request information
   2 xx xx SEQUENCE A one-up sequence starting at 2. It should be noted that this value minus one is passed in the upper word of the SNAC header reference value.
   Varies .. DATA Depending on the command value, there may or may not be more data at this point.

*/

    if(!block || sz < 0)
       sz = 0;

    pkt     = icqv8_allocatePacket(icq, cfg, sz+sizeof(prefix)+sizeof(SNAC));

    prefix.tlv_id = 0x0100;

    prefix.tlv_sz = sz+sizeof(prefix)-4;
    prefix.sz     = prefix.tlv_sz -2;
    prefix.tlv_sz = SHORT_VALUE(prefix.tlv_sz);

    prefix.uin    = icqQueryUIN(icq);
    prefix.cmd    = cmd;
    prefix.seq    = cfg->toICQSrvSeq++;

    icqv8_insertBlock(pkt, 0, sizeof(prefix), (const char *) &prefix);

    if(sz)
       icqv8_insertBlock(pkt, 0, sz, (const char *) block);

    icqv8_sendSnac(icq, cfg, 0x15, 0x02, LONG_VALUE(req), pkt);


 }

 int icqv8_getPacketSize(PACKET pkt)
 {
    struct prefix *hdr = getPrefix(pkt,0);
    return hdr->used;
 }

 void icqv8_insertPacket(HICQ icq, ICQV8 *cfg, PACKET pkt, USHORT tlv, PACKET src)
 {
    struct prefix *hdr = getPrefix(src,0);

    icqv8_insertBlock(pkt, tlv, hdr->used, (const unsigned char *) (hdr+1));
    icqv8_releasePacket(icq,cfg,src);

 }

 void icqv8_sendKeepAlive(HICQ icq, ICQV8 *cfg)
 {
    #pragma pack(1)

    struct _pkt
    {
       UCHAR prefix;
       FLAP  flap;
    } pkt		= { PACKET_START, { CHANNEL_KEEPALIVE, 0, 0 } };

    USHORT sequence;

    LOCK(cfg);

    sequence = cfg->packetSeq++;

    pkt.flap.sequence = SHORT_VALUE(sequence);

//    icqDumpPacket(icq, NULL, "Keep-alive packet", sizeof(pkt), (unsigned char *) &pkt);

    if(icqSend(cfg->sock, &pkt, sizeof(pkt)))
    {
       icqWriteNetworkErrorLog(icq, PROJECT, "Error sending keep-alive packet");
       cfg->status = 0;
    }

    UNLOCK(cfg);

 }

