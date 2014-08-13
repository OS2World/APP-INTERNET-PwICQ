/*
 * init.c - Initialize system, load skin
 */
 
 #include <malloc.h>
 #include <string.h>
 
 #include "gtk_skinned.h"
 

/*---[ Constantes ]------------------------------------------------------------------*/

 							  
/*---[ Implementacao ]---------------------------------------------------------------*/

 MAINWIN * icqgtk_initialize(void)
 {							 
    MAINWIN 		*ret 	= malloc(sizeof(MAINWIN));
    GdkPixbuf  		*icons;
    GdkPixbuf  		*work;
	int				c;
	int				f;
	
	if(!ret)
	   return ret;
	
    memset(ret,0,sizeof(MAINWIN));
    ret->sz = sizeof(MAINWIN);
	
	/* Carrega a tabela de icones */
    icons = gdk_pixbuf_new_from_file("/usr/share/pwicq/images/icons.gif");
    DBGTracex(icons);

    ret->szIcons = gdk_pixbuf_get_height(icons);

    work    = gdk_pixbuf_new(  gdk_pixbuf_get_colorspace(icons),
    						   gdk_pixbuf_get_has_alpha(icons), 
    						   gdk_pixbuf_get_bits_per_sample(icons),
    						   ret->szIcons, ret->szIcons);
        

    for(f=c=0;f<PWICQICONBARSIZE;f++)
    {
       gdk_pixbuf_copy_area (icons,c,0,ret->szIcons,ret->szIcons,work,0,0);
       gdk_pixbuf_render_pixmap_and_mask(work, &ret->iPixmap[f], &ret->iBitmap[f],1);
       c += ret->szIcons;
    }

    gdk_pixbuf_unref(work);
    gdk_pixbuf_unref(icons);

	/* Carrega os pixbufs */
    icqgtk_loadpixbuf(ret);
	
	
	/* Cria os widgets */
	ret->frame = gtk_window_new(GTK_WINDOW_TOPLEVEL);
 

	
	return ret;
	
 }
 
