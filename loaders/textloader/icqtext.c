/*
 * ICQTEXT.C - Loader modo texto para o pwICQ
 */

#ifdef __OS2__
   #define INCL_DOSMODULEMGR
   #define INCL_DOSPROCESS
   #include <os2.h>
   #define sleep(x) DosSleep(x*1000)
#endif

#ifdef linux
   #include <unistd.h>
   #include <signal.h>
   #include <sys/time.h>
   #include <sys/types.h>
#endif

   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include <ctype.h>

   #include <icqtlkt.h>
   #include <icqreserved.h>

/*---[ Definicoes ]--------------------------------------------------------------*/

 #pragma pack(1)
 struct cmd
 {
    const char  *keyword;
    void        (* _System exec)(HICQ, char *);
    const char  *help;
 };

/*---[ Prototipos ]--------------------------------------------------------------*/


/*---[ Constantes ]--------------------------------------------------------------*/


/*---[ Statics ]-----------------------------------------------------------------*/

 BOOL           ready           = TRUE;
 static HICQ    icq                               = 0;
 static char    ppath[0x0100];

/*---[ Implementacao ]-----------------------------------------------------------*/

#ifdef linux

 void onTimer(void)
 {
    if(icqIsActive(icq))
       icqkrnl_Timer(icq);
 }

 static void startTimer(void)
 {
    struct itimerval    timer,
                        otimer;


    timer.it_interval.tv_sec  =
    timer.it_value.tv_sec     = 0;

    timer.it_interval.tv_usec =
    timer.it_value.tv_usec    = 100000L;

    signal(SIGALRM,(void *) onTimer);
    setitimer(ITIMER_REAL,&timer,&otimer);

 }


#endif

 static int _System setStatus(HICQ icq, int x)
 {
    return 0;
 }

 int main(int numpar, char *param[])
 {
    char	cmdLine[0x0100];
    int		rc;

#ifdef MEMWATCH

    char    *ptr;


    DBGMessage("Using MemWatch");

    TRACE(PROJECT " Started\n");
    mwStatistics( 2 );

    ptr = malloc(10);
    free(ptr);
    free(ptr);

#endif

    CHKPoint();

    icq = icqCoreStartup(numpar,param,setStatus);

    if(icq)
    {
       if(!icqInitialize(icq,setStatus))
       {
          icqLoadString(icq, "ppath", ".\\", ppath, 0xFF);
#ifdef linux
          startTimer();
#endif
          while(icqIsActive(icq) && ready)
          {
             memset(cmdLine,0,0x0100);
             fgets(cmdLine,0xFF,stdin);
             CHKPoint();
             rc = icqExecuteCmd(icq, cmdLine);
             CHKPoint();
             printf("\nrc=%d\n",rc);
          }

          icqTerminate(icq);
       }
       icqCoreTerminate(icq);
    }

    return 0;
 }




