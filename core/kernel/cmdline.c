/*
 * cmdline.c - Verificacao inicial da linha de comando passada
 *
 */

#ifndef linux
   #error Only for Linux version
#endif

 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include <unistd.h>

 #include <sys/ipc.h>
 #include <sys/shm.h>
 #include <sys/msg.h>

 #include <icqkernel.h>
 #include <pwmacros.h>

 #include <icqkernel.h>

/*---[ Prototipos ]------------------------------------------------------------------*/

 static int  chkParms(int, char **);
 static void chkDaemon(int, char **);
 static int  chk4Root(int, char **);
 static void stopAll(void);

/*---[ Implementacao ]---------------------------------------------------------------*/

 int EXPENTRY icqChkCmdLine(int numpar, char **param)
 {
    CHKPoint();

    if(chkParms(numpar,param))
       return 1;

    CHKPoint();
	
    if(chk4Root(numpar,param))
       return -1;

    CHKPoint();

    chkDaemon(numpar,param);

    CHKPoint();

    return 0;
	
 }

 static void chkDaemon(int numpar, char **param)
 {
    while(numpar--)
    {
       if(!(strcmp(*param,"--nodaemon") && strcmp(*param,"--text")) )
          return;
       param++;
    }
    daemon(1,0);
 }

 static void changeFile(const char *ptr, FILE *hdl)
 {
    char buffer[0x0100];
    char *ext;

    strncpy(buffer,ptr,0xFA);
    *(buffer+0xFB) = 0;

    ext = strchr(buffer, '.');
    if(ext)
       ext++;
    else
       ext = buffer+strlen(buffer);

    while(!freopen(buffer,"a",hdl))
    {
#ifdef __OS2__
       DosBeep(220,10);
       DosSleep(10);
#endif
#ifdef linux
       usleep(100);
#endif
       sprintf(ext,"%03x",(int) random());

    }

 }

 static int chkParms(int numpar, char **param)
 {
    char         wrk[70];
    time_t       ltime;
    char         *ptr;
    int          rc = 0;

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
          changeFile(ptr,stdout);
		
       }
       else if(!strncmp(*param,"--debug",7))
       {
          ptr = strchr(*param,'=');

          if(*ptr)
             ptr++;

          if(!*ptr)
             ptr = "pwicq.dbg";

          changeFile(ptr,stderr);

          time(&ltime);
          strftime(wrk, 69, "%m/%d/%Y %H:%M:%S", localtime(&ltime));

          fprintf(stderr,"\n-----[ %s ]---------------------------------------------\n",wrk);
		  fflush(stderr);
		
       }
       else if(!strncmp(*param,"--stop",6))
       {
          /* Encerra todas as instancias */
          CHKPoint();

          stopAll();
          rc = 1;
       }
       param++;
    }

    return rc;
 }

 static int chk4Root(int numpar, char **param)
 {
    if(!strcmp(getenv("HOME"),"/root"))
    {
       fprintf(stderr,"It's not a good idea running an ICQ like program as root\n");
       return -1;	
    }

    return 0;

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
       fprintf(stderr,"Can't find shared area\n");
       return;
    }

    shmID = shmget(memKey,sizeof(sizeof(ICQSHAREMEM)),0660);
    DBGTrace(shmID);

    if( ((int) shmID) == -1)
    {
   	   fprintf(stderr,"Unable to get shared area\n");
       return;
    }
 	
 	icq = shmat(shmID,0,0);
 	
 	if( ((int) icq) == -1)
    {
       fprintf(stderr,"Error attaching in the shared area\n");
       return;
    }

    if(icq->sz == sizeof(ICQSHAREMEM) || icq->si == sizeof(ICQINSTANCE))
    {
       DBGMessage("Achei um bloco de memoria compartilhada valido");

       for(f=0;f<PWICQ_MAX_INSTANCES;f++)
	   {
		  if( (icq->i+f)->uin )
			 printf("ICQ#%ld stopped\n",(icq->i+f)->uin);
		
          (icq->i+f)->pid = 0;
	   }
	   printf("Stop request sent to all instances\n");
    }
    else
    {
       fprintf(stderr,"Invalid shared area\n");
       return;
    }

    shmdt(icq); 	

 }


