/*
 * icqosd.c - LibOSD interface plugin
 *
 */

 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include <xosd.h>
 
 #include <icqtlkt.h>

/*---[ Strucutures ]----------------------------------------------------------*/

 #pragma pack(1)
 
 typedef struct _icqosdconfig
 {
	USHORT sz;
	xosd   *osd;
	ULONG  flags;
 } ICQOSDCONFIG;
 
 #pragma pack()

/*---[ Prototipes ]-----------------------------------------------------------*/

 static void loadConfig(HICQ,   ICQOSDCONFIG *);
 static void systemEvent(HICQ,  ICQOSDCONFIG *, USHORT);
 static void userEvent(HICQ,    ICQOSDCONFIG *, HUSER, USHORT);
 static void messageEvent(HICQ, ICQOSDCONFIG *, ULONG, USHORT, HMSG);

 void icqosd_Write(HICQ, ICQOSDCONFIG *, const char *);
 
/*---[ Constants ]------------------------------------------------------------*/
 
 #define getValue(i,k,d)  icqLoadValue(i, "OSD." k, d)
 #define getString(i,k,d) icqLoadString(i, "OSD." k, d, buffer, 0xFF)

 /*---[ Statics ]--------------------------------------------------------------*/
 
 static HMODULE module;
 
 /*---[ Implementation ]-------------------------------------------------------*/

 int EXPENTRY icqosd_Configure(HICQ icq, HMODULE mod)
 {
	module = mod;
    icqWriteSysLog(icq, PROJECT,"LIBOSD interface plugin loaded (Build " BUILD ")");
    return sizeof(ICQOSDCONFIG);
 }

 int EXPENTRY icqosd_Start(HICQ icq, HPLUGIN p, ICQOSDCONFIG *cfg)
 {
	CHKPoint();
	cfg->sz  = sizeof(ICQOSDCONFIG);
	cfg->osd = xosd_create(getValue(icq,"Lines",2));

	DBGTracex(cfg->osd);
	loadConfig(icq,cfg);

#ifdef DEBUG	
	icqosd_Write(icq,cfg,"pwICQ OSD Debug");
#endif	
	
	return 0;
 }

 int EXPENTRY icqosd_Stop(HICQ icq, HPLUGIN p, ICQOSDCONFIG *cfg)
 {
	CHKPoint();
	
	xosd_destroy(cfg->osd);
	
	return 0;
 }
 
 void EXPENTRY icqosd_rpcRequest(HICQ icq, struct icqpluginipc *ipc)
 {
	ICQOSDCONFIG *cfg = icqGetPluginDataBlock(icq, module);
	
	DBGTracex(cfg);
	
	if(!cfg)
	   return;
	
	if(cfg->sz != sizeof(ICQOSDCONFIG))
	{
	   icqWriteSysLog(icq,PROJECT,"Invalid plugin data block received");
	   return;
	}
	
    icqosd_Write(icq,cfg, (char *) (ipc+1) );	
 }
 
 void icqosd_Write(HICQ icq, ICQOSDCONFIG *cfg, const char *text)
 {
	icqWriteSysLog(icq,PROJECT,text);
    xosd_hide(cfg->osd);
    xosd_show(cfg->osd);
    xosd_display(cfg->osd, 0, XOSD_string, text);
 }

 static void loadConfig(HICQ icq, ICQOSDCONFIG *cfg)
 {
	char buffer[0x0100];
	
	cfg->flags = getValue(icq,"Flags",0x07);
	
	xosd_set_pos(cfg->osd,               getValue(icq,  "Pos",XOSD_top));
    xosd_set_align(cfg->osd,             getValue(icq,  "Align",XOSD_right));
	xosd_set_colour(cfg->osd,            getString(icq, "Color","Yellow"));
	xosd_set_vertical_offset(cfg->osd,   getValue(icq,  "VOffset",0));
    xosd_set_horizontal_offset(cfg->osd, getValue(icq,  "HOffset", 0));
	
	getString(icq, "Font", "");
	if(*buffer)
	   xosd_set_font(cfg->osd, buffer);
	
    xosd_set_shadow_offset(cfg->osd,	 getValue(icq,  "ShadowOffset", 0));
//    xosd_set_shadow_colour(cfg->osd,     getString(icq, "ShadowColor", "Black"));
//    xosd_set_outline_colour(cfg->osd,    getString(icq, "OutlineColor", "Black"));
	
	xosd_set_timeout(cfg->osd, getValue(icq,"Timeout",60));
	
 }

 void EXPENTRY icqosd_Event(HICQ icq, ICQOSDCONFIG *cfg, ULONG uin, char id, USHORT eventCode, ULONG parm)
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

 static void messageEvent(HICQ icq, ICQOSDCONFIG *cfg, ULONG uin, USHORT msgType, HMSG msg)
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
    icqosd_Write(icq,cfg,buffer);

 }

 static void systemEvent(HICQ icq, ICQOSDCONFIG *cfg, USHORT eventCode)
 {
    /*
     * Called in every system event
     *
     * Event codes in icqdefs.H (#define ICQEVENT_*)
     *
     */

    if(!cfg)
       return;

	/*
    if(!(cfg->flags&1))
    {
       if(eventCode == ICQEVENT_OFFLINE)
          icqOSD_Write(icq,cfg,"");
       return;
    }
    */
	
    switch(eventCode)
    {
#ifdef DEBUG	   
    case ICQEVENT_SECONDARY:
       icqosd_Write(icq,cfg,"Started");
       break;
#endif	   

    case ICQEVENT_OFFLINE:
       icqosd_Write(icq,cfg,"Disconnected");
       break;

    case ICQEVENT_CONTACT:
       icqosd_Write(icq,cfg,"Connecting");
       break;

    }

 }

 static void userEvent(HICQ icq, ICQOSDCONFIG *cfg, HUSER usr, USHORT eventCode)
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
       icqosd_Write(icq,cfg,text);
    }
    else if((cfg->flags & 2) && eventCode == ICQEVENT_OFFLINE)
    {
       sprintf(text,"%s offline",icqQueryUserNick(usr));
       icqosd_Write(icq,cfg,text);
    }
    else if((cfg->flags & 8) && eventCode == ICQEVENT_PEERBEGIN)
    {
       sprintf(text,"Connected with %s",icqQueryUserNick(usr));
       icqosd_Write(icq,cfg,text);
    }
    else if((cfg->flags & 8) && eventCode == ICQEVENT_PEEREND)
    {
       sprintf(text,"Disconnected from %s",icqQueryUserNick(usr));
       icqosd_Write(icq,cfg,text);
    }
 }

 
