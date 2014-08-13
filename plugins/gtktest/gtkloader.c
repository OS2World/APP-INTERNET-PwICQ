/*
 * gtkloader.c - Test of the pwICQ-Skinned functions
 */

 #include <malloc.h>
 #include <string.h>
 #include <stdlib.h>

 #include <pwicqgui.h>
 #include <gdk-pixbuf/gdk-pixbuf.h>

 static void ICQCALLBACK buttonCallback(hWindow,void *);

 int main(int argc, char *argv[] )
 {
     hWindow window;
	 hWindow box;
	 hWindow temp;
    
     gtk_init (&argc, &argv);
    
     window = icqframe_new(0,"gtktest",150,375,TRUE,FALSE);
	 icqgui_setWindowID(window,100);
	 DBGTracex(window);
	
     DBGMessage(gtk_widget_get_name(window));
	
	 box = gtk_vbox_new(FALSE,0);
	 gtk_container_add(GTK_CONTAINER(window), box);
	 DBGTracex(box);

	 temp = icqlabel_new(window, 500, "label", "Label");
 	 gtk_box_pack_start(GTK_BOX(box), temp, FALSE, FALSE, 0);

	 temp = icqbutton_new(window, 501, "button", 0xFFFF, "Button");
	 icqskin_setButtonCallback(temp,buttonCallback,0);

 	 gtk_box_pack_end(GTK_BOX(box), temp, FALSE, FALSE, 0);
	
//   DBGTracex(temp);
//   DBGMessage(gtk_widget_get_name(temp));
//	 DBGTracex(icqframe_queryChild(window, 500));

     gtk_widget_show_all(window);
    
     gtk_main();
    
     return 0;
 }

 static void ICQCALLBACK buttonCallback(hWindow hwnd, void *parm)
 {
	CHKPoint()
 }
