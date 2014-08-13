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

 BEGIN_TCP_PACKET_PROCESSOR( BEGINTRANSFER )
 {
    ISHARE_DECLARE_TCP_PREFIX(h,SENDINGFILE);

    ISHARE_TCP_FILEREQ 		    req;
    const ISHARED_FILEDESCR 	*fl;
    ULONG			            bytes;
    int                         f;
    ISHARE_TCP_SENDINGFILE      *snd = (ISHARE_TCP_SENDINGFILE *) buffer;

    CHKPoint();

    if(!ishare_RecBlock(adm->sock, sizeof(ISHARE_TCP_FILEREQ), &req, &adm->rc))
    {
       adm->rc = ISHARE_STATUS_MALFORMED;
       return 0;
    }

    DBGTrace(req.r.fileID);
    DBGTrace(req.speed);
    DBGTrace(req.mtu);

    if(adm->hFile)
    {
       adm->rc = ISHARE_STATUS_BUSY;
       return 0;
    }

    DBGTrace(req.start);
    DBGTrace(req.end);

    for(fl=(void *) cfg->sharedFiles.first; fl && fl->fileID != req.r.fileID; fl = (void *) fl->l.down);

    DBGTracex(fl);

    if(!fl || req.end <= req.start)
    {
       adm->rc = ISHARE_STATUS_INVALIDREQUEST;
       return 0;
    }

    if(memcmp((void *) fl->md5,(void *) req.r.md5,16))
    {
       adm->rc = ISHARE_STATUS_INVALIDREQUEST;
       return 0;
    }

    adm->hFile = fopen(fl->filePath,"rb");

    DBGTracex(adm->hFile);

    if(!adm->hFile)
    {
       adm->rc = ISHARE_STATUS_FILEERROR;
       return 0;
    }

    if(fseek(adm->hFile, req.start, SEEK_SET))
    {
       adm->rc = ISHARE_STATUS_FILEERROR;
       return 0;
    }

    ishare_Send(adm->sock,&h,sizeof(ISHARE_TCP_PREFIX));

    bytes = req.end - req.start;

    snd->speed  = 0xFFFF;
    snd->offset = req.start;
    snd->bytes  = bytes;

    for(f=0;f<16;f++)
       snd->md5[f] = fl->md5[f];

    ishare_Send(adm->sock,snd,sizeof(ISHARE_TCP_SENDINGFILE));

    while(bytes > 0 && adm->sock > 0)
    {
       f = bytes > 0xFF ? 0xFF : bytes;

       if(adm->rc)
       {
          DBGMessage("Canceling!");
          ishare_CloseSocket(adm->sock);
          adm->sock = -1;
       }
       else
       {
          if(!fread(buffer, f, 1, adm->hFile))
          {
             DBGMessage("Error reading file");
             ishare_CloseSocket(adm->sock);
             adm->sock = -1;
          }

          if(ishare_Send(adm->sock,buffer,f))
          {
             DBGMessage("Error sending data block");
             ishare_CloseSocket(adm->sock);
             adm->sock = -1;
          }
       }
       bytes -= f;

    }

    fclose(adm->hFile);

    END_TCP_PACKET_PROCESSOR( BEGINTRANSFER );
 }



