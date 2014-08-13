/*
 * gtkconfig.c - GTK Configuration window Processing
 */

 #include <malloc.h>
 #include <string.h>
 #include <pwicqgui.h>

/*---[ Defines ]------------------------------------------------------------------------------------------*/

 #define CHILD_WINDOW(h,i) icqskin_getDlgItemHandle(h,i)
 #define getNode(h,i)    gtk_ctree_find_by_row_data_custom(GTK_CTREE(h),NULL,(gpointer) &i,recSearch)

/*---[ Structures ]---------------------------------------------------------------------------------------*/

 #pragma pack(1)
 
 typedef struct _record
 {
	USHORT      sz;
    hWindow     hwnd;
    USHORT      id;
 } RECORD;

 #pragma pack()
 
/*---[ Constants ]----------------------------------------------------------------------------------------*/

 static const BUTTONLIST buttonList[] =
 {
#ifdef GTK2
    { MSGID_CLOSEBUTTON, "_Close"         },
    { MSGID_SAVE,        "_Save"          },
	{ MSGID_RELOAD,      "Re_load"        },
    { MSGID_REFRESH,     "_Refresh"       }
#else
    { MSGID_CLOSEBUTTON, "Close"          },
    { MSGID_SAVE,        "Save"           },
	{ MSGID_RELOAD,      "Reload"         },
    { MSGID_REFRESH,     "Refresh"        }
#endif	
 };



/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static void     destroy( GtkWidget *, ICQCONFIGDIALOG *);
 static void     selected(GtkCList *, gint, gint,  GdkEventButton *, hWindow);
 static void     destroy_row(RECORD *);
 static gint     recSearch(gconstpointer, gconstpointer);

 static gboolean expose(GtkWidget *, GdkEventExpose *, ICQCONFIGDIALOG *);

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 hWindow icqskin_createConfigWindow(HICQ icq, ULONG uin, int userBCount, const ICQUSERBUTTON *userButtons)
 {
	SKINDATA        *skn   = icqGetSkinDataBlock(icq);
	ICQCONFIGDIALOG *cfg;
	hWindow         ret;
	hWindow         temp;
	hWindow         vBox;
	hWindow         hBox;
	hWindow         tree;

    cfg = (ICQCONFIGDIALOG *) icqframe_createDataBlock(icq, sizeof(ICQCONFIGDIALOG), "Config" );

	DBGTracex(cfg);
	
    if(!cfg)
	   return 0;

	cfg->fr.cx      = 578;
	cfg->fr.cy      = 475;
    cfg->uin        = uin;
    ret             = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	
	DBGTracex(ret);
	
	if(!ret)
	{
	   free(cfg);
	   return 0;
	}
	
    gtk_object_set_user_data(GTK_OBJECT(ret), (gpointer) cfg);
	
    icqskin_setWindowManagerName(ret,"Config");
	
	CHKPoint();
    icqframe_initialize(ret);
	
	CHKPoint();
	
    gtk_signal_connect( GTK_OBJECT(ret),"destroy",GTK_SIGNAL_FUNC(destroy),cfg );

	vBox = gtk_vbox_new(FALSE,0);
	
	/* Create top bar */
	temp = icqskin_createUserBox(ret,skn,userBCount,userButtons);
    gtk_box_pack_start(GTK_BOX(vBox), temp, FALSE, FALSE, 0);

	/* Monta a tela de dialogo vazio */
    cfg->noDialog = gtk_label_new("Not implemented");
	
	/* Mid box */
    hBox = gtk_hpaned_new();
    icqskin_setDlgItemHandle(ret, MSGID_PANEL, hBox);

    tree = gtk_ctree_new(1,0);
    icqskin_setDlgItemHandle(ret, MSGID_OPTIONS, tree);
    icqskin_setFont(tree, FONT_NORMAL);

    gtk_paned_pack1(GTK_PANED(hBox),tree,TRUE,TRUE);
	
    cfg->dlgBox = gtk_frame_new(NULL);
    gtk_object_set_user_data(GTK_OBJECT(cfg->dlgBox), (gpointer) cfg);
    gtk_signal_connect(GTK_OBJECT(tree),"select-row",GTK_SIGNAL_FUNC(selected),cfg->dlgBox);

    gtk_container_add(GTK_CONTAINER(cfg->dlgBox),cfg->dialog = cfg->noDialog);
    gtk_widget_show_all(cfg->dlgBox);

/*	
  GTK_SHADOW_NONE,
  GTK_SHADOW_IN,
  GTK_SHADOW_OUT,
  GTK_SHADOW_ETCHED_IN,
  GTK_SHADOW_ETCHED_OUT
*/

	gtk_frame_set_shadow_type(GTK_FRAME(cfg->dlgBox),GTK_SHADOW_ETCHED_OUT);
	
    gtk_paned_pack2(GTK_PANED(hBox),cfg->dlgBox,TRUE,TRUE);

    gtk_paned_set_position(GTK_PANED(hBox),120);
	
    temp = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(temp),hBox);
    gtk_box_pack_start(GTK_BOX(vBox), temp, TRUE, TRUE, 0);

	/* Create button bar */
	temp = icqskin_createButtonBar(icq,ret,(sizeof(buttonList)/sizeof(BUTTONLIST)),buttonList);

    gtk_box_pack_end(GTK_BOX(vBox), temp, FALSE, FALSE, 0);

	/* Put the box in the window, show contents */
    gtk_widget_show_all(GTK_WIDGET(vBox));
    gtk_container_add(GTK_CONTAINER(ret), vBox);
	gtk_signal_connect( GTK_OBJECT(ret), "expose_event",  GTK_SIGNAL_FUNC(expose),   cfg );
	
	return ret;
 }

 void icqskin_setWindowUserID(hWindow hwnd, ULONG uin)
 {
    ICQCONFIGDIALOG *cfg = icqskin_getWindowDataBlock(hwnd);
	cfg->uin = uin;
 }

 static void destroy( GtkWidget *hwnd, ICQCONFIGDIALOG *cfg)
 {
	if(cfg->noDialog)
       gtk_widget_destroy(cfg->noDialog);

	if(cfg->gc)
       gdk_gc_unref(cfg->gc);
	
	icqskin_destroyCfgWindow(hwnd);
	icqframe_destroy(hwnd,(ICQFRAME *) cfg);
 }

 static gint recSearch(gconstpointer p1, gconstpointer p2)
 {
    const RECORD *rec = (RECORD *) p1;
    if(rec->sz != sizeof(RECORD))
       return 0;
    return rec->id != * ((USHORT *) p2);
 }

 static void destroy_row(RECORD *rowData)
 {
    if(rowData->hwnd)
       gtk_widget_destroy(rowData->hwnd);
    free(rowData);
 }

 static GtkCTreeNode * insertRecord(HWND hwnd, GtkCTreeNode *node, USHORT id, hWindow dialog, const char *text)
 {
	/* Insert a new node, return it */
	RECORD *rec   = malloc(sizeof(RECORD));
	char   buffer[0x0100];
	char   *txt[] = { buffer };
	char   *ptr;

	if(!rec)
	   return NULL;
	
	strncpy(buffer,text,0xFF);
	for(ptr=buffer;*ptr && *ptr >= ' ';ptr++);
	*ptr = 0;
	
	memset(rec,0,sizeof(RECORD));
	rec->sz   = sizeof(RECORD);
	rec->hwnd = dialog;
	rec->id   = id;
	
    node = gtk_ctree_insert_node(	GTK_CTREE(hwnd),
                                    node,
                                    NULL,
                                    txt,
                                    1,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    FALSE,
                                    FALSE);


    gtk_ctree_node_set_row_data_full(   GTK_CTREE(hwnd),
                                        node,
                                        (gpointer) rec,
                                        ( void (*)(gpointer) ) destroy_row );
	
	
	return node;
	
 } 
 
 int icqskin_insertConfigPage(hWindow hwnd, USHORT parent, hWindow dialog, USHORT id, const char *text)
 {
    GtkCTreeNode *node = NULL;
	
	if(!text)
	   return -1;
	
	DBGMessage(text);
	
	hwnd = CHILD_WINDOW(hwnd,MSGID_OPTIONS);

    if(parent)
       node = getNode(hwnd,parent);

    insertRecord(hwnd,node,id,dialog,text);
	
	return 0;
 }	

 void icqconfig_setWindowType(hWindow hwnd, USHORT type)
 {
	
 }

 void icqconfig_initialize(hWindow hwnd)
 {
	
 }

 void icqconfig_load(hWindow hwnd)
 {
	
 }
 
 void icqconfig_save(hWindow hwnd)
 {
	
 }
 
 static void selected(GtkCList *clist, gint row, gint column,  GdkEventButton *event, hWindow hwnd)
 {
    ICQCONFIGDIALOG *cfg   = icqskin_getWindowDataBlock(hwnd);
    RECORD          *rec   = gtk_clist_get_row_data(clist,row);
	hWindow         dialog;
	
	DBGTracex(cfg);
	DBGTracex(cfg->dialog);
	
	DBGTracex(rec);
	DBGTrace(rec->id);
	DBGTrace(rec->hwnd);
	
	dialog = rec->hwnd ? rec->hwnd : cfg->noDialog;
	
	DBGTracex(dialog);
	
	if(cfg->dialog != dialog)
	{
	   DBGMessage("Changing dialog box");
   	   if(cfg->dialog)
	   {
	      gtk_widget_hide(cfg->dialog);
	      g_object_ref(cfg->dialog);
	      gtk_container_remove(GTK_CONTAINER(hwnd),cfg->dialog);
	   }
	   gtk_container_add(GTK_CONTAINER(hwnd),cfg->dialog = dialog);
	   gtk_widget_show(cfg->dialog);
    }

	
 }
 
 static gboolean expose(GtkWidget *hwnd, GdkEventExpose *event, ICQCONFIGDIALOG *cfg)
 {
    if(!cfg->gc)
       cfg->gc = gdk_gc_new(hwnd->window);
    return FALSE;
 }
	
