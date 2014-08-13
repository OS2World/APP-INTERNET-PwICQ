/*
 * ACTIONS.C - Acoes de menu/Botoes
 */
 
 #include <stdio.h>
 #include <malloc.h>
 #include <string.h>

 #include <icqgtk.h>

/*---[ Estruturas ]--------------------------------------------------------------------------*/

/*---[ Constantes ]--------------------------------------------------------------------------*/


/*---[ Prototipos ]--------------------------------------------------------------------------*/
 
 static void updateAll(HICQ);
 static void removeUser(HICQ, ULONG);
 static void removeButton(GtkWidget *, HUSER);
 
/*---[ Implementacao ]-----------------------------------------------------------------------*/

 int icqgtk_sysmenu(HICQ icq, ULONG uin, USHORT menu, ULONG id)
 {
    DBGMessage("System menu");

    switch(id)
    {
    case 38:	// Disconnect
       icqWriteSysLog(icq,PROJECT,"Disconnect request received");
       icqDisconnect(icq);
       break;
       
    case 16:	// Update all users
       icqWriteSysLog(icq,PROJECT,"Updating all users in the contact list");
       updateAll(icq);
       break;
       
    case 17:	// Shutdown
       gtk_widget_destroy( ( (MAINWIN *) icqGetSkinDataBlock(icq))->main );
       break;

    case 27:    // Janela de configuracao
       icqgtk_OpenConfigWindow(icq, 0, 1, NULL);
       break;
       
#ifdef DEBUG
    default:
       DBGPrint("Comando inesperado: %lu",id);
#endif       
    }  
    return 0;  
 }
 
 int icqgtk_modemenu(HICQ icq, ULONG uin, USHORT menu, ULONG id)
 {
    const ICQMODETABLE *mode;
    for(mode = icqQueryModeTable(icq); mode && mode->icon != (USHORT) id && mode->descr; mode++);
    
    if(mode && mode->descr)
    {
       DBGMessage(mode->descr);
       icqSetOnlineMode(icq,mode->mode);
    }
    
    return 0;
 }

 int icqgtk_usermenu(HICQ icq, ULONG uin, USHORT menu, ULONG id)
 {
    DBGTrace(uin);
    
    if(!uin)
       return -1;
       
    switch(id)
    {
    case 0:	// Send Message
       icqNewUserMessage(icq,uin,MSG_NORMAL);
       break;
       
    case 2:	// Send URL
       icqNewUserMessage(icq,uin,MSG_URL);
       break;
    
    case 8:	// Ask for authorization
       icqNewUserMessage(icq,uin,MSG_REQUEST);
       break;
    
    case 29:	// Send authorization
       icqNewUserMessage(icq,uin,MSG_AUTHORIZED);
       break;
    
    case 16:	// Update basic information
       icqRequestUserUpdate(icq,uin);
       break;
    
    case 17:	// Remove from list
       removeUser(icq, uin);
       break;
    
#ifdef DEBUG
    default:
       DBGPrint("Acao de menu inesperada: %lu",id);
#endif    
    }
    
    return 0;
 }

 static void updateAll(HICQ icq)
 {
    CHKPoint();
    
 }

 static void removeUser(HICQ icq, ULONG uin)
 {
   char	  buffer[0x0100];
   HUSER	 usr				= icqQueryUserHandle(icq,uin);
   GtkWidget *dialog, 
             *table,
   		  *confirm,
   		  *cancel,
   		  *temp;

   if(!usr)
      return;
      
   dialog  = gtk_dialog_new();
      
   sprintf(buffer,"Remove user %s (ICQ#%lu) from contact-list?",icqQueryUserNick(usr),uin);
   
   gtk_window_set_title(GTK_WINDOW(dialog),buffer);
   
   table   = gtk_table_new(2,4,FALSE);
   
   temp = gtk_label_new("ICQ#:");
   gtk_label_set_justify(GTK_LABEL(temp),GTK_JUSTIFY_LEFT);
   gtk_table_attach(GTK_TABLE(table), temp, 0, 1, 0, 1, GTK_FILL, 0, 2, 2);
   
   temp = gtk_entry_new();
   sprintf(buffer,"%lu",usr->uin);
   gtk_entry_set_text(GTK_ENTRY(temp),buffer);
   gtk_entry_set_editable(GTK_ENTRY(temp),FALSE);
   gtk_table_attach(GTK_TABLE(table), temp, 1, 2, 0, 1,GTK_EXPAND|GTK_SHRINK|GTK_FILL,0, 2, 2);

   temp = gtk_label_new("Name:");
   gtk_label_set_justify(GTK_LABEL(temp),GTK_JUSTIFY_LEFT);
   gtk_table_attach(GTK_TABLE(table), temp, 2, 3, 0, 1, GTK_FILL, 0, 2, 2);

   temp = gtk_entry_new();
   strncpy(buffer,icqQueryUserFirstName(usr),0xFF);
   strncat(buffer," ",0xFF);
   strncat(buffer,icqQueryUserLastName(usr),0xFF);
   gtk_entry_set_text(GTK_ENTRY(temp),buffer);
   gtk_entry_set_editable(GTK_ENTRY(temp),FALSE);
   gtk_table_attach(GTK_TABLE(table), temp, 3, 4, 0, 1,GTK_EXPAND|GTK_SHRINK|GTK_FILL,0, 2, 2);
    
   temp = gtk_label_new("Nick:");
   gtk_label_set_justify(GTK_LABEL(temp),GTK_JUSTIFY_LEFT);
   gtk_table_attach(GTK_TABLE(table), temp, 0, 1, 1, 2, GTK_FILL, 0, 2, 2);
   
   temp = gtk_entry_new();
   gtk_entry_set_text(GTK_ENTRY(temp),icqQueryUserNick(usr));
   gtk_entry_set_editable(GTK_ENTRY(temp),FALSE);
   gtk_table_attach(GTK_TABLE(table), temp, 1, 2, 1, 2,GTK_EXPAND|GTK_SHRINK|GTK_FILL,0, 2, 2);

   temp = gtk_label_new("E-Mail:");
   gtk_label_set_justify(GTK_LABEL(temp),GTK_JUSTIFY_LEFT);
   gtk_table_attach(GTK_TABLE(table), temp, 2, 3, 1, 2, GTK_FILL, 0, 2, 2);

   temp = gtk_entry_new();
   gtk_entry_set_text(GTK_ENTRY(temp),icqQueryUserEmail(usr));
   gtk_entry_set_editable(GTK_ENTRY(temp),FALSE);
   gtk_table_attach(GTK_TABLE(table), temp, 3, 4, 1, 2,GTK_EXPAND|GTK_SHRINK|GTK_FILL,0, 2, 2);
   
   confirm = gtk_button_new_with_label("Remove this user");
   cancel  = gtk_button_new_with_label("Cancel");
   
   gtk_signal_connect_object(GTK_OBJECT(cancel), "clicked",GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer) dialog);

   gtk_object_set_user_data(GTK_OBJECT(dialog), (gpointer) icq);
   gtk_object_set_user_data(GTK_OBJECT(confirm),(gpointer) dialog);
   gtk_signal_connect(GTK_OBJECT(confirm),  "clicked",    GTK_SIGNAL_FUNC(removeButton), (gpointer) usr);

   gtk_container_add (GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),confirm);
   gtk_container_add (GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),cancel);
   gtk_container_add (GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), table);

   gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
   gtk_window_set_policy(GTK_WINDOW(dialog), FALSE, FALSE, TRUE);

   DBGTracex(icq);
   gtk_widget_show_all(dialog);
}

 static void removeButton( GtkWidget *widGet, HUSER usr)
 {
    GtkWidget *dialog  = gtk_object_get_user_data(GTK_OBJECT(widGet));
    HUSER     icq      = gtk_object_get_user_data(GTK_OBJECT(dialog));
    DBGMessage("Remove user");
    DBGTracex(icq);
    DBGTracex(usr);
    DBGTrace(usr->uin);
    icqRemoveUser(icq,usr);    
    gtk_widget_destroy(dialog);
 }

