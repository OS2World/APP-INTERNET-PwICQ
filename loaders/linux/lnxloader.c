/*
 * lnxloader.c - Modulo de carga para a versao linux
 */
 
 #include <string.h>
 #include <stdio.h>
 #include <sys/time.h>
 #include <signal.h>
 #include <dlfcn.h>

 #include <icqtlkt.h>

#ifdef ICQSTATIC
 #include ICQSTATIC
#endif

// #ifdef __cplusplus
//   extern "C" {
// #endif


/*---[ Definicoes ]--------------------------------------------------------------*/


/*---[ Prototipos ]--------------------------------------------------------------*/

 static void  textMode(HICQ);
 
 void _System icqloader_setStartupStage(HICQ,int);
	  
 static void  startTimer(void);
 static void  onTimer(int);
 static int   exec(int, char **);
	  
#ifdef PWICQ_DINAMIC	  
 static int loadpwICQCore(void);
#endif
	  
/*---[ Static ]------------------------------------------------------------------*/

 static const char *ident   = "pwICQ Loader Build " BUILD " by " USER "@" HOSTNAME;

 static HICQ    icq         = NULL;
 static BOOL	active	    = FALSE;

#ifdef PWICQ_DINAMIC	  

 static HMODULE 				core	= 0;
 static struct coreEntryPoints	*ce		= 0;
	
 #define icqChkCmdLine(c,p) 	ce->icqChkCmdLine(c,p)
 #define icqCoreStartup(n,p,s)	ce->icqCoreStartup(n,p,s)
 #define icqCoreTerminate(i)	ce->icqCoreTerminate(i)
 #define icqTerminate(i)		ce->icqTerminate(i)

#endif

/*---[ Implementacao ]-----------------------------------------------------------*/

#ifdef __DLL__
   #error Cant use loader in shared library
#endif

 int main(int numpar, char *param[])
 {
	int	    rc			= 0;
	
	CHKPoint();
	
#ifdef PWICQ_DINAMIC	  
    rc = loadpwICQCore();
#endif

	if(!rc)
	   exec(numpar,param);
    
#ifdef PWICQ_DINAMIC	  
    DBGTracex(core);
	if(core)
	   dlclose(core);
#endif	
	
	DBGTrace(rc);
    
	return rc;
 }
	
 static int exec(int numpar, char **param)
 {	
	int     		rc = 0;
    ULONG			sknParm;
	const SKINMGR	*skin;
	
	if(icqChkCmdLine(numpar,param))
	   return 0;
	
    icq = icqCoreStartup(numpar,param,(STATUSCALLBACK *) icqloader_setStartupStage);
    
    if(!icq)
       return -1;

	skin = icqQuerySkinManager(icq);
	DBGTracex(skin);
	   
    DBGMessage(ident);

    if(icqQueryTextModeFlag(icq))
	{
	   textMode(icq);
	}
    else if(!skin)
    {
       icqWriteSysLog(icq,PROJECT,"Unable to load GUI module");
    }
    else if(skin->sz != sizeof(SKINMGR))
    {
       icqWriteSysLog(icq,PROJECT,"Invalid or incompatible GUI module");
    }
    else
    {
	   icqWriteSysLog(icq,PROJECT,ident);
		  
       if(!(skin->flags & SKINMGR_HASTIMER) )
	      startTimer();
		  
       CHKPoint();
	   DBGTracex(skin->loadSkin);
		  
	   if(skin->loadSkin)
          sknParm = skin->loadSkin(icq, (STATUSCALLBACK *) icqloader_setStartupStage,numpar,param);
	   else
          sknParm = 0;			

	   if(sknParm)
	   {
	      active = TRUE;
          skin->executeSkin(icq,sknParm);
          active = FALSE;
          icqTerminate(icq);
       }
    }

    CHKPoint();
    icqCoreTerminate(icq);
    CHKPoint();

    icq = NULL;

    DBGMessage("Terminado");

    return rc;
 }

 static void startTimer(void)
 {
    struct itimerval	timer, 
    			otimer;
 
    timer.it_interval.tv_sec  =
    timer.it_value.tv_sec     = 0;
   
    timer.it_interval.tv_usec = 
    timer.it_value.tv_usec    = 100000L;

    signal(SIGALRM,onTimer);
    setitimer(ITIMER_REAL,&timer,&otimer);

 }

 void onTimer(int sig)
 {
    if(active)
       icqkrnl_Timer(icq);
 }

 void _System icqloader_setStartupStage(HICQ icq, int sts)
 {
	DBGTrace(sts);
#ifdef STATIC_REGISTER	   
    if(sts == 1)
	   STATIC_REGISTER(icq);
#endif	   

    
 }
 
 static void textMode(HICQ icq)
 {
    char cmdLine[0x0100];
	int  rc;
	
    startTimer();
	
	if(!icqInitialize(icq,(STATUSCALLBACK *) icqloader_setStartupStage))
	{
   	   printf("\npwICQ Text mode interface enabled\n");
	   active = TRUE;
       while(icqIsActive(icq))
       {
          memset(cmdLine,0,0x0100);
          fgets(cmdLine,0xFF,stdin);
          CHKPoint();
          rc = icqExecuteCmd(icq, cmdLine);
          CHKPoint();
          printf("\nrc=%d\n",rc);
       }
	   active = FALSE;
	}
 }

#ifdef PWICQ_DINAMIC	  

 void EXPENTRY icqkrnl_Timer(HICQ i)
 {
	ce->icqkrnl_Timer(i);
 }

 static void findCoreModule(char *buffer)
 {
	strcpy(buffer,"/usr/share/pwicq/icqkrnl.so");
 }
 
 static int loadpwICQCore(void)
 {
	/* Load icqkrnl.so dinamically */
	char 				buffer[0x0100];
	struct icqftable 	*table	= NULL;
	char				*error;
	
	printf("%s\n",ident);
	
	findCoreModule(buffer);
	
    core = dlopen(buffer,RTLD_NOW);
	
	if(!core)
	{
	   fprintf(stderr,"%s when loading core\n",dlerror());
	   return -1;
	}

    ce    = (struct coreEntryPoints *) dlsym(core,"pwICQCoreEntryPoints");
	error = dlerror();
	if(error)
	{
	   fprintf(stderr,"%s when loading startup symbol table\n",error);
	   return -1;
	}

	if(ce->sz != sizeof(struct coreEntryPoints))
	{
	   fprintf(stderr,"Unexpected startup symbol table identifier\n");
	   return -3;
	}

	table = (struct icqftable *) dlsym(core,"pwICQLibraryEntryPoints");
	error = dlerror();
	if(error)
	{
	   fprintf(stderr,"%s when loading main symbol table\n",error);
	   return -1;
	}

	CHKPoint();

	if(pwICQConfigureLibrary(table))
	{
	   fprintf(stderr,"Invalid or unexpected core module\n");
	   return -2;
	}

	printf("%s\n",table->ident);

	return 0;
 }
#endif

// #ifdef __cplusplus
//   }
// #endif

