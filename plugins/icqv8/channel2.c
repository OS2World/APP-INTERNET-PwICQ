/*
 * CHANNEL2.C
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>

 #include "icqv8.h"

/*---[ Defines ]----------------------------------------------------------------------------------------------*/

 #pragma pack(1)

 struct snacproc
 {
    UCHAR               id;
    const SNAC_CMD      *tbl;
 };

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static void writeDump(HICQ, int, SNAC *);
 static void logError(HICQ,int,USHORT, USHORT, int, SNAC *);

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 void icqv8_processChannel2(HICQ icq, ICQV8 *cfg, int sz, SNAC *snac)
 {
    static const struct snacproc snacs[] = {    { 0x01, icqv8_snac01_table },
                                                { 0x02, icqv8_snac02_table },
                                                { 0x03, icqv8_snac03_table },
                                                { 0x04, icqv8_snac04_table },
                                                { 0x09, icqv8_snac09_table },
                                                { 0x0B, icqv8_snac0B_table },
                                                { 0x13, icqv8_snac13_table },
                                                { 0x15, icqv8_snac15_table },
                                                { 0x00, NULL               }
                                           };

    const SNAC_CMD              *ptr;
    const struct snacproc       *sproc;
    int                         rc;

    snac->family  = SHORT_VALUE(snac->family);
    snac->subType = SHORT_VALUE(snac->subType);

//    DBGTracex(snac->family);
//    DBGTracex(snac->subType);

    for(sproc = snacs;sproc->tbl && sproc->id != snac->family;sproc++);

    ptr = sproc->tbl;

//    DBGMessage("***********************************************************************");
//    DBGTracex(ptr);
//    DBGTracex(&icqv8_snac13_table);

//    CHKPoint();

    while(ptr && ptr->subType)
    {
//       DBGTracex(ptr->subType);

       if(ptr->subType == snac->subType)
       {
//          CHKPoint();

          rc = ptr->exec(icq, cfg, sz-sizeof(SNAC), snac->request, (void *) (snac+1));

          if(rc)
             logError(icq,rc,snac->family,snac->subType,sz,snac);

          return;
       }
       ptr++;
    }

//    DBGTracex(ptr->subType);


//    if(icqPacket(icq, 7, pkt, sz, 0, 0))
//      return;

    writeDump(icq,sz,snac);

//    DBGMessage("***********************************************************************");

 }

 static void writeDump(HICQ icq, int sz, SNAC *pkt)
 {
    char log[80];
    sprintf(log,"Unknown SNAC packet (0x%04x/0x%04x - %d/%d)",pkt->family,pkt->subType,pkt->family,pkt->subType);
    icqDumpPacket(icq, NULL, log, sz, (unsigned char *) pkt);
 }

 static void logError(HICQ icq, int rc, USHORT family, USHORT subType, int sz, SNAC *snac)
 {
    char logBuffer[0x0100];
    sprintf(logBuffer,"Error %d processing SNAC(0x%02x,0x%02x)",rc,family,subType);
    icqDumpPacket(icq, NULL, logBuffer, sz, (unsigned char *) snac);
 }


