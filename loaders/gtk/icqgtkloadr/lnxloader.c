/*
 * lnxloader.c - Modulo de carga para a versao linux
 */

 #include <stdio.h>
 #include <unistd.h>
 #include <pwd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <sys/time.h>
 #include <sys/types.h>

 #include <signal.h>

 #include <sys/ipc.h>
 #include <sys/shm.h>
 #include <sys/msg.h>

 #include <icqkernel.h>
 #include <pwmacros.h>
 
 #include <icqgtk.h>

/*---[ Definicoes ]--------------------------------------------------------------*/


/*---[ Prototipos ]--------------------------------------------------------------*/

 static int  chkParms(int, char **);
 static void chkDaemon(int, char **);
 static int  chk4Root(int, char **);
 static void startTimer(void);
 static void stopAll(void);
 
 void onTimer(void);

/*---[ Static ]------------------------------------------------------------------*/

 static HICQ    icq       = NULL;
 static BOOL	active	= FALSE;

/*---[ Implementacao ]-----------------------------------------------------------*/

#ifdef __DLL__
   #error Shared lib nao utiliza loader
#endif

 int main(int numpar, char *param[])
 {
    ULONG	sknParm;
    
    CHKPoint();
    
    if(chk4Root(numpar,param))
       return -1;

    CHKPoint();
    
    chkDaemon(numpar,param);
    
    CHKPoint();
    
    if(chkParms(numpar,param))
       return 0;

    icq = icqCoreStartup(numpar,param,icqgtk_setStartupStage);
    
    if(!icq)
       return -1;

    if(icq->sz != sizeof(ICQ))
    {
       fprintf(stderr,"*** Invalid core identifier");
    }
    else
    {
       startTimer();
       icqgtk_Configure(icq, 0);

       if(!icq->skin)
       {
          icqWriteSysLog(icq,PROJECT,"Unable to load GUI module");
       }
       else if(icq->skin->sz != sizeof(SKINMGR))
       {
          icqWriteSysLog(icq,PROJECT,"Invalid or incompatible GUI module");
       }
       else
       {
          CHKPoint();
	      DBGTracex(icq->skin->loadSkin);
          sknParm = icq->skin->loadSkin(icq,icqgtk_setStartupStage,numpar,param);

	      if(sknParm)
	      {
	         active = TRUE;
             icq->skin->executeSkin(icq,sknParm);
             active = FALSE;
             icqTerminate(icq);
          }
       }
    }

    icqCoreTerminate(icq);

    icq = NULL;

    DBGMessage("Terminado");
        
    return 0;
 }

 static void chkDaemon(int numpar, char **param)
 {
    while(numpar--)
    {
       if(!strcmp(*param,"--nodaemon"))
          return;
       param++;
    }
    daemon(1,0);
 }

 static int chkParms(int numpar, char **param)
 {
    char *ptr;
    int  rc = 0;
    
    CHKPoint();
    
    while(numpar--)
    {
       if(!strncmp(*param,"--log",5))
       {
          DBGMessage(*param);
          
          ptr = strchr(*param,'=');

          DBGTracex(ptr);

          if(*ptr)
             ptr++;
             
          if(!*ptr)
             ptr = "pwicq.log";
          
          DBGMessage(ptr);
             
          freopen(ptr,"a",stdout);
       }   
       else if(!strncmp(*param,"--debug",7))
       {
          ptr = strchr(*param,'=');
          
          if(*ptr)
             ptr++;
             
          if(!*ptr)
             ptr = "pwicq.dbg";
             
          freopen(ptr,"a",stdout);
       }   
       else if(!strncmp(*param,"--stop",6))
       {
          /* Encerra todas as instancias */
          CHKPoint();
          
          stopAll();
          rc = -1;
       }
       param++;
    }
    
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

    signal(SIGALRM,(void *) onTimer);
    setitimer(ITIMER_REAL,&timer,&otimer);

 }

 static int chk4Root(int numpar, char **param)
 {
    if(!strcmp(getenv("HOME"),"/root"))
    {
       fprintf(stderr,"It's not a good idea running an ICQ like program as root\n");
       return -1;	
    }

    return 0;

/*   uid_t	  uid  = geteuid();

//    struct passwd *pwd = getpwuid(geteuid());
    
//    if(!pwd)
//    {
//       fprintf(stderr,"Error getting current user information\n");
//       return 0;
//    }
    
//    DBGMessage(pwd->pw_name);

//    DBGMessage( (getpwuid(geteuid()))->pw_name );

*/    
    
/* 	
    char buffer[L_cuserid];
    char *ptr	= cuserid(NULL); // getlogin();

    getpwuid(geteuid())
    if(!ptr)
    {
       fprintf(stderr,"Error getting current user ID\n");
       return 0;
    }
    
    strncpy(buffer,ptr,L_cuserid);
    
    DBGMessage(buffer);
    
    if(!strcmp(buffer,"root"))
    {
       fprintf(stderr,"It's not a good idea running an ICQ like program as root\n");
       return -1;	
    }
    return 0;
*/    
 }

 void onTimer(void)
 {
    if(active)
       icqkrnl_Timer(icq);
 }
   
 void _System icqgtk_setStartupStage(int sts)
 {
 
 }
 
 static void stopAll(void)
 {
    /* Encerra todas as intancias */
    key_t 		   memKey 	=  ftok(PWICQ_SHAREMEM,0x230167);
    int   			shmID;
    ICQSHAREMEM	*icq;
    int			   f;

    DBGTrace(memKey);
        
    if( ((int) memKey) == -1)
    {
    	 fprintf(stderr,"Can't find shared area");
       return;
    }
       
    shmID = shmget(memKey,sizeof(sizeof(ICQSHAREMEM)),0660);
    DBGTrace(shmID);
    
    if( ((int) shmID) == -1)
    {
    	 fprintf(stderr,"Unable to get shared area");
       return;
    }
 	
 	 icq = shmat(shmID,0,0);
 	 
 	 if( ((int) icq) == -1)
    {
    	 fprintf(stderr,"Error attaching in the shared area");
       return;
    }

    if(icq->sz == sizeof(ICQSHAREMEM) || icq->si == sizeof(ICQINSTANCE))
    {
       DBGMessage("Achei um bloco de memoria compartilhada valido");

       for(f=0;f<PWICQ_MAX_INSTANCES;f++)
          (icq->i+f)->pid = 0;
    }
    else
    {
    	 fprintf(stderr,"Invalid shared area");
       return;
    }

    shmdt(icq); 	    

 }


