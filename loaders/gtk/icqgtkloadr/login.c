/*
 * login.c - Janela de login
 */
 
 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include <icqgtk.h>

 #include "login_bitmap.xpm"

/*---[ Constantes ]------------------------------------------------------------------------*/

 #pragma pack(1)
 struct config
 {
    HICQ 		icq;
    GtkWidget	*main;
    GtkWidget	*entry[2];
    GtkWidget	*save;
 };

/*---[ Constantes ]------------------------------------------------------------------------*/
 
 static const char *stringTable[] = {	"ICQ Number:",	  // 0
 									    "Password:",		// 1
 									    "Save password",	// 2
 									    "Cancel",   		// 3
 									    "Login" 		    // 4
 								   };
 
/*---[ Prototipos ]------------------------------------------------------------------------*/

 static void destroy( GtkWidget *, struct config *);
 static void login_ok(GtkWidget *, struct config *);
 static void login_fail(GtkWidget *, struct config *);
 
/*---[ Implementacao ]---------------------------------------------------------------------*/

 void icqgtk_login(HICQ icq)
 {
    struct config *wnd = g_malloc(sizeof(struct config));
    
    GdkPixmap         *image;
    GtkWidget         *hwnd  = gtk_window_new(GTK_WINDOW_DIALOG);
    GtkWidget         *box   = gtk_hbox_new(FALSE,0);
    GtkWidget         *table;
    GtkWidget         *temp;
    GtkWidget         *button;
    USHORT		    cols;
    GtkRequisition	size;
    char			  buffer[20];
    int	   f;

    /* Janela principal */
    gtk_object_set_user_data(GTK_OBJECT(hwnd),(gpointer) wnd);
    gtk_signal_connect(GTK_OBJECT(hwnd), "destroy", GTK_SIGNAL_FUNC (destroy), wnd);
    gtk_window_set_title(GTK_WINDOW(hwnd),"pwICQ startup");
    gtk_window_set_policy(GTK_WINDOW(hwnd), FALSE, FALSE, TRUE);

    wnd->main = hwnd;
    wnd->icq  = icq;
    gtk_widget_realize(hwnd);
    
    /* Caixa da esquerda */
    image = gdk_pixmap_create_from_xpm_d(hwnd->window,NULL,NULL,login_bitmap_xpm);

/*
    temp  = gtk_vbox_new(FALSE,0);
    style = gtk_style_copy(gtk_widget_get_default_style());  // gtk_widget_get_style(hwnd);
    gtk_widget_set_style(temp,style);
    gtk_box_pack_start(GTK_BOX(temp), gtk_pixmap_new(image,NULL), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(temp), gtk_label_new("pwICQ"), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box),temp, FALSE, FALSE,0);
*/

    gtk_box_pack_start(GTK_BOX(box), gtk_pixmap_new(image,NULL), FALSE, FALSE, 0);

    
    /* Caixa da direita */
    table  = gtk_table_new(4,2,FALSE);
    gtk_table_set_col_spacings(GTK_TABLE(table),4);
    
    cols = 0;
    
    for(f=0;f<2;f++)
    {
       temp = gtk_label_new(stringTable[f]);
       gtk_misc_set_alignment(GTK_MISC(temp), 0, .5);
       gtk_widget_size_request(temp,&size);
       
       if(size.width > cols)
          cols = size.width;

       gtk_table_attach_defaults(GTK_TABLE(table), temp, 0, 1, 0+f, 1+f);

       wnd->entry[f] = gtk_entry_new();
       gtk_entry_set_editable(GTK_ENTRY(wnd->entry[f]),TRUE);
       gtk_entry_set_max_length(GTK_ENTRY(wnd->entry[f]),f ? 8 : 10);
       gtk_table_attach(GTK_TABLE(table), wnd->entry[f], 1, 2, 0+f, 1+f,GTK_EXPAND|GTK_SHRINK|GTK_FILL,0, 2, 2);
    }
    gtk_widget_size_request(wnd->entry[0],&size);
    gtk_widget_set_usize(wnd->entry[0],cols,size.height);
    gtk_widget_set_usize(wnd->entry[1],cols,size.height);

    DBGTrace(cols);
    gtk_entry_set_visibility(GTK_ENTRY(wnd->entry[1]),FALSE);

    wnd->save = gtk_check_button_new_with_label(stringTable[2]);
    gtk_table_attach(GTK_TABLE(table), wnd->save, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 0, 2, 2);

    temp   = gtk_hbox_new(FALSE,0);
    button = gtk_button_new_with_label(stringTable[3]);
    gtk_signal_connect(GTK_OBJECT(button),  "clicked",    GTK_SIGNAL_FUNC(login_fail), wnd);
    gtk_box_pack_end(GTK_BOX(temp), button,TRUE,TRUE,0);
    
    button = gtk_button_new_with_label(stringTable[4]);
    gtk_signal_connect(GTK_OBJECT(button),  "clicked",    GTK_SIGNAL_FUNC(login_ok), wnd);
    gtk_box_pack_end(GTK_BOX(temp), button,TRUE,TRUE,0);
    
    gtk_table_attach(GTK_TABLE(table), temp, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 0, 2, 2);
    gtk_box_pack_start(GTK_BOX(box), table, TRUE, TRUE, 5);

    /* Carrega campos */
    icqLoadProfileString(icq,"MAIN","uin","",buffer,8);
    DBGMessage(buffer);
    
    if(*buffer)
       gtk_entry_set_text(GTK_ENTRY(wnd->entry[0]),buffer);

    gtk_window_set_focus(GTK_WINDOW(hwnd),wnd->entry[*buffer ? 1 : 0]);

    /* Finaliza */
    gtk_container_add(GTK_CONTAINER(hwnd),box);
    gtk_window_set_position(GTK_WINDOW(hwnd),GTK_WIN_POS_CENTER);
    gtk_widget_show_all(hwnd);

 }

 static void destroy( GtkWidget *widget, struct config *wnd )
 {
    HICQ icq = wnd->icq;
    
    DBGTracex(wnd);
    
    g_free(wnd);

    if(icqQueryLogonStatus(icq))
       icqEvent(icq,0,'G',0,0);
    else
       gtk_main_quit();

 }

 static void login_ok(GtkWidget *wd, struct config *wnd )
 {
    ULONG	uin;
    char	 pass[15];
    BOOL	 save;
    DBGMessage("Login");

    uin  = atol(gtk_entry_get_text(GTK_ENTRY(wnd->entry[0])));
    save = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wnd->save));
    
    strncpy(pass,gtk_entry_get_text(GTK_ENTRY(wnd->entry[1])),14);
  
    DBGTrace(uin);
    DBGMessage(pass);
    DBGTracex(save);
    DBGTracex(wnd->icq);
    
    icqSetCurrentUser(wnd->icq,uin,pass,save);

    CHKPoint();

    if(icqQueryLogonStatus(wnd->icq))
    {
       sprintf(pass,"%lu",uin);
       icqSaveProfileString(wnd->icq,"MAIN","uin",pass);
       gtk_widget_destroy(wnd->main);
    }
 }

 static void login_fail(GtkWidget *wd, struct config *wnd )
 {
    DBGMessage("Cancel");
    gtk_widget_destroy(wnd->main);
 }


