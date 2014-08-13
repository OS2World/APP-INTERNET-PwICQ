/*
 * gtkframe.c - Skkinable frame window for GTK2
 *
 * Reference: http://www.mit.edu/afs/athena/contrib/mp3/src/xmms-solaris/wmxmms/wmxmms.c
 *
 */

 #include <malloc.h>
 #include <string.h>
 #include <stdlib.h>

 #include <pwicqgui.h>
 #include <gdk-pixbuf/gdk-pixbuf.h>

/*---[ Defines ]--------------------------------------------------------------*/

 #define max(a,b) a > b ? a : b
 #define IDMASC_SIZE 20
 #define VALIDATE_WINDOW(h,c) ICQFRAME *c = gtk_object_get_user_data(GTK_OBJECT(h));if(c->sz < sizeof(ICQFRAME) || !c->msg) return 0;

 #define TOP_BORDER_SIZE    5
 #define LEFT_BORDER_SIZE   5
 #define BOTTOM_BORDER_SIZE 5
 #define RIGHT_BORDER_SIZE  5

// #ifdef DEBUG 
// #define BEGIN_RESIZE(x) DBGMessage( #x );gtk_window_begin_resize_drag(GTK_WINDOW(ancestor),x,event->button,event->x_root, event->y_root,event->time)
// #else
 #define BEGIN_RESIZE(x) gtk_window_begin_resize_drag(GTK_WINDOW(widget),x,event->button,event->x_root, event->y_root,event->time)
// #endif 
	   

/*---[ Structures ]-----------------------------------------------------------*/

 #pragma pack(1)
 
 typedef struct _childsearch
 {
	USHORT  sz;
	char    key[IDMASC_SIZE];
	hWindow ret;
 } CHILDSEARCH;

 #pragma pack()

/*---[ Constants ]------------------------------------------------------------*/
 
 static const char *id_masc = "pwICQ%04x";
 
/*---[ Prototipes ]-----------------------------------------------------------*/

 static void clean( GtkWidget *, ICQFRAME *);
 
#ifdef SKINNED_GUI	

 static gboolean icqframe_expose(  GtkWidget *, GdkEventExpose *, ICQFRAME *);
 static gboolean button_press(GtkWidget *, GdkEventButton *, ICQFRAME *);
 static void     icqframe_createBackground(GtkWidget *, ICQFRAME *, int, int);
 
#endif 

  static void     resize(  GtkWidget *, GtkAllocation  *, ICQFRAME *);

/*---[ Implementation ]-------------------------------------------------------*/

 ICQFRAME * EXPENTRY icqframe_createDataBlock(HICQ icq, int sz, const char *name)
 {
	ICQFRAME *cfg          = malloc(sz);
	
	if(!cfg)
	   return cfg;

	memset(cfg,0,sz);
	cfg->sz   = sz;
	cfg->msg  = icqFrameWindow;
	cfg->icq  = icq;
	cfg->name = name;
	
	return cfg;
 }
 
 hWindow EXPENTRY icqframe_new(HICQ icq, const char *name, int cx, int cy, BOOL top, BOOL border)
 {
	ICQFRAME *cfg = icqframe_createDataBlock(icq, sizeof(ICQFRAME), name);
	hWindow  ret;
	
	if(!cfg)
	   return NULL;

    cfg->cx = cx;
    cfg->cy = cy;	
	
    if(border)
	   cfg->flags |= ICQFRAME_FLAG_BORDER;
	
	if(top)
	   cfg->flags = ICQFRAME_FLAG_TOPWINDOW;
	
	ret = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	
	if(!ret)
	{
	   free(cfg);
	   return ret;
	}

    gtk_object_set_user_data(GTK_OBJECT(ret), (gpointer) cfg);
	icqframe_initialize(ret);
	
    gtk_signal_connect( GTK_OBJECT(ret),"destroy",GTK_SIGNAL_FUNC(icqframe_destroy),cfg );
	
	return ret;
	
 }

 void EXPENTRY icqframe_initialize(hWindow ret)
 {
	ICQFRAME *cfg = gtk_object_get_user_data(GTK_OBJECT(ret));
	
	DBGTracex(cfg);
	
#ifdef SKINNED_GUI	
	
    cfg->base[ICQFRAME_TITLELEFT]      = gdk_pixbuf_new_from_file("titleleft.gif",NULL);
    cfg->base[ICQFRAME_TITLESTRETCH]   = gdk_pixbuf_new_from_file("titlestretch.gif",NULL);
    cfg->base[ICQFRAME_TITLERIGHT]     = gdk_pixbuf_new_from_file("titleright.gif",NULL);

    cfg->base[ICQFRAME_TOPLEFT]        = gdk_pixbuf_new_from_file("topleft.gif",NULL);
    cfg->base[ICQFRAME_TOPRIGHT]       = gdk_pixbuf_new_from_file("topright.gif",NULL);

    cfg->base[ICQFRAME_MIDLEFT]        = gdk_pixbuf_new_from_file("midleft.gif",NULL);
    cfg->base[ICQFRAME_MIDSTRETCH]     = gdk_pixbuf_new_from_file("midstretch.gif",NULL);
    cfg->base[ICQFRAME_MIDRIGHT]       = gdk_pixbuf_new_from_file("midright.gif",NULL);
	
    cfg->base[ICQFRAME_BOTTOMLEFT]     = gdk_pixbuf_new_from_file("bottomleft.gif",NULL);
    cfg->base[ICQFRAME_BOTTOMMIDLEFT]  = gdk_pixbuf_new_from_file("bottommidleft.gif",NULL);
    cfg->base[ICQFRAME_BOTTOMSTRETCH]  = gdk_pixbuf_new_from_file("bottomstretch.gif",NULL);
    cfg->base[ICQFRAME_BOTTOMMIDRIGHT] = gdk_pixbuf_new_from_file("bottommidright.gif",NULL);
    cfg->base[ICQFRAME_BOTTOMRIGHT]    = gdk_pixbuf_new_from_file("bottomright.gif",NULL);
	
    gtk_signal_connect(GTK_OBJECT(ret),  "expose_event",       GTK_SIGNAL_FUNC(icqframe_expose), cfg);
    gtk_signal_connect( GTK_OBJECT(ret), "size-allocate",      GTK_SIGNAL_FUNC(resize),cfg );

    if(cfg->flags & ICQFRAME_FLAG_BORDER)
	{
	   gtk_window_set_decorated(GTK_WINDOW(ret),TRUE);
	}
	else
	{
	   gtk_window_set_decorated(GTK_WINDOW(ret),FALSE);
  	   gtk_widget_add_events(ret,GDK_BUTTON_PRESS_MASK);
       gtk_signal_connect(GTK_OBJECT(ret),  "button-press-event", GTK_SIGNAL_FUNC(button_press), cfg);
    }	   

    icqframe_createBackground(ret,cfg,cfg->cx,cfg->cy);

	gtk_widget_set_app_paintable(ret,TRUE);

#else
    gtk_signal_connect( GTK_OBJECT(ret), "size-allocate",      GTK_SIGNAL_FUNC(resize),cfg );
#endif	

#ifdef GTK2	
	gtk_widget_set_size_request(ret,cfg->cx,cfg->cy);
#else
    gtk_window_set_default_size(GTK_WINDOW(ret),cfg->cx,cfg->cy);
#endif	

 }

 static void resize(GtkWidget *hwnd, GtkAllocation *sz, ICQFRAME *cfg)
 {
	if( cfg->cx == sz->width && cfg->cy == sz->height)
	   return;
	
	DBGPrint("Frame \"%s\" resized",cfg->name);
	
	cfg->flags |= ICQFRAME_RESIZED;
	cfg->cx     = sz->width;
	cfg->cy     = sz->height;

    gtk_widget_queue_draw_area(hwnd,0,0,sz->width,sz->height);
	
 }
 
 
#ifdef SKINNED_GUI	
 
 static void icqframe_createBackground(GtkWidget *hwnd, ICQFRAME *cfg, int width, int height)
 {
	int       left;
	int       right;
	int       imgRows;
	int       imgCols;
	int       row;
	int       col;
	int       szRow;
	int       szCol;
	int       top      = 0;
	GdkPixbuf *bgImage;

    DBGPrint("Creating backgound for \"%s\"",cfg->name);

    cfg->hwnd = hwnd;
	bgImage   = gdk_pixbuf_new(	GDK_COLORSPACE_RGB,    
							    TRUE,
							    8,
							    width,
							    height
					        );

	/* Draw top line */
	
	if(cfg->base[ICQFRAME_TITLESTRETCH])
	{
	   left  = 0;
	   right = width;
	   
	   if(cfg->base[ICQFRAME_TITLELEFT])
	   {
	      imgRows = gdk_pixbuf_get_height(cfg->base[ICQFRAME_TITLELEFT]);
	      left    = gdk_pixbuf_get_width(cfg->base[ICQFRAME_TITLELEFT]);
		  
          gdk_pixbuf_copy_area(	cfg->base[ICQFRAME_TITLELEFT],
	   							0,0,
	   							left,imgRows,
	   							bgImage,
	   							0,0 );
	   }

	   if(cfg->base[ICQFRAME_TITLERIGHT])
	   {
	      imgRows = gdk_pixbuf_get_height(cfg->base[ICQFRAME_TITLERIGHT]);
	      imgCols = gdk_pixbuf_get_width(cfg->base[ICQFRAME_TITLERIGHT]);
		  
		  right  -= imgCols;
		  
          gdk_pixbuf_copy_area(	cfg->base[ICQFRAME_TITLERIGHT],
	   							0,0,
	   							imgCols,imgRows,
	   							bgImage,
	   							right,0 );
	   }

       top     =
	   imgRows = gdk_pixbuf_get_height(cfg->base[ICQFRAME_TITLESTRETCH]);
	   imgCols = gdk_pixbuf_get_width(cfg->base[ICQFRAME_TITLESTRETCH]);
	   
  	   for(col = left; col < right; col += imgCols)
	   {
	      szCol = right - col;
		  if(szCol > imgCols)
		     szCol = imgCols;
          gdk_pixbuf_copy_area(	cfg->base[ICQFRAME_TITLESTRETCH],
	   						    0,0,
	   						    szCol,imgRows,
	   						    bgImage,
	   						    col,0 );
	   }
	   
	}
	
	/* Draw stretch images */
	
	if(cfg->base[ICQFRAME_MIDSTRETCH])
	{
	   if(cfg->base[ICQFRAME_MIDLEFT])
	   {
	      imgRows = gdk_pixbuf_get_height(cfg->base[ICQFRAME_MIDLEFT]);
		  
		  left    = 
	      imgCols = gdk_pixbuf_get_width(cfg->base[ICQFRAME_MIDLEFT]);
		  
		  for(row = top;row < height;row += imgRows)
		  {
			 szRow = height-row;
			 if(szRow > imgRows)
				szRow = imgRows;
             gdk_pixbuf_copy_area(	cfg->base[ICQFRAME_MIDLEFT],
	   							    0,0,
	   							    imgCols,szRow,
	   							    bgImage,
	   							    0,row );
		  }
	   }
	   else
	   {
		  left = 0;
	   }

       imgRows = gdk_pixbuf_get_height(cfg->base[ICQFRAME_MIDSTRETCH]);
	   imgCols = gdk_pixbuf_get_width(cfg->base[ICQFRAME_MIDSTRETCH]);

  	   for(row = top;row < height;row += imgRows)
	   {
	      szRow = height-row;
		  if(szRow > imgRows)
		     szRow = imgRows;
		  
		  for(col = left; col < width; col += imgCols)
		  {
			 szCol = width - col;
			 if(szCol > imgCols)
				szCol = imgCols;
             gdk_pixbuf_copy_area(	cfg->base[ICQFRAME_MIDSTRETCH],
	   							    0,0,
	   							    szCol,szRow,
	   							    bgImage,
	   							    col,row );
		  }
	   }
	   
	   if(cfg->base[ICQFRAME_MIDRIGHT])
	   {
          imgRows = gdk_pixbuf_get_height(cfg->base[ICQFRAME_MIDRIGHT]);
	      imgCols = gdk_pixbuf_get_width(cfg->base[ICQFRAME_MIDRIGHT]);
		  
		  col     = width-imgCols;
  	      for(row = top;row < height;row += imgRows)
	      {
	         szRow = height-row;
		     if(szRow > imgRows)
		        szRow = imgRows;
			 
             gdk_pixbuf_copy_area(	cfg->base[ICQFRAME_MIDRIGHT],
	   							    0,0,
	   							    imgCols,szRow,
	   							    bgImage,
	   							    col,row );
		  }
	   }
	}
	
	/* Draw top */
	
	if(cfg->base[ICQFRAME_TOPLEFT])
	{
       imgRows = gdk_pixbuf_get_height(cfg->base[ICQFRAME_TOPLEFT]);
	   imgCols = gdk_pixbuf_get_width(cfg->base[ICQFRAME_TOPLEFT]);
       gdk_pixbuf_copy_area(	cfg->base[ICQFRAME_TOPLEFT],
  							    0,0,
	   							imgCols,imgRows,
	   							bgImage,
	   							0,top );
	}

	if(cfg->base[ICQFRAME_TOPRIGHT])
	{
       imgRows = gdk_pixbuf_get_height(cfg->base[ICQFRAME_TOPRIGHT]);
	   imgCols = gdk_pixbuf_get_width(cfg->base[ICQFRAME_TOPRIGHT]);
       gdk_pixbuf_copy_area(	cfg->base[ICQFRAME_TOPRIGHT],
  							    0,0,
	   							imgCols,imgRows,
	   							bgImage,
	   							width-imgCols,top );
	}
	
	/* Draw bottom */
    left  = 0;
    right = width;
	
	if(cfg->base[ICQFRAME_BOTTOMLEFT])
	{
       imgRows = gdk_pixbuf_get_height(cfg->base[ICQFRAME_BOTTOMLEFT]);
	   
	   left    =
	   imgCols = gdk_pixbuf_get_width(cfg->base[ICQFRAME_BOTTOMLEFT]);
	   
       gdk_pixbuf_copy_area(	cfg->base[ICQFRAME_BOTTOMLEFT],
  							    0,0,
	   							imgCols,imgRows,
	   							bgImage,
	   							0,height-imgRows );
	}
	   
	if(cfg->base[ICQFRAME_BOTTOMRIGHT])
	{
       imgRows = gdk_pixbuf_get_height(cfg->base[ICQFRAME_BOTTOMRIGHT]);
	   imgCols = gdk_pixbuf_get_width(cfg->base[ICQFRAME_BOTTOMRIGHT]);
	   right   = width-imgCols;
	   
       gdk_pixbuf_copy_area(	cfg->base[ICQFRAME_BOTTOMRIGHT],
  							    0,0,
	   							imgCols,imgRows,
	   							bgImage,
	   							right,height-imgRows );
	}

	if(cfg->base[ICQFRAME_BOTTOMMIDLEFT])
	{
       imgRows = gdk_pixbuf_get_height(cfg->base[ICQFRAME_BOTTOMMIDLEFT]);
	   imgCols = gdk_pixbuf_get_width(cfg->base[ICQFRAME_BOTTOMMIDLEFT]);
	   
       gdk_pixbuf_copy_area(	cfg->base[ICQFRAME_BOTTOMMIDLEFT],
  							    0,0,
	   							imgCols,imgRows,
	   							bgImage,
	   							left,height-imgRows );
	   
	   left += imgCols;
	}
	
	if(cfg->base[ICQFRAME_BOTTOMMIDRIGHT])
	{
       imgRows = gdk_pixbuf_get_height(cfg->base[ICQFRAME_BOTTOMMIDRIGHT]);
	   imgCols = gdk_pixbuf_get_width(cfg->base[ICQFRAME_BOTTOMMIDRIGHT]);

	   right -= imgCols;
	   
       gdk_pixbuf_copy_area(	cfg->base[ICQFRAME_BOTTOMMIDRIGHT],
  							    0,0,
	   							imgCols,imgRows,
	   							bgImage,
	   							right,height-imgRows );
	   
	}

	if(cfg->base[ICQFRAME_BOTTOMSTRETCH])
	{
       imgRows = gdk_pixbuf_get_height(cfg->base[ICQFRAME_BOTTOMSTRETCH]);
	   imgCols = gdk_pixbuf_get_width(cfg->base[ICQFRAME_BOTTOMSTRETCH]);
	   row     = height-imgRows;

	   for(col = left; col < right; col += imgCols)
 	   {
	      szCol = right - col;
		  if(szCol > imgCols)
		     szCol = imgCols;
          gdk_pixbuf_copy_area(	cfg->base[ICQFRAME_BOTTOMSTRETCH],
   							    0,0,
   							    szCol,imgRows,
   							    bgImage,
   							    col,row );
	   }
	}

    if(cfg->bg)
       gdk_pixmap_unref(cfg->bg);

    gdk_pixbuf_render_pixmap_and_mask(bgImage, &cfg->bg, NULL,1);
    gdk_pixbuf_unref(bgImage);
	
 }	

 static gboolean icqframe_expose(GtkWidget *hwnd, GdkEventExpose *event, ICQFRAME *cfg)
 {
	if(!cfg->gc)
       cfg->gc = gdk_gc_new(hwnd->window);
	
	if(cfg->flags & ICQFRAME_RESIZED)
	{
       icqframe_createBackground(hwnd,cfg,cfg->cx,cfg->cy);	
       gdk_window_set_back_pixmap(hwnd->window,cfg->bg,FALSE);
       cfg->flags &= ~ICQFRAME_RESIZED;
	}
    gdk_draw_pixmap(    hwnd->window,
                        cfg->gc,
                        cfg->bg,
                        event->area.x,event->area.y,
                        event->area.x,event->area.y,
                        event->area.width,event->area.height );
	
	return FALSE;
 }
 
#endif
 
 static void clean( GtkWidget *hwnd, ICQFRAME *cfg)
 {

#ifdef SKINNED_GUI	

	int f;
	
	if(cfg->bg)
	   gdk_pixmap_unref(cfg->bg);
	
	for(f=0;f<ICQFRAME_SKINBITMAPS;f++)
	{
	   if(cfg->base[f])
	   {
          gdk_pixbuf_unref(cfg->base[f]);
		  cfg->base[f] = NULL;
	   }
	}
	
#endif	
	
 }
 
 void EXPENTRY icqframe_destroy( hWindow hwnd, ICQFRAME *cfg)
 {
	if(!cfg->sz)
	   return;

    clean(hwnd,cfg);	
	
#ifdef SKINNED_GUI	
    gdk_gc_unref(cfg->gc);
#endif

	DBGPrint("Frame \"%s\" destroyed",cfg->name);
	
	if(cfg->flags & ICQFRAME_FLAG_TOPWINDOW)
    {
	   DBGTracex(cfg->flags);
       DBGPrint("Top level frame \"%s\" destroyed",cfg->name);	   
	   gtk_main_quit();
	}

	cfg->sz = 0;
	free(cfg);
 }

 
 int EXPENTRY icqframe_loadSkin(hWindow hwnd,const char *name, SKINFILESECTION *fl)
 {
#ifdef SKINNED_GUI	

	VALIDATE_WINDOW(hwnd,cfg);
	
	
	DBGMessage("Load skin definition for frame...");
	
#endif
	
	return 0;
 }

 static void queryChild(GtkWidget *widget, CHILDSEARCH *sr)
 {
	if(sr->sz != sizeof(CHILDSEARCH) || sr->ret)
	   return;
	
	if(!strcmp(sr->key,gtk_widget_get_name(widget)))
	   sr->ret = widget;

	if(GTK_IS_CONTAINER(widget))
	   gtk_container_foreach(GTK_CONTAINER(widget),(GtkCallback) queryChild, sr);

 }

 hWindow EXPENTRY icqframe_queryChild(hWindow hwnd, USHORT id)
 {
	CHILDSEARCH sr;
	
	memset(&sr,0,sizeof(sr));
	sr.sz = sizeof(CHILDSEARCH);
	
	sprintf(sr.key,id_masc,id);
	
	gtk_container_foreach(GTK_CONTAINER(hwnd),(GtkCallback) queryChild, &sr);
	
    return sr.ret;	
 }

 
 MRESULT _System icqFrameWindow(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
	
	return 0;
 }

 void EXPENTRY icqgui_setWindowID(hWindow hwnd, USHORT id)
 {
	char key[IDMASC_SIZE];
	
	sprintf(key,"pwICQ%04x",id);
	gtk_widget_set_name(hwnd,key);  
	
 }
	
 
#ifdef SKINNED_GUI	

 static gboolean button_press(GtkWidget *widget, GdkEventButton *event, ICQFRAME *cfg) 
 {
	USHORT cx = cfg->cx - RIGHT_BORDER_SIZE;
	USHORT cy = cfg->cy - BOTTOM_BORDER_SIZE;

    if(event->button == 1)
	{
/*	   
       gtk_window_begin_move_drag(  GTK_WINDOW(widget),
                                    event->button,
                                    event->x_root, event->y_root,
                                    event->time);
*/
       return FALSE;	   
	}
	else if(  event->x > LEFT_BORDER_SIZE 
	   		  && event->y > TOP_BORDER_SIZE
			  && event->x < cx
	          && event->y < cy )
	{
       gtk_window_begin_move_drag(  GTK_WINDOW(widget),
                                    event->button,
                                    event->x_root, event->y_root,
                                    event->time);
	   
	}
	else if(event->y < TOP_BORDER_SIZE)
	{
	   if( event->x < LEFT_BORDER_SIZE )
	   {
		  BEGIN_RESIZE(GDK_WINDOW_EDGE_NORTH_WEST);
	   }
	   else if(event->x > cx)
	   {
		  BEGIN_RESIZE(GDK_WINDOW_EDGE_NORTH_EAST);
	   }
	   else
	   {
		  BEGIN_RESIZE(GDK_WINDOW_EDGE_NORTH);
	   }
	}
	else if(event->y > cy)
	{
	   if(event->x < LEFT_BORDER_SIZE )
	   {
		  BEGIN_RESIZE(GDK_WINDOW_EDGE_SOUTH_WEST);
	   }
	   else if(event->x > cx)
	   {
		  BEGIN_RESIZE(GDK_WINDOW_EDGE_SOUTH_EAST);
	   }
	   else
	   {
		  BEGIN_RESIZE(GDK_WINDOW_EDGE_SOUTH);
	   }
	}
	else if(event->x < LEFT_BORDER_SIZE ) 
	{
	   BEGIN_RESIZE(GDK_WINDOW_EDGE_WEST);
	}
	else if(event->x > cx)
	{
	   BEGIN_RESIZE(GDK_WINDOW_EDGE_EAST);
	}
								 
    return TRUE;
 }	

#endif


 int icqskin_setFrameIcon(hWindow hwnd, SKINDATA *cfg, USHORT id)
 {
#ifdef GTK2
    gtk_window_set_icon(GTK_WINDOW(hwnd),cfg->icons[id]);
#endif
    return 0;
 }

 int icqskin_setFrameTitle(hWindow hwnd, const char *title)
 {
    gtk_window_set_title(GTK_WINDOW(hwnd),title);
    return 0;
 }
 

 
