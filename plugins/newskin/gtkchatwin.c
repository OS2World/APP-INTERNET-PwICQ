/*
 * gtkmsgwin.c - GTK Message Window
 */


 #include <malloc.h>
 #include <ctype.h>
 #include <string.h>
 #include <unistd.h>
 #include <pwicqgui.h>

/*---[ Constants ]------------------------------------------------------------*/

 static const BUTTONLIST buttonList[] =
 {
#ifdef GTK2
    { MSGID_CLOSEBUTTON, "_Close"         },
    { MSGID_SENDBUTTON,  "_Send"          },
	{ MSGID_SENDTO,      "Send _To"       },
    { MSGID_ADDBUTTON,   "_Add"           }
#else
    { MSGID_CLOSEBUTTON, "Close"          },
    { MSGID_SENDBUTTON,  "Send"           },
	{ MSGID_SENDTO,      "Send To"        },
    { MSGID_ADDBUTTON,   "Add"            }
#endif	
 };

 
 
/*---[ Prototipes ]-----------------------------------------------------------*/

 static void hstSelected(GtkList *, GtkWidget *, hWindow);

/*---[ Implementation ]-------------------------------------------------------*/
 	
 hWindow icqskin_createChatWindow(HICQ icq)
 {
	SKINDATA       *skn   = icqGetSkinDataBlock(icq);
	hWindow        ret    = icqmsgdialog_new(icq,"Chat",378,300);
	
	ICQMSGDIALOG   *wnd;
    GtkWidget      *hBox;
    GtkWidget      *box;
	GtkWidget      *vBox;
	GtkWidget      *temp;

	if(!ret)
	   return ret;
	
    wnd        = icqskin_getWindowDataBlock(ret);
    wnd->flags = ICQMSGDIALOG_CHAT;

	icqskin_setWindowManagerName(ret,"Chat");

	vBox = gtk_vbox_new(FALSE,0);
	
	/* Create top bar */
	temp = icqskin_createUserBox(ret,skn,icqgui_msgUserBoxButtonTableElements,icqgui_msgUserBoxButtonTable);
    gtk_box_pack_start(GTK_BOX(vBox), temp, FALSE, FALSE, 0);
	
	/* Create edit box */
    box = gtk_vpaned_new();
    icqskin_setDlgItemHandle(ret, MSGID_PANEL, box);

    /* History area */
    temp = gtk_list_new();
    icqskin_setDlgItemHandle(ret, MSGID_HISTORY, temp);
    gtk_signal_connect(GTK_OBJECT(temp), "select-child", GTK_SIGNAL_FUNC(hstSelected), (gpointer) ret);
	
#ifdef GTK2

    hBox       = gtk_scrolled_window_new(NULL, NULL);
    wnd->scBar = GTK_SCROLLED_WINDOW(hBox);
	
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(hBox),GTK_POLICY_NEVER,GTK_POLICY_ALWAYS);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(hBox),GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(hBox),temp);

    gtk_paned_pack1(GTK_PANED(box),hBox,TRUE,TRUE);

    /* Text editing area */
	
    temp = gtk_text_view_new();
    icqskin_setDlgItemHandle(ret, MSGID_EDITBOX, temp);

    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(temp),GTK_WRAP_WORD);

	hBox       = gtk_scrolled_window_new(NULL, NULL);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(hBox),GTK_POLICY_NEVER,GTK_POLICY_ALWAYS);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(hBox),GTK_SHADOW_ETCHED_IN);
    gtk_container_add(GTK_CONTAINER(hBox),temp);

    gtk_widget_set_size_request(hBox,-1,50);
    gtk_paned_pack2(GTK_PANED(box),hBox,FALSE,TRUE);

#endif

#ifdef GTK1

    hBox = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(hBox),GTK_POLICY_NEVER,GTK_POLICY_ALWAYS);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(hBox),temp);

    gtk_paned_pack1(GTK_PANED(box),hBox,TRUE,TRUE);

    /* Text editing area */
    temp = gtk_text_new(NULL,NULL);
    icqskin_setDlgItemHandle(ret, MSGID_EDITBOX, temp);

    gtk_text_set_editable(GTK_TEXT(temp),TRUE);
    gtk_text_set_word_wrap(GTK_TEXT(temp),TRUE);

    hBox = gtk_hbox_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(hBox), temp,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hBox),gtk_vscrollbar_new(GTK_TEXT(temp)->vadj),FALSE,FALSE,0);

    gtk_widget_set_usize(hBox,-1,50);
	
    gtk_paned_pack2(GTK_PANED(box),hBox,FALSE,TRUE);

#endif	

    temp = gtk_frame_new(NULL);
    gtk_container_add (GTK_CONTAINER(temp),box);
    gtk_box_pack_start(GTK_BOX(vBox), temp, TRUE, TRUE, 0);
	
	temp = icqskin_createButtonBar(icq,ret,(sizeof(buttonList)/sizeof(BUTTONLIST)),buttonList);
    gtk_box_pack_end(GTK_BOX(vBox), temp, FALSE, FALSE, 0);

	/* Put the box in the window, show contents */
	gtk_container_add(GTK_CONTAINER(ret), vBox);
    gtk_widget_show_all(GTK_WIDGET(vBox));

    gtk_widget_show_all(GTK_WIDGET(ret));
	
    return ret;
 }
 
 int icqskin_setMsgWindowText(hWindow hwnd, USHORT id, const char *text)
 {
	hWindow h = icqskin_getDlgItemHandle(hwnd, id);
	
	DBGTrace(id);
	DBGTracex(h);
	DBGMessage(text);
	
	if(!h)
	{
	   return -1;
	}
	else if(GTK_IS_LABEL(h))
	{
	   CHKPoint();
	   gtk_label_set(GTK_LABEL(h), text);
	}
	else if(GTK_IS_ENTRY(h))
	{
	   CHKPoint();
       gtk_entry_set_text(GTK_ENTRY(h),text);
	}
#ifdef GTK2	
	else if(GTK_IS_TEXT_VIEW(h))
	{
	   CHKPoint();
	   gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(h)),text,-1);
	}
#endif	
#ifdef GTK1
	else if(GTK_IS_TEXT(h))
	{
	   CHKPoint();
	   gtk_text_insert(GTK_TEXT(h), NULL, NULL, NULL, text, -1);
	}
#endif	
	else
	{
	   DBGMessage("************ INVALID CALL TO icqskin_setMsgWindowText");
	   return 1;
	}
	
    CHKPoint();
	
    return 0;	
 }

 void icqskin_getMsgWindowText(hWindow hwnd, USHORT id, int sz, char *txt)
 {
	hWindow h = icqskin_getDlgItemHandle(hwnd, id);
    icqskin_getWindowText(h,sz,txt);
 }
 
 void icqskin_getWindowText(hWindow h, int sz, char *txt)
 {
	char    *ret        = NULL;
#ifdef GTK2
	char    *loc;
	gsize   bytes_read;
	gsize   bytes_written;
#endif	
	
	*txt = 0;
	
	if(!h)
	{
	   return;
	}
#ifdef GTK2
	else if(GTK_IS_TEXT_VIEW(h))
	{
	   GtkTextIter   start;
	   GtkTextIter   end;
	   GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(h));
	   
	   if(!buffer)
          return;		 

 	   gtk_text_buffer_get_bounds(buffer, &start, &end);
       ret = gtk_text_iter_get_text(&start, &end);
	  
       if(ret)
	   {
		  loc = g_locale_from_utf8(ret,-1,&bytes_read,&bytes_written,NULL);
		  
		  if(loc)
	      {
	         strncpy(txt,loc,sz);
			 g_free(loc);
		  }
		  else
		  {
	         strncpy(txt,ret,sz);
		  }
	   }

       g_free(ret);
 	   ret = NULL;
	   
	}
#endif	
	else if(GTK_IS_LABEL(h))
	{
       gtk_label_get(GTK_LABEL(h),&ret); 	   
	   if(ret)
	      strncpy(txt,ret,sz);
	}
	
	*(txt+sz) = 0;
 }
 
 static void hstSelected(GtkList *list, GtkWidget *widget, GtkWidget *frame)
 {
	ICQMSGDIALOG   *cfg   = icqskin_getWindowDataBlock(frame);
    GtkLabel       *label = icqskin_getWindowDataBlock(widget);
	const char     *text  = NULL;
	int            f;
	
#ifdef GTK2	
    GtkTextIter   start;
    GtkTextIter   end;
    GtkTextBuffer *buffer;
	char          *contents;
#endif
	
	cfg->hstSelected = widget;
	
#ifdef GTK2
	
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(label));
	DBGTracex(buffer);
	
    gtk_text_buffer_get_bounds(buffer, &start, &end);
	
	text     =
	contents = gtk_text_iter_get_text(&start, &end);
	
	DBGTracex(text);
	
#else	

    gtk_label_get(label,(gchar **) &text);
	
#endif	

	if(text)
	   text = (char *) icqQueryURL(cfg->fr.icq, text);

    if(text)
	{
	   for(f=0;*text && !isspace(*text) && f < 0xFE;f++)
		  cfg->sendToParm[f] = *(text++);
	   cfg->sendToParm[f] = 0;
	}
	else
	{
       *cfg->sendToParm = 0;
	}

    DBGMessage(cfg->sendToParm);	
	icqskin_setChildEnabled(frame,MSGID_SENDTO, *cfg->sendToParm != 0);
	
#ifdef GTK2
	g_free(contents);
#endif	
	
 }
 
 int icqskin_insertMessageHistory(HWND hwnd, ULONG flags, const char *hdl, const char *txt)
 {
    static const char *title   = FONT_BOLD;
    static const char *txtFont = FONT_NORMAL;

    ICQMSGDIALOG      *cfg     = icqskin_getWindowDataBlock(hwnd);
    SKINDATA          *skn     = icqGetSkinDataBlock(cfg->fr.icq);

    hWindow           h        = icqskin_getDlgItemHandle(hwnd, MSGID_HISTORY);
    hWindow           itn      = gtk_list_item_new();
	USHORT            icon     = (flags&1) ? ICQICON_MESSAGESENT : ICQICON_MESSAGERECEIVED;
	
	GtkWidget         *table;
	GtkWidget         *temp;
	
	GtkAdjustment     *adj;

#ifndef GTK1
	gsize             bytes_read;
	gsize		      bytes_written;
	gchar             *text;
#endif	
	
#ifdef EXTENDED_LOG	
    icqWriteSysLog(cfg->fr.icq,PROJECT,"Adding message in history box");
#endif

	/* Message header */
	table = gtk_table_new(2,2,FALSE);

	temp = gtk_pixmap_new(skn->iPixmap[icon],skn->iBitmap[icon]);
    gtk_table_attach(GTK_TABLE(table),temp,0,1,0,1,0,0,1,1);

#ifdef GTK1
	temp = gtk_label_new(hdl);
#else
	text = g_locale_to_utf8(hdl,-1,&bytes_read,&bytes_written,NULL);
	temp = gtk_label_new(text);
	g_free(text);
#endif	

    gtk_misc_set_alignment(GTK_MISC(temp), 0, .5);
    icqskin_setFont(temp, title);
    gtk_table_attach(GTK_TABLE(table),temp,1,2,0,1,GTK_EXPAND|GTK_SHRINK|GTK_FILL,0,2,2);

#ifdef GTK2
	text = g_locale_to_utf8(txt,-1,&bytes_read,&bytes_written,NULL);
    temp = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(temp),FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(temp),GTK_WRAP_WORD);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(temp)),text,-1);
	g_free(text);
#endif
	
#ifdef GTK1
	temp = gtk_label_new(txt);
    gtk_label_set_line_wrap(GTK_LABEL(temp),TRUE);
    gtk_misc_set_alignment(GTK_MISC(temp), 0, .5);
#endif

    icqskin_setWindowDataBlock(itn, temp);

    icqskin_setFont(temp, txtFont);
    gtk_table_attach(GTK_TABLE(table),temp,1,2,1,2,GTK_EXPAND|GTK_SHRINK|GTK_FILL,GTK_EXPAND|GTK_SHRINK|GTK_FILL,1,1);

    gtk_container_add(GTK_CONTAINER(itn),table);
    gtk_widget_show_all(GTK_WIDGET(itn));
    gtk_list_append_items(GTK_LIST(h),g_list_append(NULL,itn));

    if(!cfg->hstLast || cfg->hstSelected == cfg->hstLast)
	{
	   gtk_list_select_child(GTK_LIST(h),itn);
	   adj = gtk_scrolled_window_get_vadjustment(cfg->scBar);
	   if(adj)
		  gtk_adjustment_set_value(adj,adj->upper);
	}
	
    cfg->hstLast = itn;

#ifdef EXTENDED_LOG	
    icqWriteSysLog(cfg->fr.icq,PROJECT,"Message added in history box");
#endif
	
    return 0;
 }



