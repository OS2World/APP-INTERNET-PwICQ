/*
 * gtkulist.c - User's list management for GTK
 */

 #include <pwicqgui.h>
 #include <string.h>
 #include <malloc.h>

/*---[ Enums ]----------------------------------------------------------------*/

 enum
 {
    USER_ICON,
	USER_NICK,
	
	N_COLUMNS
 };

/*---[ Structures ]-----------------------------------------------------------*/

 #pragma pack(1)

 typedef struct _icqulist
 {
	/* Standard header */
	USHORT       sz;
	const char   *className;
	ULONG        flags;

	GtkTreeModel *model;
	HICQ         icq;
	HUSER        selected;
	
 } ICQULIST;


/*---[ Prototipes ]-----------------------------------------------------------*/

 static void destroy( GtkWidget *, ICQULIST *);
 
/*---[ Implementation ]-------------------------------------------------------*/

 hWindow icqSkin_createStandardUserList(HICQ icq, hWindow hwnd, USHORT id)
 {
	ICQULIST          *cfg;
	hWindow           ret;
	hWindow           temp;
    GtkCellRenderer   *renderer;	
    GtkTreeViewColumn *column;
	
	cfg = malloc(sizeof(ICQULIST));
	if(!cfg)
	{
	   icqWriteSysLog(icq,PROJECT,"Memory allocation error when creating user list");
	   return NULL;
	}
	
	memset(cfg,0,sizeof(ICQULIST));
	cfg->sz  = sizeof(ICQULIST);
	cfg->icq = icq;
	
    /* Create tree model (the default one) */
	cfg->model = GTK_TREE_MODEL(gtk_tree_store_new(N_COLUMNS,GDK_TYPE_PIXBUF,G_TYPE_STRING));
    ret = gtk_tree_view_new_with_model(cfg->model);	

    gtk_object_set_user_data(GTK_OBJECT(ret), (gpointer) cfg );
    gtk_signal_connect(GTK_OBJECT(ret), "destroy", GTK_SIGNAL_FUNC(destroy), cfg);

	/* Set the renderers */
	renderer = gtk_cell_renderer_pixbuf_new();
    column   = gtk_tree_view_column_new_with_attributes 
	           (   
					"",
        	        renderer,
                    "pixbuf", 
	                USER_ICON,
                    NULL
	           );
    gtk_tree_view_append_column(GTK_TREE_VIEW(ret), column);	
	
    renderer = gtk_cell_renderer_text_new();
    column   = gtk_tree_view_column_new_with_attributes 
	           (   
					"",
        	        renderer,
                    "text", 
	                USER_NICK,
                    NULL
	           );
			   
    gtk_tree_view_append_column(GTK_TREE_VIEW(ret), column);	
	
	/* And finalize */
    icqskin_setDlgItemHandle(hwnd, id, ret);
	icqskin_initializeUserList(icq, ret);

    temp = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(temp),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);

    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(temp),ret);
    gtk_box_pack_start(GTK_BOX(hwnd), temp, TRUE, TRUE, 0);
    gtk_widget_show_all(GTK_WIDGET(temp));

    DBGMessage("*******************");
    icqskin_insertUserGroup(ret, 0, "*** teste ***");
    DBGMessage("*******************");


	return ret;
 }	

 void icqskin_insertUserGroup(hWindow hwnd, int id, const char *group)
 {
    ICQULIST    *cfg = gtk_object_get_user_data(GTK_OBJECT(hwnd));
	SKINDATA    *skn   = icqGetSkinDataBlock(cfg->icq);
    GtkTreeIter iter;
    GtkTreeIter child;

    gtk_tree_store_append(GTK_TREE_STORE(cfg->model), &iter, NULL); 	

	gtk_tree_store_set( GTK_TREE_STORE(cfg->model), &iter,
	                    USER_ICON, skn->icons[0],
                  	    USER_NICK, group,
                        -1);

 }
 
 static void destroy( GtkWidget *widget, ICQULIST *cfg)
 {
    if(cfg->sz != sizeof(ICQULIST))
       return;

    DBGMessage("User list was destroyed");
    icqskin_terminateUserList(cfg->icq, widget);
    memset(cfg,0,sizeof(ICQULIST));
    free(cfg);
 }
 
 ULONG * icqskin_getUserListFlagPointer(hWindow hwnd)
 {
    ICQULIST *cfg = gtk_object_get_user_data(GTK_OBJECT(hwnd));
	return &cfg->flags;
 }	

 void icqskin_setUserListFlag(hWindow hwnd, ULONG flags)
 {
    ICQULIST *cfg = gtk_object_get_user_data(GTK_OBJECT(hwnd));
	cfg->flags |= flags;
 }
 
 void icqskin_switchFlag(hWindow hwnd, ULONG masc)
 {
    ICQULIST *cfg = gtk_object_get_user_data(GTK_OBJECT(hwnd));
	cfg->flags ^= masc;
 }

 void icqskin_blinkUser(hWindow hwnd, ULONG uin, BOOL text, BOOL icon)
 {

 }

 HUSER icqskin_queryUserListEntry(hWindow hwnd, ULONG uin)
 {

    return NULL;
 }
 
 HUSER icqskin_updateUserHandle(hWindow hwnd, ULONG uin)
 {

	
	return NULL;
 }
 
 void  icqskin_delUser(hWindow hwnd, ULONG uin)
 {
	
 }
 
 void icqskin_updateUserIcon(hWindow hwnd, ULONG uin)
 {
	
 }
 

 void icqskin_addUser(hWindow hwnd, HUSER usr)
 {
	
 }

