/*
 * msg.c - Receive Greetings message
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>

 #include "peer.h"

 #pragma pack(1)

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static char *getString(HICQ, int, char **);
 static int  urlAck(HICQ, PEERSESSION *, USHORT, const char *, const char *, const char *);

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 int icqv8_rcvGreet(HICQ icq, PEERSESSION *cfg, int sz, PEER_MSG_HEADER *h)
 {
    #pragma pack(1)

    struct _pkt
    {
       USHORT cmd;          // WORD.L  xx xx   GCMD  The command in this packet. Valid values are:
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

    char   logBuffer[0x0100];
    int    strSize;
    char   *ptr       = (char *) (pkt+1);
    ULONG  msgNumber  = cfg->seqMasc|((ULONG) h->sequence);
    char   *title     = getString(icq,pkt->txtSize,&ptr);
    char   *reason    = NULL;
    char   *filename  = NULL;
    char   *textBlock = NULL;
    int    port       = 0;
    ULONG  fileSize   = 0;

    sz -= (sizeof(PEER_MSG_HEADER)+sizeof(struct _pkt)+h->msgSize+pkt->txtSize);

#ifdef DEBUG
    icqDumpPacket(icq, NULL, "ID Block 1", 16, (unsigned char *) pkt->x1);
#endif


    /* Bloco nao utilizado ate agora */

#ifdef DEBUG
    icqDumpPacket(icq, NULL, "ID Block 2", 16, (unsigned char *) ptr);
#endif

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

#ifdef DEBUG
    if(sz > 0)
    {
       DBGTracex(*( (ULONG *) ptr ));
    }
#endif


    /* Monta o bloco de dados, registra a requisicao e a mensagem */

    switch(pkt->cmd)
    {
    case MSG_GREET_URL:	        /* URL Message */
       icqInsertMessage(icq, cfg->uin, msgNumber, MSG_URL, 0, 0, strlen(reason), reason);
       urlAck(icq,cfg,h->sequence,title,reason,"");
       break;

    case MSG_GREET_FILE:	/* File message */

//   void * ICQAPI icqCreateRequest(HICQ, ULONG, BOOL, ULONG, ULONG, const REQUESTMGR *, void *, int);
    
       ptr = icqCreateRequest(    icq,
                                  cfg->uin,
                                  FALSE,
                                  (ULONG) cfg,
                                  msgNumber,
                                  &icqv8p_requestManager,
                                  (void *) cfg,
                                  strlen(filename)+strlen(reason)+strlen(title)+16 );


       if(ptr)
       {
          *( (USHORT *) ptr) = pkt->cmd;
          ptr += 2;
          *( (USHORT *) ptr) = port;
          ptr += 2;
          *( (ULONG *) ptr)  =  fileSize;
          ptr += 4;

          sprintf(ptr,"%s\x01%s\x01%s",title,reason,filename);

          textBlock = malloc(strlen(title)+strlen(reason)+strlen(filename)+40);

          if(!textBlock)
          {
             icqWriteSysLog(icq,PROJECT,"Memory allocation error when trying to store file transfer");
             icqAbend(icq,0);
          }
          else
          {
             sprintf(textBlock,"%s\x01%s\x01%s\x01%ld\x01%04d",title,reason,filename,fileSize,pkt->cmd);
             icqInsertMessage(icq, cfg->uin, msgNumber, MSG_FILE, 0, 0, strlen(textBlock), textBlock);
             free(textBlock);
          }
       }

       break;

    default:
       sprintf(logBuffer,"Unexpected msg code 0x%04x: ",(int) pkt->cmd);
       strncat(logBuffer,title,0xFF);
       icqWriteSysLog(icq,PROJECT,logBuffer);

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
       DBGMessage(ret);
       *ptr += sz;
    }
    else
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error");
       icqAbend(icq,0);
    }
    return ret;

 }

#ifdef DEBUG
 void debugUrlAck(HICQ icq, PEERSESSION *cfg, USHORT sequence)
 {
    urlAck(icq, cfg, sequence, "Send Web Page Address (URL)", "Site do MJ foi alterado......þhttp://www.infoguerra.com.br/infonews/viewnews.cgi?newsid1044045346,10847,/","");
 }
#endif

 static int urlAck(HICQ icq, PEERSESSION *cfg, USHORT sequence, const char *title, const char *url, const char *awayMsg)
 {
    static const char ackBlock[] = { 0x37, 0x1c, 0x58, 0x72, 0xe9, 0x87, 0xd4, 0x11, 0xa4, 0xc1, 0x00, 0xd0, 0xb7, 0x59, 0xb1, 0xd9 };

    int                 sz    = sizeof(PEER_MSG_HEADER)+strlen(title)+strlen(url)+strlen(awayMsg)+48;
    PEER_MSG_HEADER     *h    = malloc(sz+5);
    char		*ptr;
    int                 f;

    if(!h)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when creating URL Ack packet");
       return -1;
    }

    memset(h,0,sz);

    h->cmd      = 0x02;
    h->action   = 0x07da;
    h->x1       = 0x0e;
    h->sequence = sequence;
    h->msgType  = 0x1a;
    h->msgSize  = strlen(awayMsg)+1;

    ptr = (char *) (h+1);
    strcpy(ptr,awayMsg);
    ptr += h->msgSize;

    *( (USHORT *) ptr ) = MSG_GREET_URL;
    ptr += 2;

    for(f=0;f<sizeof(ackBlock);f++)
       *(ptr++) = ackBlock[f];

    ptr += 2;

    *( (USHORT *) ptr ) = strlen(title);
    ptr += 4;

    strcpy(ptr,title);
    ptr += strlen(title);

    *(ptr+5) = 0x01;

    ptr += 15;

    *( (USHORT *) ptr ) = strlen(url)+4;
    ptr += 4;

    *( (USHORT *) ptr ) = strlen(url);
    ptr += 4;

    strcpy(ptr,url);
    ptr += strlen(url);

    DBGTrace(sz);
    DBGTrace( ((ULONG) ptr) - ((ULONG) h));

    ptr = (UCHAR *) h;

    icqDumpPacket(icq, NULL, "URL Ack packet", sz-1, ptr+1);

    icqv8_encript(ptr+1, sz-1);
    icqv8_sendBlock(cfg->sock, sz, h);

    return 0;
 }





