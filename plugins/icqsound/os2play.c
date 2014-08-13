/*
 * os2play.c - Play sound
 */

 #pragma strings(readonly)

 #define INCL_DOSQUEUES
 #define INCL_OS2MM
 #define INCL_MCIOS2
 #define INCL_DOSMODULEMGR
 #define INCL_DOSPROCESS

 #include <os2.h>
 #include <os2me.h>
 #include <mciapi.h>

 #include <io.h>
 #include <stdlib.h>
 #include <stdio.h>

 #include "icqsound.h"

/*---[ Definitions ]-------------------------------------------------------------------------*/

 #define WAVALIAS "%08lx.%02x"

/*---[ Prototipes ]--------------------------------------------------------------------------*/

 static APIRET execMCI(HICQ,PSZ);

/*---[ Statics ]-----------------------------------------------------------------------------*/

 static UCHAR  Sequence = 0;

/*---[ Implement ]---------------------------------------------------------------------------*/

 void icqSound_Speaker(char *src)
 {

   ULONG freq    = 0;
   char  pos     = 0;
   char  *ptr    = src;

   while(*src)
   {
      if(*src == ',')
      {
         *src = 0;
         if(pos == 0)
         {
            freq = atoi(ptr);
            pos  = 1;
         }
         else
         {
            pos  = 0;
            if(freq)
               DosBeep(freq,atol(ptr));
            else
               DosSleep(atol(ptr));
         }
         ptr     = (src+1);
      }
      src++;
   }

   if(*ptr && pos == 1 && freq)
      DosBeep(freq,atol(ptr));

 }

 void icqSound_WavAudio(HICQ icq, char *fileName)
 {
   ULONG        uin		= icqQueryUIN(icq);
   int          share	= icqLoadValue(icq,"sound:Share",1);
   UCHAR        counter = Sequence++;

   HAB          hab;
   HMQ          hmq;
   char         linha[0xFF];

   if(!uin || access(fileName,4))
      return;

   hab = WinInitialize(0);
   DBGTracex(hab);

   if(hab)
   {

      hmq = WinCreateMsgQueue( hab, 0 );
      DBGTracex(hmq);

      if(hmq)
      {
         DBGMessage(fileName);

         sprintf(linha,"open %s%s type waveaudio alias " WAVALIAS " wait",
                              fileName,
                              share ? " shareable" : "",
                              uin, counter);
         execMCI(icq,linha);

         sprintf(linha,"play " WAVALIAS " wait", uin, counter);
         execMCI(icq,linha);

         sprintf(linha,"close " WAVALIAS " wait" , uin, counter);
         execMCI(icq,linha);

         WinDestroyMsgQueue(hmq);
      }

      WinTerminate(hab);
   }

 }

 static APIRET execMCI(HICQ icq, PSZ cmd)
 {
#ifdef LOG_COMMANDS
    icqWriteSysLog(icq,cmd);
#endif

#ifdef NOPLAY
    return 0;
#else
    return mciSendString(cmd,0,0,0,0 );
#endif
 }



