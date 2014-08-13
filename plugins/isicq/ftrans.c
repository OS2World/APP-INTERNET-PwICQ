/*
 * FTRANS.C - File transfer procedures
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>

 #include "ishared.h"

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/


/*---[ Implementation ]----------------------------------------------------------------------------------*/

 int ICQAPI ishare_requestFile(ISHARED_CONFIG *cfg, long ip, USHORT port, USHORT fileID, const UCHAR *md5, const char *fileName, ULONG fileSize)
 {
    /*
     * Try to request a specific file
     */
    ISHARED_TRANSFER    *trf = malloc(sizeof(ISHARED_TRANSFER)+strlen(fileName)+1);
    ISHARE_TCP_ERROR    sts;
    ISHARE_TCP_FILEREQ  req;
    int                 f;
//    char		buffer[0x0100];

    if(!trf)
    {
       log(cfg,"Memory allocation error when starting transfer");
       return -1;
    }

    memset(trf,0,sizeof(ISHARED_TRANSFER));

    trf->adm.type      = 1;	// 0 = Admin session 1 = File transfer
//    trf->adm.sock;
//    trf->adm.level;
//    trf->adm.rc;
//    trf->adm.thread;
    trf->adm.cfg       = cfg;
    trf->fileSize      = fileSize;
    trf->fileID        = fileID;

    for(f=0;f<16;f++)
       trf->md5[f] = md5[f];

    strcpy(trf->filename,fileName);

    if(ishare_createControlFile(trf))
    {
       free(trf);
       return -1;
    }

    trf->adm.sock = ishare_OpenTCP(0);

    if(trf->adm.sock < 0)
    {
       netlog(cfg,"Can't create file request sock");
       free(trf);
       return -1;
    }

    trf->adm.sock = ishare_ConnectSock(trf->adm.sock,ip,ISHARE_TCP_PORT);
    DBGTrace(trf->adm.sock);

    if(trf->adm.sock < 0)
    {
       /*
        * TO DO:
        *
        * Send an UDP request to the other side asking for a reverse connection
        *
        */
       log(cfg,"Can't connect user");
       free(trf);
       return -1;
    }

    memset(&sts,0,sizeof(sts));

    sts.h.marker = ISHARE_TCP_MARKER;
    sts.h.id     = ISHARETCP_BEGINTRANSFER;

    memset(&req,0,sizeof(req));

    ishare_Send(trf->adm.sock,&sts.h,sizeof(sts.h));

    req.r.fileID = fileID;
    req.speed    = 0xFFFF;
    req.mtu      = 0xFFFF;
    req.start    = 0;
    req.end      = fileSize;

    for(f=0;f<16;f++)
       req.r.md5[f] = trf->md5[f];

    ishare_Send(trf->adm.sock,&req,sizeof(req));
    ishare_insertItem(&cfg->admUsers,&trf->adm.l);

    return ishare_beginSession((ISHARED_ADMIN *) trf);
 }


