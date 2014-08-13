/*
 * os2pipe.C    pwICQ plugin to send messages to a Pipe
 *              (Designed for use with SysBar/2)
 *
 */

 #pragma strings(readonly)

 #include <string.h>

 #include "icqpipe.h"


/*---[ Implementation ]-------------------------------------------------------------------------------*/

 void icqpipe_loadConfig(HICQ icq, struct icqpipe_config *cfg)
 {
    icqLoadString(icq,"icqPipe:Name","\\PIPE\\pwICQ",cfg->pipeName,0xFF);
    cfg->flags = icqLoadValue(icq, "icqPipe:Flags", 0x03);
 }


 void icqpipe_Write(HICQ icq, struct icqpipe_config *cfg, const char *text)
 {
    FILE *f = fopen(cfg->pipeName,"w");

    if(f)
    {
       fprintf(f,"%s\n",text);
       fclose(f);
    }
    else
    {
       icqWriteSysLog(icq, PROJECT,"Can't open pipe");

       /* The line below is wrote in file pwicq.dbg if pwICQ is started with -d */
       /* perror("ICQPIPE File error"); */
    }

    #ifdef LOG_ALL
       icqWriteSysLog(icq, PROJECT,text);
    #endif

 }


