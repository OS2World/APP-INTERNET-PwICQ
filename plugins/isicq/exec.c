/*
 * send.c - I-Share send packets
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #include <types.h>
 #include <sys\socket.h>
#endif

 #include <stdio.h>
 #include <malloc.h>
 #include <string.h>

 #include "ishared.h"

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/

 static void listResults(HICQ, ISHARED_CONFIG *, const char *);
 static void pingHost(HICQ, ISHARED_CONFIG *, const char *);

/*---[ Implementation ]----------------------------------------------------------------------------------*/

#ifndef PWICQ
 #error PWICQ Plugin Only
#endif

 int ICQAPI isicq_Interpret(HICQ icq, const char *cmd, const char *parm)
 {
    ISHARED_CONFIG  *cfg = icqGetPluginDataBlock(icq, module);

    DBGTracex(cfg);
	
    if(!strcmp(cmd,"isearch"))
    {
       DBGMessage(parm);
       ishare_search(cfg, parm, 0);
       return 0;
    }
    else if(!strcmp(cmd,"ilist"))
    {
       listResults(icq,cfg,parm);
       return 0;
    }
    else if(!strcmp(cmd,"iget"))
    {
       CHKPoint();
       return 0;
    }
    else if(!strcmp(cmd,"iping"))
    {
       pingHost(icq,cfg,parm);
       return 0;
    }

    return -1;
 }

 static void listResults(HICQ icq, ISHARED_CONFIG *cfg, const char *key)
 {
    CHKPoint();

    if(!icqQueryTextModeFlag(icq))
       return;

    DBGMessage(key);

 }

 static void pingHost(HICQ icq, ISHARED_CONFIG *cfg, const char *key)
 {
    long ip = ishare_GetHostByName((unsigned char *) key);
    int  f;
    char logBuffer[0x0100];

    if(ip)
    {
       strcpy(logBuffer,"Searching for ");
       ishare_formatIP(ip,logBuffer+14);
       icqWriteSysLog(icq,PROJECT,logBuffer);
       for(f=0;f<10;f++)
       {
          ishare_hello(cfg, ip);
#ifdef __OS2__
          DosSleep(200);
#endif

#ifdef linux
          usleep(2000);
#endif
       }
    }
 }


