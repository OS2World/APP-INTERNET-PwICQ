/*
 * gtkupopup.c - GTK User popup window
 */

 #include <string.h>
 #include <stdlib.h>
 #include <malloc.h>
 #include <gdk/gdkkeysyms.h>
 
 #include <pwicqgui.h>

/*---[ Defines ]------------------------------------------------------------------------------------------*/

/*---[ Structures ]---------------------------------------------------------------------------------------*/

/*---[ Constants ]----------------------------------------------------------------------------------------*/

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static MRESULT _System userPopupWindow(HWND, ULONG, MPARAM, MPARAM);

 static void setPos(SKINDATA *);
 static void destroy(GtkWidget *, ICQFRAME * );

 static gboolean keyPress(GtkWidget *, GdkEventKey *, ICQFRAME *);

/*---[ Implementation ]-----------------------------------------------------------------------------------*/
 
 void icqskin_popupUserList(HICQ icq)
 {
	static const char *name = "userpopup";
	
	ICQFRAME  *cfg;
	SKINDATA  *skn     = icqskin_getDataBlock(icq);
	hWindow   frame;
	hWindow   box;
	hWindow   ulist;
	
	if(skn->userPopup)
	{
       setPos(skn);
	   return;
	}
	
	/* Create frame */
	
	cfg = icqframe_createDataBlock(icq, sizeof(ICQFRAME), name);
	if(!cfg)
	   return;
	
	cfg->cx = 150;
	cfg->cy = 300;

#ifdef GTK2	
    frame = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#else	
    frame = gtk_window_new(GTK_WINDOW_POPUP);
#endif
	
	if(!frame)
	{
	   free(cfg);
	   return;
	}

    gtk_object_set_user_data(GTK_OBJECT(frame), (gpointer) cfg);
    icqframe_initialize(frame);

    gtk_signal_connect( GTK_OBJECT(frame),"destroy",GTK_SIGNAL_FUNC(destroy),cfg );

    icqskin_setWindowManagerName(frame,name);

    /* Create user list */
	
    box   = gtk_vbox_new(FALSE,0);

    DBGTracex(box);
	
    gtk_object_set_user_data(GTK_OBJECT(box),cfg);

    gtk_container_add(GTK_CONTAINER(frame), box);
	
	ulist = icqSkin_createStandardUserList(icq, box, ID_USERLIST);
	DBGTracex(ulist);

#ifdef GTK2
    gtk_window_set_resizable(GTK_WINDOW(frame),FALSE);
    gtk_window_set_decorated(GTK_WINDOW(frame),FALSE);
#endif

	if(!ulist)
	{
	   icqskin_closeWindow(frame);
	   return;
	}

    icqskin_setUserListFlag(ulist,ICQULIST_POPUP);
	
	icqskin_loadUsersList(ulist, icq);

	cfg->msg = userPopupWindow;
    gtk_signal_connect( GTK_OBJECT(frame),"key-release-event",GTK_SIGNAL_FUNC(keyPress),cfg );
	
    skn->userPopup = frame;
	
    setPos(skn);	
 }
 
 static void setPos(SKINDATA *skn)
 {
	gtk_window_set_position(GTK_WINDOW(skn->userPopup),GTK_WIN_POS_MOUSE);
    gtk_widget_show_all(GTK_WIDGET(skn->userPopup));
 }
 
 static MRESULT _System userPopupWindow(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    return icqFrameWindow(hwnd, msg, mp1, mp2);
 }

 static gboolean keyPress(GtkWidget *widget, GdkEventKey *event, ICQFRAME *cfg)
 {
	if(event->keyval == GDK_Escape)
	{
       icqskin_closeWindow(widget);
	   return TRUE;
	}
	
    return FALSE;	
 }
 
 static void destroy(GtkWidget *hwnd, ICQFRAME *cfg )
 {
	SKINDATA  *skn     = icqskin_getDataBlock(cfg->icq);

    DBGMessage("Closing userpopup window");
	
	skn->userPopup = 0;
    icqframe_destroy(hwnd, (ICQFRAME *) cfg);

 }
