/*
 * OS2THREAD.C - Thread manager for OS2 version
 */

#define INCL_DOSPROCESS
#define INCL_DOSEXCEPTIONS    /* DOS exception values */
#define INCL_ERRORS           /* DOS error values     */

#ifndef __OS2__
   #error Only for OS2 version
#endif

 #include <stdio.h>
 #include <string.h>
 #include <process.h>
 #include <icqkernel.h>

/*---[ Defines ]---------------------------------------------------------------------*/

 struct parm
 {
    BOOL        flag;
    void        (* _System start_address) (ICQTHREAD *);
    int         sz;
    ICQTHREAD   thd;
 };

 struct thdStart
 {
    BOOL        flag;
    void        (* _System start_address) (ICQTHREAD *, void *);
    int         sz;
    ICQTHREAD   thd;
 };


/*---[ Prototipes ]------------------------------------------------------------------*/

 static void _System tidLoader(struct parm *);
 static void _System tidStarter(struct thdStart *p);

/*---[ Implementation ]--------------------------------------------------------------*/

 int EXPENTRY icqCreateThread(HICQ icq, void (* _System start_address)(ICQTHREAD *), unsigned int stack_size, int sz, void *arglist, const char *id)
 {
    /* Start a thread, pass parameters */
    int         rc;
    struct parm p;
    int         f;

#ifdef EXTENDED_LOG
    char        idLog[4];
    char        extLog[0x0100];
#endif

    memset(&p,0,sizeof(struct parm));
    p.flag              = TRUE;
    p.start_address     = start_address;
    p.sz                = sz;

    p.thd.icq           = icq;
    p.thd.parm          = arglist;

    strncpy(p.thd.id,id,3);

    rc = DosCreateThread(&p.thd.tid, (PFNTHREAD) &tidLoader, (ULONG) &p, 0x00000002, stack_size);
    DBGTrace(rc);

    if(rc)
    {
       icqWriteSysRC(icq,PROJECT,rc,"Error starting thread");
       return -rc;
    }

    /* Aguarda ate 6 segundos para que a thread inicie */
    for(f=0;f<600 && p.flag;f++)
       DosSleep(10);

    if(p.flag)
    {
       /* Thread nao iniciou */
       DosKillThread(p.thd.tid);
       return -1;
    }

#ifdef EXTENDED_LOG
    strncpy(idLog,id,3);
    *(idLog+3) = 0;
    sprintf(extLog,"Thread [%s] started. TID=%d",idLog,p.thd.tid);
    icqWriteSysLog(icq,PROJECT,extLog);
    DBGMessage(extLog);
#endif

    return p.thd.tid;
 }

 int EXPENTRY icqKillThread(HICQ icq,ICQTHREAD *thread)
 {
    char log[100];
    int  rc                     = DosKillThread(thread->tid);

    if(rc)
    {
       sprintf(log,"Error killing thread %d [%3s]",thread->tid,thread->id);
       icqWriteSysRC(icq, PROJECT, rc, log);
    }

    DosSleep(10);

    icqRemoveElement(icq->threads, thread);
    return rc;
 }

 int EXPENTRY icqStartThread(HICQ icq, void (* _System start_address)(ICQTHREAD *, void *), unsigned int stack_size, int sz, const void *parm, const char *id)
 {
    /* Start a thread, pass parameters */
    int             rc;
    struct thdStart p;
    int             f;

    CHKPoint();

    if(sz == -1)
       sz = strlen(parm)+1;

    if(!stack_size)
       stack_size = 8192;

    memset(&p,0,sizeof(struct thdStart));
    p.flag              = TRUE;
    p.start_address     = start_address;
    p.sz                = sz;

    p.thd.icq           = icq;
    p.thd.parm          = (void *) parm;

    strncpy(p.thd.id,id,3);

    rc = DosCreateThread(&p.thd.tid, (PFNTHREAD) &tidStarter, (ULONG) &p, 0x00000002, stack_size);
    DBGTrace(rc);

    if(rc)
    {
       icqWriteSysRC(icq,PROJECT,rc,"Error starting thread");
       return -rc;
    }

    /* Aguarda ate 6 segundos para que a thread inicie */
    for(f=0;f<600 && p.flag;f++)
       DosSleep(10);

    if(p.flag)
    {
       /* Thread nao iniciou */
       DosKillThread(p.thd.tid);
       DosSleep(10);
       return -1;
    }

    return p.thd.tid;
 }

 ULONG _System icqExceptionHandler(     PEXCEPTIONREPORTRECORD       p1,
                                        PEXCEPTIONREGISTRATIONRECORD p2,
                                        PCONTEXTRECORD               p3,
                                        PVOID                        pv )
 {
/*
    #pragma pack(1)

    struct _table
    {
       ULONG      code;
       const char *descr;
    } tbl[] = {
                { XCPT_GUARD_PAGE_VIOLATION,            NULL                            },
                { XCPT_UNABLE_TO_GROW_STACK,            NULL                            },
                { XCPT_UNWIND,                          NULL                            },
                { XCPT_ASYNC_PROCESS_TERMINATE,         "Assincronous termination"      },
                { XCPT_ACCESS_VIOLATION,                "Access violation"              },

                { 0,                                    NULL                            }
              };


    const struct _table *ptr;
    char                line[0x0100];
    PTIB                ptib                   = NULL;
    APIRET              rc;
    HICQ                icq                     = icqQueryHandler();
    HTHREAD             thread                  = NULL;

    DosBeep(3000,100);

    rc = DosGetInfoBlocks(&ptib, NULL);
    if(rc)
    {
       icqWriteSysRC(NULL, PROJECT, rc, "Error in DosGetInfoBlocks()");
       sprintf(line,"Exception %08lx ",p1->ExceptionNum);
       icqWriteSysLog(NULL,PROJECT,line);
       return XCPT_CONTINUE_SEARCH;
    }

    for(ptr=tbl;ptr->code && ptr->code != p1->ExceptionNum;ptr++);

    if(!ptr->code)
    {
       sprintf(line,"Exception %08lx",p1->ExceptionNum);
    }
    else if(!ptr->descr)
    {
       return XCPT_CONTINUE_SEARCH;
    }
    else
    {
       strcpy(line,ptr->descr);
    }

    if(ptib && ptib->tib_ptib2)
    {
       if(icq)
          for(thread = icqQueryFirstElement(icq->threads); thread && thread->tid != ptib->tib_ptib2->tib2_ultid; thread == icqQueryNextElement(icq->threads,thread));

       if(thread)
          sprintf(line+strlen(line)," (Thread: %3s TID: %d)",thread->id,ptib->tib_ptib2->tib2_ultid);
       else
          sprintf(line+strlen(line)," (TID=%d)",ptib->tib_ptib2->tib2_ultid);
    }

    icqWriteSysLog(NULL,PROJECT,line);
*/

    return XCPT_CONTINUE_SEARCH;          /* Exception not resolved... */
 }

 static void _System tidLoader(struct parm *p)
 {
    #pragma handler ( tidLoader, _Exception )

    ICQTHREAD                   *thread;
    EXCEPTIONREGISTRATIONRECORD RegRec  = {0};
    void                        (* _System start_address) (ICQTHREAD *);

    DBGTracex(p);

    while(!p->thd.tid)
       DosSleep(10);

//    DBGTrace(p->thd.tid);
//    DBGTracex(p->thd.icq->threads);

    thread = icqAddElement(p->thd.icq->threads, sizeof(ICQTHREAD)+p->sz);
    if(!thread)
    {
       p->flag = FALSE;
       _endthread();
    }

    memset(thread,0,sizeof(ICQTHREAD)+p->sz);

    thread->icq  = p->thd.icq;
    thread->parm = p->thd.parm;
    thread->tid  = p->thd.tid;

    strncpy(thread->id,p->thd.id,3);

    start_address = p->start_address;

    p->flag = FALSE;
    DosSleep(0);

    DBGPrint("Iniciando thread [%3s]",thread->id);
    DBGTrace(thread->tid);

    memset(&RegRec,0,sizeof(RegRec));
    RegRec.ExceptionHandler = (ERR)icqExceptionHandler;
    DosSetExceptionHandler( &RegRec );

    start_address(thread);

    DosUnsetExceptionHandler( &RegRec );

    icqRemoveElement(thread->icq->threads, thread);

    CHKPoint();

    _endthread();
 }

 static void _System tidStarter(struct thdStart *p)
 {
    #pragma handler ( tidStarter, _Exception )

    ICQTHREAD                   *thread;
    EXCEPTIONREGISTRATIONRECORD RegRec  = {0};
    void                        (* _System start_address) (ICQTHREAD *, void *);
    char                        *src, *dst;
    int                         f;

    DBGTracex(p);

    while(!p->thd.tid)
       DosSleep(10);

    thread = icqAddElement(p->thd.icq->threads, sizeof(ICQTHREAD)+p->sz);
    if(!thread)
    {
       p->flag = FALSE;
       _endthread();
    }

    memset(thread,0,sizeof(ICQTHREAD)+p->sz);

    thread->icq  = p->thd.icq;
    thread->parm = p->thd.parm;
    thread->tid  = p->thd.tid;

    src = p->thd.parm;
    dst = (char *) (thread+1);

    for(f=0;f<p->sz;f++)
       *(dst++) = *(src++);
    thread->parm = (thread+1);

    strncpy(thread->id,p->thd.id,3);

    start_address = p->start_address;

    p->flag = FALSE;
    DosSleep(0);

    DBGPrint("Iniciando thread [%3s]",thread->id);
    DBGTrace(thread->tid);

    memset(&RegRec,0,sizeof(RegRec));
    RegRec.ExceptionHandler = (ERR)icqExceptionHandler;
    DosSetExceptionHandler( &RegRec );

    start_address(thread,thread->parm);

    DosUnsetExceptionHandler( &RegRec );

    icqRemoveElement(thread->icq->threads, thread);

    CHKPoint();

    _endthread();
 }



