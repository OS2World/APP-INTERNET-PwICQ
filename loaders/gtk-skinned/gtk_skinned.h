/*
 * gtk_skinned.h - Prototipos e definicoes para a versao Skinned do pwICQ for Linux
 */

 #include <stdio.h>
 #include <gtk/gtk.h>
 #include <gdk-pixbuf/gdk-pixbuf.h>
 #include <icqtlkt.h>

/*---[ Definitions ]-----------------------------------------------------------------*/

 enum SKIN_IMAGES
 {
	SKNIMG_TITLE_LEFT,
	SKNIMG_TITLE_STRETCH,
	SKNIMG_TITLE_RIGHT,
	
	SKNIMG_TOP_LEFT,
	SKNIMG_TOP_RIGHT,
	
	SKNIMG_MID_LEFT,
	SKNIMG_MID_STRETCH,
	SKNIMG_MID_RIGHT,
	
	SKNIMG_BOTTOM_LEFT,
	SKNIMG_BOTTOM_STRETCH,
	SKNIMG_BOTTOM_RIGHT,
	
    SKIN_IMAGE_COUNT
 };


/*---[ Structures ]------------------------------------------------------------------*/ 
 #pragma pack(1)
 
 typedef struct _mainwin
 {
    USHORT		sz;
	
	/* Widgets */
	GtkWidget    *frame;

	/* Base images */
	GdkPixbuf    *baseImage[SKIN_IMAGE_COUNT];
	
    /* Icon bar */
	USHORT	     szIcons;
    GdkPixmap	 *iPixmap[PWICQICONBARSIZE];
    GdkBitmap    *iBitmap[PWICQICONBARSIZE];
	
 } MAINWIN;
 
 
 /*---[ Prototipes ]-----------------------------------------------------------------*/

  MAINWIN * icqgtk_initialize(void);
  int 		icqgtk_loadpixbuf(MAINWIN *);
 
 
