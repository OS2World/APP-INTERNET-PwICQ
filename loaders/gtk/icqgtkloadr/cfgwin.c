/*
 * cfgwin.c - Janela principal de configuracao
 */

 #include <malloc.h>
 #include <stdio.h>
 #include <string.h>
 #include <icqgtk.h>
 
 #include "mainconfig_bitmap.xpm"

 #define USE_PANED

/*---[ Defines ]--------------------------------------------------------------------------------------------*/

 #pragma pack(1)
 struct cfg
 {
    USHORT   	sz;
    HICQ	     icq;
    ULONG		uin;
    USHORT	   type;

    GtkWidget    *main;
    GtkWidget	*logo;
    GtkWidget    *title;
    GtkWidget    *container;
    GtkWidget    *page;
    GtkWidget	*selected;
    
    GtkWidget	*ok;
    GtkWidget	*cancel;
    
 };
 
 typedef struct rowdata
 {
    USHORT	   sz;
    UCHAR		tag;
    const DLGMGR *mgr;
    BOOL		 isTable;
    GtkWidget	*widget;
    const char   *str;
 } ROWDATA;

/*---[ Constants ]------------------------------------------------------------------------------------------*/

 static const char *stringTable[] = { 	"Network",	   // CFGWIN_NETWORK
 										 "Mode",	      // CFGWIN_MODE
		       						   "Multimedia",    // CFGWIN_MULTIMEDIA
                                          "Users",         // CFGWIN_USERS
                                          "Options",       // CFGWIN_OPTIONS
                                          "Security",      // CFGWIN_SECURITY
                                          "Miscellaneous", // CFGWIN_MISC
                                          "Advanced",      // CFGWIN_ADVANCED        
                                          "About",         // CFGWIN_ABOUT           
                                          "Information",   // CFGWIN_INFO            
                                          "Events"         // CFGWIN_EVENTS          
                                     };
                                     
/*---[ Prototipes ]-----------------------------------------------------------------------------------------*/

 static void      destroy( GtkWidget *, struct cfg *);
 static GtkWidget *aboutBox(struct cfg *);
 static void      destroy_row(ROWDATA *);
 static void      selectPage(GtkWidget *, GtkCTreeNode *, gint, struct cfg *);
 static int       newPage(struct cfg *, WIDGET, USHORT, const DLGMGR *, const char *, BOOL);

 static int  _System insertPage(struct cfg *, WIDGET, USHORT, const DLGMGR *, const char *);
 static int  _System setTitle(struct cfg *, const char *);
 static int  _System insertTable(HWND,USHORT,const TABLEDEF *, const DLGMGR *, const char *);

 static void _System event(HICQ, ULONG, char, USHORT, ULONG, struct cfg *);

 static void doSave(GtkWidget *, struct cfg *);
 static void doCancel(GtkWidget *, struct cfg *);

/*---[ Callback tables ]------------------------------------------------------------------------------------*/ 

 static const DLGINSERT dlgInsert = {	sizeof(DLGINSERT),
 									    (int (*)(HWND, WIDGET, USHORT, const DLGMGR *, const char *)) insertPage,
 									    insertTable,
 									    (int (*)(HWND,const char *)) setTitle
 								   };
 								   
/*---[ Implement ]------------------------------------------------------------------------------------------*/

 int _System icqgtk_OpenConfigWindow(HICQ icq, ULONG uin, USHORT type, PAGELOADER *loader)
 {
    char		   buffer[0x0100];
    GtkWidget	  *box;
    GtkWidget	  *hBox;
    GtkWidget	  *vBox;
    GtkWidget	  *temp;
    struct cfg 	*wnd 	 = malloc(sizeof(struct cfg));

    DBGTrace(type);
    
    if(!wnd)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when opening configuration window");
       return -1;
    }

    memset(wnd,0,sizeof(struct cfg));
    wnd->sz   = sizeof(struct cfg);
    wnd->icq  = icq; 
    wnd->uin  = uin;
    wnd->type = type;

    wnd->main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    DBGTracex(wnd->main);
    
//    wnd->main = gtk_window_new(GTK_WINDOW_DIALOG);
    
    sprintf(buffer,"ICQ#%lu",icqQueryUIN(icq));
    sprintf(buffer+0x80,"cfg%04x",type);
    
    gtk_window_set_wmclass(GTK_WINDOW(wnd->main),buffer+0x80, buffer);
    
    gtk_signal_connect(GTK_OBJECT(wnd->main), "destroy", GTK_SIGNAL_FUNC(destroy), wnd);
    gtk_widget_realize(wnd->main);

#ifdef USE_PANED
    box	   = gtk_hpaned_new();
#else
    box       = gtk_hbox_new(FALSE,2);
#endif

    /* Monta o lado esquerdo */
    wnd->container = gtk_ctree_new(1,0);
    gtk_widget_set_usize(wnd->container,100,100);
    
#ifdef USE_PANED
    gtk_paned_add1(GTK_PANED(box),wnd->container);
#else
    gtk_box_pack_start(GTK_BOX(box), wnd->container,TRUE,TRUE,0);
#endif    
    
    /* Monta o lado direito  */
    
    vBox = gtk_vbox_new(FALSE,2);
    
    /* Area de titulo */

    temp = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(temp),GTK_SHADOW_IN);

    wnd->title  = gtk_label_new("");
    gtk_misc_set_alignment(GTK_MISC(wnd->title), 0, .5);

    gtk_container_add(GTK_CONTAINER(temp), wnd->title);    
    
    gtk_box_pack_start(GTK_BOX(vBox), temp, FALSE, FALSE, 2);    

    /* Paginas de configuracao */
    
    wnd->page   = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(wnd->page),GTK_SHADOW_NONE);

    gtk_box_pack_start(GTK_BOX(vBox), wnd->page, FALSE, FALSE, 2);    
    
    wnd->ok     = gtk_button_new_with_label(" Ok ");
    wnd->cancel = gtk_button_new_with_label(" Cancel ");

    gtk_signal_connect(GTK_OBJECT(wnd->ok),      "clicked", GTK_SIGNAL_FUNC(doSave), wnd);
    gtk_signal_connect(GTK_OBJECT(wnd->cancel),  "clicked", GTK_SIGNAL_FUNC(doCancel), wnd);

    hBox = gtk_hbox_new(FALSE,2);
    gtk_box_pack_end(GTK_BOX(hBox),wnd->cancel,FALSE,FALSE,0);
    gtk_box_pack_end(GTK_BOX(hBox),wnd->ok,FALSE,FALSE,0);

    gtk_box_pack_end(GTK_BOX(vBox), hBox, FALSE, FALSE, 0);    
     
#ifdef USE_PANED
    gtk_paned_add2(GTK_PANED(box),vBox);
#else
    gtk_box_pack_end(GTK_BOX(box), vBox,FALSE,FALSE,0);
#endif
    
    /* Finaliza */        
    gtk_widget_show_all(box);
    gtk_container_add(GTK_CONTAINER(wnd->main), box);

    /* Chama o loader para configurar a janela */

    if(loader)
    {
       loader(icq,uin,type,wnd,&dlgInsert);
    }
    else
    {
       switch(type)
       {
       case 0:
          icqLoadString(icq,"nickname","John Doe",buffer,40);
          sprintf(buffer+strlen(buffer),"'s (ICQ#%lu) configuration",icqQueryUIN(icq));
          setTitle(wnd,buffer);
          break;
          
       case 1:
          break;
          
       default:
          setTitle(wnd,"pwICQ Configuration");
       }
    }
       
    /* Deixa os plugins inserirem suas paginas */

    icqInsertConfigPage(icq, uin, type, (HWND) wnd, &dlgInsert,buffer);

#ifndef __DLL__
    icqgtk_ConfigPage(icq, NULL, uin, type, (HWND) wnd, &dlgInsert,buffer);
#endif

    /* Conecta eventos */
    gtk_signal_connect(GTK_OBJECT(wnd->container), "tree-select-row", GTK_SIGNAL_FUNC(selectPage), wnd);

    /* Restaura posicao e apresenta */

    if(!wnd->logo)
       wnd->logo = aboutBox(wnd);
       
    gtk_container_add(GTK_CONTAINER(wnd->page),wnd->logo);

    gtk_widget_show_all(wnd->main);
//    gtk_widget_show(wnd->main);

    icqgtk_insertListener(wnd->icq, (void (_System *)(HICQ,ULONG,char,USHORT,ULONG,void *)) event, wnd);

    return 0; 
 }

 static void destroy( GtkWidget *widget, struct cfg *wnd)
 {
    DBGMessage("Janela de configuracao destruida");
    
    if(wnd->sz != sizeof(struct cfg))
       return;

    icqgtk_removeListener(wnd->icq, (void (_System *)(HICQ,ULONG,char,USHORT,ULONG,void *)) event, wnd);

    memset(wnd,0,sizeof(struct cfg));    
    free(wnd);
 }

 struct nodeevent
 {
    HICQ   icq;
    ULONG  uin;
    char   type;
    USHORT eventCode;
    ULONG  parm;
 };

 static void nodeEvent(GtkCTree *tree, GtkCTreeNode *node, struct nodeevent *event)
 {
    ROWDATA *rowData = gtk_ctree_node_get_row_data(tree,node);
    char    buffer[0x0100];
    
    if(!(rowData &&  rowData->sz == sizeof(ROWDATA) && rowData->widget))
       return;
       
    if(rowData->mgr && rowData->mgr->event)
    {
       rowData->mgr->event(	&icqgtk_tableHelper,
       						rowData->widget,
       						event->icq,
       						event->uin,
       						event->type,
       						event->eventCode,
       						buffer);
    }       						
       						
 } 

 static void _System event(HICQ icq, ULONG uin, char type, USHORT eventCode, ULONG parm, struct cfg *wnd)
 {
    /* Processa eventos */
    struct nodeevent d;

    d.icq       = icq;    
    d.uin       = uin;
    d.type      = type;
    d.eventCode = eventCode;
    d.parm      = parm;

    gtk_ctree_post_recursive(GTK_CTREE(wnd->container),NULL,GTK_CTREE_FUNC(nodeEvent),&d);
    
 }

 static int _System insertTable(HWND hwnd, USHORT tab, const TABLEDEF *def, const DLGMGR *dlg, const char *title)
 {
    WIDGET table;

    if(dlg && dlg->sz != sizeof(DLGMGR))
       return -2;

    DBGMessage(title);    
    table = icqgtk_MakeTable(((struct cfg *) hwnd)->icq, ((struct cfg *) hwnd)->uin, def, dlg);
    
    if(!table)
       return -1;
    
    return newPage( (struct cfg *) hwnd, table, tab, dlg, title, TRUE);
 }

 static int _System insertPage(struct cfg *wnd, WIDGET dlg, USHORT tab, const DLGMGR *mgr, const char *title)
 {
    if(mgr && mgr->sz != sizeof(DLGMGR))
       return -2;
       
    return newPage(wnd, dlg, tab, mgr, title, FALSE);
 }

 static gint treeSearch(gconstpointer p1, gconstpointer p2)
 {
    const ROWDATA *rowData = (ROWDATA *) p1;
    if(rowData->sz != sizeof(ROWDATA))
       return 0;
    return rowData->str != p2;
 }
 
 static int newPage(struct cfg *wnd, WIDGET dlg, USHORT tab, const DLGMGR *mgr, const char *title, BOOL isTable)
 {
    /* Insere uma pagina na caixa */ 
    ROWDATA	  *rowData;
    char		 *texto[1];
    char		 buffer[80];
    char		 *ptr;
    GtkCTreeNode *node;
    
    if(tab == 0xFFFF)
    {
       if(!wnd->logo)
       {
          DBGMessage("Inserir pagina logo");
          wnd->logo = dlg;
          gtk_widget_show_all(dlg);
          return 0;
       }
       return -1;
    }

    node = gtk_ctree_find_by_row_data_custom(GTK_CTREE(wnd->container),NULL,(gpointer) stringTable[tab], treeSearch);

    if(!node)
    {
       texto[0] = (char *) stringTable[tab];
       node = gtk_ctree_insert_node(	GTK_CTREE(wnd->container), 
       								 NULL, 
       								 NULL, 
       								 texto, 
       								 0, 
       								 NULL, 
       								 NULL, 
       								 NULL, 
       								 NULL, 
       								 FALSE, 
       								 FALSE);

       rowData = malloc(sizeof(ROWDATA));
       DBGTracex(rowData);
       
       if(!rowData)
       {
          icqWriteSysLog(wnd->icq,PROJECT,"Memory allocation error when adding page");
          return -1;
       }
       memset(rowData,0,sizeof(ROWDATA));
       rowData->sz     = sizeof(ROWDATA);
       rowData->str    = stringTable[tab];
       								 
       gtk_ctree_node_set_row_data_full(   GTK_CTREE(wnd->container),
                                           node,
                                           (gpointer) rowData,
                                           ( void (*)(gpointer) ) destroy_row );

       gtk_ctree_node_set_selectable (     GTK_CTREE(wnd->container),
                                           node,
                                           FALSE);

    }
    
    if(!dlg)
       return 0;
    
    /* Tem dialogo para apresentar, anexa */

    strncpy(buffer,title,79);
    for(ptr = buffer;*ptr && *ptr >= ' ';ptr++);

    if(*ptr)
       *(ptr++) = 0;
    else
       *ptr=0;
    
    rowData = malloc(sizeof(ROWDATA)+1+strlen(ptr));
    
    if(!rowData)
    {
       icqWriteSysLog(wnd->icq,PROJECT,"Memory allocation error when adding page");
       return -1;
    }

    DBGMessage(ptr);
    
    strcpy( (char *)(rowData+1),ptr);
    
    memset(rowData,0,sizeof(ROWDATA));
    rowData->sz      = sizeof(ROWDATA);
    rowData->widget  = dlg;
    rowData->isTable = isTable;
    rowData->mgr     = mgr;
    
    texto[0] = buffer;
    node = gtk_ctree_insert_node(	GTK_CTREE(wnd->container),
                                     node,
                                     NULL,  //        GtkCTreeNode *sibling,
                                     texto, //        gchar *text[],
                                     1,     //        guint8 spacing,
       							  NULL, 
       							  NULL, 
       							  NULL, 
       							  NULL, 
       							  FALSE, 
       							  FALSE);


    DBGTracex(node);
    
    gtk_ctree_node_set_row_data_full(   GTK_CTREE(wnd->container),
                                        node,
                                        (gpointer) rowData,
                                        ( void (*)(gpointer) ) destroy_row );
                                        
    gtk_widget_ref(dlg);

    return 0;    
 }

 static void selectPage(GtkWidget *ctree, GtkCTreeNode *node, gint column, struct cfg *wnd)
 {
    ROWDATA *rowData = gtk_ctree_node_get_row_data(GTK_CTREE(ctree),node);
    
    if(!rowData || !rowData->widget)
       return;
       
    if(rowData->widget == wnd->selected)
       return;
       
    if(wnd->selected)
    {    
       DBGMessage("Remover pagina selecionada");
       gtk_widget_hide(wnd->selected);
       gtk_container_remove (GTK_CONTAINER(wnd->page),wnd->selected);
    }
    else if(wnd->logo)
    {
       DBGMessage("Remover e destruir pagina logo");
       gtk_container_remove (GTK_CONTAINER(wnd->page),wnd->logo);
       wnd->logo = NULL;
    }

    DBGMessage("Anexar uma nova pagina");
    wnd->selected = rowData->widget;
    gtk_widget_ref(wnd->selected);
    gtk_container_add(GTK_CONTAINER(wnd->page),wnd->selected);

    gtk_label_set_text(GTK_LABEL(wnd->title),(char *)(rowData+1));
    
    gtk_widget_show_all(wnd->selected);

 }

 static void destroy_row(ROWDATA *rowData)
 {
    if(rowData->widget)
       gtk_widget_destroy(rowData->widget);
    free(rowData);
 }

 static int _System setTitle(struct cfg *wnd, const char *title)
 {
    gtk_window_set_title(GTK_WINDOW(wnd->main),title);
    return 0;
 }

 static GtkWidget *aboutBox(struct cfg *wnd)
 {
    GdkPixmap *image;
    GtkWidget *ret = gtk_vbox_new(FALSE,2);
    GtkWidget *hwnd = wnd->main;

    image = gdk_pixmap_create_from_xpm_d(hwnd->window,NULL,NULL,mainconfig_bitmap_xpm);
    gtk_box_pack_start(GTK_BOX(ret), gtk_pixmap_new(image,NULL), FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(ret), gtk_label_new("Code by Perry Werneck"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(ret), gtk_label_new("(pwicq@terra.com.br - ICQ#27241234)"),FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(ret), gtk_label_new("Graphics,Scripts & Research by Dr. Slaughter"),FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(ret), gtk_label_new("(slaughter@malaconet.org)"),FALSE, FALSE, 0);

    gtk_label_set_text(GTK_LABEL(wnd->title),"About pwICQ " PWICQVERSION);

    return ret;
 }

 static void doSave(GtkWidget *wdg, struct cfg *wnd)
 {
    int 		row = 0;
    ROWDATA	 *rowData;
    char		buffer[0x0100];
    
    DBGMessage("Save");

    rowData = gtk_clist_get_row_data(GTK_CLIST(wnd->container),row++);
    while(rowData)
    {
       if(rowData->mgr && rowData->mgr->save)
          rowData->mgr->save(&icqgtk_tableHelper,rowData->widget,wnd->icq,wnd->uin, buffer);
       rowData = gtk_clist_get_row_data(GTK_CLIST(wnd->container),row++);
    }

    DBGTrace(row);    
    gtk_widget_destroy(wnd->main);
 }
 
 static void doCancel(GtkWidget *wdg, struct cfg *wnd)
 {
    int 		row = 0;
    ROWDATA	 *rowData;
    char	    buffer[0x0100];
    
    DBGMessage("Cancel");

    rowData = gtk_clist_get_row_data(GTK_CLIST(wnd->container),row++);
    while(rowData)
    {
       if(rowData->mgr && rowData->mgr->cancel)
          rowData->mgr->cancel(&icqgtk_tableHelper,rowData->widget,wnd->icq,wnd->uin,buffer);
       rowData = gtk_clist_get_row_data(GTK_CLIST(wnd->container),row++);
    }

    DBGTrace(row);    
    gtk_widget_destroy(wnd->main);
 }

