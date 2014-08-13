/*
 * rcvfile - Receive a file
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>

 #include "md5.h"
 #include "ishared.h"

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/


/*---[ Implementation ]----------------------------------------------------------------------------------*/

 BEGIN_TCP_PACKET_PROCESSOR( SENDINGFILE )
 {
    ISHARE_TCP_SENDINGFILE *pkt = (ISHARE_TCP_SENDINGFILE *) buffer;
    ULONG                  bytes;
    ULONG		           offset;
    ULONG		           begin;
    ULONG		           fileKey;
    MD5_CTX                mdContext;
    int			           f;

    DBGTrace(sizeof(ISHARE_TCP_SENDINGFILE));

    if(!ishare_RecBlock(adm->sock, sizeof(ISHARE_TCP_SENDINGFILE), pkt, &adm->rc))
    {
       adm->rc = ISHARE_STATUS_MALFORMED;
       return 0;
    }

    offset = pkt->offset;
    bytes  = pkt->bytes;

    DBGTracex(adm->hFile);
    if(adm->hFile)
    {
       adm->rc = ISHARE_STATUS_BUSY;
       return 0;
    }

    DBGTrace(adm->type);

    if(adm->type == 1)
    {
       /* I'm waiting for the file, lets get-it */
       fileKey = ((ISHARED_TRANSFER *) adm)->fileKey;
    }
    else
    {
       /*
        * TO-DO:
        *
        * The current process is really unsafe! The best way is checking to see if I asked
        * the other side for this file.
        *
        * For now let's trust in the fileKey sent.
        *
        */
       fileKey = pkt->identifier;
    }

    sprintf(ishare_makeControlFilePath(cfg, buffer),"%08lx.dat", fileKey );
    DBGMessage(buffer);

    adm->hFile = fopen(buffer,"r+b");

    DBGTracex(adm->hFile);

    if(!adm->hFile)
    {
       adm->rc = ISHARE_STATUS_FILEERROR;
       return 0;
    }

    if(fseek(adm->hFile, offset, SEEK_SET))
    {
       adm->rc = ISHARE_STATUS_FILEERROR;
       return 0;
    }

    log(adm->cfg,"Receiving file");
    DBGTrace(bytes);
    begin = offset;

    MD5Init(&mdContext);

    while(bytes > 0 && !adm->rc && adm->sock > 0)
    {
       f = bytes > 0xFF ? 0xFF : bytes;
       f = ishare_Recv(adm->sock, buffer, f);

       if(f < 0 || adm->rc)
       {
          DBGMessage("Canceling!");
          ishare_CloseSocket(adm->sock);
          adm->sock = -1;
       }
       else if(f)
       {
          if( fwrite(buffer, f, 1, adm->hFile) == 1)
          {
             MD5Update (&mdContext, buffer, f);
             bytes  -= f;
             offset += f;
          }
          else
          {
             DBGMessage("Error writing file");
             ishare_CloseSocket(adm->sock);
             adm->sock = -1;
          }
       }
    }

    MD5Final (&mdContext);

    DBGTrace(bytes);
    DBGTrace(begin);
    DBGTrace(offset);

    fclose(adm->hFile);

    /*
     * Update the control-file
     */

    ishare_updateControlFile(adm->cfg,mdContext.digest,fileKey,begin,offset);

    END_TCP_PACKET_PROCESSOR( SENDINGFILE );
 }




