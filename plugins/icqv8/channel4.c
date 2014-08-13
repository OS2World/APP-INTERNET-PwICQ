/*
 * CHANNEL4.C
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>

 #include "icqv8.h"

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

// static void sendLoginPacket(HICQ, ICQV8 *);

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 void icqv8_processChannel4(HICQ icq, ICQV8 *cfg, int sz, void *buffer)
 {
    char        logLine[0x0100];
    TLV         tlv;
    UCHAR       *uin            = NULL;
    UCHAR       *Server         = NULL;
    UCHAR       *Cookie         = NULL;
    int         szCookie        = 0;
    char        *ptr;

    CHKPoint();

    if(cfg->status == ICQV8_STATUS_ONLINE)
    {
       /* Se estou online ‚ um pedido de desconexao */
       icqDumpPacket(icq, NULL, "Disconnect request received", sz, (unsigned char *) buffer);
       icqv8_goodbye(icq,cfg);
       return;
    }

    /* Nao estou online, e o pacote com o cookie */

    while( (buffer = (void *) icqv8_getTLV(icq, cfg, &tlv, &sz, buffer)) != NULL)
    {
       switch(tlv.id)
       {
       case 0x01:       // TLV01  00 01 xx xx ..  UIN  The user's UIN in ASCII format.
          uin = (UCHAR *) tlv.data;
          break;

       case 0x05:       // TLV05  00 05 xx xx ..  SERVER  The actual server and port for the client to connect with for this session.
          Server = (UCHAR *) tlv.data;
          break;

       case 0x06:       // TLV06  00 06 xx xx ..  COOKIE  The actual cookie that needs to be passed to the next server.
          Cookie   = (UCHAR *) tlv.data;
          szCookie = tlv.sz;
          break;

       default:
          icqv8_dumptlv(icq, "Unexpected TLV %d received in hello packet", 0, &tlv);
        }
    }

    DBGMessage(uin);
    DBGMessage(Server);
    DBGTracex(Cookie);
    DBGTrace(szCookie);

    icqv8_goodbye(icq,cfg);

    DBGTrace(cfg->sock);

    if(!Server)
    {
       CHKPoint();
       icqWriteSysLog(icq,PROJECT,"Failure getting the service port address");
       return;
    }

    if(!Cookie)
    {
       CHKPoint();
       icqWriteSysLog(icq,PROJECT,"Failure getting the authentication cookie");
       return;
    }

    if(!uin)
    {
       CHKPoint();
       icqWriteSysLog(icq,PROJECT,"Failure getting UIN from the authentication cookie");
       return;
    }


    /* Formato o pacote com o cookie para envio */

    cfg->wrkPacket = icqv8_allocatePacket(icq, cfg, 270);

    DBGTracex(cfg->wrkPacket);

    icqv8_insertLong(cfg->wrkPacket, 0, 0x00000001);
    icqv8_insertBlock(cfg->wrkPacket, 6, szCookie, Cookie);

    /* Conecta ao novo servidor */
    // setStatus(icq,cfg,ICQV8_STATUS_CONNECTING);
    cfg->status = ICQV8_STATUS_CONNECTING;
    sprintf(logLine,"Connecting to %s",Server);
    icqWriteSysLog(icq,PROJECT,logLine);

    for(ptr = Server;*ptr && *ptr != ':';ptr++);

    if(!*ptr)
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected server ID received");
       return;
    }

    *(ptr++) = 0;

    DBGMessage(Server);
    DBGMessage(ptr);

    cfg->sock =  icqConnectHost(Server, atoi(ptr));

    DBGTrace(cfg->sock);
    DBGTrace(cfg->wrkPacket);

 }


 void icqv8_goodbye(HICQ icq, ICQV8 *cfg)
 {
    /* Envia o pacote de adeus */
    FLAP          flap;
    static char   prefix        = PACKET_START;
    int           rc;

    DBGTracex(cfg);
	DBGTrace(cfg->sock);
	
    rc = icqSend(cfg->sock, &prefix, 1);
	
	DBGTrace(rc);

    if(!rc)
    {
       flap.channel     = CHANNEL_CLOSE;
       flap.size        = 0;

       LOCK(cfg);
       flap.sequence    = SHORT_VALUE(cfg->packetSeq++);
       UNLOCK(cfg);

       rc = icqSend(cfg->sock, &flap, sizeof(FLAP));

    }

    DBGTrace(rc);

    icqCloseSocket(cfg->sock);
    cfg->sock = -1;
 }


