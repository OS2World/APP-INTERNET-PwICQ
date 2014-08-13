/*
 * os2play.c - Play sound
 */

 #include <stdlib.h>
 #include <stdio.h>

 #include <gnome.h>

 #include "icqsound.h"

/*---[ Definitions ]-------------------------------------------------------------------------*/

/*---[ Prototipes ]--------------------------------------------------------------------------*/

/*---[ Statics ]-----------------------------------------------------------------------------*/

/*---[ Implement ]---------------------------------------------------------------------------*/

 void icqSound_Speaker(char *src)
 {

 }

 void icqSound_WavAudio(HICQ icq, char *fileName)
 {
    DBGMessage(fileName);
#ifdef GNOMEAUDIO
    gnome_sound_play(fileName);
#endif	
 }


