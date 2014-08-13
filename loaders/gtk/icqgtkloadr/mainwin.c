/*
 * MainWin.c - Janela principal
 */

 #include <malloc.h>
 #include <stdio.h>
 #include <string.h>

 #include <icqgtk.h>

/*---[ Prototipos ]------------------------------------------------------------------------*/

 static void 		  destroy( GtkWidget *, MAINWIN *);
 static void          size(GtkWidget *widget, GtkAllocation *, MAINWIN *);
 static const char 	*loadIcons(MAINWIN *wnd);

 static void 		  setFilter( GtkWidget *, MAINWIN *);
 static void 		  selected (GtkCList *, gint, gint,  GdkEventButton *, MAINWIN *);

 static gint 		  popupmenu(GtkWidget *, GdkEvent *); 
 static gint 		  usermenu(GtkWidget *, GdkEvent *); 

 static gint           wndMainEvent(GtkWidget *, GdkEvent *); 

 static gint           listBoxSort(GtkCList *, gconstpointer, gconstpointer);

 static void           mkSysMenu(MAINWIN *);
 static void           mkModeMenu(MAINWIN *);
 static void           mkUserMenu(MAINWIN *);

 static void 		  openMsgBox(MAINWIN *);

/*---[ Implementacao ]---------------------------------------------------------------------*/

// MAINWIN * createMainWindow(HICQ icq, int argc, char **argv)
 int icqgtk_createMainWindow(HICQ icq, MAINWIN *wnd)
 {
    const char  *msg;
    GtkWidget   *temp;
    char		buffer[0x0100];
    int		 f;

    /* Janela principal */
    wnd->main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    sprintf(buffer,"ICQ#%lu",icqQueryUIN(icq));
    gtk_window_set_wmclass(GTK_WINDOW(wnd->main),"main", buffer);

    gtk_object_set_user_data(GTK_OBJECT(wnd->main),(gpointer) wnd);

    gtk_signal_connect(GTK_OBJECT(wnd->main), "destroy", GTK_SIGNAL_FUNC (destroy), wnd);
    gtk_signal_connect(GTK_OBJECT(wnd->main), "size_allocate", GTK_SIGNAL_FUNC(size), wnd);

    msg = loadIcons(wnd);
    if(msg)
    {
       icqWriteSysLog(icq,PROJECT,"Error loading images");
       return -1;
    }

    /* Linha do topo */
    wnd->topButton = gtk_hbox_new(TRUE,0);

    wnd->all = gtk_toggle_button_new();
    temp     = gtk_hbox_new(FALSE,5);
    gtk_box_pack_start(GTK_BOX(temp), gtk_label_new(""),  TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(temp), gtk_pixmap_new(wnd->iPixmap[15],wnd->iBitmap[15]),  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(temp), gtk_label_new("All"),  FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(temp), gtk_label_new(""),  TRUE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(wnd->all),temp);

    wnd->online = gtk_toggle_button_new();
    temp        = gtk_hbox_new(FALSE,5);
    gtk_box_pack_start(GTK_BOX(temp), gtk_label_new(""),  TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(temp), gtk_pixmap_new(wnd->iPixmap[43],wnd->iBitmap[43]),  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(temp), gtk_label_new("Online"),  FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(temp), gtk_label_new(""),  TRUE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(wnd->online),temp);

    gtk_box_pack_start(GTK_BOX(wnd->topButton), wnd->all, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(wnd->topButton), wnd->online,  TRUE, TRUE, 0);

    /* Parte de baixo da janela */
    wnd->baseButton = gtk_hbox_new(FALSE,0);

    wnd->system     = gtk_button_new_with_label("pwICQ");

    wnd->modePix    = gtk_pixmap_new(wnd->iPixmap[7],wnd->iBitmap[7]);
    wnd->modeText   = gtk_label_new("Offline");
    wnd->mode       = gtk_button_new();
    temp            = gtk_hbox_new(FALSE,5);
    gtk_box_pack_start(GTK_BOX(temp), gtk_label_new(""),  TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(temp), wnd->modePix,  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(temp), wnd->modeText,  FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(temp), gtk_label_new(""),  TRUE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(wnd->mode),temp);

    gtk_box_pack_start(GTK_BOX(wnd->baseButton), wnd->system,  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(wnd->baseButton), wnd->mode,    TRUE, TRUE, 0);
    

    wnd->search  = gtk_button_new();
    temp         = gtk_hbox_new(FALSE,5);
    gtk_box_pack_start(GTK_BOX(temp), gtk_label_new(""),  TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(temp), gtk_pixmap_new(wnd->iPixmap[11],wnd->iBitmap[11]),  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(temp), gtk_label_new("Search/Add"),  FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(temp), gtk_label_new(""),  TRUE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(wnd->search),temp);

    wnd->eventIcon  = wnd->readIcon = 10;
    wnd->sysMsgPix  = gtk_pixmap_new(wnd->iPixmap[10],wnd->iBitmap[10]);
    wnd->sysMsg     = gtk_button_new();
    temp            = gtk_hbox_new(FALSE,5);
    gtk_box_pack_start(GTK_BOX(temp), gtk_label_new(""),  TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(temp), wnd->sysMsgPix,  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(temp), gtk_label_new("System"),  FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(temp), gtk_label_new(""),  TRUE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(wnd->sysMsg),temp);

    wnd->box            = gtk_vbox_new(FALSE,0);

    gtk_box_pack_start(GTK_BOX(wnd->box), wnd->topButton,   FALSE, FALSE, 0);

    /* Listbox */
    temp = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(temp),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);

    wnd->listBox = gtk_clist_new(2);
    gtk_clist_set_column_auto_resize(GTK_CLIST(wnd->listBox),0,0);
    gtk_clist_set_column_width(GTK_CLIST(wnd->listBox),0,wnd->szIcon);
    gtk_clist_set_row_height(GTK_CLIST(wnd->listBox),wnd->szIcon+2);

    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(temp),wnd->listBox);
    
    gtk_box_pack_start(GTK_BOX(wnd->box), temp,     TRUE,  TRUE, 0);
    
    /* Botoes da parte inferior */
    
    gtk_box_pack_start(GTK_BOX(wnd->box), wnd->search,      FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(wnd->box), wnd->sysMsg,      FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(wnd->box), wnd->baseButton,  FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(wnd->main), wnd->box);

//    gtk_window_set_default_size(GTK_WINDOW(wnd->main),150,375);

    /* Cria os menus */
    for(f=0;f<3;f++)
    {
       wnd->menu[f] = gtk_menu_new();
       gtk_object_set_user_data(GTK_OBJECT(wnd->menu[f]),(gpointer) wnd);
    }

    /*
     * Faco as conexoes da janela
     */
     
    mkSysMenu(wnd);
    mkModeMenu(wnd);
    mkUserMenu(wnd);

    DBGMessage("Configurando listbox");
    gtk_clist_set_selection_mode (GTK_CLIST(wnd->listBox),GTK_SELECTION_SINGLE);
    gtk_clist_set_compare_func(GTK_CLIST(wnd->listBox), listBoxSort);

    gtk_signal_connect(GTK_OBJECT(wnd->online),  "toggled",    GTK_SIGNAL_FUNC(setFilter), wnd);
    gtk_signal_connect(GTK_OBJECT(wnd->all),     "toggled",    GTK_SIGNAL_FUNC(setFilter), wnd);
    gtk_signal_connect(GTK_OBJECT(wnd->search),  "clicked",    GTK_SIGNAL_FUNC(icqgtk_searchButton),    wnd);
    gtk_signal_connect(GTK_OBJECT(wnd->sysMsg),  "clicked",    GTK_SIGNAL_FUNC(icqgtk_systemMessageButton), wnd);
    gtk_signal_connect(GTK_OBJECT(wnd->listBox), "select-row", GTK_SIGNAL_FUNC(selected), wnd);

    gtk_signal_connect_object(GTK_OBJECT(wnd->system),  "event", GTK_SIGNAL_FUNC(popupmenu),    GTK_OBJECT(wnd->menu[ICQMENU_SYSTEM]));
    gtk_signal_connect_object(GTK_OBJECT(wnd->mode),    "event", GTK_SIGNAL_FUNC(popupmenu),    GTK_OBJECT(wnd->menu[ICQMENU_MODES]));
    gtk_signal_connect_object(GTK_OBJECT(wnd->listBox), "event", GTK_SIGNAL_FUNC(usermenu),     GTK_OBJECT(wnd->menu[ICQMENU_USER]));
    gtk_signal_connect_object(GTK_OBJECT(wnd->main),    "event", GTK_SIGNAL_FUNC(wndMainEvent), GTK_OBJECT(wnd->main));

    return 0;
 }
 
 static void destroy(GtkWidget *hwnd, MAINWIN *wnd )
 {
    /* Janela principal está sendo fechada */
    gtk_main_quit();
 }

 static const char *loadIcons(MAINWIN *wnd)
 {
    char	   fileName[0x0100];
    GdkPixbuf  *work;
    GdkPixbuf  *icons;
    int		f, c;

    icqLoadString(wnd->icq,"image-files","",fileName,0xFF);
    
    if(!*fileName)
       icqQueryProgramPath(wnd->icq,"images/",fileName,0xFF);
    
    strncat(fileName,"icons.gif",0xFF);
    DBGMessage(fileName);
        
    icons = gdk_pixbuf_new_from_file(fileName);

    if(!icons)
       return "Unable to load icon table";

    wnd->szIcon = gdk_pixbuf_get_height(icons);

    work = gdk_pixbuf_new(	gdk_pixbuf_get_colorspace(icons),
    						  gdk_pixbuf_get_has_alpha(icons), 
    						  gdk_pixbuf_get_bits_per_sample(icons),
    						  wnd->szIcon, wnd->szIcon);
        


//    gdk_pixbuf_render_pixmap_and_mask(icons, &wnd->iconsPixmap, &wnd->iconsBitmap,1);

    for(f=c=0;f<PWICQICONBARSIZE;f++)
    {
       gdk_pixbuf_copy_area (icons,c,0,wnd->szIcon,wnd->szIcon,work,0,0);
       gdk_pixbuf_render_pixmap_and_mask(work, &wnd->iPixmap[f], &wnd->iBitmap[f],1);
       c += wnd->szIcon;
    }

//    gdk_pixbuf_render_pixmap_and_mask(work, &wnd->iconsPixmap, &wnd->iconsBitmap,1);

    gdk_pixbuf_unref(icons);
    gdk_pixbuf_unref(work);

//    DBGTracex(wnd->iconsPixmap);
//    DBGTracex(wnd->iconsBitmap);
        
    return NULL;
 }

 static void setFilter( GtkWidget *widGet, MAINWIN *wnd)
 {
    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widGet)))
       return;

    ICQLOCK(wnd);
    if(widGet == wnd->all)
    {
       DBGMessage("All");
       wnd->filter = 0;
       ICQMAKEFILTER(wnd);
       gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wnd->online),0);
    }
    else
    {
       DBGMessage("Online");
       wnd->filter = 1;
       ICQMAKEFILTER(wnd);
       gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wnd->all),0);
       
    }
    ICQUNLOCK(wnd);

    icqSaveValue(wnd->icq,"wndFilterMode",wnd->filter);    
    
 }
 
 static gint popupmenu(GtkWidget *widget, GdkEvent *event)
 {
    if (event->type == GDK_BUTTON_PRESS) 
    {
        GdkEventButton *bevent = (GdkEventButton *) event; 
        gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL,bevent->button, bevent->time);
        return TRUE;
    }
    return FALSE;
 }

 static gint wndMainEvent(GtkWidget *widget, GdkEvent *event)
 {
    if (event->type == GDK_CONFIGURE)
    {
       MAINWIN *wnd = gtk_object_get_user_data(GTK_OBJECT(widget));
       if(wnd && wnd->sz == sizeof(MAINWIN))
       {
          /* Janela foi reconfigurada, desliga animações e reordenações por um tempo */
          wnd->fTick =
          wnd->aTick = 0;
       }
    } 
    return FALSE;
 } 

 static void selected(GtkCList *clist, gint row, gint column,  GdkEventButton *event, MAINWIN *wnd)
 {
    HUSER usr = gtk_clist_get_row_data(clist,row);
    if(usr)
       wnd->selected = usr->uin;
    else
       wnd->selected = 0;
 }
 
 static gint listBoxSort(GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
 {
    HUSER h1	= ((GtkCListRow *) ptr1)->data;
    HUSER h2	= ((GtkCListRow *) ptr2)->data;

    if(h1->index != h2->index)
       return h1->index - h2->index;
       
    return stricmp(icqQueryUserNick(h1),icqQueryUserNick(h2));
 }

 static void mkSysMenu(MAINWIN *wnd)
 {
    /* Constroi menu de sistema */
    icqgtk_addMenuOption(wnd, ICQMENU_SYSTEM, 27, icqgtk_sysmenu, "Configure");
    icqgtk_addMenuOption(wnd, ICQMENU_SYSTEM, 38, icqgtk_sysmenu, "Disconnect");
    icqgtk_addMenuOption(wnd, ICQMENU_SYSTEM, 16, icqgtk_sysmenu, "Update all users");
    icqgtk_addMenuOption(wnd, ICQMENU_SYSTEM, 17, icqgtk_sysmenu, "Shutdown");
 }

 static void mkUserMenu(MAINWIN *wnd)
 {
    icqgtk_addMenuOption(wnd, ICQMENU_USER,  0, icqgtk_usermenu, "Send Message");
    icqgtk_addMenuOption(wnd, ICQMENU_USER,  2, icqgtk_usermenu, "Send URL");
    icqgtk_addMenuOption(wnd, ICQMENU_USER,  8, icqgtk_usermenu, "Ask for authorization");
    icqgtk_addMenuOption(wnd, ICQMENU_USER, 29, icqgtk_usermenu, "Send authorization");
    icqgtk_addMenuOption(wnd, ICQMENU_USER, 16, icqgtk_usermenu, "Update basic info");
    icqgtk_addMenuOption(wnd, ICQMENU_USER, 17, icqgtk_usermenu, "Remove from list");
 }
 
 static void mkModeMenu(MAINWIN *wnd)
 {
    const ICQMODETABLE *mode;
    for(mode = icqQueryModeTable(wnd->icq); mode && mode->descr; mode++)
       icqgtk_addMenuOption(wnd, ICQMENU_MODES, mode->icon, icqgtk_modemenu, mode->descr);
 }

 static gint usermenu(GtkWidget *widget, GdkEvent *event)
 {
    switch(event->type)
    {
    case GDK_BUTTON_PRESS:
        if(((GdkEventButton *) event)->button != 3)
           return FALSE;
        gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL,((GdkEventButton *) event)->button, ((GdkEventButton *) event)->time);
        break;
       
    case GDK_2BUTTON_PRESS:
        if(((GdkEventButton *) event)->button != 1)
           return FALSE;
        openMsgBox( ((MAINWIN *) gtk_object_get_user_data(GTK_OBJECT(widget))) );
        break;
        
    default: 
       return FALSE;
       
    }
    return TRUE;
 }

 static void openMsgBox(MAINWIN *wnd)
 {        
    if(wnd->selected)
       icqOpenUserMessageWindow(wnd->icq,icqQueryUserHandle(wnd->icq,wnd->selected));
 }
        
 static void size(GtkWidget *widget, GtkAllocation *all, MAINWIN *wnd)
 {
    wnd->flags |= ICQGTK_SAVE;
 }



