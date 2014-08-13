/*
 * icqSound.h - Prototipos para o modulo de audio
 */

 #include <icqtlkt.h>

 extern const DLGMGR sndConfig;
 extern const DLGMGR grpConfig;

#ifdef linux
 extern const TABLEDEF event_table[];
#endif

 void icqSound_Speaker(char *);
 void icqSound_WavAudio(HICQ icq, char *);

 int  EXPENTRY icqsound_Play(HICQ, const char *);

   

