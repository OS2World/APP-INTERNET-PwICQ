/*
 * LNXSTART.C - Codigo de inicializacao
 */


#ifndef linux
   #error Only for Linux version
#endif

 #include <icqkernel.h>

 #include <time.h>
 #include <string.h>
 #include <stdio.h>
 #include <sys/time.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <sys/ipc.h>
 #include <sys/shm.h>
 #include <stdlib.h>
 #include <sys/msg.h>
 #include <unistd.h>

 #include <pwMacros.h>

 #include "icqcore.h"


/*---[ Prototipos ]---------------------------------------------------------------*/


/*---[ Statics ]------------------------------------------------------------------*/

#ifdef __DLL__

   const struct coreEntryPoints pwICQCoreEntryPoints =
   {
	  sizeof(struct coreEntryPoints),
	  sizeof(ICQ),
	  icqCoreStartup,
	  icqCoreTerminate,
	  icqTerminate,
	  icqChkCmdLine,
	  icqkrnl_Timer
   };

#endif

/*---[ Funcoes ]------------------------------------------------------------------*/

HICQ EXPENTRY icqCoreStartup(int numpar, char **param, STATUSCALLBACK *startupStage)
{
   /*
    * Inicializa o nucleo
    */
   HICQ    		   icq				= malloc(sizeof(ICQ));
   key_t 			memKey;
   const char     *shmName       = PWICQ_SHAREMEM;

#ifdef MEMWATCH
    DBGMessage("Using MemWatch");
    mwStatistics( 2 );
#endif

   memset(icq,0,sizeof(icq));
   icq->sz    = sizeof(ICQ);
   icq->usz   = sizeof(USERINFO);
   icq->queue = -1;

   /* Crio o bloco de memoria compartilhada */
    mknod(shmName,0666,0);
 memKey = ftok(shmName,0x230167);
 	
 	 DBGTrace(memKey);
 	
    if( ((int) memKey) != -1)
    {
       /* Conseguiu criar a chave para o bloco de memoria compartilhada */
   	 if( (icq->shmID = shmget(memKey,sizeof(sizeof(ICQSHAREMEM)),IPC_CREAT | 0660)) != -1)
 	    {
 	       icq->shareBlock = shmat(icq->shmID, 0, 0);
 	
 	       if( ((int) icq->shareBlock) == -1)
 	       {
 	          icq->shareBlock = 0;
 	       }
 	       else if(!icq->shareBlock->sz)
 	       {
 	          DBGMessage("Shared mem block created");
 	          memset(icq->shareBlock,0,sizeof(ICQSHAREMEM));
 	          icq->shareBlock->sz = sizeof(ICQSHAREMEM);
 	          icq->shareBlock->si = sizeof(ICQINSTANCE);
 	          icq->shareBlock->instances = 1;
          }
 	       else if(icq->shareBlock->sz != sizeof(ICQSHAREMEM) || icq->shareBlock->si != sizeof(ICQINSTANCE))
 	       {
 	       	 DBGTrace(icq->shareBlock->sz);
 	       	 DBGTrace(icq->shareBlock->si);
 	          DBGMessage("Invalid shared memory block");
         	 shmdt(icq->shareBlock);
             icq->shareBlock = 0;
 	       }
 	       else
 	       {
 	          DBGMessage("Connected to the shared memory block");
  	          icq->shareBlock->instances++;
 	       }
 	    }
    }


   /* Faco a inicializacao final */
   initialize(icq,numpar,param, startupStage);

   return icq;

}

void EXPENTRY icqkrnl_Timer(HICQ icq)
{
   UCHAR 	 	  Flags = 0;
   ICQQUE_PACKET  packet;
   ICQQUEUEBLOCK  *req;
   int            bytes;

   /* Verifica estado do bloco de memoria compartilhada */
   if(icq->info)
   {
      if(icq->info->pid != icq->myPID)
      {
		 icqPrintLog(icq,PROJECT,"Unexpected PID in shared memory block (Current: %d, Found: %d)",icq->myPID,icq->info->pid);
         icqAbend(icq,0);
      }
   }


   /* Verifica temporizadores */

   if(icq->timerTick++ == 10 && icq->ready)
   {
      icq->timerTick = 0;
      Flags     |= 0x01;
      oneSecond(icq);
      if(icq->seconds++ > 58)
      {
		 
         if(icq->cntlFlags & ICQFC_SAVECONFIG)
	        icqSaveConfigFile(icq);
		 
         icq->seconds = oneMinute(icq);
         Flags |= 0x02;
      }
   }

   if(icq->skin && icq->skin->timer)
      icq->skin->timer(icq,icq->skinData,Flags);

   if(icq->queue != -1)
   {
      /* Verifica estado da fila de mensagems */
      memset(&packet,0,sizeof(ICQQUE_PACKET));
      bytes = msgrcv(icq->queue, &packet, sizeof(ICQQUE_MESSAGE), icq->uin, IPC_NOWAIT|MSG_NOERROR);

      while(bytes > 0)
      {
         /* Verifico o bloco recebido */

         if(packet.msg.sz != sizeof(ICQQUE_MESSAGE))
         {
            icqWriteSysLog(icq,PROJECT,"Invalid request");
         }
         else
         {
            req = shmat(packet.msg.shmID,0,0);

            if( ((int) req) == -1)
            {
               icqWriteSysLog(icq,PROJECT,"Unable to get access to the request block");
            }
            else
            {
               icqProcessRemoteBlock(icq,req);
               shmdt( (char *) req);
            }
         }
      	
         memset(&packet,0,sizeof(ICQQUE_PACKET));
         bytes = msgrcv(icq->queue, &packet, sizeof(ICQQUE_MESSAGE), icq->uin, IPC_NOWAIT|MSG_NOERROR);
      }
   }

   return;
}

int EXPENTRY icqCoreTerminate(HICQ icq)
{
   int instances = 9999;

   if(icq->sz != sizeof(ICQ))
      return -1;

#ifdef EXTENDED_LOG
   sysLog(icq,"pwICQ core stopping");
#endif

   terminate(icq);

   if(icq->shareBlock)
   {
      sysLog(icq,"Releasing instance indicator");
      icq->shareBlock->instances--;
      instances = icq->shareBlock->instances;
   	  shmdt(icq->shareBlock);
      icq->shareBlock = 0;
   }

   DBGTrace(instances);

   if(!instances)
   {
      sysLog(icq,"Cleaning up shared resources");
      remove("/tmp/pwicq.queue");
      remove("/tmp/pwicq.shared");
   }

   free(icq);

   sysLog(icq,"pwICQ core stopped");

   return 0;
}

