/*
 * icqgtk.c - Loader for the pwICQ GTK GUI
 */

 #include <pwicqgui.h>
 #include <sys/types.h>
 #include <unistd.h>

/*---[ Structures ]-----------------------------------------------------------*/

 #pragma pack(1)
 struct icqevent
 {
	ULONG  uin;
	char   id;
	USHORT event;
	ULONG  parm;
 };

/*---[ Prototipes ]-----------------------------------------------------------*/

 static gboolean idleProcessor(SKINDATA *);

/*---[ Skin Manager entry points ]--------------------------------------------*/

 static ULONG _System loadSkin(HICQ, STATUSCALLBACK *, int, char **);
 static int   _System executeSkin(HICQ,ULONG);
 static int   _System event(HICQ, ULONG, char, USHORT, ULONG, SKINDATA *);
 static int   _System timer(HICQ, SKINDATA *, char);
 static int   _System validateEdit(USHORT);
 static int   _System popupUserList(HICQ);

/*---[ Skin Manager definition table ]----------------------------------------*/

 const SKINMGR icqgui_descr =
 {
    sizeof(SKINMGR),
    0,                  // flags
	PLUGIN_ID,
    loadSkin,
    executeSkin,
    (int (_System *)(HICQ,void *,char)) timer,               // timer,
    NULL,                     // warning
    icqskin_viewMessage,      // view
    icqskin_newMessage,       // newMessage
    icqskin_newWithMgr,       // msgWithMgr
	NULL,                     // msgMultDest
    NULL,                     // awayMessage
    icqskin_popupMenu,        // popupmenu
    popupUserList,            // popupUserList
    icqskin_insertMenu,       // insertMenu,
    (HWND (*)(HICQ,ULONG,USHORT)) icqskin_openConfig,       // openConfig,
	validateEdit,             // validateEdit,
    (int (_System *)(HICQ,ULONG,char,USHORT,ULONG,void *)) event       // event,
 };

/*---[ Statics ]--------------------------------------------------------------*/

#ifdef EXTENDED_LOG   
 static pid_t  eventPid            = 0;
#endif
 
/*---[ Implementation ]-------------------------------------------------------*/

 static ULONG _System loadSkin(HICQ icq, STATUSCALLBACK *status, int argc, char **argv)
 {
    SKINDATA *cfg = icqGetPluginDataBlock(icq,module);
	
	if(!cfg || cfg->sz != sizeof(SKINDATA))
	{
	   DBGTracex(icqGetSkinDataBlock(icq));
	   DBGTracex(cfg);
	   DBGTrace(cfg->sz);
	   DBGTrace(sizeof(SKINDATA));
	   icqWriteSysLog(icq,PROJECT,"Invalid or non existent GUI Data Block");
	   return 0;
	}

    g_thread_init(NULL);

#ifdef GTK2	
    gdk_threads_init();
#endif	

    gtk_set_locale();
	
    gtk_init(&argc, &argv);
	
	icqskin_Initialize(icq,cfg);
	
    if(icqInitialize(icq,status))
	   return 0;

	return (ULONG) cfg;
 }

 static void logStartup(HICQ icq)
 {
	icqPrintLog(icq,PROJECT,"Skin manager %s running (pid=%d)",icqgui_descr.id,getpid());
 }
 
 static void addIdleHook(HICQ icq, SKINDATA *cfg)
 {
	int id;
	
    id = g_timeout_add_full(	G_PRIORITY_LOW,
								100,
                            	(gboolean (*)(gpointer)) idleProcessor,
                            	(gpointer) cfg,
                                NULL);
	
	icqPrintLog(icq,PROJECT,"Timeout processor %d added",id);
	
 }
 
 static int  _System executeSkin(HICQ icq, ULONG parm)
 {
   SKINDATA *cfg = icqskin_Start(icq, parm);
	
   if(!cfg)
      return -1;	

   cfg->pendingEvents = icqCreateList();

   logStartup(icq);

#ifdef EXTENDED_LOG   
   icqPrintLog(icq,PROJECT,"GTK Main loop running in thread %d",getpid());
#endif

#ifdef DEBUG   
   icqskin_loadString(icq, module, 100, NULL, 0);
#endif   

   addIdleHook(icq,cfg);
   
#ifdef GTK2	
   gdk_threads_enter();
#endif

   gtk_main();

#ifdef GTK2	
   gdk_threads_leave();
#endif

   usleep(1000);

   icqDestroyList(cfg->pendingEvents);

   return icqskin_Stop(icq,cfg);
 }

 static gboolean idleProcessor(SKINDATA *cfg)
 {
    /* 
	   http://developer.gnome.org/doc/API/glib/glib-the-main-event-loop.html
	 */
	
    struct icqevent	*e;
	
	if(!cfg->pendingEvents)
	   return TRUE;

#ifdef EXTENDED_LOG   
    if(eventPid != getpid())
       icqPrintLog(cfg->icq,PROJECT,"Event processor running on PID %d",eventPid=getpid());
#endif
	
	e = icqQueryFirstElement(cfg->pendingEvents);

	if(e)
	{
       do 
	   {
          icqskin_event(cfg->icq, e->uin, e->id, e->event, e->parm);
          icqRemoveElement(cfg->pendingEvents,e);	
  	      e = icqQueryFirstElement(cfg->pendingEvents);
	   }
       while(e && !g_main_pending());
	}

	return TRUE;
 }	

 int icqskin_postEvent(HICQ icq, ULONG uin, char id, USHORT ev, ULONG parm)
 {
    return event(icq, uin, id, ev, parm, icqskin_getDataBlock(icq));
 }

 static int _System event(HICQ icq, ULONG uin, char id, USHORT event, ULONG parm, SKINDATA *cfg)
 {
    struct icqevent	*e;
	
    if(cfg && cfg->pendingEvents)
    {
	   e = icqAddElement(cfg->pendingEvents,sizeof(struct icqevent));
	   if(e)
	   {
		  e->uin   = uin;
		  e->id    = id;
		  e->event = event;
		  e->parm  = parm;
	   }
	   else
	   {
		  icqWriteSysLog(icq,PROJECT,"Failure adding event in list");
		  icqAbend(icq,0);
	   }
	   
    }
    return 0;
 }

 static int   _System validateEdit(USHORT sz)
 {
    if(sz != sizeof(MSGEDITHELPER))
       return 2;
    return 0;
 }

 static int _System timer(HICQ icq,SKINDATA *skn,char flags)
 {
    if(skn && skn->sz == sizeof(SKINDATA))
    {
       if(skn->tick++ < 5)
          return 0;

       skn->tick = 0;

       skn->animate++;
       skn->animate &= 0x07;
	
       event(icq,0,'g',ICQEVENT_TIMER,0,skn);
    }
    return 0;
 }

 int _System popupUserList(HICQ icq)
 {
    icqskin_postEvent(icq, 0, 'g', ICQEVENT_POPUP, 0);
	return 0;
 }
