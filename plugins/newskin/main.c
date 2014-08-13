/*
 * MAIN.C - pwICQ's Skin manager entry points
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <pwicqgui.h>

/*---[ Macro functions]-----------------------------------------------------------------------------------*/

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 int EXPENTRY pwICQ_Configure(HICQ icq, HMODULE hmod)
 {
	char buffer[0x0100];
	CHKPoint();

#ifdef EXTENDED_LOG
	icqPrintLog(icq,PROJECT,"Loading %s Gui Manager Build " BUILD " (hMod=%08lx)", 
								icqgui_descr.id,
								(ULONG) hmod );
#endif
	
    module = hmod;

    CHKPoint();

    if(icqQueryUserDataBlockLength() != sizeof(ICQUSER))
    {
       sprintf(buffer,"Can't load %s GUI due to invalid pwICQ core version", icqgui_descr.id);
	   icqWriteSysLog(icq,PROJECT,buffer);
       DBGMessage(buffer);
       return -1;
    }

    if(!icqRegisterSkinManager(icq,&icqgui_descr))
    {
       sprintf(buffer,"Can't register %s GUI manager",icqgui_descr.id);
	   icqWriteSysLog(icq,PROJECT,buffer);
       DBGMessage(buffer);
       return -2;
    }

    return sizeof(SKINDATA);

 }

 void icqskin_Initialize(HICQ icq, SKINDATA *cfg)
 {
    char       buffer[0x0100];

    DBGTracex(cfg);	
    icqSetSkinDataBlock(icq,cfg);

    sprintf(buffer,"Initializing %s GUI Build " BUILD, icqgui_descr.id);
    icqWriteSysLog(icq,PROJECT,buffer);
    DBGMessage(buffer);
    	
    memset(cfg,0,sizeof(SKINDATA));
    cfg->sz  = sizeof(SKINDATA);
    cfg->icq = icq;

#ifdef __OS2__
    cfg->iconImage = NO_IMAGE;
    cfg->iconMasc  = NO_IMAGE;
#endif

    DBGTracex(icqQueryBuild());
    DBGTracex(XBUILD);

    if(icqQueryBuild() > XBUILD)
       sprintf(buffer,"WARNING: %s Skin Manager is older than core", icqgui_descr.id);
    else
       sprintf(buffer,"Starting %s Skin Manager", icqgui_descr.id);

	icqWriteSysLog(icq,PROJECT,buffer);
	DBGMessage(buffer);

#if defined(STANDARD_GUI) && defined(SKINNED_GUI)
	if(icqLoadValue(icq,"withskin",1))
       icqskin_CreateSkinnedWindow(icq, cfg);
	else
       icqskin_CreateStandardWindow(icq, cfg);
#else	
   #ifdef STANDARD_GUI
      icqskin_CreateStandardWindow(icq, cfg);
   #else
      icqskin_CreateSkinnedWindow(icq, cfg);
   #endif
#endif	

	icqskin_setFrameIcon(cfg->frame,cfg,7);
	sprintf(buffer,"%ld",icqQueryUIN(icq));
	icqskin_setFrameTitle(cfg->frame,buffer);

    icqSetCaps(icq,ICQMF_EXTENDEDMENU);

 }

 SKINDATA * _System icqskin_Start(HICQ icq, ULONG parm)
 {
    SKINDATA *cfg = (SKINDATA *) parm;
	
	if(cfg->sz != sizeof(SKINDATA))
	{
	   icqWriteSysLog(icq,PROJECT,"Unexpected GUI manager data block");
	   return 0;
	}

    return cfg;	
 }

 int _System icqskin_Stop(HICQ icq, SKINDATA *cfg)
 {
	char     buffer[0x0100];
	
    sprintf(buffer,"Stopping %s GUI", icqgui_descr.id);
	icqWriteSysLog(icq,PROJECT,buffer);
	DBGMessage(buffer);

    icqskin_closeSkinFile(cfg);
    icqSetSkinDataBlock(icq,NULL);

    return 0;
 }

