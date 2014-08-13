/*
 * OS2QUEUE.C - Envia comandos para a fila de mensagems
 */

#ifndef __OS2__
   #error Only for OS2 version
#endif

 #pragma strings(readonly)

 #define INCL_ERRORS
 #define INCL_DOSQUEUES
 #define INCL_DOSPROCESS
 #include <os2.h>

 #include <stdio.h>

 #include <pwMacros.h>
 #include <icqtlkt.h>
 #include <icqqueue.h>

/*---[ Prototipos ]----------------------------------------------------------------------------------------*/


/*---[ Implementacao ]-------------------------------------------------------------------------------------*/

 ICQSHAREMEM * EXPENTRY icqIPCGetSharedBlock(void)
 {
    int                 rc = -1;
    ICQSHAREMEM         *shareBlock;

    if(DosGetNamedSharedMem((PPVOID) &shareBlock,PWICQ_SHAREMEM,PAG_READ|PAG_WRITE))
       return NULL;

    if(shareBlock->sz == sizeof(ICQSHAREMEM) && shareBlock->si == sizeof(ICQINSTANCE))
       return shareBlock;

    DosFreeMem(shareBlock);

    return NULL;
 }

 int EXPENTRY icqIPCReleaseSharedBlock(ICQSHAREMEM *shareBlock)
 {
    if(shareBlock)
       return DosFreeMem(shareBlock);
    return -1;
 }

 int EXPENTRY icqIPCQueryInstance(ULONG *instance)
 {
    int                 rc = -1;
    ICQSHAREMEM         *shareBlock;
    int                 f;

    rc = DosGetNamedSharedMem((PPVOID) &shareBlock,PWICQ_SHAREMEM,PAG_READ|PAG_WRITE);

    DBGTrace(rc);

    if(rc)
       return rc;

    if(shareBlock->sz == sizeof(ICQSHAREMEM) && shareBlock->si == sizeof(ICQINSTANCE))
    {
       /* Tenho um bloco valido, procuro por um UIN ativo */
       *instance = 0;
       rc        =  ERROR_FILE_NOT_FOUND;

       for(f=0; f < PWICQ_MAX_INSTANCES && !*instance;f++)
       {
          if( (shareBlock->i+f)->uin )
          {
             rc = 0;
             *instance = (shareBlock->i+f)->uin;
          }
       }

    }
    else
    {
       rc = ERROR_INVALID_BLOCK;
    }

    DosFreeMem(shareBlock);

    return rc;
 }

 int EXPENTRY icqIPCSendCommand(ULONG uin, ICQQUEUEBLOCK *cmd)
 {
    /* Envia um comando */
    char                buffer[0x0100];
    int                 rc                      = 0;
    PID                 pid;
    HQUEUE              queue;
    ICQQUEUEBLOCK       *shr;
    int                 f;
    char                *src;
    char                *dst;
    USHORT              status;
    REQUESTDATA         req;
    int                 sz;

    if(cmd->szPrefix != sizeof(ICQQUEUEBLOCK) || cmd->sz < sizeof(ICQQUEUEBLOCK))
       return ERROR_INVALID_PARAMETER;

    if(!uin)
    {
       rc = icqIPCQueryInstance(&uin);
       if(rc)
          return rc;
    }

    sz = cmd->sz;

    /* Abre a fila */

    sprintf(buffer,"\\QUEUES\\PWICQ\\%lu",uin);
    rc = DosOpenQueue ( &pid,&queue, buffer);

    if(rc)
       return rc;

    rc = DosAllocSharedMem((PVOID) &shr, NULL, sz, PAG_COMMIT|OBJ_GETTABLE|PAG_READ|PAG_WRITE);

    if(!rc)
    {
       DBGTracex(shr);

       src = (char *) cmd;
       dst = (char *) shr;

       for(f=0;f<sz;f++)
          *(dst++) = *(src++);

       rc = DosWriteQueue(queue, 0, sizeof(ULONG), (PVOID) shr, 0);

       if(!rc)
       {
          DBGMessage("Mensagem escrita na fila");

          for(f=0;f<100 && shr->status == cmd->status;f++)
             DosSleep(10);

          DBGTrace(shr->status == cmd->status);

          src = (char *) shr;
          dst = (char *) cmd;

          for(f=0;f<sz;f++)
             *(dst++) = *(src++);
       }

       DosFreeMem(shr);
    }

    DosCloseQueue(queue);

    return rc;
 }



