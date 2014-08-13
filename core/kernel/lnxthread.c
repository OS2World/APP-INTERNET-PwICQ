/*
 * LNXTHREAD.C - Thread manager for linux version
 */

#ifndef linux
   #error Only for linux version
#endif

 #include <string.h>
 #include <malloc.h>
 #include <unistd.h>
 #include <stdio.h>
 
 #include <icqkernel.h>
 #include "icqcore.h"

/*---[ Defines ]---------------------------------------------------------------------*/

 #pragma pack(1)

/*---[ Prototipes ]------------------------------------------------------------------*/

 static void tidLoader(ICQTHREAD *);
 static void tidStarter(ICQTHREAD *);

/*---[ Implementation ]--------------------------------------------------------------*/

 int EXPENTRY icqCreateThread(HICQ icq, void (* _System start_address)(ICQTHREAD *), unsigned int stack_size, int sz, void *arglist, const char *id)
 {
    /* Start a thread, don't pass parameters */

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

    if(pthread_create( &thread->tid, NULL, (void*)&tidLoader, (void*) thread))
    {
       sysLog(icq,"Unable to start a new thread");
       return -1;
    }

    return thread->tid;
 }

 int EXPENTRY icqStartThread(HICQ icq, void (* _System start_address)(ICQTHREAD *, void *), unsigned int stack_size, int sz, const void *parm, const char *id)
 {
    /* Start a thread, pass parameters */
    ICQTHREAD *thread;

    if(sz == -1)
       sz = strlen(parm)+1;

    thread = icqAddElement(icq->threads, sizeof(ICQTHREAD)+sz);
    
    if(!thread)
    {
       sysLog(icq,"Unable to allocate thread element");
       return -1;
    }

    memset(thread,0,sizeof(ICQTHREAD));
    
//    DBGMessage(parm);
    
    memcpy((thread+1),parm,sz);
    
    thread->icq  = icq;
    thread->parm = (void *) parm;
    strncpy(thread->id,id,3);

    thread->start_address = (void *) start_address;

    if(pthread_create( &thread->tid, NULL, (void*)&tidStarter, (void*) thread))
    {
       sysLog(icq,"Unable to start a new thread");
       return -1;
    }

    return thread->tid;
 }

 int EXPENTRY icqKillThread(HICQ icq,ICQTHREAD *thread)
 {
    return -1;
 }
 
 static void logThread(ICQTHREAD *thd, const char *txt)
 {
	char buffer[0x0100];
	char tidName[4];
	int  f;
	
	for(f=0;f<100 && !thd->tid;f++)
	   usleep(100);
	
    for(f=0;f<3;f++)
	   tidName[f] = thd->id[f];
	tidName[4] = 0;
	
	sprintf(buffer,"Thread \"%3s\" %s (pid=%d, tid=%d)",tidName,txt,getpid(),(int) thd->tid);
	icqWriteSysLog(thd->icq,PROJECT,buffer);
 }
 

 static void tidLoader(ICQTHREAD *thread)
 {
    pthread_detach(pthread_self());

	CHKPoint();
	logThread(thread,"begin");
    thread->start_address(thread);
	logThread(thread,"end");

    icqRemoveElement(thread->icq->threads, thread);
    pthread_exit(0);
 }

 static void tidStarter(ICQTHREAD *thread)
 {
    void (* _System start_address)(ICQTHREAD *, void *) = (void *) thread->start_address;

    pthread_detach(pthread_self());
	
	CHKPoint();
	logThread(thread,"begin");
    start_address(thread,(thread+1));
	logThread(thread,"end");
	
    icqRemoveElement(thread->icq->threads, thread);
    pthread_exit(0);
 }


