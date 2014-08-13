/*
 * ishare.c - Loader module for the stand-alone version
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

#ifdef linux
 #include <unistd.h>
 #include <pwd.h>
#endif

 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>

 #include "ishared.h"

#ifdef STDLINUX
 #include <sys/time.h>
 #include <signal.h>
#endif

/*---[ Structures and Macros ]---------------------------------------------------------------------------*/


 #pragma pack(1)

 typedef struct _parm_processor
 {
    const char	*key;
    int		(*exec)(const char *);
 } PARAM_PROCESSOR;


 #define DECLARE_PARAMETER(x)             static int parm_##x(const char *);
 #define BEGIN_PARAMETER_TABLE(x)         static const PARAM_PROCESSOR x[] = {
 #define USE_PARAMETER(x)                 { "--" #x, (int (*)(const char *)) parm_##x },
 #define END_PARAMETER_TABLE()            { 0, NULL } };
 #define BEGIN_PARAMETER_PROCESSOR(x)     static int parm_##x(const char *parm)

 #pragma pack()


/*---[ Parameter processors ]----------------------------------------------------------------------------*/


#ifdef linux
 DECLARE_PARAMETER(start)
 DECLARE_PARAMETER(stop)
 DECLARE_PARAMETER(status)
 DECLARE_PARAMETER(user)
#endif

 DECLARE_PARAMETER(log)

 BEGIN_PARAMETER_TABLE(parmlist)

#ifdef linux
 USE_PARAMETER(start)
 USE_PARAMETER(stop)
 USE_PARAMETER(status)
 USE_PARAMETER(user)
#endif

 USE_PARAMETER(log)

 END_PARAMETER_TABLE()

/*---[ Statics ]-----------------------------------------------------------------------------------------*/

 static       ISHARED_CONFIG cfg;
 static void  chkCmdLine(int, char **);

/*---[ Implementation ]----------------------------------------------------------------------------------*/

#ifdef STDLINUX
 static void onTimer(int sig)
 {
    ishare_timer(&cfg);
 }
#endif

 int main(int numpar, char *param[])
 {
#ifdef STDLINUX
    struct itimerval	timer,
    			otimer;
#endif

    memset(&cfg,0,sizeof(cfg));
    cfg.sz       = sizeof(cfg);

    chkCmdLine(numpar, param);
    ishare_loadConfig(&cfg);

#ifdef STDLINUX
    timer.it_interval.tv_sec  =
    timer.it_value.tv_sec     = 1;
    timer.it_interval.tv_usec =
    timer.it_value.tv_usec    = 0;
    signal(SIGALRM,onTimer);
    setitimer(ITIMER_REAL,&timer,&otimer);
#endif

    log(&cfg,"I-Share standalone module (build " __DATE__ " " __TIME__ ")");
    ishare_main(&cfg);
    ishare_deleteList(&cfg.config);
    return 0;
 }

 static void  chkCmdLine(int numpar, char **param)
 {
    const PARAM_PROCESSOR *cmd;
    char		  *ptr;
    char		  *parm;

    param++;

    while(--numpar)
    {
       ptr = *param;
       for(parm=ptr;*parm && *parm != '=';parm++);
       if(*parm == '=')
	  *(parm++) = 0;
       for(cmd = parmlist; cmd->exec && strcmp(cmd->key,ptr); cmd++);
       if(cmd->exec)
       {
	  cmd->exec(parm);
       }
       else
       {
	  fprintf(stderr,"Unexpected parameter: %s\n",*param);
	  exit(-1);
       }
       param++;
    }

 }

#ifdef linux
 BEGIN_PARAMETER_PROCESSOR(start)
 {
    return daemon(1,0);
 }
#endif

#ifdef linux
 BEGIN_PARAMETER_PROCESSOR(stop)
 {
    exit(-1);
    return 0;
 }
#endif

 BEGIN_PARAMETER_PROCESSOR(log)
 {
    freopen(parm,"a",stdout);
    return 0;
 }

#ifdef linux
 BEGIN_PARAMETER_PROCESSOR(status)
 {
    exit(-1);
    return 0;
 }
#endif

#ifdef linux
 BEGIN_PARAMETER_PROCESSOR(user)
 {
    struct passwd *pwd = getpwnam(parm);

    DBGMessage(parm);
    DBGTracex(pwd);

    if(!pwd)
    {
       fprintf(stderr,"Can't find user\n");
       exit(-1);
    }

    DBGTrace(pwd->pw_uid);
    if(setuid(pwd->pw_uid))
    {
       fprintf(stderr,"Can't set user\n");
       exit(-1);
    }

    return 0;
 }
#endif
