/*
 * icqSound.h - Prototipos para o modulo de audio
 */

 #include <icqtlkt.h>

 extern const DLGMGR sndConfig;
 extern const DLGMGR grpConfig;

 enum SCHEME_ERRORS
 {
	SCHEME_NOERROR,
	SCHEME_NOEVENTS,
	SCHEME_CANTCOPY,
	SCHEME_NOOUTFILE,
	SCHEME_CANTSEEK,
	SCHEME_READERROR,
	SCHEME_CRCERROR,
	SCHEME_INVALIDRECORD,
	SCHEME_CANTOPEN,
	SCHEME_INVALIDFILE,
	
	SCHEME_UNKNOWN
 };

 void icqSound_Speaker(char *);
 void icqSound_WavAudio(HICQ icq, char *);

 int  EXPENTRY icqsound_saveScheme(HICQ, const char *);
 int  EXPENTRY icqsound_loadScheme(HICQ, const char *);
 int  EXPENTRY icqsound_Play(HICQ, const char *);


