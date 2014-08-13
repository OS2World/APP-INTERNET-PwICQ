/*
 * gtkulist.c - User's list management for GTK
 */

 #include <pwicqgui.h>
 #include <string.h>
 #include <malloc.h>
 
 #include <gdk/gdkkeysyms.h>

/*---[ Structures ]-----------------------------------------------------------*/

 #pragma pack(1)

 struct group_record
 {
    UCHAR  type;
    USHORT id;
 };

 struct user_record
 {
    UCHAR  type;
    HUSER  usr;
 };

 typedef struct _icqulist
 {
    ICQGUI_STANDARD_PREFIX;
	
	USHORT     id;
	ULONG      flags;
	HUSER      selected;
	
 } ICQULIST;


/*---[ Prototipes ]-----------------------------------------------------------*/

 static gint     userListSort(GtkCList *, gconstpointer, gconstpointer);
 static void     destroy_row(struct group_record *);
 static gint     findGroup(gconstpointer, gconstpointer);
 static gint     eventProc(GtkWidget *, GdkEvent *);
 static void     selected(GtkCList *, gint, gint,  GdkEventButton *, ICQULIST *);
 static void     destroy( GtkWidget *, ICQULIST *);
 static gboolean keyPress(GtkWidget *, GdkEventKey *, ICQULIST *);

 static struct user_record *queryRecord(hWindow, ULONG);

 static GtkCTreeNode *queryNode(hWindow, ULONG);

/*---[ Implementation ]-------------------------------------------------------*/

 void icqskin_insertUserGroup(hWindow hwnd, int id, const char *group)
 {
	
	char		        *texto[] = { (char *) group };
	struct group_record *grp     = malloc(sizeof(struct group_record));
    GtkCTreeNode        *node;

    DBGTracex(grp);
	
	if(!grp)
	   return;
	
	memset(grp,0,sizeof(struct group_record));
	grp->id = id;
	DBGMessage(group);
	
	node = gtk_ctree_insert_node(    GTK_CTREE(hwnd),
    							     NULL,
       								 NULL,
       								 texto,
       								 0,
       								 NULL,
       								 NULL,
       								 NULL,
       								 NULL,
       								 FALSE,
       								 TRUE);
	
    gtk_ctree_node_set_selectable(GTK_CTREE(hwnd),node,FALSE);
    gtk_ctree_node_set_row_data_full(GTK_CTREE(hwnd),node,(gpointer) grp,( void (*)(gpointer) ) destroy_row );
	
 }

 void icqskin_addUser(hWindow hwnd, HUSER usr)
 {
    GtkCTreeNode       *node;
    struct user_record *rec;
	gchar	           *tbl[] = { "*Failed*" };
    char               key[20];
    char               buffer[0x0100];
	char               *ptr;
	ICQULIST           *wnd   = gtk_object_get_user_data(GTK_OBJECT(hwnd));
	SKINDATA *ctl;

    if(!(wnd && wnd->sz == sizeof(ICQULIST)) )
    {
	   DBGMessage("**** INVALID CALL TO ADD USER");
	   return;
	}	
	
	ctl = icqskin_getDataBlock(wnd->icq);
	
    node = gtk_ctree_find_by_row_data_custom(   GTK_CTREE(hwnd),
                                               NULL, //	GtkCTreeNode *node,
                                               (gpointer) usr,
                                               findGroup );
    if(!node)
    {
	   if(usr->c2sGroup == 0x0F00)
	   {
		  ptr = "Not in list";
	   }
	   else
	   {
          sprintf(key,"UserGroup%02d",usr->c2sGroup);
          icqLoadString(ctl->icq, key, "Unknown", buffer, 0xFF);
          ptr = strstr(buffer, ",");
	      if(!ptr)
		     ptr = buffer;
	      else
		     ptr++;
	   }
       icqskin_insertUserGroup(hwnd, usr->c2sGroup, ptr);
       node = gtk_ctree_find_by_row_data_custom(   GTK_CTREE(hwnd),
                                                  NULL, //	GtkCTreeNode *node,
                                                  (gpointer) usr,
                                                  findGroup );
    }	

    rec = malloc(sizeof(struct user_record));
	if(!rec)
	   return;
	
	memset(rec,0,sizeof(struct user_record));
	rec->type = 1;
	rec->usr  = usr;

    node = gtk_ctree_insert_node(    GTK_CTREE(hwnd),
     							     node,
       								 NULL,
       								 tbl,
       								 0,
       								 NULL,
       								 NULL,
       								 NULL,
       								 NULL,
       								 FALSE,
       								 FALSE);


     gtk_ctree_node_set_row_data_full(GTK_CTREE(hwnd),node,(gpointer) rec,( void (*)(gpointer) ) destroy_row );
	
     gtk_ctree_node_set_pixtext(	GTK_CTREE(hwnd),
                                    node,
                                    0,
                                    icqQueryUserNick(usr),
                                    4,
                                    ctl->iPixmap[usr->modeIcon],
                                    ctl->iBitmap[usr->modeIcon]);	

#ifdef DEBUG
     if(queryNode(hwnd, usr->uin) != node)
	 {
		DBGMessage("**************** ERROR!!! FAILURE IN NODE SEARCH");
		DBGTracex(queryNode(hwnd, usr->uin));
		DBGTracex(node);
	 }
#endif

 }

 hWindow icqSkin_createStandardUserList(HICQ icq, hWindow hwnd, USHORT id)
 {
	ICQFRAME  *wnd = gtk_object_get_user_data(GTK_OBJECT(hwnd));
	ICQULIST  *ctl;
	hWindow   temp;
	hWindow   ret;
	SKINDATA  *cfg = icqskin_getDataBlock(icq);
	
	if(!(wnd && wnd->sz == sizeof(ICQFRAME)))
	{
	   DBGTracex(wnd);
	   DBGMessage("Invalid frame window in icqSkin_createStandardUserList");
	   icqWriteSysLog(icq,PROJECT,"Invalid frame window in icqSkin_createStandardUserList");
	   return NULL;
	}
	
	DBGTracex(cfg);
	
	ctl = malloc(sizeof(ICQULIST));
	if(!ctl)
	{
	   icqWriteSysLog(icq,PROJECT,"Memory allocation error when creating user list");
	   return NULL;
	}
	
	memset(ctl,0,sizeof(ICQULIST));
	ctl->sz  = sizeof(ICQULIST);
	ctl->id  = id;
	ctl->icq = icq;
	
	temp = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(temp),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);

    ret = gtk_ctree_new(1,0);

	DBGTracex(ret);
	
    gtk_object_set_user_data(GTK_OBJECT(ret), (gpointer) ctl );
    gtk_signal_connect(GTK_OBJECT(ret), "destroy", GTK_SIGNAL_FUNC(destroy), ctl);

    icqskin_setFont(ret, FONT_NORMAL);

    CHKPoint();

    gtk_clist_set_column_auto_resize(GTK_CLIST(ret),0,0);
    gtk_clist_set_column_width(GTK_CLIST(ret),0,cfg->iconSize);
    gtk_clist_set_row_height(GTK_CLIST(ret),cfg->iconSize+2);

    CHKPoint();
    gtk_clist_set_selection_mode (GTK_CLIST(ret),GTK_SELECTION_SINGLE);
    gtk_clist_set_compare_func(GTK_CLIST(ret), userListSort);
	
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(temp),ret);
    gtk_box_pack_start(GTK_BOX(hwnd), temp, TRUE, TRUE, 0);
    gtk_widget_show_all(GTK_WIDGET(temp));
	
    CHKPoint();
    icqskin_setDlgItemHandle(hwnd, id, ret);
    CHKPoint();

    gtk_signal_connect(GTK_OBJECT(ret),        "select-row",        GTK_SIGNAL_FUNC(selected),  ctl);
    gtk_signal_connect_object(GTK_OBJECT(ret), "event",             GTK_SIGNAL_FUNC(eventProc), GTK_OBJECT(ret));
    gtk_signal_connect( GTK_OBJECT(ret),       "key-release-event", GTK_SIGNAL_FUNC(keyPress),  ctl );
/*
  GTK_BUTTON_IGNORED = 0,
  GTK_BUTTON_SELECTS = 1 << 0,
  GTK_BUTTON_DRAGS   = 1 << 1,
  GTK_BUTTON_EXPANDS = 1 << 2
*/

    gtk_clist_set_button_actions(GTK_CLIST(ret),2,GTK_BUTTON_SELECTS);

    icqskin_initializeUserList(icq, ret);

    DBGTracex(ret);
	return ret;
 }	

 static gint userListSort(GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
 {
    struct user_record *h1	= ((GtkCListRow *) ptr1)->data;
    struct user_record *h2	= ((GtkCListRow *) ptr2)->data;
	
	if(!h1->type)
	   return ( ((struct group_record *) h1)->id - ((struct group_record *) h2)->id);
	
    if(h1->usr->index != h2->usr->index)
       return h1->usr->index - h2->usr->index;
    return strcasecmp(icqQueryUserNick(h1->usr),icqQueryUserNick(h2->usr));
 }

 static void destroy_row(struct group_record *cfg)
 {
    free(cfg);
 }

 static gint findGroup(gconstpointer grp, gconstpointer usr)
 {
	if( ((struct group_record *) grp)->type)
	   return TRUE;
	
	if(  ((struct group_record *) grp)->id == ((HUSER) usr)->c2sGroup )
	   return FALSE;

	return TRUE;
 }

 static gint eventProc(GtkWidget *widget, GdkEvent *event)
 {
    ICQULIST *cfg;
	
    switch(event->type)
    {
    case GDK_BUTTON_PRESS:
        if(((GdkEventButton *) event)->button != 3)
           return FALSE;
		DBGMessage("Open user's context menu");
		cfg = gtk_object_get_user_data(GTK_OBJECT(widget));
        icqskin_userContextMenu(cfg->icq, cfg->selected);
        break;

	case GDK_2BUTTON_PRESS:
        if(((GdkEventButton *) event)->button != 1)
           return FALSE;
		DBGMessage("Open message dialog for the user");
		cfg = gtk_object_get_user_data(GTK_OBJECT(widget));
        icqskin_userlistClick(cfg->icq, cfg->selected);
		
		if(cfg->flags & ICQULIST_POPUP)
		   icqskin_closeWindow(gtk_widget_get_toplevel(widget));
		
        break;

    default:
       return FALSE;

    }
    return TRUE;
 }

 static void destroy( GtkWidget *widget, ICQULIST *wnd)
 {
    if(wnd->sz != sizeof(ICQULIST))
       return;
	
    DBGMessage("User list was destroyed");
	icqskin_terminateUserList(wnd->icq, widget);
    memset(wnd,0,sizeof(ICQULIST));
    free(wnd);
 }

 static void selected(GtkCList *clist, gint row, gint column,  GdkEventButton *event, ICQULIST *cfg)
 {
    struct user_record *rec = gtk_clist_get_row_data(clist,row);
	
	if(rec->type == 1)
       cfg->selected = rec->usr;
    else
       cfg->selected = NULL;

	DBGTracex(cfg->selected);
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
	USHORT           iconID;
 	ICQULIST         *cfg    = gtk_object_get_user_data(GTK_OBJECT(hwnd));
	HUSER            usr     = icqQueryUserHandle(cfg->icq,uin);
	SKINDATA         *skn;
    GtkCTreeNode     *node;

	if(!usr)
	   return;
	
	skn  = icqskin_getDataBlock(cfg->icq);
	
    if(icon)
	{
	   /* Blink user icon */
	   iconID = (cfg->flags & ICQULIST_BLINK) == 0 ? usr->modeIcon : usr->eventIcon;
	
       node =  queryNode(hwnd, uin);
	
	   if(node)
	   {
          gtk_ctree_node_set_pixtext(  GTK_CTREE(hwnd),
                                       node,
                                       0,
                                       icqQueryUserNick(usr),
                                       4,
                                       skn->iPixmap[iconID],
                                       skn->iBitmap[iconID]);
		
	   }
	}
	
 }

 static gint searchUIN(gconstpointer rec, gconstpointer uin)
 {
	if( ((struct user_record *) rec)->type != 1)
	   return TRUE;
	
	if(  ((struct user_record *) rec)->usr->uin == ((ULONG) uin) )
	   return FALSE;
	
	return TRUE;
 }


 static GtkCTreeNode *queryNode(hWindow hwnd, ULONG uin)
 {
 	ICQULIST         *cfg    = gtk_object_get_user_data(GTK_OBJECT(hwnd));
	GtkCTreeNode     *node;

    if(!(cfg && cfg->sz == sizeof(ICQULIST)) )
	   return NULL;

    node = gtk_ctree_find_by_row_data_custom(GTK_CTREE(hwnd),NULL,(gpointer) uin, searchUIN);
	
    return node;	
 }

 static struct user_record *queryRecord(hWindow hwnd, ULONG uin)
 {
	GtkCTreeNode *node = queryNode(hwnd, uin);
	if(!node)
	   return NULL;
    return (struct user_record *) gtk_ctree_node_get_row_data(GTK_CTREE(hwnd),node);	
 }

 static void updatePixtext(hWindow hwnd, GtkCTreeNode *node)
 {
 	ICQULIST           *cfg    = gtk_object_get_user_data(GTK_OBJECT(hwnd));
	SKINDATA           *skn;
	struct user_record *rec;
	
    if(!node)
       return;	

	skn = icqskin_getDataBlock(cfg->icq);
	rec = gtk_ctree_node_get_row_data(GTK_CTREE(hwnd),node);

    gtk_ctree_node_set_pixtext(  GTK_CTREE(hwnd),
                                 node,
                                 0,
                                 icqQueryUserNick(rec->usr),
                                 4,
                                 skn->iPixmap[rec->usr->modeIcon],
                                 skn->iBitmap[rec->usr->modeIcon]);
 }

 void icqskin_updateUserIcon(hWindow hwnd, ULONG uin)
 {
    updatePixtext(hwnd,queryNode(hwnd,uin));
 }

 HUSER icqskin_queryUserListEntry(hWindow hwnd, ULONG uin)
 {
    struct user_record *rec = queryRecord(hwnd, uin);
	
	if(rec)
	   return rec->usr;

    return NULL;
 }

 HUSER icqskin_updateUserHandle(hWindow hwnd, ULONG uin)
 {
 	ICQULIST           *cfg = gtk_object_get_user_data(GTK_OBJECT(hwnd));
    struct user_record *rec = queryRecord(hwnd, uin);
	
	if(!rec)
	   return NULL;
	
	rec->usr = icqQueryUserHandle(cfg->icq,uin);
    updatePixtext(hwnd,queryNode(hwnd,uin));
	
	return rec->usr;
	
 }

 void icqskin_delUser(hWindow hwnd, ULONG uin)
 {
    GtkCTreeNode *node = queryNode(hwnd,uin);
	
	if(!node)
	   return;
	
	gtk_ctree_remove_node(GTK_CTREE(hwnd),node);
	
 }
 
 static gboolean keyPress(GtkWidget *hwnd, GdkEventKey *event, ICQULIST *cfg)
 {
	if(!cfg->selected)
	   return FALSE;
	
	switch(event->keyval)
	{
	case GDK_space:
       icqShowPopupMenu(cfg->icq,cfg->selected->uin,ICQMNU_USER,0,0);
	   break;
	
	case GDK_F10:
	   CHKPoint();
       icqskin_updateUserHandle(hwnd, cfg->selected->uin);
	   break;
	}

    return FALSE;
 }
