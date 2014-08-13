/*
 * TOOLS.C
 */

 #include "icqv8.h"

 USHORT icqv8_convertShort(USHORT vlr)
 {
    return ((vlr & 0xFF00) >> 8) | ( (vlr & 0x00FF) << 8 );
 }


 ULONG icqv8_convertLong(ULONG l)
 {
    ULONG  ret;
    UCHAR  *o   = (UCHAR *) &l;
    UCHAR  *d   = (UCHAR *) &ret;

    // nwLong(0x 01 23 45 67) = 67452301

    *(d++) = *(o+3);
    *(d++) = *(o+2);
    *(d++) = *(o+1);
    *(d++) = *o;

    /*
    #ifdef _DBG_MODE_
       if(ret != htonl(l))
       {
         DBGMessage("**** Erro na conversao");
         DBGTracex(ret);
         DBGTracex(htonl(l));
         exit(-1);
       }
    #endif
    */

    return ret;
 }

 void icqv8_lock(ICQV8 *cfg)
 {
#ifdef linux
   pthread_mutex_lock(&cfg->sendFlag);
#endif

#ifdef __OS2__
   if(DosRequestMutexSem(cfg->sendFlag, 2000))
      icqWriteSysLog(NULL,PROJECT,"Unable to get semaphore");
#endif
 }

 void icqv8_unlock(ICQV8 *cfg)
 {
#ifdef linux
   pthread_mutex_unlock(&cfg->sendFlag);
#endif

#ifdef __OS2__
   DosReleaseMutexSem(cfg->sendFlag);
#endif

 }

 int _System icqv8_isOnline(HICQ icq)
 {
    ICQV8 *cfg = icqGetPluginDataBlock(icq, module);
    return cfg->status == ICQV8_STATUS_ONLINE;
 }



