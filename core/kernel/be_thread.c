/*
 * BE_THREAD.C - Thread manager for BEOS version
 */

#ifndef BEOS
   #error Only for BEOS version
#endif

 #include <icqkernel.h>
 #include "icqcore.h"

/*---[ Defines ]---------------------------------------------------------------------*/

 #pragma pack(1)

/*---[ Prototipes ]------------------------------------------------------------------*/

 void tidLoader(ICQTHREAD *);

/*---[ Implementation ]--------------------------------------------------------------*/

 int EXPENTRY icqCreateThread(HICQ icq, void (* _System start_address)(ICQTHREAD *), unsigned int stack_size, int sz, void *arglist, const char *id)
 {
    /* Start a thread, pass parameters */
    ICQTHREAD *thread = icqAddElement(icq->threads, sizeof(ICQTHREAD)+sz);

    if(!thread)
    {
       sysLog(icq,"Unable to allocate thread element");
       return -1;
    }

    memset(thread,0,sizeof(ICQTHREAD)+sz);
    thread->icq  = icq;
    thread->parm = arglist;
    strncpy(thread->id,id,3);

    thread->start_address = start_address;

    thread->tid = spawn_thread( tidLoader, "pwICQ Thread", B_NORMAL_PRIORITY, (void *) thread);

    if(thread->tid < B_NO_ERROR)
    {
       sysLog(icq,"Unable to start a new thread");
       return -1;
    }

    resume_thread(thread->tid);

    return 0;
 }

 int EXPENTRY icqKillThread(HICQ icq,ICQTHREAD *thread)
 {
    if(kill_thread(thread->tid) != B_NO_ERROR)
       return 0;
    return -1;
 }

 void tidLoader(ICQTHREAD *thread)
 {
    thread->start_address(thread);
    icqRemoveElement(thread->icq->threads, thread);
    exit_thread(0);
 }

