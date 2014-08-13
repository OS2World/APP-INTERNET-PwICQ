/*
 * request.c - Accept/Refuse requests
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

 static void ackFile(HICQ, ULONG, PEERDATA *, ULONG, char *, const char *, USHORT, USHORT);

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 int _System icqv8_accept(HICQ icq, ULONG id, PEERDATA *cfg, ULONG session, char *ptr, const char *msg)
 {
    DBGTracex(id);
    DBGTracex(  *( (USHORT *) ptr) );

    switch(*( (USHORT *) ptr))
    {
    case MSG_GREET_FILE:
       DBGTrace(id);
       icqSetRequestSession(icq, id, FALSE, (ULONG) cfg->peer);
       CHKPoint();
       ackFile(icq,id,cfg,*( (ULONG *) (ptr+4) ), ptr+8, msg, MSG_GREET_FILE_ACCEPTED, 0x00);
       CHKPoint();
       break;

//    case MSG_GREET_CHAT:
//       break;

    default:
       icqWriteSysLog(icq,PROJECT,"Unexpected accept request received");
       break;

    }

    return 0;
 }


 int _System icqv8_refuse(HICQ icq, ULONG id, PEERDATA *cfg, ULONG session, char *ptr, const char *msg)
 {
    DBGTracex(id);
    DBGTracex(  *( (USHORT *) ptr) );

    switch(*( (USHORT *) ptr))
    {
    case MSG_GREET_FILE:
       ackFile(icq,id,cfg,*( (ULONG *) (ptr+4) ), ptr+8, msg, MSG_GREET_FILE_REJECTED, 0x01);
       break;

//    case MSG_GREET_CHAT:
//       break;

    default:
       icqWriteSysLog(icq,PROJECT,"Unexpected refuse request received");
       break;

    }


    return 0;
 }

 static void ackFile(HICQ icq, ULONG id, PEERDATA *cfg, ULONG fileSize, char *info, const char *msg, USHORT type, USHORT status)
 {
    char *tag[] = { info, "", "" };
    int  f;

    DBGTrace(fileSize);
    DBGMessage(info);

    for(f=0;f<3 && *info; info++)
    {
       if(*info == 0x01)
       {
          tag[++f] = info+1;
          *info = 0;
       }
    }

    DBGMessage(tag[0]);
    DBGMessage(tag[1]);
    DBGMessage(tag[2]);

    icqPeer_sndGreet(icq, cfg, type, id, status, msg, tag[0], -1, tag[1], -1, tag[2], -1, fileSize);

 }

 int _System icqv8_accepted(HICQ icq, ULONG  id, PEERDATA *cfg, ULONG session, int port, char *req)
 {

    DBGTrace(port);
    DBGTracex( *( (USHORT *) req) );

    switch(*( (USHORT *) req))
    {
    case MSG_GREET_FILE:
       return icqv8_sendFile(icq, cfg, port, req+2);

    default:
       icqWriteSysLog(icq,PROJECT,"Invalid accepted request type");
    }

    return -1;
 }


