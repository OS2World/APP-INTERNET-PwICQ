/*
 * gtktools.c - GTK Implementation of pwICQGUI Functions
 */

 #include <pwicqgui.h>
 #include <string.h>
 #include <unistd.h>

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 int icqskin_setVisible(SKINDATA *cfg, BOOL visible)
 {
	DBGTrace(visible);
	
    if(visible)
       gtk_widget_show(cfg->frame);
	else
	   gtk_widget_hide(cfg->frame);
    return 0;
 }

 GdkPixbuf *icqskin_loadImage(HICQ icq, const char *fileName)
 {
	char       buffer[0x0100];
	
	if(!fileName)
	   return NULL;

#ifdef GTK2	
    if(!access(fileName,R_OK))
	   return gdk_pixbuf_new_from_file(fileName,NULL);
#else
    if(!access(fileName,R_OK))
	   return gdk_pixbuf_new_from_file(fileName);
#endif	

	/* Can't find requested file, try the default one */
    icqLoadString(icq,"image-files","",buffer,0xFF);
	
    if(!*buffer)
       icqQueryProgramPath(icq,"images/",buffer,0xFF);
	
	strncat(buffer,fileName,0xFF);
	
	DBGMessage(buffer);

    DBGTracex(access(buffer,R_OK));
	
    if(access(buffer,R_OK))
	   return NULL;

    DBGMessage(buffer);	
#ifdef GTK2	
	return gdk_pixbuf_new_from_file(buffer,NULL);
#else
	return gdk_pixbuf_new_from_file(buffer);
#endif	
 }

 GtkWidget * gtk_pixmap_new_from_pixbuf(GdkPixbuf *px)
 {
    GdkPixmap *Pix;
    GdkBitmap *Btm;
	if(!px)
	   return NULL;
    gdk_pixbuf_render_pixmap_and_mask(px, &Pix, &Btm,1);
    return gtk_pixmap_new(Pix,Btm);
 }

 int icqskin_loadIcons(SKINDATA *wnd, const char *fileName)
 {
	/* Load default icon table */
	GdkPixbuf  *icons;
    int		   f,
	           c;

    DBGMessage(fileName);

    icons = icqskin_loadImage(wnd->icq,fileName);

    if(!icons)
	{
	   icqWriteSysLog(wnd->icq,PROJECT,"Unable to load icon table");
	   return -1;
	}

    wnd->iconSize = gdk_pixbuf_get_height(icons);

    for(f=c=0;f<PWICQICONBARSIZE;f++)
    {
       wnd->icons[f] = gdk_pixbuf_new(	  gdk_pixbuf_get_colorspace(icons),
    						              gdk_pixbuf_get_has_alpha(icons),
    						              gdk_pixbuf_get_bits_per_sample(icons),
    						              wnd->iconSize, wnd->iconSize);

       gdk_pixbuf_copy_area (icons,c,0,wnd->iconSize,wnd->iconSize,wnd->icons[f],0,0);
       gdk_pixbuf_render_pixmap_and_mask(wnd->icons[f], &wnd->iPixmap[f], &wnd->iBitmap[f],1);
	
       c += wnd->iconSize;
    }

    gdk_pixbuf_unref(icons);

	return 0;
	
 }

 int _System icqskin_queryClipboard(HICQ icq, int sz, char *buffer)
 {
#ifdef GTK2
	GtkClipboard *clipboard = gtk_clipboard_get(GDK_NONE);
	char         *ptr;
#endif	
	
    *buffer = 0;

#ifdef GTK2
	*buffer = 0;
	
	if(!gtk_clipboard_wait_is_text_available(clipboard))
	   return 0;
	
    ptr = gtk_clipboard_wait_for_text(clipboard);	
	
    if(ptr)
    {
	   DBGMessage(ptr);
       strncpy(buffer,ptr,sz);
       *(buffer+sz) = '0';
    }
#endif

	DBGMessage(buffer);
	
	return strlen(buffer);
 }

 void * icqskin_getWindowDataBlock(hWindow hwnd)
 {
    return gtk_object_get_user_data(GTK_OBJECT(hwnd));
 }

 int icqskin_setWindowDataBlock(hWindow hwnd, void *ptr)
 {
    gtk_object_set_user_data(GTK_OBJECT(hwnd), (gpointer) ptr);
	return 0;
 }
 
 void icqskin_setDlgItemIcon(hWindow hwnd,USHORT id, USHORT icon)
 {
	hWindow  h    = icqskin_getDlgItemHandle(hwnd, id);
	ICQFRAME *cfg = gtk_object_get_user_data(GTK_OBJECT(hwnd));
	SKINDATA *skn = icqGetSkinDataBlock(cfg->icq);
	
	if(!h)
	{
	   icqWriteSysLog(cfg->icq,PROJECT,"Invalid Control-ID when setting dialog icon");
	   return;
	}

    gtk_pixmap_set(GTK_PIXMAP(h),skn->iPixmap[icon],skn->iBitmap[icon]);
	
 }

 void icqskin_setFont(hWindow hwnd, const char *name)
 {
#ifdef GTK2	
	
	char buffer[0x0100];
	char *ptr;
	int  f;
	char *parms[] = { NULL, NULL, NULL };
	
	strncpy(buffer,name,0xFF);
	
	ptr = strtok(buffer,".");
	for(f=0;f<3&&ptr;f++)
    {
	   parms[f] = ptr;
	   ptr      = strtok(NULL,".");
	}
	
    PangoFontDescription *font = pango_font_description_new();

	if(parms[1])
	{
       pango_font_description_set_family(font,parms[1]);
	}

    if(parms[2])
    {
	   if(!strcmp(parms[2],"Bold"))
   	      pango_font_description_set_weight(font,PANGO_WEIGHT_BOLD);
	}
	
	gtk_widget_modify_font(hwnd,font);
	pango_font_description_free(font);
#endif	
 }

 void icqskin_setWindowText(hWindow h, const char *text)
 {
	if(!h)
	{
	   return;
	}
	else if(GTK_IS_LABEL(h))
	{
	   gtk_label_set(GTK_LABEL(h), text);
	}
	else if(GTK_IS_ENTRY(h))
	{
       gtk_entry_set_text(GTK_ENTRY(h),text);
	}
#ifdef GTK2	
	else if(GTK_IS_TEXT_VIEW(h))
	{
	   gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(h)),text,-1);
	}
#endif	
#ifdef GTK1
	else if(GTK_IS_TEXT(h))
	{
	   gtk_text_insert(GTK_TEXT(h), NULL, NULL, NULL, text, -1);
	}
#endif	
#ifdef DEBUG	
	else
	{
	   DBGMessage("************ INVALID CALL TO icqskin_setWindowText");
	}
#endif	
	
 }

 void icqskin_setChecked(hWindow hwnd)
 {
	
 }

 void icqskin_setUnchecked(hWindow hwnd)
 {
	
 }

 int icqskin_setICQHandle(hWindow hwnd, HICQ icq)
 {
	#pragma pack(1)
	
	struct _prefix
	{
       ICQGUI_STANDARD_PREFIX	
	} *cfg = gtk_object_get_user_data(GTK_OBJECT(hwnd));

	#pragma pack()

	if(cfg && cfg->sz >= sizeof(struct _prefix))
	{
       cfg->icq = icq;
       return 0;
	}
	return -1;
 }

 int icqskin_setFrameName(hWindow hwnd, const char *name)
 {
	#pragma pack(1)
	
	struct _prefix
	{
       ICQGUI_STANDARD_PREFIX	
	} *cfg = gtk_object_get_user_data(GTK_OBJECT(hwnd));

	#pragma pack()

	if(cfg && cfg->sz >= sizeof(struct _prefix))
	{
       cfg->name = name;
       return 0;
	}
	return -1;
 }

 void icqskin_restoreFrame(hWindow hwnd, const char *key, USHORT w, USHORT h)
 {
	
 }

 void icqskin_storeFrame(hWindow hwnd,const char *key)
 {
	
 }

 int EXPENTRY icqskin_selectFile(HWND hwnd, HICQ icq, USHORT id, BOOL save, const char *key, const char *masc, const char *title, char *vlr, int (* _System callBack)(const DIALOGCALLS *,HWND,HICQ,char *))
 {
	return 0;
 }
