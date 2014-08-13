/*
 * gtkbutton.c - Skinned button for GTK
 */
 
 #include <malloc.h>
 #include <string.h>
 #include <stdlib.h>

 #include <pwicqgui.h>
 #include <gdk-pixbuf/gdk-pixbuf.h>

/*---[ Defines ]--------------------------------------------------------------*/

 #define max(a,b) a > b ? a : b
 #define min(a,b) a < b ? a : b
 #define VALIDATE_WIDGET(h) ICQBUTTON *cfg = gtk_object_get_user_data(GTK_OBJECT(hwnd)); if(!cfg || cfg->st.sz < sizeof(ICQBUTTON)) { DBGMessage("****** Invalid button in " __FUNCTION__);return -1; }
 

/*---[ Structures ]-----------------------------------------------------------*/

 #pragma pack(1)

 typedef struct _icqbutton
 {
    ICQSTATIC  st;
	
	hWindow    icon;
	hWindow    label;
	
 } ICQBUTTON;

 #pragma pack()

/*---[ Constants ]------------------------------------------------------------*/
 
 
/*---[ Prototipes ]-----------------------------------------------------------*/

 static void EXPENTRY icqbutton_destroy(hWindow, ICQBUTTON *);

#ifdef SKINNED_GUI	
 static void     resize(   GtkWidget *, GtkAllocation  *, ICQBUTTON *);
 static gboolean expose(   GtkWidget *, GdkEventExpose *, ICQBUTTON *);
 static void     pressed(  GtkButton *, ICQBUTTON *);
 static void     released( GtkButton *, ICQBUTTON *);
 static void     enter(    GtkButton *, ICQBUTTON *);
 static void     leave(    GtkButton *, ICQBUTTON *);
#endif 

/*---[ Implementation ]-------------------------------------------------------*/

 hWindow EXPENTRY icqbutton_new(hWindow owner, USHORT id, const char *name, USHORT img, const char *text)
 {
#ifdef USEICQLIB	
    #pragma pack(1)

    struct _ownerinfo
    {
       ICQGUI_STANDARD_PREFIX
    } *ownerinfo = gtk_object_get_user_data(GTK_OBJECT(owner));

    #pragma pack()
	
    SKINDATA  *skn     = icqskin_getDataBlock(ownerinfo->icq);
	
#endif	

	hWindow   ret      = NULL;
	hWindow   temp;
    ICQBUTTON *cfg     = (ICQBUTTON *) icqlabel_createDataBlock(owner, sizeof(ICQBUTTON), name);
	GdkPixbuf *pixBuf  = NULL;
	
	if(!cfg)
	   return ret;

    ret = gtk_button_new();
	if(!ret)
	   return ret;

    gtk_object_set_user_data(GTK_OBJECT(ret), (gpointer) cfg);
    gtk_signal_connect( GTK_OBJECT(ret),"destroy",GTK_SIGNAL_FUNC(icqbutton_destroy), cfg );
	
    icqgui_setWindowID(ret, id);

    temp = gtk_hbox_new(FALSE,5);
	
    gtk_box_pack_start(GTK_BOX(temp), gtk_label_new(""),  TRUE, FALSE, 0);
	
#ifdef USEICQLIB	
	if(img == 0xFFFF)
	{
       sprintf(buffer,"%s.gif",name);
       pixBuf = icqskin_loadImage(ownerinfo->icq,buffer);
	}
#endif	
	
	if(pixBuf)
	{
	   /* The button has image, use it */
       gtk_box_pack_start(GTK_BOX(temp),gtk_pixmap_new_from_pixbuf(pixBuf), FALSE, FALSE, 0);
       gdk_pixbuf_unref(pixBuf);
	}
	else
	{
#ifdef USEICQLIB	
       if(img != 0xFFFF)
       {
          cfg->icon = gtk_pixmap_new(skn->iPixmap[img],skn->iBitmap[img]);
          gtk_box_pack_start(GTK_BOX(temp), cfg->icon,  FALSE, FALSE, 0);
	   }
#endif	
       cfg->label = gtk_label_new(text);
       gtk_box_pack_start(GTK_BOX(temp), cfg->label,  FALSE, FALSE, 0);
    }

    gtk_box_pack_end(GTK_BOX(temp), gtk_label_new(""),  TRUE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(ret),temp);

#ifdef SKINNED_GUI	
//    gtk_widget_set_app_paintable(ret,TRUE);
	
    icqlabel_setBackground(ret, gdk_pixbuf_new_from_file("button.gif",NULL));
    icqlabel_setForegroundRGB(ret, 0xFFFF, 0xFFFF, 0xFFFF);
	
    gtk_signal_connect( GTK_OBJECT(ret), "size-allocate", GTK_SIGNAL_FUNC(resize),   cfg );
    gtk_signal_connect( GTK_OBJECT(ret), "expose_event",  GTK_SIGNAL_FUNC(expose),   cfg );
    gtk_signal_connect( GTK_OBJECT(ret), "pressed",       GTK_SIGNAL_FUNC(pressed),  cfg );
    gtk_signal_connect( GTK_OBJECT(ret), "released",      GTK_SIGNAL_FUNC(released), cfg );
    gtk_signal_connect( GTK_OBJECT(ret), "enter",         GTK_SIGNAL_FUNC(enter),    cfg );
    gtk_signal_connect( GTK_OBJECT(ret), "leave",         GTK_SIGNAL_FUNC(leave),    cfg );

#endif	

    gtk_widget_show_all(ret);
	return ret;
 }

 void EXPENTRY icqbutton_destroy( hWindow hwnd, ICQBUTTON *cfg)
 {
	if(cfg->st.sz < sizeof(ICQBUTTON))
	   return;

	

    icqlabel_destroy(hwnd, (ICQSTATIC *) cfg);

 }

#ifdef SKINNED_GUI	
 
 static void resize(GtkWidget *hwnd, GtkAllocation *sz, ICQBUTTON *cfg)
 {
    icqlabel_createBackground(hwnd,(ICQSTATIC *) cfg,sz->width,sz->height);	
 }

 static gboolean expose(GtkWidget *hwnd, GdkEventExpose *event, ICQBUTTON *cfg)
 {
// gtk_widget_get_pango_context  (GtkWidget *widget);	
	
    if(icqlabel_expose(hwnd, event, (ICQSTATIC *) cfg))
	{
	   /* Widget has background image */

/*	   
http://developer.gnome.org/doc/API/2.0/gtk/GtkStyle.html#gtk-paint-layout	   
http://www.es.gnome.org/cgi-bin/cvsweb/diasce/src/gtksourceview.c?annotate=1.2&cvsroot=GNOME	   
*/
	   
	   return TRUE;
	}

	return FALSE;
 }

 static void pressed(GtkButton *button, ICQBUTTON *cfg)
 {
	CHKPoint();
 }

 static void released(GtkButton *button, ICQBUTTON *cfg)
 {
	CHKPoint();
 }

 static void enter(GtkButton *button, ICQBUTTON *cfg)
 {
	CHKPoint();
 }
 
 static void leave(GtkButton *button, ICQBUTTON *cfg)
 {
	CHKPoint();
 }
 
#endif
 
 MRESULT _System icqButtonWindow(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {

        return 0;
 }

 int icqskin_setButtonIcon(hWindow hwnd, USHORT icon)
 {
#ifdef USEICQLIB	
    ICQBUTTON  *ctl = gtk_object_get_user_data(GTK_OBJECT(hwnd));
    SKINDATA   *skn;
	
    if(!ctl || ctl->st.sz < sizeof(ICQBUTTON))
    {
       DBGTracex(hwnd);
       DBGMessage("****** Invalid button in icqskin_setButtonIcon");
       return -1;
    }
	
	skn = icqskin_getDataBlock(ctl->st.icq);
	
    gtk_pixmap_set(GTK_PIXMAP(ctl->icon),skn->iPixmap[icon],skn->iBitmap[icon]);
	
    return 0;
	
#else
	
    return -1;
	
#endif	
 }

 int icqskin_setButtonText(hWindow hwnd, const char *text)
 {
	VALIDATE_WIDGET(hwnd);
    gtk_label_set_text(GTK_LABEL(cfg->label),text);
	
    return 0;
 }
 
