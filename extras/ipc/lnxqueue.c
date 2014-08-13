/*
 * LNXQUEUE.C - Envia comandos para a fila de mensagems
 */

 #include <stdio.h>

 #include <pwMacros.h>
 #include <icqtlkt.h>
 #include <icqqueue.h>
 #include <unistd.h>
 #include <string.h>

 #include <sys/ipc.h>
 #include <sys/shm.h>
 #include <sys/msg.h>

/*---[ Prototipos ]----------------------------------------------------------------------------------------*/


/*---[ Implementacao ]-------------------------------------------------------------------------------------*/

 ICQSHAREMEM * EXPENTRY icqIPCGetSharedBlock(void)
 {
    key_t 		   memKey 	=  ftok(PWICQ_SHAREMEM,0x230167);
    int            shmID;
    ICQSHAREMEM	   *icq		=  NULL;

    shmID = shmget(memKey,sizeof(sizeof(ICQSHAREMEM)),0660);

    if( ((int) shmID) == -1)
       return NULL;  
 	
 	icq = shmat(shmID,0,0);
 	 
 	if( ((int) icq) == -1)
 	   return NULL;

	if(icq->sz != sizeof(ICQSHAREMEM) || icq->si != sizeof(ICQINSTANCE))
	{
       shmdt(icq); 	    
	   return NULL;
	}
		
    return icq;
 }

 int EXPENTRY icqIPCReleaseSharedBlock(ICQSHAREMEM *icq)
{
	if(!icq)
	   return -1;
	
	shmdt(icq);
	
	return 0;
}
 
 static int validateUIN(ULONG *uin)
 {
    key_t 		   memKey 	=  ftok(PWICQ_SHAREMEM,0x230167);
    int   			shmID;
    ICQSHAREMEM	*icq;
    int			   f;

    DBGTrace(memKey);
        
    if( ((int) memKey) == -1)
       return -1;
       
    shmID = shmget(memKey,sizeof(sizeof(ICQSHAREMEM)),0660);
    DBGTrace(shmID);
    
    if( ((int) shmID) == -1)
       return -1;  
 	
 	 icq = shmat(shmID,0,0);
 	 DBGTracex(icq);
 	 
 	 if( ((int) icq) == -1)
 	    return -1;

    CHKPoint();

    for(f=0;f<PWICQ_MAX_INSTANCES && (icq->i+f)->uin != *uin;f++)
    {
       if((icq->i+f)->uin && !*uin)
       {
          *uin = (icq->i+f)->uin;
          f--;
       }
    }

    shmdt(icq); 	    

    DBGTrace(*uin);
        
    return (f == PWICQ_MAX_INSTANCES) ? -1 : 0;
 }

 int EXPENTRY icqIPCQueryInstance(ULONG *instance)
 {
 	 *instance = 0;
 	 if(validateUIN(instance))
 	    return -1;
 	 
 	 DBGTrace(*instance);   
    return 0;
 }

 int EXPENTRY icqIPCSendCommand(ULONG uin, ICQQUEUEBLOCK *cmd)
 {
    /* Envia um comando */
    
    ICQQUE_PACKET	 packet;
    ICQQUEUEBLOCK  *req;
    int			    rc		= 0;
    key_t			 key;
    int			    queue;
    int				 f;

 	 if(validateUIN(&uin))
 	    return -1;
 	
    if(cmd->szPrefix != sizeof(ICQQUEUEBLOCK) || cmd->sz < sizeof(ICQQUEUEBLOCK))
       return -2;

    memset(&packet,0,sizeof(ICQQUE_PACKET));
    packet.uin    = uin;
    packet.msg.sz = sizeof(ICQQUE_MESSAGE);

 	 if( (packet.msg.shmID = shmget(IPC_PRIVATE,cmd->sz,IPC_CREAT | 0660)) == -1)
 	    return -3;

 	 req = shmat(packet.msg.shmID,0,0);
 	 if( ((int) req) == -1)
 	 {
 	    return -4;
 	 }
 	 else
 	 {
 	    memcpy(req,cmd,cmd->sz);

       key = ftok("/tmp/pwicq.queue",0x230167); 	    
       
       if( ((int) key) == -2)
       {
          rc = -5;
       }
       else
       {
          queue = msgget(key,0660);
          
          if(queue == -1)
          {
             rc = -6;
          }
          else
          {
             if(msgsnd(queue, (struct msgbuf *) &packet, sizeof(ICQQUE_PACKET), 0) == -1)
             {
                rc = -7;
             }
             else
             {
                for(f=0;f<100 && req->status == cmd->status;f++)
                   usleep(10000);
                DBGTrace(req->status);
                DBGTrace(cmd->status);
 	             memcpy(cmd,req,cmd->sz);
             }
          }
       }
 	    shmdt(req);
 	 }
 	
    return rc;
 }



