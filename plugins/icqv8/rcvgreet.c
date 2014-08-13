/*
 * greetings.c - Greetings message processing
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>

 #include "icqv8.h"

 #pragma pack(1)

/*---[ Structs ]----------------------------------------------------------------------------------------------*/

 static int fileRequest(HICQ, ULONG, ULONG, const char *, const char *, const char *, ULONG, int);

 static const struct cmd
 {
    USHORT id;
    int    ( *exec)(HICQ, ULONG, ULONG, const char *, const char *, const char *, ULONG, int);
 } procTable[] =
 {
    { 0x0029, fileRequest }
 };

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static char *getString(HICQ, int, char **);

/*---[ Static functions ]-------------------------------------------------------------------------------------*/

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 int _System icqv8_srvRecGreet(HICQ icq, ULONG uin, USHORT sz, struct tlv2711 *h)
 {
    ICQV8             *cfg            = icqGetPluginDataBlock(icq, module);
    const struct cmd  *cmd;

    struct _pkt
    {
       USHORT id;           // WORD.L  xx xx   GCMD  The command in this packet. Valid values are:
                            // 0x0029 =  41 - file request
                            // 0x002d =  45 - chat request
                            // 0x0032 =  50 - file request granted/refused
                            // 0x003a =  58 - Send / Start ICQ Chat

       UCHAR  x1[16];       // 16 ... ID Those 16 bytes seem to be fixed for different commands. Seen:
                            // bf f7 20 b2 37 8e d4 11 bd 28 00 04 ac 96 d9 05 - chat request
                            // f0 2d 12 d9 30 91 d3 11 8d d7 00 10 4b 06 46 2e - file request or file request granted/refused
       USHORT x2;           // EMPTY  Unknown: empty.
       USHORT txtSize;
       USHORT x3;
    } *pkt = (struct _pkt *) ( ((unsigned char *) (h+1)) + h->msgSize);

    char   *ptr       = (char *) (pkt+1);
    ULONG  msgNumber  = cfg->seqMasc|((ULONG) h->seq1);
    char   *title     = getString(icq,pkt->txtSize,&ptr);
    char   *reason    = NULL;
    char   *filename  = NULL;
    int    f;
    int    strSize;
    ULONG  fileSize   = 0;
    int    port       = 0;

    icqDumpPacket(icq, NULL, "Greetings message", sz, (unsigned char *) h);

    sz -= (h->msgSize + sizeof(struct _pkt) + sizeof(struct tlv2711));

    ptr += 19;
    sz  -= 18; /* Gatilho */
    /*-------------------------------*/

    strSize  = (int) *( (USHORT *) ptr);
    FORWARD(4,strSize);
    reason   = getString(icq, strSize, &ptr);

    DBGTrace(sz);
    DBGTrace(strSize);

    if(sz > 0)
    {
       port     = SHORT_VALUE( *( (USHORT *) ptr) );
       FORWARD(4,0)
    }

    if(sz > 0)
    {
       strSize  = (int) *( (USHORT *) ptr);
       FORWARD(2,strSize);
       filename = getString(icq, strSize, &ptr);
    }


    DBGTrace(sz);

    if(sz > 0)
    {
       fileSize = *( (ULONG *) ptr );
       FORWARD(4,0);
    }

    DBGPrint("Title:    [%s]",title);
    DBGPrint("Reason:   [%s]",reason);
    DBGPrint("Filename: [%s]",filename);
    DBGTrace(fileSize);

    DBGTrace(sz);
    DBGTrace(port);
    DBGTracex(port);

    cmd = procTable;
    for(f=0;f<(sizeof(procTable)/sizeof(struct cmd)) && cmd->id != pkt->id;f++)
       cmd++;

    if(f<(sizeof(procTable)/sizeof(struct cmd)))
    {
       cmd->exec(icq, uin, msgNumber, title, reason, filename, fileSize, port);
    }
    else
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected Greetings message code");
       icqInsertMessage(icq, uin, msgNumber, MSG_NORMAL, 0, 0, strlen(reason), reason);
    }

    /* Finaliza */

    if(filename)
       free(filename);

    if(reason)
       free(reason);

    if(title)
       free(title);

    return 0;
 }

 static char *getString(HICQ icq, int sz, char **ptr)
 {
    char *ret = malloc(sz+2);
    if(ret)
    {
       strncpy(ret,*ptr,sz);
       *(ret+sz) = 0;
       *ptr += sz;
    }
    else
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error");
       icqAbend(icq,0);
    }
    return ret;

 }

 static int fileRequest(HICQ icq, ULONG uin, ULONG msgNumber, const char *title, const char *reason, const char *filename, ULONG filesize, int port)
 {
    DBGMessage("Criar um file-request");
    icqInsertMessage(icq, uin, msgNumber, MSG_NORMAL, 0, 0, strlen(reason), reason);
    return 0;
 }

