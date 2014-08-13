/*
 * gtkmsgwin.c - GTK Message Window
 */


 #include <malloc.h>
 #include <string.h>
 #include <unistd.h>
 #include <pwicqgui.h>

/*---[ Constants ]------------------------------------------------------------*/

 #pragma pack(1)

 static const BUTTONLIST buttonList[] =
 {
#ifdef GTK2
    { MSGID_CLOSEBUTTON,   "_Close"       },
    { MSGID_NEXTBUTTON,    "_Next"        },
    { MSGID_SENDBUTTON,    "_Send"        },
    { MSGID_REPLYBUTTON,   "_Reply"       },
    { MSGID_ACCEPTBUTTON,  "Acce_pt"      },
	{ MSGID_REFUSEBUTTON,  "Re_fuse"      },
	{ MSGID_SENDTO,        "Send _To"     },
    { MSGID_ADDBUTTON,     "_Add"         }
#else
    { MSGID_CLOSEBUTTON,   "Close"        },
    { MSGID_NEXTBUTTON,    "Next"         },
    { MSGID_SENDBUTTON,    "Send"         },
    { MSGID_REPLYBUTTON,   "Reply"        },
    { MSGID_ACCEPTBUTTON,  "Accept"       },
	{ MSGID_REFUSEBUTTON,  "Refuse"       },
	{ MSGID_SENDTO,        "Send To"      },
    { MSGID_ADDBUTTON,     "Add"          }
#endif	
 };

/*---[ Prototipes ]-----------------------------------------------------------*/

/*---[ Implementation ]-------------------------------------------------------*/

 hWindow icqskin_createPagerWindow(HICQ icq)
 {
	SKINDATA       *skn   = icqGetSkinDataBlock(icq);
	hWindow        ret    = icqmsgdialog_new(icq,"Pager",378,200);
	
	ICQMSGDIALOG   *wnd;
	GtkWidget      *vBox;
	GtkWidget      *temp;
	GtkWidget      *hBox;
	GtkWidget      *Box;
	
	if(!ret)
	   return ret;
	
    wnd  = icqskin_getWindowDataBlock(ret);

	vBox = gtk_vbox_new(FALSE,0);
	
	/* Create top bar */
	temp = icqskin_createUserBox(ret,skn,icqgui_msgUserBoxButtonTableElements,icqgui_msgUserBoxButtonTable);
    gtk_box_pack_start(GTK_BOX(vBox), temp, FALSE, FALSE, 0);

    /* Create text areas */	
	Box = gtk_vbox_new(FALSE,0);

#ifdef GTK2
    temp = gtk_text_view_new();
    icqskin_setDlgItemHandle(ret, MSGID_EDITBOX, temp);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(temp),GTK_WRAP_WORD);
    hBox = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(hBox),GTK_POLICY_NEVER,GTK_POLICY_ALWAYS);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(hBox),GTK_SHADOW_ETCHED_IN);
    gtk_container_add(GTK_CONTAINER(hBox),temp);
#endif	
	
#ifdef GTK1
    temp = gtk_text_new(NULL,NULL);
    icqskin_setDlgItemHandle(ret, MSGID_EDITBOX, temp);
    gtk_widget_set_usize(temp,10,10);
    gtk_text_set_editable(GTK_TEXT(temp),FALSE);
    gtk_text_set_word_wrap(GTK_TEXT(temp),TRUE);
    hBox = gtk_hbox_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(hBox), temp,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hBox), gtk_vscrollbar_new(GTK_TEXT(temp)->vadj),FALSE,FALSE,0);
#endif	

    gtk_box_pack_start(GTK_BOX(Box), hBox,TRUE,TRUE,0);

    hBox = gtk_hbox_new(FALSE,0);
    temp = gtk_entry_new();
    icqskin_setDlgItemHandle(ret, MSGID_ENTRY, temp);
    gtk_entry_set_editable(GTK_ENTRY(temp),FALSE);
    gtk_box_pack_start(GTK_BOX(hBox), temp,TRUE,TRUE,0);
	
#ifdef GTK2
    temp = gtk_button_new_with_mnemonic("_Browse");
#else
    temp = gtk_button_new_with_label("Browse");
#endif	
    icqskin_setDlgItemHandle(ret, MSGID_BROWSEBUTTON, temp);
    gtk_box_pack_end(GTK_BOX(hBox), temp,FALSE,FALSE,0);

    gtk_box_pack_start(GTK_BOX(Box), hBox,FALSE,FALSE,0);

    temp = gtk_frame_new(NULL);
    gtk_container_add(GTK_CONTAINER(temp), Box);
    gtk_box_pack_start(GTK_BOX(vBox), temp, TRUE,TRUE, 0);


    /* Create button bar */
	temp = icqskin_createButtonBar(icq,ret,(sizeof(buttonList)/sizeof(BUTTONLIST)),buttonList);
	
    gtk_box_pack_end(GTK_BOX(vBox), temp, FALSE, FALSE, 0);

	/* Put the box in the window, show contents */
    gtk_widget_show_all(GTK_WIDGET(vBox));
    gtk_container_add(GTK_CONTAINER(ret), vBox);

    return ret;
 }

