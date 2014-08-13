/*
 * ICQV8 - Protocol manager V8
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

 #include <stdio.h>
 #include <time.h>
 #include <stdlib.h>
 #include <string.h>

#ifdef linux
 #include <unistd.h>
#endif

 #include "icqv8.h"

/*---[ Statics ]----------------------------------------------------------------------------------------------*/


 HMODULE module = NULLHANDLE;


/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 int EXPENTRY icqv8_Configure(HICQ icq, HMODULE mod)
 {
    char buffer[0x0100];

    strcpy(buffer,"Loading ");
    strncat(buffer,icqv8_ftable.descr,0xFF);
    icqWriteSysLog(icq,PROJECT,buffer);

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"**** Extensive logging enabled ****");
#endif

    module = mod;

    icqv8_initPacketList();

    srand(time(NULL));

    if(icqQueryUserDataBlockLength() != sizeof(ICQUSER))
    {
       icqWriteSysLog(icq,PROJECT,"Can't start protocol manager due to invalid pwICQ core version");
       return -1;
    }

    if(!icqRegisterServerProtocolManager(icq,&icqv8_ftable))
    {
       icqWriteSysLog(icq,PROJECT,"Can't register V8 protocol manager");
       return -2;
    }

    return sizeof(ICQV8);

 }

 static int _System modeCallBack(HICQ icq, ULONG uin, USHORT id, ULONG mode)
 {
    DBGTracex(mode);
    icqSetOnlineMode(icq, mode);
    return 0;
 }

 int EXPENTRY icqv8_Start(HICQ icq, HPLUGIN p, ICQV8 *cfg)
 {
    char               temp[0x0100];
    const ICQMODETABLE *md;

    if(cfg->sz != sizeof(ICQV8))
    {
       icqDeRegisterServerProtocolManager(icq,&icqv8_ftable);
       icqWriteSysLog(icq,PROJECT,"Unexpected data block size");
       icqAbend(icq,0);
       return -1;
    }

#ifdef linux
    if(pthread_mutex_init(&cfg->sendFlag, 0))
    {
       icqDeRegisterServerProtocolManager(icq,&icqv8_ftable);
       icqWriteSysLog(icq,PROJECT,"Unable to initialize semaphore");
       icqAbend(icq,0);
       return -1;
    }
#endif

#ifdef __OS2__
    sprintf(temp,"\\SEM32\\PWICQ\\MTX\\V8PROTOCOL\\%ld",icqQueryUIN(icq));
    if(DosCreateMutexSem(temp,&cfg->sendFlag,0,FALSE))
    {
       icqDeRegisterServerProtocolManager(icq,&icqv8_ftable);
       icqWriteSysLog(icq,PROJECT,"Unable to create semaphore");
       icqAbend(icq,0);
       return -1;
    }
#endif

    if(icqGetCaps(icq) & ICQMF_EXTENDEDMENU)
    {
       for(md= icqv8_modes;md->descr;md++)
          icqInsertMenuOption(icq, ICQMNU_MODES, md->descr, md->icon, modeCallBack, md->mode);
    }

    cfg->downSequence = 0xFFFF;
    cfg->seqMasc      = (ULONG) icqQueryShortSequence(icq);
    cfg->seqMasc    <<= 16;

    sprintf(temp,"Client<->Server V8 protocol manager started using session ID %08lx",cfg->seqMasc);
    icqWriteSysLog(icq,PROJECT,temp);

    return 0;
 }

 int EXPENTRY icqv8_Terminate(HICQ icq, ICQV8 *cfg)
 {
    int f;

    CHKPoint();

    if(!cfg || cfg->sz != sizeof(ICQV8))
    {
       icqWriteSysLog(icq,PROJECT,"Client<->Server V8 protocol wasn't started");
       return 0;
    }

    icqSetOnlineMode(icq,ICQ_OFFLINE);

    DBGTrace(cfg->sock);

#ifdef __OS2__
    for(f=0;f<100 && cfg->sock > 0;f++)
       DosSleep(10);
    DosCloseMutexSem(cfg->sendFlag);
#endif

#ifdef linux
    for(f=0;f<100 && cfg->sock > 0;f++)
       usleep(1000);
#endif

    if(!icqDeRegisterServerProtocolManager(icq,&icqv8_ftable))
    {
       icqWriteSysLog(icq,PROJECT,"Unable to deregister protocol manager");
       return 0;
    }

    if(cfg->packets)
       icqWriteSysLog(icq,PROJECT,"WARNING: Packet leak detected");

    if(cfg->sock > 0 || cfg->packets)
       icqWriteSysLog(icq,PROJECT,"Client<->Server V8 protocol stopped abnormally");
    else
       icqWriteSysLog(icq,PROJECT,"Client<->Server V8 protocol stopped normally");

    return 0;
 }

 int EXPENTRY icqv8_Timer(HICQ icq, ICQV8 *cfg)
 {
#ifdef EXTENDED_LOG
    char logBuffer[0x0100];
#endif

#ifdef DEBUG
    if(cfg->sz != sizeof(struct _icqv8))
    {
       DBGMessage("********* ERRO GRAVE!!!!! ENTROU COM AREA DE DADOS DANIFICADA");
       return 0;
    }
#endif

    if(!icqIsOnline(icq))
       return 0;

    if(cfg->kplTimer++ > 60)
    {
       if(cfg->status == ICQV8_STATUS_ONLINE && icqLoadValue(icq,"Keep-Alive",0))
       {
//#ifdef EXTENDED_LOG
//          icqWriteSysLog(icq,PROJECT,"Sending keep-alive");
//#endif
          icqv8_sendKeepAlive(icq,cfg);
          DBGMessage("Enviar keep-alive");
       }

       cfg->kplTimer = 0;
    }

    cfg->watchDogCounter--;

    if(cfg->watchDogCounter < -10)
    {
       DBGMessage("Hang!");
#ifdef EXTENDED_LOG
       sprintf(logBuffer,"Watchdog Message: Hang detected in service thread (ct=%d)",cfg->watchDogCounter);
       icqWriteSysLog(icq,PROJECT,logBuffer);
#else
       icqWriteSysLog(icq,PROJECT,"ERROR: The service thread still locked");
#endif
       if(cfg->sock > 0)
       {
          icqWriteSysLog(icq,PROJECT,"Closing socket");
          icqCloseSocket(cfg->sock);
          cfg->sock = -1;
       }
       icqAbend(icq,0);
    }
    else if(cfg->watchDogCounter < -5)
    {
       DBGMessage("Possible hang, going to offline");

#ifdef EXTENDED_LOG
       sprintf(logBuffer,"Watchdog Message: Hang detected in service thread (ct=%d)",cfg->watchDogCounter);
       icqWriteSysLog(icq,PROJECT,logBuffer);
#else
       icqWriteSysLog(icq,PROJECT,"ERROR: The service thread seens to be hanged");
#endif

       if(icqLoadValue(icq,"c2s.watchDog",1))
       {
          icqSetOnlineMode(icq,ICQ_OFFLINE);
#ifdef __OS2__
          if(cfg->thread > 0)
          {
             icqWriteSysLog(icq,PROJECT,"Changing thread priority to IDLE");
             DosSetPriority(PRTYS_THREAD, PRTYC_IDLETIME, PRTYD_MINIMUM, cfg->thread);
          }
#endif
       }
       else
       {
          cfg->watchDogCounter = 0;
       }

    }
    else if(cfg->watchDogCounter > 10)
    {
       cfg->watchDogCounter = 0;
    }
    else if(cfg->watchDogCounter > 100)
    {
       icqWriteSysLog(icq,PROJECT,"WARNING: Too many packets per second");
       cfg->watchDogCounter = 0;
    }



    return 0;
 }
