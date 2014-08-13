/*
 * gtkbutton.c - GTK Implementation of pwICQGUI Button system
 */

 #include <pwicqgui.h>
 #include <stdlib.h>
 #include <malloc.h>
 #include <string.h>
 #include <dlfcn.h>
 
 #include <pwicqrc.h>

/*---[ Structures ]----------------------------------------------------------*/

 #pragma pack(1)

 typedef struct _icqdialog
 {
    ICQFRAME_STANDARD_PREFIX
	
	USHORT                           elements;
    const struct icqDialogDescriptor *ds;
    USHORT       					 szDataBlock;
	hWindow                          owner;
    const DLGMGR *					 mgr;
	USHORT                           cx;
	USHORT                           cy;

 } ICQDIALOG;

 struct dlgcreate
 {
	unsigned short sz;
 };

 #pragma pack()
 
/*---[ Object macros ]--------------------------------------------------------*/

 #define DECLARE_WINDOW_CONSTRUCTOR(e) static hWindow dialog_##e(const char *, unsigned long, struct dlgcreate *)
 #define WINDOW_CONSTRUCTOR(e)         { e, dialog_##e }
 #define BEGIN_WINDOW_CONSTRUCTOR(e)   static hWindow dialog_##e(const char *title, unsigned long style, struct dlgcreate *info) {
 #define END_WINDOW_CONSTRUCTOR(x,h)   return h; }
 
/*---[ Object constructors ]--------------------------------------------------*/

 DECLARE_WINDOW_CONSTRUCTOR(WC_STATIC);
 DECLARE_WINDOW_CONSTRUCTOR(WC_BUTTON);
 DECLARE_WINDOW_CONSTRUCTOR(WC_ENTRYFIELD);
 
 #pragma pack(1)
 
 static const struct wdgCreate
 {
	ULONG   className;
	hWindow (*create)(const char *,unsigned long, struct dlgcreate *); 
 }
 wdgTable[] =
 {
	WINDOW_CONSTRUCTOR(WC_STATIC),
	WINDOW_CONSTRUCTOR(WC_BUTTON),
	WINDOW_CONSTRUCTOR(WC_ENTRYFIELD)
 };
 
 
 #pragma pack()

/*---[ Constants ]------------------------------------------------------------*/
 
 
/*---[ Prototipes ]-----------------------------------------------------------*/

 static void icqdialog_destroy( hWindow, ICQDIALOG *);

 static gboolean expose(GtkWidget *, GdkEventExpose *, ICQDIALOG *);

/*---[ Implementation ]-------------------------------------------------------*/

 void icqskin_setDialogManager(hWindow hwnd, const struct dlgmgr *mgr)
 {
	
 }
 
 static hWindow createObject(const struct icqDialogDescriptor *dlg, struct dlgcreate *info)
 {
	int f;
	
	for(f=0;f < (sizeof(wdgTable)/sizeof(struct wdgCreate)); f++)
    {
	   if(wdgTable[f].className == dlg->className)
		  return wdgTable[f].create(dlg->title,dlg->flags, info);
    }	   
	
    DBGTracex(dlg->className);
    DBGTrace(dlg->id);
    DBGTrace(dlg->x);
    DBGTrace(dlg->y);
    DBGTrace(dlg->cx);
    DBGTrace(dlg->cy);

	return 0;
 }	

 hWindow icqskin_loadDialog(HICQ icq, hWindow owner, HMODULE hMod, USHORT id, const DLGMGR *mgr)
 {
    int                              qtd    = 0;
	int                              cxMax  = 0;
	int                              cyMax  = 0;
	
	const struct icqDialogDescriptor *dlg = icqGetDialogDescriptor(icq,hMod,id,qtd);

    hWindow                          h;	
	int                              f;
	int                              tmp;

	unsigned short                   x;
	unsigned short                   y;
	unsigned short                   cx     = 0;
	unsigned short                   cy     = 0;

    hWindow                          hwnd   = 0;	 
#ifdef GTK2
    GtkRequisition                   sizereq;
#endif	

    ICQDIALOG	                     *cfg;
    struct dlgcreate				 info;

	memset(&info,0,sizeof(struct dlgcreate));
	
	if(!dlg)
	   return 0;

#ifdef DEBUG

	f   = sizeof(ICQDIALOG)+mgr->userDataSize;

    cfg = (ICQDIALOG *)  icqframe_createDataBlock(icq, f, "Dialog");
	if(!cfg)
	   return 0;
	
	cfg->ds          = dlg;
	cfg->owner       = owner;
	cfg->szDataBlock = mgr->userDataSize;
	cfg->mgr         = mgr;
    cfg->elements    = qtd;

	for(f=0;f<qtd;f++)
	{
	
       tmp =dlg[f].x + dlg[f].cx;	   
	   if(tmp > cxMax)
		  cxMax = tmp;
	   
       tmp =dlg[f].y + dlg[f].cy;	   
	   if(tmp > cyMax)
		  cyMax = tmp;
	}
	
    cfg->cy = cyMax = cyMax<<1;
    cfg->cx = cxMax = cxMax+(cxMax>>1);

	DBGTrace(cxMax);
	DBGTrace(cyMax);

    hwnd = gtk_fixed_new();

#ifdef GTK1
	gtk_widget_set_usize(hwnd,cfg->cx,cfg->cy);
#endif
	
#ifdef GTK2
    gtk_widget_set_size_request(hwnd,cfg->cx,cfg->cy);
#endif	

    if(!hwnd)
    {
	   icqWriteSysLog(icq,PROJECT,"Failure creating table layout");
	   free(cfg);
	   return 0;
    }	   

    gtk_object_set_user_data(GTK_OBJECT(hwnd), (gpointer) cfg);
    gtk_signal_connect(GTK_OBJECT(hwnd),"destroy",GTK_SIGNAL_FUNC(icqdialog_destroy), cfg);
	
	for(f=0;f<qtd;f++)
    {
	   x  = dlg->x+(dlg->x>>1);
	   cx = dlg->cx+(dlg->cx>>1);

	   y  = dlg->y<<1;
	   cy = dlg->cy<<1;

	   if(dlg->className == WC_STATIC && (dlg->flags & SS_GROUPBOX)) 
	   {
          h = NULL;
	   }
	   else
	   {
          h = createObject(dlg,&info);  
#ifdef GTK2
	      gtk_widget_size_request(h,&sizereq);
	      gtk_widget_set_size_request(h,cx,sizereq.height);
#endif	
       }

       if(h)
       {
          gtk_object_set_user_data(GTK_OBJECT(h), (gpointer) cfg);
          icqgui_setWindowID(owner,dlg->id);
#ifdef GTK2
	      gtk_widget_size_request(h,&sizereq);
	      gtk_widget_set_size_request(h,cx,sizereq.height);
#endif	
	      gtk_fixed_put(GTK_FIXED(hwnd),h,x,cfg->cy-y);
		  
       }
       else
       {
	      icqPrintLog(icq,PROJECT,"Error creating dialog element class %08lx",dlg->className);
       }		  

       dlg++;
    }

	gtk_signal_connect( GTK_OBJECT(hwnd), "expose_event",  GTK_SIGNAL_FUNC(expose),   cfg );
    gtk_widget_show_all(GTK_WIDGET(hwnd));	
	
#endif
	
    return hwnd;
 }

 static void icqdialog_destroy( hWindow hwnd, ICQDIALOG *cfg)
 {
	if(!cfg->sz)
	   return;

	DBGPrint("Dialog \"%s\" destroyed",cfg->name);
	

	cfg->sz = 0;
	free(cfg);
 }

 static gboolean expose(GtkWidget *hwnd, GdkEventExpose *event, ICQDIALOG *cfg)
 {
    ICQCONFIGDIALOG *parent = gtk_object_get_user_data(GTK_OBJECT(cfg->owner));
	int             f;
	gint            x,y,cx,cy;

    DBGTracex(parent);

	if(!parent)
	   return FALSE;

	DBGTracex(parent->gc);
	
	
/*	
 gdk_draw_line                   (GdkDrawable *drawable,
                                             GdkGC *gc,
                                             gint x1_,
                                             gint y1_,
                                             gint x2_,
                                             gint y2_);	
    gdk_draw_line(	hwnd->window,
                    parent->gc,
                    0,0,
					300,300 );
*/

	for(f=0;f<cfg->elements;f++)
	{
	   if(cfg->ds[f].className == WC_STATIC && (cfg->ds[f].flags & SS_GROUPBOX))
	   {
	      x  = cfg->ds[f].x+(cfg->ds[f].x>>1);
	      cx = cfg->ds[f].cx+(cfg->ds[f].cx>>1);

	      y  = cfg->cy - (cfg->ds[f].y<<1);
	      cy = cfg->ds[f].cy<<1;

          gtk_widget_translate_coordinates(     hwnd,
		  										gtk_widget_get_toplevel(hwnd),
                                                x, y,
                                                &x,&y );
		  
//          gdk_draw_line(gtk_widget_get_toplevel(hwnd)->window,parent->gc,x,y,x+cx,y);
		  
	   }
	}

    return FALSE;
 } 
/*---[ Window constructors ]--------------------------------------------------*/

 BEGIN_WINDOW_CONSTRUCTOR(WC_STATIC)

    hWindow hwnd = NULL;
 
    if(style & SS_TEXT)
    {	   
       hwnd = gtk_label_new(title);
       gtk_misc_set_alignment(GTK_MISC(hwnd), 0, .5);
       icqskin_setFont(hwnd, FONT_NORMAL);
    }	   

 END_WINDOW_CONSTRUCTOR(WC_STATIC,hwnd)
 
 BEGIN_WINDOW_CONSTRUCTOR(WC_ENTRYFIELD)
	
    hWindow hwnd = gtk_entry_new();
    icqskin_setFont(hwnd, FONT_NORMAL);
 
 END_WINDOW_CONSTRUCTOR(WC_ENTRYFIELD,hwnd)
 
 
 BEGIN_WINDOW_CONSTRUCTOR(WC_BUTTON)
	
    hWindow hwnd   = NULL;
	hWindow label;

    if(style & (BS_CHECKBOX|BS_AUTOCHECKBOX) )
    {	   
       hwnd = gtk_check_button_new_with_label(title);
    }	
    else if(style & (BS_RADIOBUTTON|BS_AUTORADIOBUTTON) )
    {	   
       hwnd = gtk_radio_button_new_with_label(NULL,title);
	}	   
 	else
	{
       hwnd = gtk_button_new_with_label(title);
	}
	
    label = gtk_bin_get_child(GTK_BIN(hwnd));
	
	if(label)
	{
       gtk_misc_set_alignment(GTK_MISC(label),0,.5);
       icqskin_setFont(label, FONT_NORMAL);
	}
	   
 END_WINDOW_CONSTRUCTOR(WC_BUTTON,hwnd)
 
