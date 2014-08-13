/*
 * ICQPipe.C    pwICQ plugin to send messages to a Pipe
 *              (Designed for use with SysBar/2)
 *
 */


#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>

 #include "icqpipe.h"

/*----------------------------------------------------------------------------*/

 struct msgtype
 {
    USHORT      type;
    char        *masc;
 };

/*----------------------------------------------------------------------------*/

 static void systemEvent(HICQ, struct icqpipe_config *, USHORT);
 static void userEvent(HICQ, struct icqpipe_config *, HUSER, USHORT);
 static void messageEvent(HICQ, struct icqpipe_config *, ULONG, USHORT, HMSG);

/*----------------------------------------------------------------------------*/

 HMODULE module                  = NULLHANDLE;

/*----------------------------------------------------------------------------*/

 int EXPENTRY icqpipe_Configure(HICQ icq, HMODULE mod)
 {
    /*
     *
     * PLUGIN configuration routine, the first called by pwICQ, must return 0
     * or the a size for the plugin data-area to be allocated by the core
     *
     * * If returns a negative value the plugin is unloaded
     *
     */
    module = mod;

    CHKPoint();

    return sizeof(struct icqpipe_config);
 }

 int EXPENTRY icqpipe_Start(HICQ icq, HPLUGIN p, struct icqpipe_config *cfg)
 {
    cfg->sz = sizeof(struct icqpipe_config);
#ifdef __OS2__	
    icqWriteSysLog(icq, PROJECT,"Pipewriter plugin starting (Build " BUILD ")");
#endif
	
#ifdef GNOME_APPLET	
    icqWriteSysLog(icq, PROJECT,"Gnome Panel writer plugin starting (Build " BUILD ")");
#endif
	
    icqpipe_loadConfig(icq,cfg);
    return 0;
 }

 void EXPENTRY icqpipe_Event(HICQ icq, struct icqpipe_config *cfg, ULONG uin, char id, USHORT eventCode, ULONG parm)
 {
    switch(id)
    {
    case 'S':
       systemEvent(icq,cfg,eventCode);
       break;

    case 'U':
       userEvent(icq, cfg, icqQueryUserHandle(icq,uin), eventCode);
       break;

    case 'M':
       messageEvent(icq,cfg,uin,eventCode, (HMSG) parm);
       break;

    }
 }

 static void messageEvent(HICQ icq, struct icqpipe_config *cfg, ULONG uin, USHORT msgType, HMSG msg)
 {
    char buffer[0x0100];

    if(!(cfg->flags&4))
       return;

    if(!msg || msg->sz != sizeof(ICQMSG))
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected message block");
       return;
    }

    icqQueryMessageTitle(icq, uin, FALSE, msg, buffer, 0xFF); // Use the message manager plugins to get title
    icqpipe_Write(icq,cfg,buffer);

 }

 static void systemEvent(HICQ icq, struct icqpipe_config *cfg, USHORT eventCode)
 {
    /*
     * Called in every system event
     *
     * Event codes in icqdefs.H (#define ICQEVENT_*)
     *
     */

    if(!cfg)
       return;

    if(!(cfg->flags&1))
    {
       if(eventCode == ICQEVENT_OFFLINE)
          icqpipe_Write(icq,cfg,"");
       return;
    }

    switch(eventCode)
    {
    case ICQEVENT_SECONDARY:
       icqpipe_Write(icq,cfg,"Startup complete");
       break;

    case ICQEVENT_OFFLINE:
       icqpipe_Write(icq,cfg,"Disconnected");
       break;

    case ICQEVENT_CONTACT:
       icqpipe_Write(icq,cfg,"Connecting");
       break;

    }

 }

 static void userEvent(HICQ icq, struct icqpipe_config *cfg, HUSER usr, USHORT eventCode)
 {
    /*
     * Called in every user event
     *
     * Event codes in icqdefs.H (#define ICQEVENT_*)
     *
     */
    ULONG       flags;
    char        text[0x0100];

    if(!cfg)
       return;

    flags = cfg->flags;

    flags &=  0xffff0000;
    flags >>= 16;

    if(usr && flags && !(usr->groupMasc & flags))
       return;

    if((cfg->flags & 2) && eventCode == ICQEVENT_ONLINE)
    {
       sprintf(text,"%s online",icqQueryUserNick(usr));
       icqpipe_Write(icq,cfg,text);
    }
    else if((cfg->flags & 8) && eventCode == ICQEVENT_PEERBEGIN)
    {
       sprintf(text,"Connected with %s",icqQueryUserNick(usr));
       icqpipe_Write(icq,cfg,text);
    }
    else if((cfg->flags & 8) && eventCode == ICQEVENT_PEEREND)
    {
       sprintf(text,"Disconnected from %s",icqQueryUserNick(usr));
       icqpipe_Write(icq,cfg,text);
    }
 }


