/*
 * request.c - Accept/Refuse requests
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

#ifdef linux
 #include <unistd.h>
#endif

 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>

 #include "peer.h"

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

// static void ackFile(HICQ, ULONG, PEERDATA *, ULONG, char *, const char *, USHORT, USHORT);
 static void sendFile(HICQ , PEERSESSION *);
// static int  initialize(HICQ, PEERSESSION *);
 static void initFileSend(HICQ, PEERSESSION *);

 static void _System beginSession(ICQTHREAD *thd);

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 int icqv8_sendFile(HICQ icq, PEERDATA *msgSes, int port, char *req)
 {
    char        buffer[2050];
    HPEER       cfg              = msgSes->peer; // icqGetPluginDataBlock(icq, module);
    char        *fileName        = req+6;
    PEERSESSION *session;

    DBGMessage(fileName);

    session = icqAddElement(cfg->sessions, sizeof(PEERSESSION)+strlen(fileName)+5);

    if(!session)
    {
       icqWriteSysLog(icq,PROJECT,"Session creation error when sending file");
       return 2;
    }

    icqv8_initSession(icq, cfg, session, msgSes->uin);

    strcpy( (char *) (session+1), fileName );

    session->port    = port;
    session->flags   = PEERFLAG_ACTIVE|PEERFLAG_FILE;
    session->counter = *((USHORT *) req     );
    session->bytes   = *((ULONG *)  (req+2) );

    session->thread  = icqCreateThread(icq, beginSession, 16384, 0, session, "pfs");

    sprintf(buffer,"Peer session %08lx sending file to %s (ICQ#%ld),tid=%d",(ULONG) session, icqQueryUserNick(icqQueryUserHandle(icq,msgSes->uin)), msgSes->uin, (int) session->thread);
    icqWriteSysLog(icq,PROJECT,buffer);

    return 0;
 }

 static void _System beginSession(ICQTHREAD *thd)
 {
    while( ! ((PEERSESSION *) thd->parm)->thread )
    {
#ifdef __OS2__
       DosSleep(100);
#else
       usleep(100);
#endif
    }

    DBGMessage("Thread de envio de arquivo iniciada");
    sendFile(thd->icq,(PEERSESSION *) thd->parm);
    DBGMessage("Thread de envio de arquivo terminada");
 }

 static void sendFile(HICQ icq, PEERSESSION *cfg)
 {
    unsigned char *pkt = NULL;
    int           sz;
    char	  buffer[2050];

    cfg->flags |= PEERFLAG_ACTIVE;

    if(icqv8_connectPeer(icq,cfg,FALSE))
    {
       icqRemoveElement(cfg->peer->sessions, cfg);
       return;
    }

    DBGMessage("---[ Sessao de envio de arquivo iniciada ]---");

    DBGTracex(cfg->flags & PEERFLAG_ACTIVE);

    icqv8_sendPeerHello(icq,cfg);

    cfg->idle = 0;
    DBGTracex(cfg->flags & PEERFLAG_ACTIVE);

    while( (cfg->flags & PEERFLAG_ACTIVE) && cfg->peer->active && icqIsActive(icq))
    {
       cfg->idle = 0;

       DBGTrace(icqPeekSocket(cfg->sock, cfg->delay));

       if( !(cfg->flags & PEERFLAG_READY) || icqPeekSocket(cfg->sock, cfg->delay) )
       {
          sz = icqv8_recvPeer(icq,cfg->peer,cfg->sock,(void **) &pkt);
          DBGTrace(sz);

          if(sz <= 0)
          {
             CHKPoint();
             cfg->flags &= ~PEERFLAG_ACTIVE;
          }
          else
          {
             DBGTrace(*pkt);
             if(icqv8_execPeer(icq,cfg,sz,pkt))
                cfg->flags &= ~PEERFLAG_ACTIVE;
             else if(*pkt == 0xFF)
                initFileSend(icq,cfg);

             free((void *) pkt);
             pkt = NULL;
          }
       }
       else if(cfg->arq)
       {
          /* Carregar o arquivo */
          DBGMessage("********************* ENVIAR BLOCO DE DADOS **********************");

          *buffer = 0x06;
          sz      = fread(buffer+1, 1, 2048, cfg->arq);

          switch(sz)
          {
          case 0:	/* Fim do arquivo */
             DBGMessage("******* EOF *******");
             cfg->flags &= ~PEERFLAG_READY;
             cfg->flags |= PEERFLAG_COMPLETE;
             sprintf(buffer,"File sent by session %08lx",(ULONG) cfg);
             DBGMessage(buffer);
             icqWriteSysLog(icq,PROJECT,buffer);
             fclose(cfg->arq);
             cfg->arq = NULL;
             break;

          case -1:      /* Erro de leitura do arquivo */
             icqWriteSysRC(icq, PROJECT, -1, "Error reading data file");
             cfg->flags &= ~PEERFLAG_ACTIVE;
             break;

          default:
             DBGPrint("Enviar %d bytes de dados",sz);
             icqv8_sendBlock(cfg->sock, sz+1, buffer);

          }
       }
       else
       {
          DBGMessage("**** Envio de arquivo terminado ****");
          cfg->flags &= ~PEERFLAG_ACTIVE;
       }
    }

    DBGMessage("Terminando sessao de envio de arquivo");

    cfg->idle = 0;

    if(pkt)
       free((void *) pkt);

    icqv8_terminateSession(icq,cfg);

    cfg->thread = 0;

    DBGMessage("---[ Sessao de envio de arquivo terminada ]---");
 }


 static void initFileSend(HICQ icq, PEERSESSION *cfg)
 {
    #pragma pack(1)

    struct _packet
    {
       UCHAR  cmd;      // UBYTE  00  CMD  The command: PEER_FILE_INIT.
       ULONG  x1;       // DWORD 00 00 00 00 EMPTY Unknown: empty.
       ULONG  counter;  // DWORD.L xx xx xx xx COUNT Total number of files to be sent.
       ULONG  bytes;    // DWORD.L xx xx xx xx BYTES Total bytes of all files to sent.
       ULONG  speed;    // DWORD.L xx xx xx xx SPEED The sender's speed: 0 = PAUSE, 64 = no delay between packets, 0 < n < 64 = (64-n) * 0.05s delay.
       USHORT nickSize; // LNTS xx xx ... NICK Sender's nick.
       UCHAR  nickName[1];
    } *pkt;

    char *buffer[sizeof(struct _packet)+82];

    memset( pkt = (struct _packet *) buffer,0,sizeof(struct _packet)+81);

    DBGTrace(cfg->counter);
    DBGTrace(cfg->bytes);

    pkt->counter = cfg->counter;
    pkt->bytes   = cfg->bytes;
    pkt->speed   = icqLoadValue(icq, "ftSpeed", 64);

    cfg->flags  |= PEERFLAG_INIT;

    icqLoadString(icq, "NickName", "pwICQ User", pkt->nickName, 80);

    DBGMessage(pkt->nickName);
    pkt->nickSize = strlen(pkt->nickName)+1;

    icqv8_sendBlock(cfg->sock, sizeof(struct _packet)+pkt->nickSize-1, pkt);

 }

 int icqv8_peerFileStart(HICQ icq, PEERSESSION *cfg, const char *fileName)
 {
 /*
  Data type  Content     Name            Description
  BYTE       02          CMD             The command: PEER_FILE_START.
  BYTE       00          FILE/DIRECTORY  Indicates File with 0 and directory with 1.
  LNTS       xx xx ...   FILENAME        The file name of the file to transfer.
  LNTS       xx xx ...   TEXT            Some unknown text.
  DWORD.L    xx xx xx xx FILELEN         The length of the file to transfer.
  DWORD      00 00 00 00 EMPTY           Unknown: empty.
  DWORD.L    xx xx xx xx SPEED           The sender's speed, again.
 */
    #pragma pack(1)

    char buffer[0x0200];

    struct _beginPacket
    {
       UCHAR  cmd;	   // BYTE       02          CMD             The command: PEER_FILE_START.
       UCHAR  type;	   // BYTE       00          FILE/DIRECTORY  Indicates File with 0 and directory with 1.
       USHORT nameSize;    // LNTS       xx xx ...   FILENAME        The file name of the file to transfer.
       UCHAR  fileName[1];
    } *b = (struct _beginPacket *) buffer;

    struct _endPacket
    {
       USHORT textSize;   // LNTS       xx xx ...   TEXT            Some unknown text.
       UCHAR  text;
       ULONG  fileSize;   // DWORD.L    xx xx xx xx FILELEN         The length of the file to transfer.
       ULONG  x1;         // DWORD      00 00 00 00 EMPTY           Unknown: empty.
       ULONG  speed;      // DWORD.L    xx xx xx xx SPEED           The sender's speed, again.
    } *e;

    char *ptr = NULL;
    char *f;

    CHKPoint();

    sprintf(buffer,"Session %08lx sending file ",(ULONG) cfg);
    strncat(buffer,fileName,0xFF);

    icqWriteSysLog(icq,PROJECT,buffer);
    DBGMessage(buffer);

    memset(buffer,0,0x01FF);

    b->cmd      = 0x02;

    for(f=(char *)fileName;*f;f++)
    {
#ifdef linux
       if(*f == '/')
          ptr = f+1;
#else
       if(*f == '\\' || *f == ':')
          ptr = f+1;
#endif
    }

    DBGMessage(ptr);

    b->nameSize = strlen(ptr)+1;
    if(b->nameSize > 0xFF)
       b->nameSize = 0xFF;

    strncpy(b->fileName,ptr,b->nameSize);

    e = (struct _endPacket *) (b->fileName+b->nameSize);

    e->textSize = 1;
    e->speed    = cfg->speed;

    /* Abre o arquivo, obtem o tamanho */
    if(cfg->arq)
       fclose(cfg->arq);

    cfg->arq = fopen(fileName,"rb");

    if(!cfg->arq)
    {
       DBGMessage("Erro ao abrir arquivo de origem");
       cfg->flags &= ~PEERFLAG_ACTIVE;
       icqWriteSysRC(icq, PROJECT, -1, buffer+sizeof(struct _beginPacket));
       return 0;
    }

    if(fseek(cfg->arq, 0L, SEEK_END))
    {
       icqWriteSysRC(icq, PROJECT, -1, "Seek error");
       cfg->flags &= ~PEERFLAG_ACTIVE;
       return 0;

    }

    e->fileSize = ftell(cfg->arq);
    icqv8_sendBlock(cfg->sock, sizeof(struct _beginPacket)+sizeof(struct _endPacket)+b->nameSize-1, buffer);

    return 0;
 }


