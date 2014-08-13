/*
 * icqgtk.c - Ponto de entrada
 */

 #include <stdio.h>
 #include <malloc.h>
 #include <string.h>
 #include <stdlib.h>
 #include <unistd.h>

 #include <icqgtk.h>

 #include <gnome.h>

/*---[ Estruturas ]--------------------------------------------------------------------------*/

 #pragma pack(1)
 
 struct menu_option
 {
    MAINWIN	     *wnd;
    USHORT		  menu;
    USHORT 		 id;
    MENUCALLBACK	*cbk;
 };

/*---[ Constantes ]--------------------------------------------------------------------------*/

 static ULONG _System loadSkin(HICQ, void (* _System)(int), int, char **);
 static int   _System executeSkin(HICQ,ULONG);
 static int   _System event(HICQ,ULONG,char,USHORT,ULONG,void *);
 static int   _System insertMenu(HICQ, USHORT, const char *, short, ULONG, MENUCALLBACK *);
 static void  _System view(HICQ, ULONG, HMSG);
 static int   _System validateEdit(USHORT);
 static void  _System newMessage(HICQ,ULONG,USHORT);

//      int      (* _System insertListener)(HICQ, void (_System *)(HICQ,ULONG,char,USHORT,ULONG,HWND),HWND);
//      int      (* _System removeListener)(HICQ, void (_System *)(HICQ,ULONG,char,USHORT,ULONG,HWND),HWND);
// int _System icqgtk_insertListener(HICQ icq, void (_System *callBack)(HICQ,ULONG,char,USHORT,ULONG,void *),void *dataBlock)
// int _System icqgtk_removeListener(HICQ icq, void (_System *callBack)(HICQ,ULONG,char,USHORT,ULONG,void *),void *dataBlock)
 
 static SKINMGR caps = {       sizeof(SKINMGR),
                               loadSkin,
                               executeSkin,
                               (int (*)(HICQ,void *,char)) icqgtk_guiTimer,
                               NULL,			// warning
                               view,			// view
                               newMessage,	  // newMessage
                               NULL,			// awayMessage
                               insertMenu,
                               icqgtk_OpenConfigWindow,
                               validateEdit,
                               event,
                               icqgtk_insertListener,
                               icqgtk_removeListener

                       };

/*---[ Prototipos ]--------------------------------------------------------------------------*/

 static void menuAction(const struct menu_option *);
 static void addEvent(MAINWIN *, ULONG, char, USHORT, ULONG);

/*---[ Implementacao ]-----------------------------------------------------------------------*/

 int EXPENTRY icqgtk_Configure(HICQ icq, HMODULE mod)
 {
    icqWriteSysLog(icq,PROJECT,
#ifdef __DLL__
    "Loading"    
#else
    "Initializing"
#endif    
    " GTK Interface module Build " __DATE__ " " __TIME__ 
#ifdef DEBUG
    " (Debug Version)"
#endif
    );
    
#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"**** Extensive logging enabled ****");
#endif

    DBGTracex(icq);
    DBGTracex(&caps);

    if(!icqRegisterSkinManager(icq,&caps))
    {
       CHKPoint();
       icqWriteSysLog(icq,PROJECT,"Can't register GTK Interface plugin");
       return -1;
    }

    CHKPoint();

    return 0;
 }

 static ULONG _System loadSkin(HICQ icq, void (* _System status)(int), int argc, char **argv)
 {
    MAINWIN *wnd = malloc(sizeof(MAINWIN));

    if(!wnd)
       return 0;

    gtk_set_locale();
    gtk_init(&argc, &argv);
    g_thread_init(NULL);

    /* Inicializa campos basicos */    
    memset(wnd,0,sizeof(MAINWIN));
    wnd->sz        = sizeof(MAINWIN);
    wnd->icq       = icq;
    wnd->fTick     = 
    wnd->aTick     = 0;

    wnd->sTick     =
    wnd->iTick     =
    wnd->aPos      = 0xFFFF;
    wnd->events    = icqCreateList();
    wnd->listeners = icqCreateList();

    icqSetSkinDataBlock(icq,wnd);

    return (ULONG) wnd;
 }

 static int  _System executeSkin(HICQ icq, ULONG cfg)
 {
    MAINWIN *wnd 	= (MAINWIN *) cfg;

    DBGTracex(cfg);

    if(wnd != icqGetSkinDataBlock(icq))
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected Skin Data Block supplied");
       return -1;
    }

    /* Apresenta a janela e processa */

    if(icqQueryLogonStatus(icq))
       icqEvent(icq,0,'G',0,0);
    else
       icqgtk_login(icq);

    gtk_main();

    wnd->flags &= ~ICQGTK_FLAG_READY;
    usleep(1000);
    icqSetSkinDataBlock(icq,NULL);
    usleep(1000);
    
    icqSysLog(icq,"Stopping");

    /* Finalizacao */
    DBGMessage("Liberando area de trabalho");
    icqDestroyList(wnd->events);
    icqDestroyList(wnd->listeners);
    free(wnd);

    return 0;
 }

 static int _System insertMenu(HICQ icq, USHORT menu, const char *text, short pos, ULONG icon, MENUCALLBACK *cbk)
 {
    MAINWIN *wnd =  icqGetSkinDataBlock(icq);
    icqgtk_addMenuOption(wnd,menu,icon,cbk,text);
    return 0;
 }

 void icqgtk_addMenuOption(MAINWIN *wnd, USHORT id, USHORT icon, MENUCALLBACK *cbk, const char *text)
 {
    GtkWidget 			*box   = gtk_hbox_new(FALSE,5);
    GtkWidget 			*item  = gtk_menu_item_new();
    struct menu_option	parm   = { wnd, id, icon, cbk };

    gtk_box_pack_start(GTK_BOX(box), gtk_pixmap_new(wnd->iPixmap[icon],wnd->iBitmap[icon]),  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), gtk_label_new(text),  FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(item),box);
    gtk_menu_append(GTK_MENU(wnd->menu[id]),item);
    gtk_widget_show_all(item);

    gtk_signal_connect_object(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(menuAction), g_memdup(&parm, sizeof(parm)));

 }
 
 static void addEvent(MAINWIN *wnd, ULONG uin, char type, USHORT event, ULONG parm)
 {
    EVENT *e;
    
    e = icqAddElement( wnd->events, sizeof(EVENT));
    
    if(!e)
    {
       icqWriteSysLog(wnd->icq,PROJECT,"Error adding event in table");
       icqAbend(wnd->icq,0);
       return;
    }

    e->uin   = uin;
    e->event = event;
    e->type  = type;    
    e->parm  = parm;

 }

 static int _System event(HICQ icq, ULONG uin, char type, USHORT event, ULONG parm, void *dataBlock)
 {
    if(!(dataBlock && type))
       return 0;

    if(type == 'G')
    {
       icqgtkEvent_Gui(icq, (MAINWIN *) dataBlock, uin, event);
       return 0;
    }
  
    addEvent((MAINWIN *) dataBlock, uin, type, event, parm);    
    
    return 0;      
 }

 static void _System view(HICQ icq, ULONG uin, HMSG msg)
 {
    if(msg)
       addEvent((MAINWIN *) icqGetSkinDataBlock(icq),uin,'G', 1, (ULONG) msg);
    else
       icqWriteSysLog(icq,PROJECT,"Unexpected view message request");

/* 
    if(msg)
       icqgtk_openMessageWindow(icq,uin,msg->type,FALSE,msg);
    else
       icqWriteSysLog(icq,PROJECT,"Unexpected view message request");
*/       
 }
 
 static void  _System newMessage(HICQ icq, ULONG uin, USHORT type)
 {
    addEvent((MAINWIN *) icqGetSkinDataBlock(icq),uin,'G', 2, (ULONG) type);
/* 
    icqgtk_openMessageWindow(icq, uin, type, TRUE, NULL);
*/       
 }

 static int   _System validateEdit(USHORT sz)
 {
    if(sz != sizeof(MSGEDITHELPER))
       return 2;
    return 0;
 }
 
 static void menuAction(const struct menu_option *opt)
 {
    if(opt->cbk)
       opt->cbk(opt->wnd->icq, opt->wnd->selected, opt->menu, (ULONG) opt->id);    
 }

#ifdef DEBUG_LOG
 void EXPENTRY icqgtk_lock(MAINWIN *wnd, const char *pgm, int lin)
 {
    char debugLog[0x0100];
#else
 void EXPENTRY icqgtk_lock(MAINWIN *wnd)
 {
#endif 
    int f = 0;
    
#ifdef EXTENDED_LOG
    if(getpid() == wnd->lockOwner)
       icqWriteSysLog(wnd->icq,PROJECT,"Lock request from the same PID");
#endif
    
    while(wnd->flags & ICQGTK_LOCK)
    {
       while (gtk_events_pending())
	      gtk_main_iteration();

       if(f++ > 100)
       {
#ifdef DEBUG_LOG
          sprintf(debugLog,"Semaphore locked by %s(%d), requested by %s(%d). Timeout!",
          						wnd->lockSource,wnd->lockLine,
          						pgm,lin );
          icqWriteSysLog(wnd->icq,PROJECT,debugLog);          						
#else
          icqWriteSysLog(wnd->icq,PROJECT,"Timeout requesting semaphore");
#endif          
          f = 0;
       }
       usleep(100);
    }
    
    wnd->lockOwner  = getpid();

#ifdef DEBUG_LOG
    strncpy(wnd->lockSource,pgm,19);
    wnd->lockLine = lin;
#endif

    wnd->flags     |= ICQGTK_LOCK;
    
 }
 
#ifdef DEBUG_LOG
 void EXPENTRY icqgtk_unlock(MAINWIN *wnd, const char *pgm, int lin)
#else 
 void EXPENTRY icqgtk_unlock(MAINWIN *wnd)
#endif 
 {
    wnd->flags     &= ~ICQGTK_LOCK; 
    wnd->lockOwner  = 0;
 }
  
 void EXPENTRY icqgtk_StoreWindow(GtkWidget *hwnd, HICQ icq, ULONG uin, const char *key)
 {
    gint x,y,width,height;
    char buffer[80];
    
    if( !gdk_window_get_deskrelative_origin(hwnd->window,&x,&y))
       return;

    gdk_window_get_size(hwnd->window,&width,&height);
    
    sprintf(buffer,"%d,%d,%d,%d",x,y,width,height);

    icqSaveString(icq,key,buffer);
 }

 void EXPENTRY icqgtk_RestoreWindow(GtkWidget *hwnd, HICQ icq, ULONG uin, const char *key, short w, short h)
 {
    int x,y,width,height;
    char buffer[80];

    icqLoadString(icq,key,"",buffer,79);
    
    if(!*buffer)    
    {
       gtk_window_set_default_size(GTK_WINDOW(hwnd),w,h);
       return;
    }
    
    DBGMessage(buffer);
    if(sscanf(buffer,"%d,%d,%d,%d",&x,&y,&width,&height) != 4)
    {
       gtk_window_set_default_size(GTK_WINDOW(hwnd),w,h);
       return;
    }

//    gtk_widget_set_uposition(hwnd,x,y);
    gtk_window_set_default_size(GTK_WINDOW(hwnd),width,height);
 }

