/*
 * STARTUP.C - Codigo de inicializacao
 */

 #define INCL_DOSPROCESS
 #define INCL_DOSSEMAPHORES
 #define INCL_DOSDATETIME
 #define INCL_DOSNLS
 #define INCL_WINCOUNTRY
 #define INCL_DOSMODULEMGR

#ifndef __OS2__
   #error Only for OS2 version
#endif

 #include <string.h>

 #define INCL_WIN
 #include <icqkernel.h>

 #include <stdio.h>
 #include <conio.h>
 #include <time.h>
 #include <stdlib.h>
 #include <process.h>

 #include <pwMacros.h>

 #include "icqcore.h"

 #include <icqqueue.h>


/*---[ Prototipos ]---------------------------------------------------------------*/

 static void _System timerThread(ICQTHREAD *);

 void _System convertCpg(HICQ, ULONG, ULONG, UCHAR *, int);

 static void  readQueue(HICQ);

/*---[ Statics ]------------------------------------------------------------------*/

 static ICQ     mainConfig;
 static char    *versionString = "0000 pwICQ version " PWICQVERSION " Build " BUILD " @ " HOSTNAME;

/*---[ Funcoes ]------------------------------------------------------------------*/

 char * EXPENTRY icqQueryVersionString(void)
 {
    sprintf(versionString,"%04x",sizeof(ICQ));
    return versionString;
 }

/*
 static void geraFP(HICQ icq)
 {
    int         f;
    char        src;
    FILE        *f1;
    FILE        *f2;

    f1 = fopen("saida.850","w");
    f2 = fopen("saida.1004","w");

    for(f=128;f<256;f++)
    {
       src = f;
       icqConvertCodePage(icq, TRUE, &src, 1);
       fprintf(f1,".m:%d:%d\n",(int) f, (int) src);

       src = f;
       icqConvertCodePage(icq, FALSE, &src, 1);
       fprintf(f2,".m:%d:%d\n",(int) f, (int) src);

    }

    fclose(f1);
    fclose(f2);

 }
*/

 HICQ EXPENTRY icqCoreStartup(int numpar, char **param, STATUSCALLBACK *startupStage)
 {
    /*
     * Inicializa o n£cleo, abre arquivo de configuraÆo
     */
    ULONG               f;
    char                *ptr;
    char                buffer[0x0100];
    COUNTRYCODE         country;
    COUNTRYINFO         info;
    int                 (EXPENTRY *icqnetw_Configure)(HICQ, HMODULE, char *);

#ifdef MEMWATCH
    DBGMessage("Using MemWatch");
    mwStatistics( 2 );
#endif

    startupStage(&mainConfig,ICQSTARTUP_ENVIRONMENT);

    mainConfig.hab = WinInitialize(0);
    DBGTracex(mainConfig.hab);

    if(!mainConfig.hab)
       return NULL;

    srand(time(NULL));

    memset(&mainConfig,0,sizeof(mainConfig));

    mainConfig.sz  = sizeof(mainConfig);
    mainConfig.usz = sizeof(USERINFO);

    /* Ajusto os defaults para conversao de code-page */

    f = sizeof(COUNTRYINFO);
    country.country = country.codepage = 0;
    if(!DosQueryCtryInfo(f,&country,&info,&f))
    {
       mainConfig.cpgIn         = info.codepage;
       mainConfig.countryCode   = info.country;

       DBGTrace(info.country);

       if(info.country == 55)
          mainConfig.cpgOut = 1004;
       else
          mainConfig.cpgOut = mainConfig.cpgIn;
    }

    icqSetCodePageConverter(&mainConfig, convertCpg);
//    mainConfig.convertCpg = convertCpg;

//    geraFP(&mainConfig);


    /* Obtenho o caminho basico para o programa */

    strncpy(ptr = mainConfig.programPath,param[0],0xFF);
    for(f=0;*(mainConfig.programPath+f);f++)
    {
       if(*(mainConfig.programPath+f) == '\\' || *(mainConfig.programPath+f) == ':')
          ptr = mainConfig.programPath+f;
    }
    *ptr= 0;
    strncat(mainConfig.programPath,"\\",0xFF);

    DBGMessage(mainConfig.programPath);

    /* Inicializo dados de controle */
    mainConfig.ready = TRUE;

    /* Obtenho o bloco de mem¢ria compartilhada */

    if(!DosAllocSharedMem((PPVOID)&mainConfig.shareBlock,PWICQ_SHAREMEM,sizeof(ICQSHAREMEM),PAG_COMMIT|PAG_READ|PAG_WRITE))
    {
       DBGMessage("Bloco de memoria compartilhada criado");
       memset(mainConfig.shareBlock,0,sizeof(ICQSHAREMEM));
       mainConfig.shareBlock->sz         = sizeof(ICQSHAREMEM);
       mainConfig.shareBlock->si         = sizeof(ICQINSTANCE);
       mainConfig.shareBlock->instances  = 1;

    }
    else if(!DosGetNamedSharedMem((PPVOID) &mainConfig.shareBlock,PWICQ_SHAREMEM,PAG_READ|PAG_WRITE))
    {
       DBGMessage("Consegui acesso ao bloco de memoria compartilhada");

       if(mainConfig.shareBlock->sz == sizeof(ICQSHAREMEM) && mainConfig.shareBlock->si == sizeof(ICQINSTANCE))
       {
          mainConfig.shareBlock->instances++;
       }
       else
       {
          DosFreeMem(mainConfig.shareBlock);
          mainConfig.shareBlock = NULL;
       }
    }

    DBGTracex(mainConfig.shareBlock);

    /* Verifico o modulo de rede */

    startupStage(&mainConfig,ICQSTARTUP_MPTS);
    if(DosLoadModule(buffer, 0x0FF, (PSZ) "ICQNETW", &mainConfig.icqNetW))
    {
       startupStage(&mainConfig,-1);
       DBGMessage("Nao consegui carregar modulo de rede");
       icqCoreTerminate(&mainConfig);
       return NULL;
    }

    if(DosQueryProcAddr(mainConfig.icqNetW, 0, "icqnetw_Configure", (PFN *) &icqnetw_Configure))
    {
       icqWriteSysLog(&mainConfig,PROJECT,"Unable to verify network module");
       startupStage(&mainConfig,-2);
       icqCoreTerminate(&mainConfig);
       return NULL;
    }

    /* Faco a inicializacao do modulo comum */
    initialize(&mainConfig,numpar,param,startupStage);

    *buffer = 0;
    f = icqnetw_Configure(&mainConfig,mainConfig.icqNetW,buffer);

    if(*buffer)
       icqWriteSysLog(&mainConfig,PROJECT,buffer);

    if(f)
    {
       startupStage(&mainConfig,-3);
       icqCoreTerminate(&mainConfig);
       return NULL;
    }

    icqCreateThread(&mainConfig, timerThread, 8192, 0, NULL, "tmr");

    return &mainConfig;
 }

 int EXPENTRY icqCoreTerminate(HICQ icq)
 {
    if(!icq)
       return -2;

    if(icq->sz != sizeof(mainConfig))
       return -1;

    if(mainConfig.icqNetW)
       DosFreeModule(mainConfig.icqNetW);

    terminate(icq);

    if(icq->shareBlock)
    {
       icq->shareBlock->instances--;
       DosFreeMem(icq->shareBlock);
    }

    /* Fecha dados de controle */
    if(mainConfig.ini)
       PrfCloseProfile(mainConfig.ini);

    if(mainConfig.hab)
       WinTerminate(mainConfig.hab);

    sysLog(icq,"pwICQ core stopped");

    return 0;
 }

 char * EXPENTRY icqQueryPath(HICQ icq, const char *ext, char *buffer, int size)
 {
    icqLoadString(icq, "config-files", "", buffer, 0xFF);

    if(!*buffer)
       strncpy(buffer,icq->programPath,0xFF);
    strncat(buffer,ext,0xFF);
//    DBGMessage(buffer);

    return buffer;
 }

 static void _System timerThread(ICQTHREAD *thd)
 {
    DATETIME     d;
    HEV          sem     = 0;
    HTIMER       tmr     = 0;
    ULONG        qtd     = 0;
    int          rc;
    CHAR         Flags;
    short        minute;
    short        second;
    char         buffer[0x0100];

    DBGMessage("Iniciando timer...");

    DosSetPriority(PRTYS_THREAD , PRTYC_REGULAR, PRTYD_MAXIMUM, 0);

//    if(DosCreateEventSem(NULL,&sem,DC_SEM_SHARED,0))

    sprintf(buffer,"\\SEM32\\PWICQ\\TIMER\\%ld",(ULONG) thd->icq->uin);
    DBGMessage(buffer);

    rc = DosCreateEventSem(buffer,&sem,0,0);

    if(rc)
    {
       strncat(buffer," Can't be created",0xFF);
       icqWriteSysRC(thd->icq,PROJECT,rc,buffer);
       icqAbend(thd->icq,0);
       return;
    }

    if(DosStartTimer(100,(HSEM) sem,&tmr))
    {
       threadLog(thd,"Can't create timer");
    }
    else
    {
       threadLog(thd,"Timer enabled");

       DosGetDateTime(&d);
       minute = d.minutes;
       second = d.seconds;

       while(thd->icq->ready)
       {
          if(DosWaitEventSem(sem,2000) == 640)
          {
             threadLog(thd,"Semaphore timeout");
          }
          else if(DosResetEventSem(sem,&qtd))
          {
             threadLog(thd,"Failure in timer counter");
             icqAbend(thd->icq,0);
          }

          Flags = 0;

          DosGetDateTime(&d);
          if(second != d.seconds)
          {
             oneSecond(thd->icq);
             second = d.seconds;
             Flags |= 0x01;
          }
          if(minute != d.minutes)
          {
             oneMinute(thd->icq);
             minute = d.minutes;
             Flags |= 0x02;
          }

          /* Verifica se tem algo na queue */
          readQueue(thd->icq);

          /* Chama o timer do gerenciador de skins */

          if(thd->icq->skin && thd->icq->skin->timer)
             thd->icq->skin->timer(thd->icq,thd->icq->skinData,Flags);
       }
       DosStopTimer(tmr);
    }

    DosCloseEventSem(sem);

    saveMessageList(thd->icq);

    threadLog(thd, "Timer disabled");

 }

 void _System convertCpg(HICQ icq, ULONG cpIn, ULONG cpOut, UCHAR *ptr, int sz)
 {
    APIRET rc;
    USHORT t;

    if(sz <= 0)
       sz = strlen(ptr);

    if(cpIn == cpOut || cpIn == 0 || cpOut == 0)
       return;

    /* Loop de traducao */
    while(sz--)
    {
       /*

        Nota para a versÆo russa:

              "ya" at cp866  - code 239
              "ya" at cp1251 - code 255

        */

       if(*ptr > ' ')
       {
          t = WinCpTranslateChar(icq->hab, cpIn, *ptr, cpOut);
          *ptr = (t == 0) ? ' ' : (char) t;
       }
       ptr++;
    }
 }

 HICQ EXPENTRY icqQueryHandler(void)
 {
    return &mainConfig;
 }

 static void  readQueue(HICQ icq)
 {
    /* Esvazia a fila de entrada */
    ULONG               sz;
    REQUESTDATA         req;
    BYTE                pri;
    ICQQUEUEBLOCK       *info;

#ifdef EXTENDED_LOG
    char         logBuffer[0x0100];
#endif

    if(!icq->queue)
       return;

    sz = 0;

    if(DosQueryQueue(icq->queue,&sz))
       return;

    while(sz)
    {
       sz  = sizeof(ULONG);
       pri = 0;
       req.pid = getpid();

       if(DosReadQueue(icq->queue,&req,&sz,(PVOID) &info, 0, 0, &pri, 0))
       {
          icqWriteSysLog(icq,PROJECT,"Error reading queue");
          icqAbend(icq,0);
       }
       else
       {
#ifdef EXTENDED_LOG
          sprintf(logBuffer,"Extern request %d from process %d (0x%x)",req.ulData,req.pid,req.pid);
          icqWriteSysLog(icq,PROJECT,logBuffer);
#endif
          DBGTracex(sz);
          DBGTracex(info);

          if(!DosGetSharedMem((PVOID) info, PAG_WRITE | PAG_READ))
          {
             /* Processo o bloco em info */
             icqProcessRemoteBlock(icq,info);
             DosFreeMem(info);
          }
          else
          {
             icqWriteSysLog(icq,PROJECT,"Error when requesting shared memory block for queue command");
          }
       }

       sz = 0;
       if(DosQueryQueue(icq->queue,&sz))
          return;
    }

 }

// BOOL EXPENTRY ICQOpenWindow(HICQ ctl, HUSER user, HWND owner, const char *id, HWND (*callback)(HICQ, HWND, void *),void *parm)

 int EXPENTRY icqOpenWindow(HICQ icq, HUSER usr, const char *id, OPENWINCALLBACK *callback,void *parm)
 {

    if(!icq->skin)
       return 1;

    if(!icq->skin->openWindow)
       return 2;


 //     int      (* _System openWindow)(HICQ, HUSER, const char *, int (* _System)(HICQ, HWND, void *),void *);
    return icq->skin->openWindow(icq,usr,id,callback,parm);

 }

