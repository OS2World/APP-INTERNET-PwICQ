/*
 * ishare.c - Loader module for the stand-alone version
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #include <process.h>
#endif

#ifdef linux
 #include <pthread.h>
#endif

 #include <stdio.h>
 #include <time.h>

 #include "ishared.h"

/*---[ Constants ]---------------------------------------------------------------------------------------*/

 static const char *timeMasc = "%m/%d/%Y %H:%M:%S";

/*---[ Implementation ]----------------------------------------------------------------------------------*/

 void ICQAPI ishare_log(const char *id, const char *text)
 {
    char         wrk[70];
    time_t       ltime;

    time(&ltime);
    strftime(wrk, 69, timeMasc, localtime(&ltime));

    printf("%s %-10s %s\n",wrk,id,text);
    fflush(stdout);

 }

#ifdef STDOS2
 /*
  * OS/2 Thread abstraction layer
  */

 struct thd_parm
 {
    BOOL        flag;
    ULONG	tid;
    void        (* _System entry)(ULONG);
    ULONG	parm;
 };

 static void _System tidLoader(struct thd_parm *p)
 {
    #pragma handler ( tidLoader, _Exception )

    ULONG	parm;
    void        (* _System entry)(ULONG);

    parm  = p->parm;
    entry = p->entry;

    p->flag = 0;
    DosSleep(0);

    entry(parm);

    _endthread();

 }

 THREAD_ID ICQAPI ishare_beginThread(ISHARED_CONFIG *cfg, int stack, void (* _System entry)(ULONG), void *parm)
 {
    struct thd_parm     p;
    int			f;

    p.flag  = 1;
    p.tid   = 0;
    p.entry = entry;
    p.parm  = (ULONG) parm;

    if(DosCreateThread(&p.tid, (PFNTHREAD) &tidLoader, (ULONG) &p, 0x00000002, stack))
       return -1;

    for(f=0;f<600 && p.flag;f++)
       DosSleep(10);


    return p.tid;

 }

#endif

#ifdef STDLINUX
 /*
  * Linux Thread abstraction layer
  */

 struct thd_parm
 {
    BOOL        flag;
    ULONG	tid;
    void        (* _System entry)(ULONG);
    ULONG	parm;
 };

 static void tidLoader(struct thd_parm *p)
 {
    ULONG	parm;
    void        (* _System entry)(ULONG);

    parm    = p->parm;
    entry   = p->entry;
    p->flag = 0;

    pthread_detach(pthread_self());

    yield();
    entry(parm);

    pthread_exit(0);

 }

 THREAD_ID ICQAPI ishare_beginThread(ISHARED_CONFIG *cfg, int stack, void (* _System entry)(ULONG), void *parm)
 {
    struct thd_parm     p;
    int			f;

    p.flag  = 1;
    p.tid   = 0;
    p.entry = entry;
    p.parm  = (ULONG) parm;

    if(pthread_create( &p.tid, NULL, (void*)&tidLoader, (void *) &p))
       return -1;

    for(f=0;f<600 && p.flag;f++)
       usleep(100);

    return p.tid;

 }

#endif

 const char * ICQAPI ishare_formatIP(long IP, char *buffer)
 {
    unsigned char        *src    = (unsigned char *) &IP;
    unsigned char        *dst    = buffer;
    unsigned char        tmp[4];
    unsigned int         f,v,p;

    for(f=0;f<4;f++)
    {
       v = *(src++);
       if(v)
       {
          for(p=0;p<3&&v;p++)
          {
             *(tmp+p) = (v%10);
             v /= 10;
          }
          while(p--)
             *(dst++) = '0'+ *(tmp+p);
       }
       else
       {
          *(dst++) = '0';
       }
       if(f < 3)
          *(dst++) = '.';

    }
    *dst = 0;
    return buffer;
 }

