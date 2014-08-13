/*
 * icqSound.c - Ponto de entrada para as funcoes de audio
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdlib.h>
 #include <stdio.h>

#ifdef GNOMEAUDIO
 #include <gnome.h>
#endif

 #include <icqtlkt.h>
 #include <icqqueue.h>


 #include "icqsound.h"

/*---[ Prototipes ]--------------------------------------------------------------------------*/

 static void _System thdPlay(ICQTHREAD *, char *);

/*---[ Public ]------------------------------------------------------------------------------*/

 HMODULE module = 0;
 int     busy   = 0;

/*---[ Implement ]---------------------------------------------------------------------------*/

 int EXPENTRY icqsound_Configure(HICQ icq, HMODULE hmod)
 {

    if(icqQueryUserDataBlockLength() != sizeof(ICQUSER))
    {
       icqWriteSysLog(icq,PROJECT,"Can't start sound plugin due to invalid pwICQ core version");
       return -1;
    }

#ifdef GNOMEAUDIO
    icqWriteSysLog(icq,PROJECT,"Loading Gnome Audio plugin Build " BUILD);
    gnome_sound_init(NULL);
#endif

#ifdef __OS2__
    icqWriteSysLog(icq,PROJECT,"Loading OS/2 Audio plugin Build " BUILD);
#endif

    icqSetCaps(icq,ICQMF_SOUND);        // Turn audio capability on

    module = hmod;

//    icqSound_WavAudio(icq,"/home/perry/public/pwicq2/os2/bin/sounds/startup.wav");

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"Extensive logging enabled");
#endif

#ifdef DEBUG
//	DBGTrace(icqsound_saveScheme(icq, "debug.scm"));
//	DBGTrace(icqsound_loadScheme(icq, "/home/perry/public/pwicq2/sound-schemes/synthvoices.scm"));
//    DBGTrace(icqsound_loadScheme(icq,"\\Public\\pwicq2\\sound-schemes\\synthvoices.scm"));
//    DBGTrace(icqsound_loadScheme(icq,"W:\\Public\\pwicq2\\sound-schemes\\speaker.scm"));
#endif	

    return 0;
 }

 void EXPENTRY icqsound_ConfigPage(HICQ icq, void *dummy, ULONG uin, USHORT type, HWND hwnd, const DLGINSERT *dlg, char *buffer)
 {
    DBGTrace(type);

    if(type != 1)
       return;

    dlg->loadPage(hwnd, module, 100, &sndConfig,   CFGWIN_EVENTS);
    dlg->loadPage(hwnd, module, 101, &grpConfig,   CFGWIN_USERS);

 }


 void EXPENTRY icqsound_Terminate(HICQ icq, HMODULE hmod)
 {
#ifdef GNOMEAUDIO
    gnome_sound_shutdown();
#endif
    icqWriteSysLog(icq,PROJECT,"Audio support plugin unloaded");
 }

 void EXPENTRY icqsound_Event(HICQ icq, void *dummy, ULONG uin, char id, USHORT event, ULONG parm)
 {
    char        key[20];
    char        buffer[0x0100];
    int         f;
    ULONG       gSound  = icqLoadValue(icq,"GSound",0);
    ULONG       masc    = 0x00000001;
    HUSER       usr;

#ifdef DEBUG
    if(busy)
#else
    if( busy || !(icqQueryModeFlags(icq) & ICQMF_SOUND))
#endif
       return;

    *buffer = 0;

    if(gSound && id == 'U' && event == ICQEVENT_ONLINE)
    {
       usr = icqQueryUserHandle(icq,uin);
       if(usr && (usr->groupMasc & gSound))
       {
          gSound &= usr->groupMasc;
          for(f=0;f<PWICQGROUPCOUNTER && !*buffer;f++)
          {
             if(gSound & masc)
             {
                sprintf(key,"GSound%02d",f);
                icqLoadString(icq,key,"",buffer,0xFF);
             }
             masc <<= 1;
          }
       }
       DBGMessage(buffer);
    }

    if(!*buffer)
    {
       sprintf(key,"sound:%c%03d",id,(int) event);
       icqLoadString(icq, key, "", buffer, 0xFF);
    }

    if(!*buffer)
       return;

    icqsound_Play(icq,buffer);

 }

 int EXPENTRY icqsound_Play(HICQ icq, const char *buffer)
 {
    DBGTrace(busy);
    DBGMessage(buffer);

    if(busy)
       return 0;

    busy = TRUE;
    return icqStartThread(icq, (void (* _System)(ICQTHREAD *, void *)) thdPlay, 0xFFFF, -1, buffer, "snd");

 }

 static void _System thdPlay(ICQTHREAD *thd, char *fileName)
 {
    DBGMessage(fileName);

    icqSystemEvent(thd->icq, ICQEVENT_BEGINPLAY);

    if(*fileName == '%')
       icqSound_Speaker(fileName+1);
    else
       icqSound_WavAudio(thd->icq,fileName);

    icqSystemEvent(thd->icq, ICQEVENT_ENDPLAY);

    busy = FALSE;
 }


 int EXPENTRY icqsound_rpcRequest(HICQ icq, struct icqpluginipc *ipc)
 {
    const char *cmd = (const char *) (ipc+1);
    int        rc   = -1;

    DBGMessage(cmd);

    switch(*cmd)
    {
    case 'p':	/* Play        */
       icqsound_Play(icq,cmd+1);
       break;

    case 'l':	/* Load scheme */
       rc = icqsound_loadScheme(icq, (cmd+1));
       DBGTrace(ipc->h.rc);
       break;

    case 's':	/* Save scheme */
       rc = icqsound_saveScheme(icq, (cmd+1));
       DBGTrace(ipc->h.rc);
       break;

    default:
       icqWriteSysLog(icq,PROJECT,"Unexpected remote command");

    }

    DBGTrace(rc);
    return rc;

 }

