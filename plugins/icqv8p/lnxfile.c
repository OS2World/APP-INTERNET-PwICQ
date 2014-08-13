/*
 * lnxfile.c - Linux file management
 */

 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>
 #include <stdlib.h>

 #include "peer.h"

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 char * icqv8_mountFilePath(HICQ icq, PEERSESSION *cfg, char *buffer)
 {
	HUSER usr   = icqQueryUserHandle(icq,cfg->uin);
	char  *ptr;
	
    icqLoadString(icq, "recpath", "", buffer, 0xFF);

    if(!*buffer)
	   strcpy(buffer,getenv("HOME"));

    if(!usr)
    {
       sprintf(buffer,"Unexpected error: No valid user %lx when processing file in session %08lx",cfg->uin,(ULONG) cfg);
       icqWriteSysLog(icq,PROJECT,buffer);
       return 0;
    }

    if(*buffer)
    {
       ptr = buffer+strlen(buffer);
    }
    else
    {
       ptr = buffer;
    }

    if(usr->flags & USRF_FILEDIR)
    {
       /* Monta o nome do arquivo */
       sprintf(ptr,"/%ld",usr->uin);
    }

    DBGMessage(buffer);
    return buffer+strlen(buffer);
 }
