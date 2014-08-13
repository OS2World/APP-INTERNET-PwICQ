/*
 * gtkmsgdialog.c - GTK Message Window
 */


 #include <malloc.h>
 #include <string.h>
 #include <unistd.h>
 #include <pwicqgui.h>

/*---[ Constants ]------------------------------------------------------------*/

/*---[ Prototipes ]-----------------------------------------------------------*/

 static void icqmsgdialog_destroy(GtkWidget *, ICQMSGDIALOG * );

/*---[ Implementation ]-------------------------------------------------------*/

 hWindow EXPENTRY icqmsgdialog_new(HICQ icq, const char *name, USHORT cx, USHORT cy)
 {
	ICQMSGDIALOG *cfg = (ICQMSGDIALOG *) icqframe_createDataBlock(icq, sizeof(ICQMSGDIALOG), name);
	hWindow      ret;

	if(!cfg)
	   return 0;

	cfg->fr.cx = cx;
	cfg->fr.cy = cy;
	
    ret = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	
    gtk_object_set_user_data(GTK_OBJECT(ret), (gpointer) cfg);
	icqframe_initialize(ret);

    gtk_signal_connect( GTK_OBJECT(ret),"destroy",GTK_SIGNAL_FUNC(icqmsgdialog_destroy),cfg );

    icqskin_setWindowManagerName(ret,name);
	
	gtk_window_set_position(GTK_WINDOW(ret),GTK_WIN_POS_MOUSE);
	
    return ret;
	
 }

 static void icqmsgdialog_destroy(GtkWidget *hwnd, ICQMSGDIALOG *cfg )
 {
	/* Message window is beeing closed */
	DBGMessage("Closing message window");
    icqskin_destroyMsgWindow(cfg);
	
	if(cfg && cfg->fr.sz == sizeof(ICQMSGDIALOG))
	{
	   DBGMessage("Releasing message window");
       icqskin_terminateMessageEditor(cfg->fr.icq, hwnd);
       icqframe_destroy(hwnd, (ICQFRAME *) cfg);
	}
#ifdef DEBUG
	else
	{
	   DBGMessage("********* INVALID MESSAGE WINDOW");
	}
#endif	
	
 }
 
 hWindow icqskin_createButtonBar(HICQ icq, hWindow ret, int sz, const BUTTONLIST *buttonList)
 {
    /* Create button bar */
	
	int     f;
	hWindow temp;
    hWindow hBox = gtk_hbox_new(FALSE,0);

    gtk_box_pack_start(GTK_BOX(hBox), gtk_label_new(""), TRUE, TRUE, 0);
	
 	for(f=0;f<sz;f++)
	{
#ifdef GTK2	
       temp = gtk_button_new_with_mnemonic(buttonList[f].wText);
#else
	   temp = gtk_button_new_with_label(buttonList[f].wText);
#endif	
	   	
       icqskin_setDlgItemHandle(ret, buttonList[f].id, temp);
       gtk_box_pack_end(GTK_BOX(hBox), temp, FALSE, FALSE, 0);
	}
	
	return hBox;
 }


 hWindow icqskin_createUserBox(hWindow ret, SKINDATA *skn, int btnCount, const ICQUSERBUTTON *userButtons )
 {
	static const char *normal = FONT_NORMAL;
#ifdef GTK2
	static const char *bold   = FONT_BOLD;
#endif	
	int               f;
	ICQFRAME          *wnd    = icqskin_getWindowDataBlock(ret);
	GtkWidget         *temp;
	GtkWidget         *hBox;
    GtkWidget         *box;
    GtkRequisition    size;
	
	hBox = gtk_hbox_new(TRUE,0);

	/* Left */
    box = gtk_hbox_new(FALSE,0);
    gtk_box_set_spacing(GTK_BOX(box),2);
	
    temp  = gtk_pixmap_new(skn->iPixmap[ICQICON_OFFLINE],skn->iBitmap[ICQICON_OFFLINE]);
    icqskin_setDlgItemHandle(ret, MSGID_USERMODE, temp);
    gtk_object_set_user_data(GTK_OBJECT(temp), (gpointer) wnd);	
    gtk_box_pack_start(GTK_BOX(box), temp, FALSE, FALSE, 0);
	
	temp  = gtk_label_new("Nick:");
	icqskin_setFont(temp,normal);
	gtk_misc_set_alignment(GTK_MISC(temp), 0, .5);
    icqskin_setDlgItemHandle(ret, MSGID_NICK, temp);
    gtk_box_pack_start(GTK_BOX(box), temp, FALSE, FALSE, 0);

#ifdef GTK2
    temp = gtk_label_new("N/A");
	icqskin_setFont(temp,bold);
#else
    temp = gtk_entry_new();
    gtk_entry_set_editable(GTK_ENTRY(temp),FALSE);
#endif	
	gtk_misc_set_alignment(GTK_MISC(temp), 0, .5);
    icqskin_setDlgItemHandle(ret, MSGID_NICKENTRY, temp);
    gtk_widget_size_request(temp,&size);
    gtk_widget_set_usize(temp,10,size.height);
    gtk_box_pack_start(GTK_BOX(box), temp, TRUE, TRUE, 0);
	
	gtk_box_pack_start(GTK_BOX(hBox), box, TRUE, TRUE, 0);

    /* Right */
    box = gtk_hbox_new(FALSE,0);
    gtk_box_set_spacing(GTK_BOX(box),2);

	temp = gtk_label_new("Name:");
	icqskin_setFont(temp,normal);
	gtk_misc_set_alignment(GTK_MISC(temp), 0, .5);
    icqskin_setDlgItemHandle(ret, MSGID_NAME, temp);
    gtk_box_pack_start(GTK_BOX(box), temp, FALSE, FALSE, 0);
	
#ifdef GTK2
    temp = gtk_label_new("N/A");
	icqskin_setFont(temp,bold);
#else
    temp = gtk_entry_new();
    gtk_entry_set_editable(GTK_ENTRY(temp),FALSE);
#endif	
	gtk_misc_set_alignment(GTK_MISC(temp), 0, .5);
    icqskin_setDlgItemHandle(ret, MSGID_NAMEENTRY, temp);
    gtk_widget_size_request(temp,&size);
    gtk_widget_set_usize(temp,10,size.height);
	
    gtk_box_pack_start(GTK_BOX(box), temp, TRUE, TRUE, 0);

	for(f=0;f<btnCount;f++)
	{
       temp = gtk_button_new();
       icqskin_setDlgItemHandle(ret, userButtons[f].id, temp);
       gtk_container_add(GTK_CONTAINER(temp),gtk_pixmap_new(skn->iPixmap[userButtons[f].icon],skn->iBitmap[userButtons[f].icon]));
       gtk_button_set_relief(GTK_BUTTON(temp),GTK_RELIEF_NONE);
	   gtk_box_pack_end(GTK_BOX(box), temp, FALSE, FALSE, 0);
	}

	gtk_box_pack_start(GTK_BOX(hBox), box, TRUE, TRUE, 0);
	
	/* User box */
    temp = gtk_frame_new("User");
    icqskin_setDlgItemHandle(ret, MSGID_NAMEBOX, temp);
    gtk_container_add (GTK_CONTAINER(temp), hBox);
	
	return temp;
 }	

