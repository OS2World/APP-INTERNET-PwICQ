/*
 * INIT.C - Codigo comun de inicializacao/termino
 */

#ifdef __OS2__
 #define INCL_DOSPROCESS
 #include <process.h>
#endif

#ifdef linux

 #include <pthread.h>
 #include <unistd.h>
 #include <errno.h>

 #include <sys/types.h>
 #include <sys/ipc.h>
 #include <sys/msg.h>
 #include <sys/sem.h>
 #include <sys/shm.h>
 #include <sys/stat.h>

#endif

 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include <time.h>
 #include <icqkernel.h>

#ifndef DLL
 #include <icqplugins.h>
#endif

 #include <pwMacros.h>
 #include "icqcore.h"

/*
 #ifdef DEBUG
   #define FORCE_SAVEUSERS
 #endif
*/

/*---[ Prototipos ]-----------------------------------------------------------------*/

 static void readParms(HICQ,int,char **, ULONG *);
 static void loadExtendedPlugins(HICQ);
 static int  saveKeys(HICQ);

 static void _System startupThread(ICQTHREAD *);

/*---[ Statics ]--------------------------------------------------------------------*/


/*---[ Implementacao ]--------------------------------------------------------------*/

 static void mountKeyFileName(HICQ icq, char *buffer)
 {
    char key[0x0100];
    icqQueryPath(icq,"%ld.key",key,0xFF);
    sprintf(buffer,key,icq->uin);
 }

 #define MULTIPLIER      0x015a4e35L
 #define INCREMENT       1

 static UCHAR mountKeyValue(ULONG *seed)
 {
    *seed = MULTIPLIER * *seed + INCREMENT;
    return (UCHAR) (*seed & 0xFF);
 }

 static void mountKeyBlock(char *buffer, ULONG *seed)
 {
    int f;

    *seed = *( (ULONG *) buffer);

    for(f=4;f<0xFF;f++)
       buffer[f] = mountKeyValue(seed);

 }

 static int loadKeys(HICQ icq)
 {
	unsigned char  crc  = 0;
    ULONG          seed	= 0;
	
    unsigned char  key[0x0100];
    unsigned char  buffer[0x0100];
    unsigned char  result[0x0100];
	
    int            f;
    int            sz;
    int            pos;
    FILE           *arq;

    mountKeyFileName(icq,key);
    DBGMessage(key);

    arq = fopen(key,"r");

    if(!arq)
       return -1;

    memset(buffer,0,0xFF);
    fread(buffer,0xFF,1,arq);
    fclose(arq);

    for(f=0;f<4;f++)
       key[f] = buffer[f];

    mountKeyBlock(key,&seed);
    DBGTracex(seed);

    pos = (mountKeyValue(&seed) % 0x80);
    sz  = (key[pos] ^ buffer[pos]);

    if(sz)
    {
       icq->cntlFlags |= ICQFC_SAVEPASSWORD;
       for(f=0;f<sz;f++)
       {
          pos++;
          result[f]  = (key[pos] ^ buffer[pos]);
		  crc       ^= result[f];
       }
	
       result[f] = 0;

	   DBGTracex(crc);
	
	   pos++;
	   DBGTracex(key[pos] ^ buffer[pos]);
	
	   if( (key[pos] ^ buffer[pos]) == crc)
	   {
          if(!*icq->pwd)
             strncpy(icq->pwd,result,PASSWORD_SIZE);
	   }
	   else
	   {
		  icqWriteSysLog(icq,PROJECT,"Invalid or damaged key file");
	   }
	
       DBGMessage(result);
    }

    return 0;
 }

 static int saveKeys(HICQ icq)
 {
    unsigned char  keyFile[0x0100];
    unsigned char  buffer[0x0100];
	unsigned char  crc  = 0;
    ULONG          seed	= (ULONG) time(NULL);

    int            f;
    char           *ptr;
    int            sz;
    FILE           *arq;

    mountKeyFileName(icq,keyFile);
    DBGMessage(keyFile);

    for(f=0;f<4;f++)
       buffer[f] = mountKeyValue(&seed);

    mountKeyBlock(buffer,&seed);
    DBGTracex(seed);

    /* Copia a senha dentro do arquivo chave */
    ptr   = buffer + (mountKeyValue(&seed) % 0x80);
    sz    = strlen(icq->pwd);

    if(icq->cntlFlags & ICQFC_SAVEPASSWORD)
    {
       *ptr ^= ((char) sz);
       DBGTrace(sz);
       for(f=0;f<sz;f++)
       {
          ptr++;
		  crc  ^= icq->pwd[f];
          *ptr ^= icq->pwd[f];
       }
	   ptr++;
	   *ptr ^= crc;
    }

	DBGTracex(crc);
	
    /* Salva o arquivo chave */

    arq = fopen(keyFile,"w+");

    if(arq)
    {
       fwrite(buffer,0xFF,1,arq);
       for(f=0;f<0xFF;f++)
          buffer[f] = mountKeyValue(&seed);
       fwrite(buffer,mountKeyValue(&seed),1,arq);
       fclose(arq);
    }

    return 0;
 }

 int EXPENTRY icqSetCurrentUser(HICQ icq, ULONG uin, const char *password, BOOL save)
 {
    icq->uin = uin;

    DBGTrace(uin);
    icqSaveProfileValue(icq, "MAIN", "uin", uin);

    if(password)
    {
       strncpy(icq->pwd,password,PASSWORD_SIZE);

       if(save)
       {
          icq->cntlFlags |= ICQFC_SAVEPASSWORD;
//          icqSaveString(icq, "password", icq->pwd);
          saveKeys(icq);
       }
       else
       {
          icq->cntlFlags &= ~ICQFC_SAVEPASSWORD;
          icqSaveString(icq, "password", NULL);
       }
    }

    return 0;

 }


 BOOL EXPENTRY icqQueryLogonStatus(HICQ icq)
 {
    return icq->uin && *icq->pwd;
 }

 BOOL EXPENTRY icqIsActive(HICQ icq)
 {
    return icq->ready;
 }

 int  EXPENTRY icqInitialize(HICQ icq, STATUSCALLBACK *startupStage )
 {
    int                 f;
    char                buffer[0x0100];
    ICQINSTANCE *info                           = NULL;

#ifdef linux
    key_t key;
#endif

    icq->shortSeq  = rand() & 0xFFFF;

    if(!icq->uin)
    {
       sysLog(icq,"Can't determine UIN, initialization failed");
       icq->ready = FALSE;
       return -1;
    }

    if(!*icq->pwd)
    {
       sysLog(icq,"Can't determine user password, initialization failed");
       icq->ready = FALSE;
       return -1;
    }

#ifdef __OS2__
    sprintf(buffer,"\\QUEUES\\PWICQ\\%lu",icq->uin);
    DBGMessage(buffer);

    f = DosCreateQueue(&icq->queue,QUE_FIFO,buffer);
    switch(f)
    {
    case 332:
       sysLog(icq,"Another instance detected");
       return -1;

    case 0:      /* OK */
       break;

    default:
//       sprintf(buffer,"Error creating public queue (rc=%d)",f);
//       sysLog(icq,buffer);
       icqWriteSysRC(icq, PROJECT, f, "Can't create public queue");
       return -1;
    }
#endif

#ifdef linux

    strcpy(buffer,"/tmp/pwicq.queue");
    DBGMessage(buffer);

    mknod(buffer,0666,0);
    key = ftok(buffer,0x230167);

    if( ((int) key) != -1)
       icq->queue = msgget(key,IPC_CREAT|0660);
    else
       icq->queue = -1;

    DBGTrace(icq->queue);

#endif

    icq->ipConfirmed = icqLoadValue(icq,"localIP",0x0100007f);

    DBGTracex(icq->shareBlock);

    if(icq->shareBlock)
    {
       for(f=0;f<PWICQ_MAX_INSTANCES && (icq->shareBlock->i+f)->uin != icq->uin;f++);

       DBGTracex(f);
       DBGTrace(icq->uin);
       DBGTrace((icq->shareBlock->i+f)->uin);

       info = NULL;

       if(f < PWICQ_MAX_INSTANCES )
       {
          info = icq->shareBlock->i+f;
          sysLog(icq,"Reusing instance block");
       }
       else
       {
          sysLog(icq,"Searching for a free instance block");
          for(f=0;f<PWICQ_MAX_INSTANCES && (icq->shareBlock->i+f)->uin;f++);
          if(f < PWICQ_MAX_INSTANCES )
          {
             info       = icq->shareBlock->i+f;
             info->uin  = icq->uin;
          }
       }

       DBGTracex(info);

       if(info)
       {
          info->flags     = 0;
          info->modeIcon  = ICQICON_OFFLINE;
          info->eventIcon = 0xFF;

          icq->myPID      =
          info->pid       = getpid();

          info->uin       = icq->uin;
          sysLog(icq,"Public instance block enabled");


          icq->info       = info;
       }
    }

    startupStage(icq,ICQSTARTUP_LOADINGUSERS);

    icqSystemEvent(icq,ICQEVENT_LOADUSERS);
//    loadUserList(icq);

#ifdef __OS2__
    icq->cpgIn  = icqLoadValue(icq, "cpgLocal",   icq->cpgIn);
    icq->cpgOut = icqLoadValue(icq, "cpgRemote",  icq->cpgOut);
#endif

    sysLog(icq,"Starting plugins");

// #ifndef DLL
//    icqv5_Start(icq,0);
// #endif

    startupStage(icq,ICQSTARTUP_STARTINGPLUGINS);
    startPlugins(icq);

    sysLog(icq,"Startup sequence completed");
    icqSetServiceStatus(icq,ICQSRCV_CORE,TRUE);

    CHKPoint();
    icqSystemEvent(icq, ICQEVENT_STARTUP);

    CHKPoint();
    startupStage(icq,ICQSTARTUP_COREOK);

    CHKPoint();
    icqCreateThread(icq, startupThread, 8192, 0, NULL, "INI");
	CHKPoint();

    return 0;

 }

#ifdef DEBUG

 static void _System listenerDebug(HICQ icq, void *parm, ULONG uin, char id, USHORT event, ULONG eventParm)
 {
    if(id != 'S' && event != ICQEVENT_TIMER)
       return;

    CHKPoint();
    icqRemoveEventListener(icq,listenerDebug,parm);
 }

#endif

 static void _System startupThread(ICQTHREAD *thd)
 {
//#ifdef DEBUG
//        char dbgBuffer[0x0100];
//#endif

    DBGMessage("Entrou na thread de inicializacao secundaria");

    loadMessageList(thd->icq);

	DBGTracex(thd->icq->modeCaps & ICQMF_EXTENDEDMENU);
	
    if(thd->icq->modeCaps & ICQMF_EXTENDEDMENU)
       icqLoadMenus(thd->icq);

    /* Finish initialization procedures, notify plugins */

    icqSystemEvent(thd->icq,ICQEVENT_SECONDARY);

    if(thd->icq->ready)
    {
       thd->icq->cntlFlags |= ICQFC_STARTUPOK;

       if(thd->icq->info)
       {
#ifdef linux
          if(thd->icq->queue != -1)
             thd->icq->info->flags |= ICQINSTANCEFLAG_READY;
#else
          thd->icq->info->flags |= ICQINSTANCEFLAG_READY;
#endif
       }
    }

/*
    CHKPoint();
#ifdef DEBUG
    icqAddEventListener(thd->icq, listenerDebug, NULL);
#endif


#ifdef DEBUG
//    icqInsertRTFMessage(thd->icq, 0, 0, 0, 0, 0, "{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1046{\\fonttbl{\\f0\\fnil\\fcharset0 Times New Roman;}}{\\colortbl ;\\red0\\green0\\blue0;}\\viewkind4\\uc1\\pard\\cf1\\f0\\fs20 mais uma\\par}}");
//    icqInsertRTFMessage(thd->icq, 0, 0, 0, 0, 0,"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1046{\\fonttbl{\\f0\\fnil\\fcharset0 Times New Roman;}}\n{\\colortbl ;\\red0\\green0\\blue0;}\n\\viewkind4\\uc1\\pard\\cf1\\f0\\fs20 veio, me explica essa tua histaria de roubo de conta! n\\'e3o to entendendo nada!\\par\n}");
#endif
*/	

#ifdef DEBUG
    DBGTrace(icqCheckBadWord(thd->icq, "namorados  .  net"));
    DBGTrace(icqCheckBadWord(thd->icq, "nAmOrAdOs.nEt"));
    DBGTrace(icqCheckBadWord(thd->icq, "Must be zero"));
#endif


 }

 int  EXPENTRY icqTerminate(HICQ icq)
 {
    saveKeys(icq);

    icq->ready = FALSE;
    icqSystemEvent(icq, ICQEVENT_STOPPING);
    icqSetServiceStatus(icq,ICQSRCV_CORE,FALSE);

#ifdef __OS2__
    if(icq->queue)
    {
       DosCloseQueue(icq->queue);
       icq->queue = NULLHANDLE;
    }
#endif

#ifdef linux

    if(icq->queue != -1)
    {
       msgctl(icq->queue,IPC_RMID,0);
       icq->queue = -1;
    }

#endif

    return 0;
 }

 void initialize(HICQ icq,int numpar, char **param, STATUSCALLBACK *startupStage)
 {
    char        buffer[0x0100];
    ULONG       flags           = 0;

    icq->ready          = TRUE;
    icq->threads        = icqCreateList();
    icq->users          = icqCreateList();
    icq->plugins        = icqCreateList();
    icq->msgQueue       = icqCreateList();
    icq->msg            = icqCreateList();
    icq->listeners      = icqCreateList();
    icq->guiListeners   = icqCreateList();
    icq->pendingSearchs = icqCreateList();
    icq->requests       = icqCreateList();
    icq->userDB         = NULL;         // &defaultUserDB;
    icq->modeCaps       = ICQMF_AUTOOPEN;

    DBGMessage(icqIP2String(loopback,buffer));

    startupStage(icq,ICQSTARTUP_PARMS);
    readParms(icq,numpar,param,&flags);

    startupStage(icq,ICQSTARTUP_CONFIG);
    srand(time(NULL));
    icqChkConfigFile(icq,buffer);

    DBGTrace(icq->uin);

    DBGTrace(icqQueryLogonStatus(icq));

    sysLog(icq,"-------------------------------------------------------------------------------------------");

#ifdef DEBUG
    sysLog(icq,"pwICQ core (Debug Version " __DATE__ " " __TIME__ ")");
#else
    sysLog(icq,pwICQLibraryEntryPoints.ident);
#endif

#ifdef EXTENDED_LOG
    sysLog(icq,"*** Extensive logging enabled");
#endif

#ifdef DEBUG_LOG
    sysLog(icq,"*** DEBUG Log enabled");
#endif

    if(icq->uin && !*icq->pwd)
    {
       loadKeys(icq);

       if(*icq->pwd)
       {
          sysLog(icq,"Key file loaded");
       }
       else
       {
          icqLoadString(icq, "password", "", icq->pwd, PASSWORD_SIZE);
          if(*icq->pwd)
          {
             icq->cntlFlags |= ICQFC_SAVEPASSWORD;
             saveKeys(icq);
          }
       }
    }

    DBGMessage("Core iniciado");

    if(flags & 1)
    {
       sysLog(icq,"Starting in offline mode");
       icqSaveString(icq, "lastMode", NULL);
    }

    DBGMessage(icqIP2String(0x01020304, buffer));

    if(icq->shareBlock)
       sysLog(icq,"Shared memory area enabled");

    startupStage(icq,ICQSTARTUP_FILTERS);
    icq->badWords = loadTextFile(icq,"badword.dat",&icq->badWordTime);

    loadBlackList(icq);
    loadIgnoreList(icq);

    sysLog(icq,"Loading plugins");

    startupStage(icq,ICQSTARTUP_PLUGINS);

    if(icq->cntlFlags & ICQFC_EXTRAMODULES)
       loadExtendedPlugins(icq);

    if(icqLoadValue(icq,"plugins",1))
       loadPlugins(icq);

    startupStage(icq,ICQSTARTUP_PLUGINSOK);

#ifdef FORCE_SAVEUSERS
    icq->cntlFlags |= ICQFC_SAVEUSER;
#endif

#ifdef DEBUG
    icqPrintLog(icq,PROJECT,"ICQ Handle is %08lx",(ULONG) icq);
#endif

 }

 void terminate(HICQ icq)
 {
    int         f;
    HTHREAD     t;
    HTHREAD     next;
    HUSER       usr;
    HPLUGIN	p;
    char        buffer[0x0100];
    char        idLog[4];

    icq->ready          = FALSE;

    DBGTracex(icq->info);

    if(icq->info)
    {
       DBGTrace(icq->info->uin);
       DBGTrace(icq->info->pid);
       DBGTrace(icq->myPID);

       if(icq->info->pid == icq->myPID)
       {
          icqWriteSysLog(icq,PROJECT,"Disconnected from shared memory block");
          icq->info->pid = 0;
       }
       icq->info->uin = 0;
    }

    /* Stop plugins */

    icqWalkList(icq->plugins,p)
       icqStopPlugin(icq, p);

/*
    for(p=icqQueryFirstElement(icq->plugins);p;p=icqQueryNextElement(icq->plugins,p))
       icqStopPlugin(icq, p);
*/

    /* Release lists */

    if(icq->badWords)
    {
       free(icq->badWords);
       icq->badWords = NULL;
    }

    if(icq->blacklist)
    {
       free(icq->blacklist);
       icq->blacklist = NULL;
    }

    if(icq->ignore)
    {
       free(icq->ignore);
       icq->ignore = NULL;
    }

    if(icq->threads)
    {
       /* Finaliza threads de servico */
       DBGMessage("Aguardando queda das threads");

       /* Aguardo que as threads saiam por conta propria */
#ifdef __OS2__
       for(f=0;f < 3 && icqQueryFirstElement(icq->threads);f++)
          DosSleep(1000);
#else
       for(f=0;f < 150 && icqQueryFirstElement(icq->threads);f++)
          usleep(1000);
#endif

       DBGTrace(f);

       /* Comeco a pedir a saida de quem nao saiu */
       for(t = icqQueryFirstElement(icq->threads);t;t=icqQueryNextElement(icq->threads,t))
       {
          DBGMessage(t->id);
          if(t->stopthread)
          {
             strncpy(idLog,t->id,3);
             *(idLog+3) = 0;

#ifdef __OS2__
             sprintf(buffer,"Stopping thread [%3s] (TID=%d)",idLog,t->tid);
#else
             sprintf(buffer,"Stopping thread [%3s]",idLog);
#endif
             icqWriteSysLog(icq,PROJECT,buffer);
             t->stopthread(t);
          }
       }

       /* 10 segundos para elas fazerem o cleanup e sairem */

       t = icqQueryFirstElement(icq->threads);
       for(f=0;f < 60 && t;f++)
       {
          strncpy(idLog,t->id,3);
          *(idLog+3) = 0;

#ifdef __OS2__
          sprintf(buffer,"Waiting for thread [%3s] finalization (TID=%d)",idLog,t->tid);
#else
          sprintf(buffer,"Waiting for thread [%3s] finalization",idLog);
#endif
          icqWriteSysLog(icq,PROJECT,buffer);

#ifdef __OS2__
          DosSleep(1000);
#else
          for(f=0;f<50;f++)
             usleep(1000);
#endif
          t = icqQueryFirstElement(icq->threads);
       }

    }

    icq->cntlFlags |= ICQFC_SAVEUSER;
    icqSystemEvent(icq,ICQEVENT_SAVE);
    saveKeys(icq);

    /* Descarrega plugins */
    CHKPoint();
    if(icq->plugins)
    {
       sysLog(icq,"Unloading plugins");
       unloadPlugins(icq);
    }

    /* Finalizo a forca as threads que nao sairam */

    if(icq->threads)
    {

       t = icqQueryFirstElement(icq->threads);
       while(t)
       {
          strncpy(idLog,t->id,3);
          *(idLog+3) = 0;

#ifdef __OS2__
          sprintf(buffer,"Thread [%3s] finalization failure (TID=%d)",idLog,t->tid);
#else
          sprintf(buffer,"Thread [%3s] finalization failure",idLog);
#endif
          icqWriteSysLog(icq,PROJECT,buffer);

          next = icqQueryNextElement(icq->threads,t);
          f = icqKillThread(icq,t);

          if(f)
          {
             strncpy(idLog,t->id,3);
             *(idLog+3) = 0;

#ifdef __OS2__
             sprintf(buffer,"Error %d killing thread [%3s] (TID=%d)",f,idLog,t->id);
#else
             sprintf(buffer,"Error killing thread [%3s]",idLog);
#endif
             icqWriteSysLog(icq,PROJECT,buffer);

          }

          t = next;
       }

    }

    CHKPoint();

    /* Limpo usuarios */

   for(usr = icqQueryFirstElement(icq->users);usr;usr = icqQueryNextElement(icq->users,usr))
      icqDestroyList(usr->msg);

    /* Destroi listas */

    CHKPoint();

    icqDestroyList(icq->listeners);
    icqDestroyList(icq->guiListeners);
    icqDestroyList(icq->pendingSearchs);
    icqDestroyList(icq->requests);
    icqDestroyList(icq->users);
    icqDestroyList(icq->threads);
    icqDestroyList(icq->plugins);
    icqDestroyList(icq->msgQueue);
    icqDestroyList(icq->msg);

    /* E salva arquivo de configuracao */
    icqSaveConfigFile(icq);

    CHKPoint();
 }

 static void readParms(HICQ icq, int numpar, char **param, ULONG *flags)
 {
    char buffer[0x0100];
    char *key;
    char *vlr;

    icq->numpar = numpar;
    icq->param  = param;

    while(--numpar > 0)
    {
       param++;

       if(!strncmp(*param,"--uin=",6))
       {
          DBGMessage( *param +6 );
          icq->uin = atol( *param +6 );
       }
       else if(!strncmp(*param,"--inifile=",10))
       {
          icqLoadConfigFile(icq,*param+10);
       }
       else if(!strncmp(*param,"--module=",9) || !strncmp(*param,"--modules=",10))
       {
          icq->cntlFlags |= ICQFC_EXTRAMODULES;
       }
       else if(!strncmp(*param,"--offline",9))
       {
          *flags |= 1;
       }
       else if(!strncmp(*param,"--password=",11))
       {
          strncpy(icq->pwd,*param +11, PASSWORD_SIZE);
       }
       else if(!strncmp(*param,"--mode=",7) && icq->uin)
       {
          DBGMessage(*param +7);
          icqChkConfigFile(icq, buffer);
          icqSaveValue(icq,"lastMode",atol(*param +7));
       }
       else if(!strcmp(*param,"--save"))
       {
          if(icq->uin)
          {
             icqChkConfigFile(icq, buffer);
             icqSaveProfileValue(icq, "MAIN", "uin", icq->uin);
             icqSaveString(icq, "password", icq->pwd);
          }
       }
       else if(!strncmp(*param,"--gui=",6))
       {
          icq->skinRequired = *param + 6;
          DBGMessage(icq->skinRequired);
       }
       else if(!(strcmp(*param,"--nogui") && strcmp(*param,"--text")))
       {
          icq->cntlFlags |= ICQFC_NOGUI;
       }
       else if(!strncmp(*param,"--",2))
       {
          icqChkConfigFile(icq, buffer);

          key = (*param) + 2;
          if(*key)
          {
             vlr = strchr(*param,'=');
             if(vlr)
             {
                *(vlr++) = 0;
                icqSaveString(icq, key, vlr);
             }
             else
             {
                icqSaveString(icq, key, NULL);
             }
          }
       }
    }
 }

 static void loadExtendedPlugins(HICQ icq)
 {
    int         numpar          = icq->numpar;
    char        **param         = icq->param;
    char        *token;
    char        buffer[0x0100];
    char	fileName[0x0100];

    while(--numpar > 0)
    {
       param++;
       if(!strncmp(*param,"--module=",9))
       {
          icqWriteSysLog(icq,PROJECT,*param+9);
          icqLoadPlugin(icq,*param+9,NULL);
       }
       if(!strncmp(*param,"--modules=",10))
       {
          strncpy(buffer,*param+10,0x0FF);
          token = strtok(buffer, ",");
          while(token)
          {
             strncpy(fileName,token,0xFF);
#ifdef linux
             strncat(fileName,".so",0xFF);
#else			
             strncat(fileName,".dll",0xFF);
#endif	
             DBGMessage(fileName);			
             icqLoadPlugin(icq, NULL, fileName);
             token = strtok(NULL, ",");
          }
       }
    }

 }
