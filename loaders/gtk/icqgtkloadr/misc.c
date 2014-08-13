/*
 * Miscellaneous
 */
 
 #include <stdio.h>
 
 #include <icqgtk.h>

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 void icqgtk_searchButton( GtkWidget *widGet, MAINWIN *wnd)
 {
    DBGMessage("Search");
    
 }

 void icqgtk_systemMessageButton( GtkWidget *widGet, MAINWIN *wnd)
 {
    icqOpenSystemMessage(wnd->icq);
 }

 int icqgtk_setMainWindowTitle(HICQ icq, MAINWIN *wnd)
 {
    char buffer[0x0100];
    sprintf(buffer,"%lu",icqQueryUIN(icq));
    gtk_window_set_title(GTK_WINDOW(wnd->main),buffer);
    return 0;
 }

 
