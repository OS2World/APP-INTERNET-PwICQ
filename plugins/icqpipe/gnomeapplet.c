/*
 * GnomeApplet.C - Send pwICQ Pipe messages to GnomeApplet
 */

 #include <string.h>
 #include "icqpipe.h"


/*---[ Implementation ]-------------------------------------------------------------------------------*/

 void icqpipe_loadConfig(HICQ icq, struct icqpipe_config *cfg)
 {
    cfg->flags = icqLoadValue(icq, "icqPipe:Flags", 0x03);
 }


 void icqpipe_write(HICQ icq, struct icqpipe_config *cfg, const char *text)
 {
	
 }


