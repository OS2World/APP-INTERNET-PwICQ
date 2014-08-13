/*
 * rcvfile.c - Receive file
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>

 #include "peer.h"

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/


/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 int icqv8_recvFileStart(HICQ icq, PEERSESSION *cfg, int sz, unsigned char *pkt)
 {
    #pragma pack(1)

    char  buffer[0x0200];

    struct _ack
    {
       UCHAR cmd;		// BYTE  03  CMD  The command: PEER_FILE_STARTACK.
       ULONG offset;		// DWORD.L xx xx xx xx OFFSET The offset within the file from where to start transfer. Used to be able to resume transfer.
       ULONG x1;		// DWORD 00 00 00 00 EMPTY Unknown: empty.
       ULONG speed;		// DWORD.L xx xx xx xx SPEED The receiver's speed again.
       ULONG number;		// DWORD.L xx xx xx xx NUMBER The number of this file in the batch.
    } *ack = (struct _ack *) buffer;

    struct _header
    {
       UCHAR  cmd;	   	// BYTE 02 CMD The command: PEER_FILE_START.
       UCHAR  type;	   	// BYTE 00 FILE/DIRECTORY Indicates File with 0 and directory with 1.
       USHORT nameSize;    	// LNTS xx xx ... FILENAME The file name of the file to transfer.
       UCHAR  filename[1];
    }  *b = (struct _header *) pkt;

    struct _footer
    {
       ULONG fileSize;	   	// DWORD.L xx xx xx xx FILELEN The length of the file to transfer.
       ULONG x1;           	// DWORD 00 00 00 00 EMPTY Unknown: empty.
       ULONG speed;        	// DWORD.L xx xx xx xx SPEED The sender's speed, again.
    } *e;

    char *ptr;

    if(b->nameSize > 0x0100)
    {
       sprintf(buffer,"Filename is too large when receiving file in session %08lx",(ULONG) cfg);
       icqWriteSysLog(icq,PROJECT,buffer);
       return -1;
    }

    if(b->type)
    {
       strcpy(buffer,"Receiving folder ");
       ptr = buffer+strlen(buffer);
       strncpy(ptr,b->filename,b->nameSize);
       *(ptr+b->nameSize) = 0;
       icqWriteSysLog(icq,PROJECT,buffer);
       return 0;
    }

    cfg->number++;

    e = (struct _footer *) (b->filename + b->nameSize);

    DBGTrace(e->fileSize);
    DBGTrace(e->x1);
    DBGTrace(e->speed);
    icqv8_setPeerSpeed(icq, cfg, e->speed);

    ptr = icqv8_mountFilePath(icq, cfg, buffer);

    if(!ptr)
    {
       icqv8_fileStop(icq,cfg);
       return 0;
    }

    strncpy(ptr,b->filename,b->nameSize);
    *(ptr+b->nameSize) = 0;

    DBGMessage(buffer);

    if(cfg->arq)
       fclose(cfg->arq);

    cfg->arq = fopen(buffer,"ab");

    DBGTracex(cfg->arq);

    if(!cfg->arq)
    {
       icqWriteSysRC(icq, PROJECT, -1, buffer);
       icqv8_fileStop(icq,cfg);
       return 0;
    }

    DBGTrace(ftell(cfg->arq));

    memset(ack,0,sizeof(struct _ack));
    ack->cmd    = 0x03;
    ack->offset = ftell(cfg->arq);
    ack->speed  = cfg->speed;
    ack->number = cfg->number;

    icqv8_sendBlock(cfg->sock, sizeof(struct _ack), ack );

    return 0;
 }


