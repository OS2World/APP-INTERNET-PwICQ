/*
 * gtknoskin.c - Create GTK non Skinned pwICQ Window
 */

 #include <pwicqgui.h>
 #include <string.h>
 #include <malloc.h>


/*---[ Implementation ]-------------------------------------------------------*/

 int _System icqskin_CreateStandardWindow(HICQ icq, SKINDATA *cfg)
 {
	char        buffer[0x0100];
	ICQFRAME    *wnd;
    GtkWidget   *temp;
    GtkWidget   *box;
	int         f;

    cfg->frame = icqframe_new(icq, "mainWindow", 150, 375, TRUE, FALSE);
	if(!cfg->frame)
	   return -2;
	
	wnd = gtk_object_get_user_data(GTK_OBJECT(cfg->frame));
	if(!wnd)
	   return -1;
	
    cfg->mainWindow = gtk_vbox_new(FALSE,0);
    gtk_object_set_user_data(GTK_OBJECT(cfg->mainWindow), (gpointer) wnd);

	DBGTracex(cfg->mainWindow);
	DBGTracex(wnd);

    sprintf(buffer,"ICQ#%lu",icqQueryUIN(icq));
    icqskin_setWindowManagerName(cfg->frame,buffer);

	icqskin_loadIcons(cfg,"icons.gif");
	
	/* Create listbox control */
	icqSkin_createStandardUserList(icq,cfg->mainWindow,ID_USERLIST);
	CHKPoint();

	/* Create Buttons */
	box = gtk_hbox_new(FALSE,0);
	
	for(f=0;f<icqgui_MainButtonTableElements;f++)
	{
	   DBGTrace(icqgui_MainButtonTable[f].id);
	   
       temp = icqbutton_new(	cfg->mainWindow, 
	   							icqgui_MainButtonTable[f].id, 
	   							icqgui_MainButtonTable[f].name, 
	   							icqgui_MainButtonTable[f].icon, 
	   							icqgui_MainButtonTable[f].text
	   						);
	   
	   DBGTracex(temp);
	   
       if(icqgui_MainButtonTable[f].cbk)
		  icqskin_setButtonCallback(temp,icqgui_MainButtonTable[f].cbk,cfg);
		  
	   switch(icqgui_MainButtonTable[f].section)
	   {
	   case 0:
          gtk_box_pack_start(GTK_BOX(cfg->mainWindow),temp,FALSE,FALSE,0);
		  break;
	   
	   case 1:
          gtk_box_pack_start(GTK_BOX(box),temp,FALSE,FALSE,0);
		  break;

	   case 2:
          gtk_box_pack_start(GTK_BOX(box),temp,TRUE,TRUE,0);
		  break;
	   }
	   
	}
	
	CHKPoint();
	
    gtk_box_pack_start(GTK_BOX(cfg->mainWindow), box, FALSE, FALSE, 0);
	
    /* Add mainWindow in the frame, show widgets */
    DBGTracex(cfg->mainWindow);
    gtk_widget_show_all(GTK_WIDGET(cfg->mainWindow));
	gtk_container_add(GTK_CONTAINER(cfg->frame), cfg->mainWindow);

	CHKPoint();

    for(f=0;f<ICQMNU_COUNTER;f++)
	   cfg->menu[f] = icqskin_CreateMenu(icq, f, cfg);

	icqskin_restoreFrame(cfg->frame,"pos:MainWindow", 150, 375);

    return 0;	
 }



