/*
 * gtkmenu.c - GTK implementation of pwICQ menus
 */

 #include <pwicqgui.h>
 #include <string.h>
 #include <malloc.h>

/*---[ Structures ]-----------------------------------------------------------*/

 #pragma pack(1)
 typedef struct _option
 {
	HICQ         icq;
    hWindow		 hwnd;
    USHORT 		 id;
    MENUCALLBACK *cbk;
	ULONG        cbkParm;
 } OPTION;

 typedef struct _icqmenu
 {
    /* Standard GTK Header */
    ICQGUI_STANDARD_PREFIX;
	
	/* Menu information */
    USHORT 	   id;
	ULONG      cbkParm;
	
 } ICQMENU;

/*---[ Constants ]------------------------------------------------------------*/

// static const char *menuClass = "pwICQMENU";
 
/*---[ Prototipes ]-----------------------------------------------------------*/

 static void menuAction(const OPTION *);

/*---[ Implementation ]-------------------------------------------------------*/

 static void destroy( GtkWidget *widget, ICQMENU *cfg)
 {
    if(cfg->sz != sizeof(ICQMENU))
       return;
    DBGPrint("Menu \"%s\" destroyed",cfg->name);
    memset(cfg,0,sizeof(ICQMENU));
    free(cfg);
 }

 hWindow icqskin_CreateMenu(HICQ icq, USHORT id, SKINDATA *skn)
 {
    ICQMENU *cfg = malloc(sizeof(ICQMENU));
	hWindow ret;
	
	if(!cfg)
	   return 0;
	
	memset(cfg,0,sizeof(ICQMENU));
	cfg->sz        = sizeof(ICQMENU);
	cfg->icq       = icq;
	cfg->id        = id;
	
    ret = gtk_menu_new();
    gtk_object_set_user_data(GTK_OBJECT(ret), (gpointer) cfg);
    gtk_signal_connect(GTK_OBJECT(ret), "destroy", GTK_SIGNAL_FUNC(destroy), cfg);

    icqskin_setFont(ret, FONT_NORMAL);
	
	return ret;
 }

 void icqskin_showMenu(hWindow hwnd, ULONG cbkParm, USHORT x, USHORT y)
 {
	ICQMENU *parm = gtk_object_get_user_data(GTK_OBJECT(hwnd));
	
	if(!parm)
	   return;
	
	parm->cbkParm = cbkParm;
	
	gtk_menu_popup(GTK_MENU(hwnd),NULL,NULL,NULL,NULL,0,0);
	
 }

 void icqskin_setMenuTitle(HICQ icq, USHORT id, const char *title)
 {
	SKINDATA *skn = icqskin_getDataBlock(icq);
	DBGTrace(id);
	if(id >= ICQMNU_COUNTER || !skn->menu[id])
	   return;
    gtk_menu_set_title(GTK_MENU(skn->menu[id]),title);
 }

 static void destroyOption( GtkWidget *widget, OPTION *cfg)
 {
    memset(cfg,0,sizeof(OPTION));
    free(cfg);
 }

 int icqskin_insertMenuOption(HWND hwnd, const char *text, USHORT id, MENUCALLBACK *cbk, ULONG cbkParm)
 {
    GtkWidget   *box;
    GtkWidget   *item;
    ICQMENU     *cfg   = icqskin_getWindowDataBlock(hwnd);
	SKINDATA    *skn   = icqskin_getDataBlock(cfg->icq);
	OPTION      *opt   = malloc(sizeof(OPTION));
	
	if(!opt)
	   return -1;
	
	memset(opt,0,sizeof(OPTION));
	
	opt->icq     = cfg->icq;
	opt->hwnd    = hwnd;
	opt->id      = id;
	opt->cbk     = cbk;
	opt->cbkParm = cbkParm;
	
    item         = gtk_menu_item_new();
    gtk_object_set_user_data(GTK_OBJECT(item), (gpointer) opt);

	if(id < PWICQICONBARSIZE)
	{
       box = gtk_hbox_new(FALSE,5);
	   gtk_box_pack_start(GTK_BOX(box), gtk_pixmap_new(skn->iPixmap[id],skn->iBitmap[id]), FALSE, FALSE, 0);
       gtk_box_pack_start(GTK_BOX(box), gtk_label_new(text),  FALSE, FALSE, 0);
       gtk_container_add(GTK_CONTAINER(item),box);
	}
	else
	{
       gtk_container_add(GTK_CONTAINER(item),gtk_label_new(text));
	}

    gtk_menu_append(GTK_MENU(hwnd),item);

    CHKPoint();	
    gtk_widget_show_all(item);
	
    gtk_signal_connect_object(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(menuAction), (gpointer) opt);
    gtk_signal_connect(GTK_OBJECT(item), "destroy", GTK_SIGNAL_FUNC(destroyOption), (gpointer) opt);
	
    return 0; 	
 }

 static void menuAction(const OPTION *opt)
 {
    ICQMENU  *cfg = icqskin_getWindowDataBlock(opt->hwnd);
	SKINDATA *skn = icqskin_getDataBlock(cfg->icq);

    if(skn->userPopup)
       icqskin_closeWindow(skn->userPopup);
	
    if(opt->cbk && cfg)
       opt->cbk(opt->icq, cfg->cbkParm, opt->id, (ULONG) opt->cbkParm);
 }

