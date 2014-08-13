/*
 * Processamento do dialogo de mensagem
 */

 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>
 #include <ctype.h>

 #include <msgdlg.h>
 
 #include <icqgtk.h>

/*---[ Defines ]--------------------------------------------------------------------------------------------*/

 #pragma pack(1)
 struct cfg
 {
    USHORT   	sz;
    HICQ	      icq;
    ULONG		uin;
    HUSER	   usr;
    USHORT   	type;

    GtkWidget  *main;
    GtkWidget	*cntl[ICQMSGC_WIDGETS];
    GtkWidget  *sendTo;

    USHORT	   sendToOptions;
    BOOL		   out;
    BOOL		   sysMsg;
    
    const MSGMGR *mgr;
    
 };

 #ifdef MSGEDITWINDOW
    #undef MSGEDITWINDOW
 #endif
 
 #define MSGEDITWINDOW struct cfg *
 
 #define SETTEXT(c,i,t) gtk_entry_set_text(GTK_ENTRY(wnd->cntl[i]),t)

/*---[ Constants ]------------------------------------------------------------------------------------------*/

 static const char *stringtable[] = { "Close",
    							      "ICQ#:",
    							      "",
     							     "Nick:",
    							      "",
    							      "Name:",
    							      "",
    						          "E-Mail:",
    							      "",
    							      "",
    							      "",
    							      "",
    							      "",
    							      "Accept",
    							      "Reject",
    							      "Refuse",
    							      "Reply",
    							      "Add user",
    							      "",
    							      "Browse",
    							      "Subject:",
    							      "",
    							      "Next",
    							      "Send",
    							      "Open",
    							      "Send to",
    							      
    							      "Date:",
    							      "Auto-Open",
    							      "Urgent",
    							      "Authorize"
 							   };
 							   
 static const USHORT labels[]    = { ICQMSGC_STATICUIN,
                                     ICQMSGC_STATICNICK,
                                     ICQMSGC_STATICNAME,
                                     ICQMSGC_STATICEMAIL,
                                     ICQMSGC_STATICSUBJ,
                                     0
                                   };

 static const USHORT textBoxes[] = { ICQMSGC_UIN, 
 							        ICQMSGC_NICKNAME, 
 							        ICQMSGC_NAME, 
 							        ICQMSGC_EMAIL, 
 							        ICQMSGC_SUBJECT, 
 							        ICQMSGC_ENTRY,
 							        0 
 							      };
    
 static const USHORT buttons[]   = { ICQMSGC_NEXT,
 									ICQMSGC_ACCEPT,
 									ICQMSGC_REJECT,
 									ICQMSGC_REFUSE,
 									ICQMSGC_REPLY,
 									ICQMSGC_ADD,
 									ICQMSGC_SEND,
 									ICQMSGC_BROWSE,
 									ICQMSGC_OPEN,
 									ICQMSGC_SENDTO,
 									0
 								   };
 									
/*---[ Used by plugins ]---------------------------------------------------------------------------------*/

 static int   _System loadUserFields(MSGEDITWINDOW);
 static int   _System setMessage(MSGEDITWINDOW, USHORT, BOOL, HMSG);
 static int   _System setTitle(MSGEDITWINDOW,const char *);
 static int   _System setVisible(MSGEDITWINDOW, USHORT, BOOL);
 static int   _System setEnabled(MSGEDITWINDOW, USHORT, BOOL);
 static int   _System setString(MSGEDITWINDOW,USHORT,const char *);
 static int   _System getString(MSGEDITWINDOW,USHORT,int,char *);
 static int   _System setCheckBox(MSGEDITWINDOW,USHORT,BOOL);
 static int   _System getCheckBox(MSGEDITWINDOW,USHORT);
 static void  _System loadString(MSGEDITWINDOW, USHORT, USHORT);
 static void  _System setReply(MSGEDITWINDOW, USHORT);
 static void  _System showTime(MSGEDITWINDOW, USHORT, time_t);

 static const MSGEDITHELPER msgHelper = {	
		sizeof(MSGEDITHELPER),
                  
		(int (* _System)(void *,const char *))		   setTitle,
		(int (* _System)(void *, USHORT, BOOL, HMSG))	setMessage,

		(int (*  _System)(void *, USHORT, BOOL))         setVisible,
		(int (*  _System)(void *, USHORT, BOOL))         setEnabled,

		(int (* _System)(void *,USHORT,const char *))    setString,
		(int (* _System)(void *,USHORT,int,char *))      getString,

		(int (* _System)(void *,USHORT, BOOL))           setCheckBox,
		(int (* _System)(void *,USHORT))                 getCheckBox,

		(int (* _System)(void *))                        loadUserFields,
 		        
		(void (* _System)(void *, USHORT, USHORT))       loadString,

		(void (* _System)(void *, USHORT))               setReply,
		(void (* _System)(void *, USHORT, time_t))       showTime
 
 								 };

/*---[ Prototipes ]-----------------------------------------------------------------------------------------*/

 static void destroy( GtkWidget *, struct cfg *);
// static void size(GtkWidget *widget, GtkAllocation *, struct cfg *);
 static void closeButton(GtkWidget *, struct cfg *);
 static void sendButton(GtkWidget *, struct cfg *);
 static void replyButton(GtkWidget *, struct cfg *);
 static void ajustUser(struct cfg *, HMSG);
 static void actButton(GtkWidget *, struct cfg *);
 static BOOL readNext(struct cfg *wnd);
 static BOOL addNewUser(struct cfg *);
 static gint popupmenu(GtkWidget *, GdkEvent *); 
// static void actSendTo(GtkWidget *, struct cfg *);
 
 static void _System event(HICQ, ULONG, char, USHORT, ULONG, struct cfg *);

/*---[ Implement ]------------------------------------------------------------------------------------------*/

 static void destroySendTo(GtkWidget *widget, struct cfg *wnd)
 {
    DBGMessage("Menu Send-to foi destruido");
    DBGTracex(wnd->sendTo);
    DBGTracex(widget);
     
    if(widget == wnd->sendTo)
    {
       wnd->sendTo        = NULL;
       wnd->sendToOptions = 0;
    }
 }

 int icqgtk_openMessageWindow(HICQ icq, ULONG uin, USHORT type, BOOL out, HMSG msg)
 {
    struct cfg     *wnd;
    GtkWidget      *temp;
    GtkWidget      *box;
    GtkWidget	  *vbox;
    GtkRequisition size;
    GtkStyle	   *style;
    GtkStyle	   *src;
    const USHORT   *itn;
    char		   buffer[0x0100];
    
    if(!uin)
       return -1;

    wnd = malloc(sizeof(struct cfg));
    
    if(!wnd)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when opening message window");
       icqAbend(icq,ICQMSG_MEMORYERROR);
       return -1;
    }
    
    DBGTracex(out);
    
    memset(wnd,0,sizeof(struct cfg));
    wnd->sz   = sizeof(struct cfg);
    wnd->uin  = uin;
    wnd->icq  = icq;

    wnd->usr  = icqQueryUserHandle(wnd->icq,wnd->uin);
    if(wnd->usr)
       wnd->usr->wndCounter++;
           
    DBGMessage("Abrir caixa de mensagem");

    wnd->main = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    sprintf(buffer,"ICQ#%lu",icqQueryUIN(icq));
    gtk_window_set_wmclass(GTK_WINDOW(wnd->main),"msgEdit", buffer);

    box       = gtk_vbox_new(FALSE,2);

    gtk_signal_connect(GTK_OBJECT(wnd->main), "destroy", GTK_SIGNAL_FUNC(destroy), wnd);

    /* Constroi campos estaticos */
    for(itn = labels; *itn;itn++)
    {
       wnd->cntl[*itn]   = gtk_label_new(stringtable[*itn]);
       gtk_misc_set_alignment(GTK_MISC(wnd->cntl[*itn]), 0, .5);
    }
    
    /* Constroi campos de dados */

    for(itn = textBoxes;*itn;itn++)
    {
       temp = wnd->cntl[*itn] = gtk_entry_new();
       gtk_entry_set_editable(GTK_ENTRY(temp),FALSE);
       gtk_widget_size_request(temp,&size);
       gtk_widget_set_usize(temp,10,size.height);
    }

    /* Constroi botoes */
    wnd->cntl[ICQMSGC_CLOSE]  = gtk_button_new_with_label("Close");
    for(itn = buttons;*itn;itn++)
       wnd->cntl[*itn] = gtk_button_new_with_label(stringtable[*itn]);

    /* Constroi a caixa de edicao */
    temp = wnd->cntl[ICQMSGC_TEXT] = gtk_text_new(NULL,NULL);
    gtk_text_set_editable(GTK_TEXT(temp),FALSE);
    gtk_text_set_word_wrap(GTK_TEXT(temp),TRUE);

    style = gtk_style_copy(gtk_widget_get_default_style());
    src   = gtk_widget_get_style(wnd->cntl[ICQMSGC_UIN]);
    
    style->base[0] = src->base[0];
    style->text[0] = src->text[0];

//    style->font    = src->font;
//    memcpy(style->base,src->base,sizeof(GtkColor));
//    memcpy(style->text,src->text,sizeof(GtkColor));

    gtk_widget_set_style(temp,style);
    gtk_style_unref(style);

    gtk_widget_set_usize(temp,10,size.height);

    /* Constroi a parte superior da janela */
    wnd->cntl[ICQMSGC_TOFROM] = gtk_frame_new(out ? "To" : "From");
    temp  = gtk_table_new(2,4,FALSE);

    gtk_table_attach(GTK_TABLE(temp), wnd->cntl[ICQMSGC_STATICNICK], 0, 1, 0, 1, 0, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(temp), wnd->cntl[ICQMSGC_NICKNAME], 1, 2, 0, 1,GTK_EXPAND|GTK_SHRINK|GTK_FILL,0, 2, 2);

    gtk_table_attach(GTK_TABLE(temp), wnd->cntl[ICQMSGC_STATICNAME], 2, 3, 0, 1, 0, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(temp), wnd->cntl[ICQMSGC_NAME], 3, 4, 0, 1,GTK_EXPAND|GTK_SHRINK|GTK_FILL,0, 2, 2);
    
    gtk_table_attach(GTK_TABLE(temp), wnd->cntl[ICQMSGC_STATICUIN], 0, 1, 1, 2, 0, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(temp), wnd->cntl[ICQMSGC_UIN], 1, 2, 1, 2,GTK_EXPAND|GTK_SHRINK|GTK_FILL,0, 2, 2);

    gtk_table_attach(GTK_TABLE(temp), wnd->cntl[ICQMSGC_STATICEMAIL], 2, 3, 1, 2, 0, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(temp), wnd->cntl[ICQMSGC_EMAIL], 3, 4, 1, 2,GTK_EXPAND|GTK_SHRINK|GTK_FILL,0, 2, 2);

    gtk_container_add (GTK_CONTAINER(wnd->cntl[ICQMSGC_TOFROM]), temp);
    gtk_box_pack_start(GTK_BOX(box), wnd->cntl[ICQMSGC_TOFROM],FALSE,FALSE,0);

    /* Parte do meio */
    wnd->cntl[ICQMSGC_TEXTBOX] = gtk_frame_new(NULL);
    vbox = gtk_vbox_new(FALSE,0);

    /* Caixa de subject/User name */
    temp = gtk_hbox_new(FALSE,2);
    gtk_box_pack_start(GTK_BOX(temp), wnd->cntl[ICQMSGC_STATICSUBJ],FALSE,FALSE,0);
    gtk_box_pack_end(GTK_BOX(temp), wnd->cntl[ICQMSGC_SUBJECT],TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(vbox),temp,FALSE,FALSE,0);

    /* Caixa de texto */    
    
    temp = gtk_hbox_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(temp), wnd->cntl[ICQMSGC_TEXT],TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(temp),gtk_vscrollbar_new(GTK_TEXT(wnd->cntl[ICQMSGC_TEXT])->vadj),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),temp,TRUE,TRUE,0);

//    gtk_box_pack_start(GTK_BOX(vbox),wnd->cntl[ICQMSGC_TEXT],TRUE,TRUE,0);

    /* Area de URL/Nome de arquivo */
    temp = gtk_hbox_new(FALSE,2);

    gtk_box_pack_start(GTK_BOX(temp), wnd->cntl[ICQMSGC_ENTRY],TRUE,TRUE,0);
    gtk_box_pack_end(GTK_BOX(temp), wnd->cntl[ICQMSGC_BROWSE],FALSE,FALSE,0);

    gtk_box_pack_start(GTK_BOX(vbox),temp,FALSE,FALSE,0);
    gtk_container_add (GTK_CONTAINER(wnd->cntl[ICQMSGC_TEXTBOX]),vbox);
    gtk_box_pack_start(GTK_BOX(box),wnd->cntl[ICQMSGC_TEXTBOX],TRUE,TRUE,0);

    /* Parte de baixo */
    temp = gtk_hbox_new(FALSE,0);

    wnd->cntl[ICQMSGC_CHECKBOX] = gtk_check_button_new_with_label( stringtable[ out ? ICQMSGS_URGENT : ICQMSGS_AUTOOPEN ] );

    gtk_box_pack_start(GTK_BOX(temp), wnd->cntl[ICQMSGC_CHECKBOX],FALSE,FALSE,0);

    gtk_box_pack_end(GTK_BOX(temp), wnd->cntl[ICQMSGC_CLOSE],FALSE,FALSE,0);
    gtk_box_pack_end(GTK_BOX(temp), wnd->cntl[ICQMSGC_SEND],FALSE,FALSE,0);
    gtk_box_pack_end(GTK_BOX(temp), wnd->cntl[ICQMSGC_NEXT],FALSE,FALSE,0);
    gtk_box_pack_end(GTK_BOX(temp), wnd->cntl[ICQMSGC_ACCEPT],FALSE,FALSE,0);
    gtk_box_pack_end(GTK_BOX(temp), wnd->cntl[ICQMSGC_REJECT],FALSE,FALSE,0);
    gtk_box_pack_end(GTK_BOX(temp), wnd->cntl[ICQMSGC_REFUSE],FALSE,FALSE,0);
    gtk_box_pack_end(GTK_BOX(temp), wnd->cntl[ICQMSGC_SENDTO],FALSE,FALSE,0);
    gtk_box_pack_end(GTK_BOX(temp), wnd->cntl[ICQMSGC_REPLY],FALSE,FALSE,0);
    gtk_box_pack_end(GTK_BOX(temp), wnd->cntl[ICQMSGC_ADD],FALSE,FALSE,0);
    gtk_box_pack_end(GTK_BOX(temp), wnd->cntl[ICQMSGC_OPEN],FALSE,FALSE,0);

    gtk_box_pack_start(GTK_BOX(box),temp,FALSE,FALSE,0);
    
    /* Finaliza */        
    gtk_container_set_border_width(GTK_CONTAINER(box),4);
    gtk_widget_show_all(box);
    gtk_container_add (GTK_CONTAINER(wnd->main), box);
    
    /* Conecta eventos */
    gtk_signal_connect(GTK_OBJECT(wnd->cntl[ICQMSGC_CLOSE]),  "clicked",    GTK_SIGNAL_FUNC(closeButton), wnd);
    gtk_signal_connect(GTK_OBJECT(wnd->cntl[ICQMSGC_SEND]),   "clicked",    GTK_SIGNAL_FUNC(sendButton),  wnd);
    gtk_signal_connect(GTK_OBJECT(wnd->cntl[ICQMSGC_REPLY]),  "clicked",    GTK_SIGNAL_FUNC(replyButton), wnd);

    gtk_signal_connect(GTK_OBJECT(wnd->cntl[ICQMSGC_NEXT]),   "clicked",    GTK_SIGNAL_FUNC(actButton),   wnd);
    gtk_signal_connect(GTK_OBJECT(wnd->cntl[ICQMSGC_ACCEPT]), "clicked",    GTK_SIGNAL_FUNC(actButton),   wnd);
    gtk_signal_connect(GTK_OBJECT(wnd->cntl[ICQMSGC_REJECT]), "clicked",    GTK_SIGNAL_FUNC(actButton),   wnd);
    gtk_signal_connect(GTK_OBJECT(wnd->cntl[ICQMSGC_REFUSE]), "clicked",    GTK_SIGNAL_FUNC(actButton),   wnd);
    gtk_signal_connect(GTK_OBJECT(wnd->cntl[ICQMSGC_ADD]),    "clicked",    GTK_SIGNAL_FUNC(actButton),   wnd);
    gtk_signal_connect(GTK_OBJECT(wnd->cntl[ICQMSGC_BROWSE]), "clicked",    GTK_SIGNAL_FUNC(actButton),   wnd);
    gtk_signal_connect(GTK_OBJECT(wnd->cntl[ICQMSGC_OPEN]),   "clicked",    GTK_SIGNAL_FUNC(actButton),   wnd);
    
    gtk_object_set_user_data(GTK_OBJECT(wnd->cntl[ICQMSGC_SENDTO]),(gpointer) wnd);
    gtk_signal_connect_object(GTK_OBJECT(wnd->cntl[ICQMSGC_SENDTO]), "event", GTK_SIGNAL_FUNC(popupmenu), GTK_OBJECT(wnd->cntl[ICQMSGC_SENDTO]));

#ifdef DEBUG
    wnd->sendTo = gtk_menu_new();
    gtk_object_set_user_data(GTK_OBJECT(wnd->sendTo),(gpointer) wnd);
    gtk_signal_connect(GTK_OBJECT(wnd->sendTo),"destroy",GTK_SIGNAL_FUNC(destroySendTo),wnd);
#else
    wnd->sendTo        = NULL;
    wnd->sendToOptions = 0;
#endif    

    /* Deixa o plugin responsavel fazer seus ajustes */
    setMessage(wnd, type, out, msg);

    /* Mostra e sai */    
    icqgtk_RestoreWindow(wnd->main,wnd->icq,0,"msgWindowPos",404,247);
    gtk_widget_show(wnd->main);

    icqgtk_insertListener(wnd->icq, (void (_System *)(HICQ,ULONG,char,USHORT,ULONG,void *)) event, wnd);

    return 0;
 }

 static void _System event(HICQ icq, ULONG uin, char type, USHORT eventCode, ULONG parm, struct cfg *wnd)
 {
    /* Processa eventos */
    
 }

 static void destroy( GtkWidget *widget, struct cfg *wnd)
 {
    DBGMessage("Janela de mensagems destruida");
    
    if(wnd->sz != sizeof(struct cfg))
       return;

    icqgtk_removeListener(wnd->icq, (void (_System *)(HICQ,ULONG,char,USHORT,ULONG,void *)) event, wnd);

    wnd->usr  = icqQueryUserHandle(wnd->icq,wnd->uin);
    if(wnd->usr)
       wnd->usr->wndCounter--;

    memset(wnd,0,sizeof(struct cfg));    
    free(wnd);
    DBGMessage("Clean-up da janela de mensagems completo");
 }

 static int _System loadUserFields(struct cfg *wnd)
 {
    char  buffer[0x0100];

    if(!wnd || wnd->sz != sizeof(struct cfg))
       return -1;
       
    wnd->usr = icqQueryUserHandle(wnd->icq,wnd->uin);
    
    sprintf(buffer,"%lu",wnd->uin);
    SETTEXT(c,ICQMSGC_UIN,buffer);

    if(!wnd->usr)
       return -1;
    
    strncpy(buffer,icqQueryUserFirstName(wnd->usr),0xFF);    
    strncat(buffer," ",0xFF);
    strncat(buffer,icqQueryUserLastName(wnd->usr),0xFF);
    SETTEXT(c,ICQMSGC_NAME,buffer);
    
    SETTEXT(c,ICQMSGC_NICKNAME,icqQueryUserNick(wnd->usr));
    SETTEXT(c,ICQMSGC_EMAIL,icqQueryUserEmail(wnd->usr));
    
    if(!wnd->out)
    {
       setEnabled(wnd,ICQMSGC_CHECKBOX,TRUE);
       DBGTrace(wnd->usr->flags & USRF_AUTOOPEN);
       setCheckBox(wnd,ICQMSGC_CHECKBOX,(wnd->usr->flags & USRF_AUTOOPEN) != 0);
    }
    
    return 0;
 }

 static void closeButton(GtkWidget *widGet, struct cfg *wnd)
 {
    icqgtk_StoreWindow(wnd->main, wnd->icq, wnd->uin, "msgWindowPos");
    gtk_widget_destroy(wnd->main);
 }

 static void sendButton(GtkWidget *widGet, struct cfg *wnd)
 {
    DBGMessage("Enviar...");
    
    if(!(wnd->mgr && wnd->mgr->send))
       return;

    if(wnd->mgr->send(&msgHelper,wnd,wnd->icq,wnd->uin))
       return;
       
    if(icqLoadValue(wnd->icq,"NextOnSend",1))
    {
       if(!readNext(wnd))
          return;
    }
       
    icqgtk_StoreWindow(wnd->main, wnd->icq, wnd->uin, "msgWindowPos");
    gtk_widget_destroy(wnd->main);
 }

 static void replyButton(GtkWidget *widGet, struct cfg *cfg)
 {
    if(cfg->mgr)
       setReply(cfg,cfg->mgr->reply);
    else
       setReply(cfg,MSG_NORMAL);
 }

// void icqgtk_addMenuOption(MAINWIN *wnd, USHORT id, USHORT icon, MENUCALLBACK *cbk, const char *text)

 #pragma pack(1)
 struct menu_option
 {
    struct cfg   		*wnd;
    USHORT		  		id;
    SENDTOCALLBACK	*cbk;
 };

 #pragma pack()

 static void menuAction(const struct menu_option *opt)
 {
    if(opt->cbk)
    {
       opt->cbk(opt->wnd->icq,opt->wnd->uin,opt->wnd->mgr,"teste 1","teste 2");  
    }
 }

 static int _System insertSndMenu(struct cfg *wnd, USHORT id, const char *text, SENDTOCALLBACK *cbk)
 {
    GtkWidget 				*item  = gtk_menu_item_new_with_label(text);
    struct menu_option	parm   = { wnd, id, cbk };
    
/* 	
    GtkWidget 				*item  = gtk_menu_item_new();
    GtkWidget 				*box   = gtk_hbox_new(FALSE,5);

// "Open");

    gtk_box_pack_start(GTK_BOX(box), gtk_label_new(text),  FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(item),box);
    gtk_menu_append(GTK_MENU(wnd->sendTo),item);
    gtk_widget_show_all(item);

*/
    gtk_menu_append(GTK_MENU(wnd->sendTo),item);
    gtk_widget_show_all(item);

    gtk_signal_connect_object(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(menuAction), g_memdup(&parm, sizeof(parm)));

    wnd->sendToOptions++;

    return 0;
 }

#ifdef DEBUG

 static int _System testMenu(HICQ icq, ULONG uin, const MSGMGR *mgr, const char *text, const char *url)
 {
    DBGMessage("Teste de menu");
    return 0;
 }

#endif

 static int _System setMessage(struct cfg *wnd, USHORT type, BOOL out, HMSG msg)
 {
    MSGFORMATTER	*formatter	= NULL;
    gint 		   width,height;

    if(wnd->mgr && wnd->mgr->exit)
       wnd->mgr->exit(&msgHelper, wnd, wnd->icq, wnd->uin, wnd->type, type);
    
    wnd->type = type;
    wnd->out  = out;

    if(msg && msg->mgr)
       wnd->mgr = msg->mgr;
    else
       wnd->mgr = icqQueryMessageManager(wnd->icq,wnd->type);
 
    if(wnd->mgr)
       formatter =wnd->mgr->formatWindow;

    if(!formatter)
       formatter = &icqgtk_unknownMsgFormatter;

    gtk_object_set(	GTK_OBJECT(wnd->cntl[ICQMSGC_CHECKBOX]), 
    				   "label", 
    				   stringtable[ out ? ICQMSGS_URGENT : ICQMSGS_AUTOOPEN ], NULL);

    setEnabled(wnd, ICQMSGC_CHECKBOX, FALSE);

/*    
    setVisible(wnd, ICQMSGC_ADD, wnd->usr != NULL);
    
    if(wnd->usr)
       setEnabled(wnd, ICQMSGC_ADD, wnd->usr->flags & USRF_TEMPORARY );
*/       

//    setVisible(wnd, ICQMSGC_ADD, !wnd->usr || (wnd->usr->flags & USRF_TEMPORARY));

    ajustUser(wnd,msg);

    /* Ajusta a caixa de edicao */
    gtk_text_set_editable(GTK_TEXT(wnd->cntl[ICQMSGC_TEXT]),msg==NULL);
    gtk_entry_set_editable(GTK_ENTRY(wnd->cntl[ICQMSGC_ENTRY]),msg==NULL);

    /* Pede ao plugin para formatar a janela */
    formatter(&msgHelper,wnd,wnd->icq,wnd->uin,type,out,msg);

    /* Reconstroi menu send-to */
    if(wnd->sendTo)
    {
       DBGMessage("Retirar todas as opcoes do menu send-to");
       gtk_object_destroy(GTK_OBJECT(wnd->sendTo));  
    }
    
    wnd->sendTo = gtk_menu_new();
    gtk_object_set_user_data(GTK_OBJECT(wnd->sendTo),(gpointer) wnd);
    gtk_signal_connect(GTK_OBJECT(wnd->sendTo),"destroy",GTK_SIGNAL_FUNC(destroySendTo),wnd);

#ifdef DEBUG
    if(msg)
       DBGTracex(msg->type);

    insertSndMenu(wnd, 1, "T1", testMenu);
    insertSndMenu(wnd, 1, "T2", testMenu);

#endif    

    CHKPoint();
    icqInsertSendToOptions(wnd->icq, wnd, type, out, wnd->mgr, (void *) &insertSndMenu);

    DBGTrace(wnd->sendToOptions);

    setVisible(wnd,ICQMSGC_SENDTO, (wnd->sendToOptions > 0) );

    if(msg)
       icqRemoveMessage(wnd->icq,wnd->uin,msg);

    setEnabled(wnd,ICQMSGC_NEXT,icqQueryMessage(wnd->icq,wnd->sysMsg ? 0 : wnd->uin) != NULL);
    
    if(out)
       gtk_window_set_focus(GTK_WINDOW(wnd->main),wnd->cntl[ICQMSGC_TEXT]);

    if(wnd->main->window)
    {
       /* Forca um resize da janela */
       gdk_window_get_size(wnd->main->window,&width,&height);
       gtk_window_set_default_size(GTK_WINDOW(wnd->main),width,height);
    }

    return 0;
 }

 static int _System setTitle(struct cfg *wnd,const char *txt)
 {
    gtk_window_set_title(GTK_WINDOW(wnd->main),txt);
    return 0;
 }
 
 static int _System setVisible(struct cfg *wnd, USHORT id, BOOL modo)
 {
    if(modo)
       gtk_widget_show(wnd->cntl[id]);
    else
       gtk_widget_hide(wnd->cntl[id]);
    return 0;
 }
 
 static int _System setEnabled(struct cfg *wnd, USHORT id, BOOL modo)
 {
    DBGTracex(modo);
    gtk_widget_set_sensitive(wnd->cntl[id],modo);
    return 0;
 }

 static void ajustUser(struct cfg *cfg, HMSG msg)
 {
    char buffer[0x0100];

    CHKPoint();
        
    cfg->usr    = icqQueryUserHandle(cfg->icq,cfg->uin);
    cfg->sysMsg = (cfg->usr == NULL);
    
    *buffer = 0;
    if(cfg->mgr && cfg->mgr->title)
    {
       cfg->mgr->title(cfg->icq,cfg->usr,cfg->uin,cfg->out,msg,buffer,0xFF);
    }
    else
    {
       sprintf(buffer,"Message %04x %s %s (ICQ#%lu)",
       					cfg->type,
       					cfg->out ? "to" : "from",
       					icqQueryUserNick(cfg->usr),
       					cfg->uin );
    } 
    
    setEnabled(cfg,ICQMSGC_CHECKBOX,FALSE);

    setVisible(cfg, ICQMSGC_ADD, !cfg->sysMsg);

    if(!cfg->sysMsg)
    {
       DBGTracex(cfg->usr->flags & USRF_AUTHORIZE);
       setEnabled(cfg, ICQMSGC_ADD, (cfg->usr->flags & USRF_TEMPORARY) != 0  );
       setString(cfg,ICQMSGC_ADD,stringtable[ (cfg->usr->flags & USRF_AUTHORIZE) ? ICQMSGS_AUTHORIZE : ICQMSGS_ADD]);
    }
    
    if(*buffer)
       setTitle(cfg,buffer);
       
    if(cfg->mgr && cfg->mgr->flags & ICQMSGMF_LOADUSERFIELDS)
       loadUserFields(cfg);
 }

 static int _System setString(MSGEDITWINDOW cfg, USHORT id,const char *txt)
 {
    const USHORT *tbl;
    GtkWidget	*temp	= cfg->cntl[id];

    if(!txt)
       txt = stringtable[id];

    if(id == ICQMSGC_TEXT)
    {
       gtk_editable_delete_text(GTK_EDITABLE(temp),0,-1);
       gtk_text_insert(GTK_TEXT(temp), NULL, NULL, NULL, txt, -1);
       return 0;
    }
  
    for(tbl = textBoxes; *tbl; tbl++)  
    {
       if(*tbl == id)
       {
          gtk_entry_set_text(GTK_ENTRY(cfg->cntl[id]),txt);
          return 0;
       }
    }
    
    for(tbl = labels; *tbl; tbl++)  
    {
       if(*tbl == id)
       {
          gtk_label_set_text(GTK_LABEL(cfg->cntl[id]),txt);
          return 0;
       }
    }
    
    for(tbl = buttons; *tbl; tbl++)  
    {
       if(*tbl == id)
       {
          gtk_object_set(GTK_OBJECT(cfg->cntl[id]),"label",txt,NULL);
          return 0;
       }
    }

    DBGPrint("Atualizacao para tipo desconhecido: %d",id);
    
    return -1;
 }

 static int _System getString(MSGEDITWINDOW cfg, USHORT id, int sz, char *buffer)
 {
    const USHORT  *tbl;
    gpointer	  ptr;
    
    if(id == ICQMSGC_TEXT)
    {
       ptr = gtk_editable_get_chars(GTK_EDITABLE(cfg->cntl[id]),0,-1);
       strncpy(buffer,ptr,sz);
       g_free(ptr);
       return strlen(buffer);
    }
  
    for(tbl = textBoxes; *tbl; tbl++)  
    {
       if(*tbl == id)
       {
          ptr = gtk_editable_get_chars(GTK_EDITABLE(cfg->cntl[id]),0,-1);
          strncpy(buffer,ptr,sz);
          g_free(ptr);
          return strlen(buffer);
       }
    }
    
    for(tbl = labels; *tbl; tbl++)  
    {
       if(*tbl == id)
       {
          gtk_label_get(GTK_LABEL(cfg->cntl[id]),(gchar **) &ptr);
          if(!ptr)
          {
             *buffer = 0;
             return 0;
          }
          strncpy(buffer,ptr,sz);
          return strlen(buffer);
       }
    }
    return -1;
 }

 static int _System setCheckBox(MSGEDITWINDOW cfg, USHORT id, BOOL modo)
 {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cfg->cntl[id]),(gboolean) modo);
    return 0;
 }
 
 int _System getCheckBox(MSGEDITWINDOW cfg, USHORT id)
 {
    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cfg->cntl[id])) ? 1 : 0;
 }

 static void _System loadString(MSGEDITWINDOW cfg, USHORT id, USHORT str)
 {
    DBGTrace(id);
    DBGTrace(str);
    setString(cfg,id,stringtable[str]);
 }

 static void _System setReply(MSGEDITWINDOW cfg, USHORT type)
 {
    if(type == 0xFFFF)
       return;
    setMessage(cfg, type, TRUE, NULL);
 }
 
 static void _System showTime(MSGEDITWINDOW cfg, USHORT id, time_t tm)
 {
    char buffer[80];
    strftime(buffer,79,"%a %m/%d/%Y %T",localtime(&tm));
    setString(cfg,id,buffer);
 }

 static BOOL readNext(struct cfg *wnd)
 {
    HMSG msg = icqQueryMessage(wnd->icq,wnd->sysMsg ? 0 : wnd->uin);

    if(msg)
    {    
       setMessage(wnd, msg->type, FALSE, msg);
       return FALSE;
    }
    setEnabled(wnd,ICQMSGC_NEXT,FALSE);
    return TRUE;
 }

 static void actButton(GtkWidget *widGet, struct cfg *wnd)
 {
    const USHORT *tbl;
    char 		buffer[0x0100];
    
    DBGMessage("Acao de botao");

    for(tbl = buttons; *tbl && widGet != wnd->cntl[*tbl]; tbl++);
    
    if(!*tbl)
    {
       icqWriteSysLog(wnd->icq,PROJECT,"Unexpected button action");
       return;
    }
    
    DBGTrace(*tbl);
 
    if(wnd->mgr && wnd->mgr->action)
    {
       if(wnd->mgr->action(&msgHelper,wnd,wnd->icq,wnd->uin,*tbl))
          return;
    }
    
    switch(*tbl)
    {
    case ICQMSGC_ACCEPT:
       setReply(wnd,MSG_AUTHORIZED);
       break;
       
    case ICQMSGC_REFUSE:
       setReply(wnd,MSG_REFUSED);
       break;

    case ICQMSGC_NEXT:
       readNext(wnd);
       break;
              
    case ICQMSGC_REJECT:
       break;
     
    case ICQMSGC_ADD:
       addNewUser(wnd);
       break;

    case ICQMSGC_OPEN:
       *buffer = 0;
       getString(wnd, ICQMSGC_URL, 0xFF, buffer);
       if(*buffer)
          icqOpenURL(wnd->icq,buffer);
       break;
       
#ifdef DEBUG
    default:
       DBGPrint("Botao desconhecido: id=%d",(int) *tbl);
#endif          
    }
 
 }

 static BOOL addNewUser(MSGEDITWINDOW msg)
 {
    char nickName[20];
    char name[80];
    char *lastName;

    msg->usr = icqQueryUserHandle(msg->icq,msg->uin);
    if(!msg->usr)
       return TRUE;
     
    if(!(msg->usr->flags & USRF_TEMPORARY) )
       return TRUE;

    if(msg->usr->flags & USRF_AUTHORIZE)
    {
       setMessage(msg, MSG_REQUEST, TRUE, NULL);
       return FALSE;
    }

    getString(msg, ICQMSGC_NICKNAME, 19, nickName);
    getString(msg, ICQMSGC_NAME, 79,name);

    for(lastName = name;*lastName && !isspace(*lastName);lastName++);

    if(*lastName)
       *(lastName++) = 0;

    DBGTracex(msg);
    DBGTracex(msg->icq);
    DBGTracex(msg->uin);

    DBGMessage(nickName);
    DBGMessage(name);
    DBGMessage(lastName);

    icqAddNewUser(msg->icq, msg->uin, nickName, name, lastName, NULL, FALSE);

    CHKPoint();

    return FALSE;  
 }

/*
 static void actSendTo(GtkWidget *widGet, struct cfg *wnd)
 {
    gtk_menu_popup(GTK_MENU(wnd->sendTo), NULL, NULL, NULL, NULL, 0, 0);
 }
*/ 

 static gint popupmenu(GtkWidget *widget, GdkEvent *event)
 {
    if (event->type == GDK_BUTTON_PRESS) 
    {
    	  struct cfg     *wnd    = gtk_object_get_user_data(GTK_OBJECT(widget));
        GdkEventButton *bevent = (GdkEventButton *) event; 

        DBGTracex(widget);
        DBGTracex(wnd);

        gtk_menu_popup(GTK_MENU(wnd->sendTo), NULL, NULL, NULL, NULL,bevent->button, bevent->time);

        return TRUE;
    }
    return FALSE;
 }
