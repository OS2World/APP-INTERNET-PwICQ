/*
 * pwICQPLG.c - I-Share pwICQ's plugin
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

 #include <stdio.h>
 #include <stdlib.h>

 #include "ishared.h"

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/

 void _System isicq_mainThread(ICQTHREAD *);

 static int _System icqMessage(HICQ, ULONG, USHORT, ULONG);

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/

 HMODULE module	= 0;

/*---[ Implementation ]----------------------------------------------------------------------------------*/


 int ICQAPI isicq_Configure(HICQ icq, HMODULE mod)
 {
#ifdef DEBUG
    icqWriteSysLog(icq,PROJECT,"Loading pwICQ I-Share plugin Build " BUILD " (Debug Version)");
#else
    icqWriteSysLog(icq,PROJECT,"Loading pwICQ I-Share plugin Build " BUILD);
#endif

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"**** Extensive logging enabled ****");
#endif
    DBGTracex(mod);

    module = mod;

    return sizeof(ISHARED_CONFIG);
 }

 int ICQAPI isicq_Terminate(HICQ icq, ISHARED_CONFIG *cfg)
 {
    icqWriteSysLog(icq,PROJECT,"I-Share plugin stopping");

    DBGTracex(cfg);
    if(!cfg || cfg->sz != sizeof(ISHARED_CONFIG))
    {
       icqWriteSysLog(icq,PROJECT,"Core failure: Invalid data block received in terminate");
       return -1;
    }
    cfg->active = FALSE;

    return 0;
 }

 int _System icqMessage(HICQ icq, ULONG uin, USHORT type, ULONG id)
 {
    CHKPoint();
    icqNewMessageWithManager(icq, uin, &pwICQMessageManager);
    CHKPoint();
    return 0;
 }

 int ICQAPI isicq_Start(HICQ icq, HPLUGIN p, ISHARED_CONFIG *cfg)
 {
#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"Starting I-Share module");
#endif
	
    if(!cfg || cfg->sz != sizeof(ISHARED_CONFIG))
    {
       icqWriteSysLog(icq,PROJECT,"Core failure: Invalid data block received in startup");
       return -1;
    }

    cfg->icq        = icq;
    cfg->myIP       = icqGetHostID();
    cfg->mainThread = icqCreateThread(icq, isicq_mainThread, 16384,  0, cfg, "ISm");

    icqInsertMenuOption(icq, ICQMNU_SYSTEM, "Broadcast", ICQICON_UNREADMESSAGE, icqMessage, ICQICON_UNREADMESSAGE);

    return 0;
 }

 int ICQAPI isicq_Stop(HICQ icq, HPLUGIN p, ISHARED_CONFIG *cfg)
 {
    DBGTracex(cfg);
	
    if(!cfg || cfg->sz != sizeof(ISHARED_CONFIG))
    {
       icqWriteSysLog(icq,PROJECT,"Core failure: Invalid data block received when stopping");
       icqAbend(icq,0);
       return -1;
    }

    icqSaveValue(cfg->icq,"ISH:sid",cfg->searchID);	
    icqWriteSysLog(icq,PROJECT,"I-Share plugin stopping");
    cfg->active = FALSE;
	
    return 0;
 }


 int ICQAPI isicq_Timer(HICQ icq, ISHARED_CONFIG *cfg)
 {
    if(!cfg || cfg->sz != sizeof(ISHARED_CONFIG))
       icqWriteSysLog(icq,PROJECT,"Core failure: Invalid data block received in timer call");
    else if(cfg->icq)
       ishare_timer(cfg);
    return 0;
 }

 void _System isicq_mainThread(ICQTHREAD *thd)
 {
    DBGTracex(thd->parm);
    ishare_main((ISHARED_CONFIG *) thd->parm);
    ((ISHARED_CONFIG *) thd->parm)->mainThread = 0;
 }


