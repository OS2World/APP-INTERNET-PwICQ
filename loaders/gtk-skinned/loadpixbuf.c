/*
 * init.c - Initialize system, load skin
 */
 
 #include "gtk_skinned.h"
 

/*---[ Constantes ]------------------------------------------------------------------*/

 static const char *names[] = { "titlebar-leftcorner.bmp",
                                "titlebar-stretch.bmp",
                                "titlebar-rightcorner.bmp",

                                "leftdesign.bmp",
                                "rightdesign.bmp",

                                "leftstretch.bmp",
                                "backdrop2.gif",
                                "rightstretch.bmp",

                                "leftdesignbottom.bmp",
                                "bottomstretch.bmp",
                                "rightdesignbottom.bmp"
                              };
							  
/*---[ Implementacao ]---------------------------------------------------------------*/

 int icqgtk_loadpixbuf(MAINWIN *hwnd)
 {
	char buffer[0x0100];
	int  f;
	
    CHKPoint();
	
	for(f=0;f<SKIN_IMAGE_COUNT;f++)
    {
	   sprintf(buffer,"/usr/share/pwicq/skin/%s",names[f]);
       hwnd->baseImage[f] = gdk_pixbuf_new_from_file(buffer);
	   if(!hwnd->baseImage[f])
	   {
	      DBGPrint("Error loading %s",buffer);
		  return f;
	   }
	}
	
    DBGMessage("All base images were loaded");	
    return 0;
 }

