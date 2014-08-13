/*
 * file.c - File request processing
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>

 #include "peer.h"

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 USHORT _System icqv8_sendFileReq(HICQ icq, HUSER usr, PEERDATA *cfg, const char *fileName, int fileCount, const char *text, ULONG fileSize)
 {
    const char *baseName;
    const char *ptr;
    char       *req;
    int        sequenceNumber;

    DBGMessage("Enviar arquivo");
    DBGMessage(fileName);
    DBGMessage(text);
    DBGTrace(fileSize);

    if(fileCount > 1)
    {
       icqWriteSysLog(icq,PROJECT,"Multifile sending isn't implemented");
       return -2;
    }

    for(ptr = baseName = fileName;*ptr;ptr++)
    {
       if(*ptr == PATH_SEPARATOR)
          baseName = ptr+1;
    }

    DBGMessage(baseName);

    sequenceNumber = icqPeer_sndGreet(icq, cfg, MSG_GREET_FILE, 0, 0, "", "File", -1, text, -1, baseName, -1, fileSize);

    DBGTracex(sequenceNumber);

    if(!sequenceNumber)
       return -1;

    DBGMessage("Registrar o file request");
    req = icqCreateRequest(	icq,
        			        usr->uin,
        			        TRUE,
        			        (ULONG) cfg,
        			        sequenceNumber,
        			        &icqv8p_requestManager,
        			        cfg,
        			        strlen(fileName)+10);
    if(!req)
       return -2;

    *( (USHORT *) req) = MSG_GREET_FILE;
    req += 2;

    *( (USHORT *) req) = fileCount;
    req += 2;

    *( (ULONG *) req)  = fileSize;
    req += 4;

    strcpy(req,fileName);

    DBGMessage("Requisicao registrada");        			

    return 0;
 }

 int icqv8_fileStop(HICQ icq, PEERSESSION *cfg)
 {
    struct _packet
    {
       UCHAR cmd;	// BYTE 04 CMD The command: PEER_FILE_STOP.
       ULONG file;	// DWORD.L xx xx xx xx NUMBER Posibly the file number to abort.
    } pkt = { 0x04, 0 };

    pkt.file = cfg->number;

    icqv8_sendBlock(cfg->sock, sizeof(struct _packet), &pkt );

    return 0;
 }

 int icqv8_recvFileInit(HICQ icq, PEERSESSION *cfg, int sz, unsigned char *p)
 {
    char buffer[0x0100];

    struct _packet
    {
       UCHAR cmd;		// BYTE  03  CMD  The command: PEER_FILE_STARTACK.
       ULONG offset;		// DWORD.L xx xx xx xx OFFSET The offset within the file from where to start transfer. Used to be able to resume transfer.
       ULONG x1;		// DWORD 00 00 00 00 EMPTY Unknown: empty.
       ULONG speed;		// DWORD.L xx xx xx xx SPEED The receiver's speed again.
       ULONG number;		// DWORD.L xx xx xx xx NUMBER The number of this file in the batch.
    } *pkt = (struct _packet *) p;

    DBGTrace(pkt->offset);
    DBGTrace(pkt->x1);
    DBGTrace(pkt->speed);
    DBGTrace(pkt->number);

    if(fseek(cfg->arq, pkt->offset, SEEK_SET))
    {
       sprintf(buffer,"seek error in session %08lx",(ULONG) cfg);
       DBGMessage(buffer);
       icqWriteSysRC(icq, PROJECT, -1, buffer);
       return -1;
    }

    cfg->transfer += pkt->offset;

    sprintf(buffer,"Session %08lx starting file send at offset %08ld and speed factor %d",(ULONG) cfg, pkt->offset, (int) cfg->speed);
    DBGMessage(buffer);
    icqWriteSysLog(icq,PROJECT,buffer);

    DBGTrace(cfg->transfer);

    cfg->flags |= PEERFLAG_READY;
    DBGTracex(cfg->arq);

    return 0;
 }

