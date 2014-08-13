/*
 * sendmsg.c - Send Message
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

 static USHORT  regularMessage(HICQ, HUSER, USHORT, const char *, ULONG *, PEERDATA *);

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 USHORT  _System icqv8_sendMessage(HICQ icq, HUSER usr, USHORT type, const char *txt, ULONG *id, PEERDATA *cfg)
 {
//     int pos;

     switch(type)
     {
     case MSG_URL:
        return icqPeer_sndGreet(icq, cfg, MSG_GREET_URL, 0, 0, "", "Send Web Page Address (URL)", -1, txt, -1, NULL, 0, 0) == 0 ? -1 : 0;
        break;

     default:
        return regularMessage(icq, usr, type, txt, id, cfg);
     }

     icqWriteSysLog(icq,PROJECT,"Failure processing message type");
     return -1;

 }

 static USHORT  regularMessage(HICQ icq, HUSER usr, USHORT type, const char *txt, ULONG *id, PEERDATA *cfg)
 {
    char            buffer[0x0100];
    PEER_MSG_HEADER *h;
    UCHAR           *ptr;
    ULONG           sz;		
    USHORT	    sequence;

    DBGTracex(usr);
    DBGTracex(cfg);

    if(!(usr&&cfg))
    {
       icqWriteSysLog(icq,PROJECT,"Invalid Parameter when sending message");
       return 1;
    }

    if(cfg->sz != sizeof(PEERSESSION))
    {
       icqWriteSysLog(icq,PROJECT,"Invalid session information when sending message");
       return 2;
    }

    if(!txt)
       txt = "";

    sz = sizeof(PEER_MSG_HEADER)+strlen(txt)+9;
    h  = malloc(sz+4);

    memset(h,0,sizeof(PEER_MSG_HEADER));

    h->cmd      = 0x02;
    h->action   = 0x07ee;
    h->x1       = 0x0e;
    h->msgType  = type;
//    h->status  = ;

    sequence    =
    h->sequence = cfg->sequence--;

    h->flags    = 0x0010;
    h->msgSize  = strlen(txt)+1;

    if(id)
       *id = cfg->seqMasc | ((ULONG) h->sequence);

    ptr         = (UCHAR *) (h+1);

    strncpy(ptr,txt,h->msgSize);

    ptr += (h->msgSize-1);
    *(ptr++) = 0;

    *((ULONG *) ptr) = icqLoadValue(icq,"msgforeground",0x00000000);
    ptr += 4;

    *((ULONG *) ptr) = icqLoadValue(icq,"msgbackground",0x00FFFFFF);
    ptr += 4;

    sprintf(buffer,"Sending peer message %08lx to %s (ICQ#%ld)",cfg->seqMasc | (ULONG) sequence,icqQueryUserNick(usr),usr->uin);
    icqWriteSysLog(icq,PROJECT,buffer);

    icqDumpPacket(icq, usr, "Peer message unencripted", sz, (unsigned char *) h);

    ptr = (UCHAR *) h;
    icqv8_encript(ptr+1, sz-1);

//    icqDumpPacket(icq, usr, "Peer message encripted", sz, (unsigned char *) h);

    if(icqv8_sendBlock(cfg->sock, sz, h))
       icqWriteNetworkErrorLog(icq, PROJECT, "Error sending message");

    free(h);

    return 0;
 }





