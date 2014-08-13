/*
 * TABLE.C - Cria tabela
 */

 #include <stdio.h> 
 #include <malloc.h>
 #include <string.h>
 
 #include <icqgtk.h>

/*---[ Defines ]--------------------------------------------------------------------------------------------*/

 #pragma pack(1)

 typedef struct wcntl
 {
    USHORT		id;
    WIDGET		widget;
    const TABLEDEF	*element;
    const DLGMGR	*mgr;
    WIDGET		wdg;
 } WCNTL;
 
 typedef struct dlgData
 {
    USHORT   	   sz;
    HICQ	        icq;
    ULONG		   uin;
    USHORT		  elements;
    USHORT		  pos;
    WIDGET		  table;
    WIDGET		  radio;
    const DLGMGR	*mgr;
    
    void			*userData;
 } DLGDATA;

/*---[ Constants ]------------------------------------------------------------------------------------------*/

 static int _System setString(HWND,USHORT,const char *);
 static int _System getString(HWND,USHORT,int,char *);
 static int _System setTextLimit(HWND,USHORT,int);
 static int _System getLength(HWND,USHORT);

 static int _System setCheckBox(HWND,USHORT,BOOL);
 static int _System getCheckBox(HWND,USHORT);

 static int  _System setSpinButton(HWND,USHORT,int);
 static int  _System getSpinButton(HWND,USHORT);

 static int  _System setRadioButton(HWND,USHORT,int);
 static int  _System getRadioButton(HWND,USHORT,int);
 
 static int  _System setEnabled(HWND,USHORT,BOOL);

 static int  _System listBoxInsert(HWND,USHORT,ULONG,const char *);

 static int  _System populateEventList(HWND, USHORT);

 static int  _System loadString(HWND, ULONG, USHORT, const char *, int, const char *);
 static int  _System saveString(HWND,ULONG,USHORT,const char *,int);
 
 static int  _System loadCheckBox(HWND,ULONG,USHORT,const char *,BOOL);
 static int  _System saveCheckBox(HWND,ULONG,USHORT,const char *);

 static int  _System loadRadioButton(HWND,ULONG,USHORT,const char *, int, int);
 static int  _System saveRadioButton(HWND,ULONG,USHORT,const char *, int);

 static void * _System getUserData(HWND);

 static int  _System browseFiles(HWND,USHORT, BOOL, const char *, const char *, const char *, char *);

 const DLGHELPER icqgtk_tableHelper = { sizeof(DLGHELPER),
		                                setString,
                                        getString,
                                        setTextLimit,
                                        getLength,
                                        
                                        setCheckBox,
                                        getCheckBox,
                                        
                                        setSpinButton,
                                        getSpinButton,
                                        
                                        setRadioButton,
                                        getRadioButton,
                                        
                                        setEnabled,
                                        
                                        listBoxInsert,
                                        
                                        populateEventList,
                                        browseFiles,
                                        
                                        loadString,
                                        saveString,
                                        
                                        loadCheckBox,
                                        saveCheckBox,
                                        
                                        loadRadioButton,
                                        saveRadioButton,
                                        
                                        getUserData
                                        
                                      };

/*---[ Prototipes ]-----------------------------------------------------------------------------------------*/

 static void      buttonAction(GtkWidget *, TABLEDEF *);
 static void      lbSelected(GtkWidget *, ULONG);
 static void      changed(GtkWidget *, TABLEDEF *);

 static void      destroy( GtkWidget *, DLGDATA *);
 static WIDGET    makeTable(HICQ,ULONG,const TABLEDEF *, const DLGMGR *, DLGDATA *, gboolean);
 static DLGDATA * configure(const TABLEDEF *);
 static int       configureTable(WIDGET, DLGDATA *, const DLGMGR *);
 static int       add2ListBox(WCNTL *, ULONG, const char *);

/*---[ Implement ]------------------------------------------------------------------------------------------*/

 WIDGET icqgtk_MakeTable(HICQ icq, ULONG uin, const TABLEDEF *tbl, const DLGMGR *mgr)
 {
    DLGDATA    *dBlock;
    GtkWidget  *table	= NULL;
    
    if(mgr && mgr->sz != sizeof(DLGMGR))
       return NULL;

    dBlock = configure(tbl);
    
    if(!dBlock)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error in table creation");
       return NULL;
    }
    
    CHKPoint();

    dBlock->sz  = sizeof(DLGDATA);    
    dBlock->icq = icq;
    dBlock->uin = uin;
    dBlock->mgr = mgr;

    table = makeTable(icq, uin, tbl, mgr, dBlock, FALSE);
    
    if(configureTable(table,dBlock,mgr))
    {
       gtk_signal_connect(GTK_OBJECT(table), "destroy", GTK_SIGNAL_FUNC(destroy), dBlock);
    }
    else
    {
       if(dBlock->userData)
          free(dBlock->userData);
       free(dBlock);
    }

    return table;    
 }

 static int configureTable(WIDGET table, DLGDATA *dBlock, const DLGMGR *mgr)
 {
    int		sz;
    char	   buffer[0x0100];
    
    if(!table)
       return 0;

    if(mgr && mgr->configure)
    {
       sz = mgr->configure(&icqgtk_tableHelper,table,dBlock->icq, dBlock->uin, buffer);
       
       if(sz < 0)
       {
          gtk_widget_destroy(table);
          return 0;
       }
       else if(sz > 0)
       {
          dBlock->userData = malloc(sz);
          if(!dBlock->userData)
          {
             icqWriteSysLog(dBlock->icq,PROJECT,"Memory allocation error in dialog data area");
             gtk_widget_destroy(table);
             return 0;
          }
          memset(dBlock->userData,0,sz);
       }
    }
    
    if(mgr && mgr->load)
    {
       if(mgr->load(&icqgtk_tableHelper,table,dBlock->icq, dBlock->uin, buffer))
       {
          gtk_widget_destroy(table);
          return 0;
       }
    }
    
    return 1;
 }

 static DLGDATA * configure(const TABLEDEF *tbl)
 {
    const TABLEDEF *element;
    DLGDATA		*ret;
    int			qtd		= 0;
    int			sz;
    
    for(element = tbl;element->type;element++)
       qtd++;
       
    DBGTrace(qtd);

    sz   = qtd *sizeof(WCNTL);
    sz  += sizeof(DLGDATA);

    DBGTrace(sz);
               
    ret = malloc(sz);
    
    DBGTracex(ret);
    
    if(!ret)
       return ret;
    
    memset(ret,0,sz);

    ret->elements = qtd;;

    return ret;        
 }

 static WIDGET makeTable(HICQ icq, ULONG uin, const TABLEDEF *tbl, const DLGMGR *mgr, DLGDATA *dBlock, gboolean homogeneous)
 {
    GtkWidget      *table	= NULL;
    GtkWidget	  *temp;
    GtkWidget	  *spc;
    GtkWidget	  *hBox;
    GtkAdjustment  *adj;
    USHORT		 bottom    = 0;
    USHORT		 right     = 0;
    int		    counter   = 0;
    WCNTL		  *wndInfo  = (WCNTL *) (dBlock+1);
    const TABLEDEF *element;

    DBGMessage("Abrir tabela");
    
    element = tbl;
    while(element->type && (element->type != ICQTABLE_END || counter > 0) )
    {
       if(!counter)
       {
          bottom = element->bottom > bottom ? element->bottom : bottom;
          right  = element->right  > right  ? element->right  : right;
       }
       element++;

       if(element->type == ICQTABLE_BEGIN)
          counter++;
       else if(element->type == ICQTABLE_END)
          counter--;

    }
    
    DBGTrace(bottom);
    DBGTrace(right);

    dBlock->table = 
    table         = gtk_table_new(bottom,right,homogeneous);
    
    gtk_object_set_user_data(GTK_OBJECT(table), (gpointer) dBlock);
    
    DBGTracex(table);

    for(element = tbl;element->type && element->type != ICQTABLE_END; element++)
    {
       switch(element->type)
       {
       case ICQTABLE_LABEL:
          temp = gtk_label_new(element->text);
          gtk_misc_set_alignment(GTK_MISC(temp), 0, .5);
          gtk_table_attach(	GTK_TABLE(table), 
          				     temp, 
          				     element->left, element->right, 
          				     element->top,  element->bottom,
          				     GTK_FILL|GTK_SHRINK,   // GTK_EXPAND|GTK_SHRINK|GTK_FILL,
          				     0, 
          				     2, 2);
          
          break;
          
       case ICQTABLE_ENTRY:
          temp = gtk_entry_new();       
          
          if(mgr)
          {
             gtk_object_set_user_data(GTK_OBJECT(temp), (gpointer) dBlock);
             if(mgr->changed)
                gtk_signal_connect(GTK_OBJECT(temp),  "changed", GTK_SIGNAL_FUNC(changed), (gpointer) element);
          }
          
          gtk_table_attach(	GTK_TABLE(table), 
          				     temp, 
          				     element->left, element->right, 
          				     element->top,  element->bottom,
          				     GTK_EXPAND|GTK_SHRINK|GTK_FILL,
          				     0, 
          				     2, 2);
          break;

       case ICQTABLE_CHECKBOX:
          temp = gtk_check_button_new_with_label(element->text);

          if(mgr)
          {
             gtk_object_set_user_data(GTK_OBJECT(temp), (gpointer) dBlock);
             gtk_signal_connect(GTK_OBJECT(temp),  "toggled", GTK_SIGNAL_FUNC(buttonAction), (gpointer) element);
          }
          
          gtk_table_attach(	GTK_TABLE(table), 
          				     temp, 
          				     element->left, element->right, 
          				     element->top,  element->bottom,
          				     GTK_SHRINK|GTK_FILL|GTK_EXPAND,
          				     0, 
          				     2, 2);
          break;

       case ICQTABLE_BUTTON:
          temp = gtk_button_new_with_label(element->text);

          if(mgr)
          {
             gtk_object_set_user_data(GTK_OBJECT(temp), (gpointer) dBlock);
             gtk_signal_connect(GTK_OBJECT(temp), "clicked", GTK_SIGNAL_FUNC(buttonAction), (gpointer) element);
          }

          gtk_table_attach(	GTK_TABLE(table), 
          				     temp, 
          				     element->left, element->right, 
          				     element->top,  element->bottom,
          				     GTK_SHRINK|GTK_FILL,
          				     0, 
          				     2, 2);
          break;

       case ICQTABLE_BEGIN:
          temp =  gtk_frame_new(element->text);
          spc  = makeTable(icq, uin, element+1, mgr, dBlock, element->parm1);

          gtk_container_add (GTK_CONTAINER(temp), spc);
          
          gtk_table_attach(	GTK_TABLE(table), 
          				     temp, 
          				     element->left, element->right, 
          				     element->top,  element->bottom,
          				     GTK_SHRINK|GTK_EXPAND|GTK_FILL,
          				     0, 
          				     2, 2);
          while(element->type != ICQTABLE_END)
             element++;
             
          break;

       case ICQTABLE_DROPBOX:
          temp = gtk_combo_new();

          gtk_table_attach(	GTK_TABLE(table), 
          				     temp, 
          				     element->left, element->right, 
          				     element->top,  element->bottom,
          				     GTK_SHRINK|GTK_EXPAND|GTK_FILL,
          				     0, 
          				     2, 2);
          break;
          
       case ICQTABLE_SPIN:
          adj = GTK_ADJUSTMENT( gtk_adjustment_new(	0.0,
          			  			                   (gfloat) element->parm1,
          					  	                   (gfloat) element->parm2,
          							                 1.0,
          							                 60.0,
          							                 0.0 ));

          temp = gtk_spin_button_new(adj, 0, 0); 

          hBox = gtk_hbox_new(FALSE,2);          							                
          gtk_box_pack_start(GTK_BOX(hBox), temp, FALSE, FALSE, 0);
          
          spc = gtk_label_new(element->text ? element->text : "");
          gtk_misc_set_alignment(GTK_MISC(spc), 0, .5);
          gtk_box_pack_start(GTK_BOX(hBox),spc, TRUE,TRUE,0);

          gtk_table_attach(	GTK_TABLE(table), 
          				     hBox, 
          				     element->left, element->right, 
          				     element->top,  element->bottom,
          				     GTK_SHRINK|GTK_FILL,
          				     0, 
          				     2, 2);
          break;
                    
       case ICQTABLE_RADIOBUTTON:
          if(element->parm1)
             dBlock->radio = temp = gtk_radio_button_new_with_label(NULL,element->text);
          else
             temp = gtk_radio_button_new_with_label_from_widget(dBlock->radio,element->text);

          gtk_object_set_user_data(GTK_OBJECT(temp), (gpointer) dBlock);
          
          gtk_table_attach(	GTK_TABLE(table), 
          				     temp, 
          				     element->left, element->right, 
          				     element->top,  element->bottom,
          				     GTK_SHRINK|GTK_FILL,
          				     0, 
          				     2, 2);
       
          break;
          
       case ICQTABLE_MLE:
          temp = gtk_text_new(NULL,NULL);
          gtk_text_set_editable(GTK_TEXT(temp),TRUE);
          gtk_text_set_word_wrap(GTK_TEXT(temp),TRUE);
          
          gtk_object_set_user_data(GTK_OBJECT(temp), (gpointer) dBlock);
          
          gtk_table_attach(	GTK_TABLE(table), 
          				     temp, 
          				     element->left, element->right, 
          				     element->top,  element->bottom,
          				     GTK_SHRINK|GTK_FILL|GTK_EXPAND,
          				     0, 
          				     2, 2);
       
          break;
          
       default:
          temp = NULL;
       }
       
       if(temp)
       {
          DBGTrace(dBlock->pos);
          wndInfo[dBlock->pos].mgr     = mgr;
          wndInfo[dBlock->pos].element = element;
          wndInfo[dBlock->pos].id      = element->id;
          wndInfo[dBlock->pos].widget  = temp;
          dBlock->pos++;
       }
    }

/*
    if(mgr && mgr->configure)
    {
       counter = mgr->configure(&icqgtk_tableHelper,table,dBlock->icq, dBlock->uin, buffer);
       
       if(counter < 0)
       {
          gtk_widget_destroy(table);
          return NULL;
       }
       else if(counter > 0)
       {
          dBlock->userData = malloc(counter);
          if(!dBlock->userData)
          {
             icqWriteSysLog(dBlock->icq,PROJECT,"Memory allocation error in dialog data area");
             gtk_widget_destroy(table);
             return NULL;
          }
          memset(dBlock->userData,0,counter);
       }
    }
    
    if(mgr && mgr->load)
    {
       if(mgr->load(&icqgtk_tableHelper,table,dBlock->icq, dBlock->uin, buffer))
       {
          gtk_widget_destroy(table);
          return NULL;
       }
    }

    CHKPoint();
*/    
    return (WIDGET) table;
 }

 static void destroy( GtkWidget *widget, DLGDATA *dBlock)
 {
    DBGMessage("Tabela destruida");
    
    if(dBlock->userData)
       free(dBlock->userData);
    
    free(dBlock);
 }

 static void buttonAction(GtkWidget *wdg, TABLEDEF *element)
 {
    DLGDATA *dBlock = gtk_object_get_user_data(GTK_OBJECT(wdg));
    char	buffer[0x0100];

    DBGTrace(element->id);

    if(dBlock->mgr->click)
       dBlock->mgr->click(&icqgtk_tableHelper,dBlock->table,dBlock->icq, dBlock->uin,element->id,buffer);
 }

 static WCNTL * getWidget(HWND hwnd, USHORT id)
 {
    DLGDATA *dBlock = gtk_object_get_user_data(GTK_OBJECT(hwnd)); 
    WCNTL   *cntl;
    int	 f;
    
    if(dBlock->sz != sizeof(DLGDATA)) 
       return NULL;

    cntl = (WCNTL *) (dBlock+1);

    for(f=0;f < dBlock->elements;f++)
    {
       if(cntl->id == id)
          return cntl;
       cntl++;
    }
    
    return NULL;
 }

 static int _System setString(HWND hwnd, USHORT id, const char *txt)
 {
    WCNTL *wnd = getWidget(hwnd,id);
    
    if(!wnd)
       return -1;

    switch(wnd->element->type)
    {
    case ICQTABLE_ENTRY:
       gtk_entry_set_text(GTK_ENTRY(wnd->widget),txt);
       break;
       
    case ICQTABLE_MLE:
       gtk_editable_delete_text(GTK_EDITABLE(wnd->widget),0,-1);
       gtk_text_insert(GTK_TEXT(wnd->widget), NULL, NULL, NULL, txt, -1);
       break;
    }

    return 0;
 }
 
 static int _System getString(HWND hwnd, USHORT id, int sz, char *txt)
 {
    WCNTL *wnd = getWidget(hwnd,id);
    char  *ptr;
    
    if(!wnd)
       return -1;

    switch(wnd->element->type)
    {
    case ICQTABLE_ENTRY:
       strncpy(txt,gtk_entry_get_text(GTK_ENTRY(wnd->widget)),sz);
       break;
       
    case ICQTABLE_MLE:
       ptr = gtk_editable_get_chars(GTK_EDITABLE(wnd->widget),0,-1);
       strncpy(txt,ptr,sz);
       g_free(ptr);
       break;
    }
 
    return 0;
 }

 static int _System loadString(HWND hwnd, ULONG uin, USHORT id, const char *key, int sz, const char *def)
 {
    DLGDATA *wnd     = gtk_object_get_user_data(GTK_OBJECT(hwnd)); 
    char    *buffer  = malloc(sz+1);
    int     ret;
    
    if(!buffer)
       return -1;

    icqLoadString(wnd->icq,key,def,buffer,sz);
    ret = setString(hwnd,id,buffer);

    free(buffer);
    
    return ret;
 }
  
 static int _System saveString(HWND hwnd, ULONG uin, USHORT id, const char *key, int sz)
 {
    DLGDATA *wnd     = gtk_object_get_user_data(GTK_OBJECT(hwnd)); 
    char    *buffer  = malloc(sz+1);
    int     ret;

    if(!buffer)
       return -1;

    *buffer = 0;
    ret = getString(hwnd, id, sz, buffer);

    DBGMessage(buffer);
    
    icqSaveString(wnd->icq,key,buffer);
    
    CHKPoint();
    
    free(buffer);    
    return ret;
 }

 static int _System setCheckBox(HWND hwnd, USHORT id, BOOL modo)
 {
    WCNTL *wnd = getWidget(hwnd,id);
    if(!wnd)
       return -1;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wnd->widget),(gboolean) modo);
    return 0;
 }
 
 static int _System getCheckBox(HWND hwnd, USHORT id)
 {
    WCNTL *wnd = getWidget(hwnd,id);
    if(!wnd)
       return -1;
    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wnd->widget)) ? 1 : 0;
 }
 
 static int _System loadCheckBox(HWND hwnd, ULONG uin, USHORT id, const char *key, BOOL def)
 {
    DLGDATA *wnd     = gtk_object_get_user_data(GTK_OBJECT(hwnd)); 
    return setCheckBox(hwnd,id,icqLoadValue(wnd->icq,key,def));
 }
 
 static int _System saveCheckBox(HWND hwnd, ULONG uin, USHORT id, const char *key)
 {
    DLGDATA *wnd     = gtk_object_get_user_data(GTK_OBJECT(hwnd)); 
    icqSaveValue(wnd->icq,key,getCheckBox(hwnd,id));
    return 0; 
 }

 static int _System setSpinButton(HWND hwnd, USHORT id, int vlr)
 {
    WCNTL *wnd = getWidget(hwnd,id);
    if(!wnd)
       return -1;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(wnd->widget),vlr);
    return 0;
 }
 
 static int _System getSpinButton(HWND hwnd, USHORT id)
 {
    WCNTL *wnd = getWidget(hwnd,id);
    if(!wnd)
       return -1;
    return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(wnd->widget));
 }

 static int _System setRadioButton(HWND hwnd, USHORT id, int vlr)
 {
    return setCheckBox(hwnd, id+vlr, TRUE);
 }

 static int _System setTextLimit(HWND hwnd, USHORT id, int limit)
 {
    return 0;
 }
 
 static int _System setEnabled(HWND hwnd, USHORT id, BOOL modo)
 {
    WCNTL *wnd = getWidget(hwnd,id);
    if(!wnd)
       return -1;
    gtk_widget_set_sensitive(wnd->widget,modo);
    return 0;
 }

 
 static int _System getRadioButton(HWND hwnd, USHORT id, int qtd)
 {
    int f;
    
    for(f=0;f<qtd;f++)
    {
       if(getCheckBox(hwnd,id+f))
          return f;
    }
    return 0; 
 }
 
 static int _System loadRadioButton(HWND hwnd, ULONG uin, USHORT id, const char *key, int qtd, int def)
 {
    DLGDATA *dBlock = gtk_object_get_user_data(GTK_OBJECT(hwnd)); 
    return setRadioButton(hwnd,id,icqLoadValue(dBlock->icq,key,def)%qtd) ; 
 }

 static void * _System getUserData(HWND hwnd)
 {
    DLGDATA *dBlock = gtk_object_get_user_data(GTK_OBJECT(hwnd)); 
    return dBlock->userData;  
 }
 
 static int _System saveRadioButton(HWND hwnd, ULONG uin, USHORT id, const char *key, int qtd)
 {
    int     vlr     = getRadioButton(hwnd,id,qtd);
    DLGDATA *dBlock = gtk_object_get_user_data(GTK_OBJECT(hwnd)); 
    
    DBGTrace(vlr);
    icqSaveValue(dBlock->icq,key,vlr);
    
    return 0; 
 }

 static int _System listBoxInsert(HWND hwnd, USHORT id, ULONG hdl, const char *txt)
 {
    WCNTL     *wnd   = getWidget(hwnd,id);
    wnd->wdg = hwnd;
    return add2ListBox(wnd,hdl,txt);
 }
 
 static int add2ListBox(WCNTL *wnd, ULONG hdl, const char *txt)
 {
    GtkWidget *item  = gtk_list_item_new();
    GtkWidget *label = gtk_label_new(txt);

    gtk_widget_show(item);
    gtk_widget_show(label);

    gtk_object_set_user_data(GTK_OBJECT(item), (gpointer) wnd);

    gtk_misc_set_alignment(GTK_MISC(label), 0, .5);

    gtk_container_add(GTK_CONTAINER(item), label);

    gtk_combo_set_item_string(GTK_COMBO(wnd->widget), GTK_ITEM(item), txt);
    gtk_container_add(GTK_CONTAINER(GTK_COMBO(wnd->widget)->list), item);

    if(wnd->mgr && wnd->mgr->selected)
       gtk_signal_connect(GTK_OBJECT(item),  "select", GTK_SIGNAL_FUNC(lbSelected), (gpointer) hdl);
    
    return 0;
 }

 static void lbSelected(GtkWidget *hwnd, ULONG hdl)
 {
    WCNTL    *wnd    = gtk_object_get_user_data(GTK_OBJECT(hwnd)); 
    DLGDATA  *dBlock = gtk_object_get_user_data(GTK_OBJECT(wnd->wdg)); 
    char     buffer[0x0100];
    wnd->mgr->selected(&icqgtk_tableHelper,wnd->wdg,dBlock->icq,dBlock->uin,wnd->id,hdl,buffer);
 }
 
 static int _System getLength(HWND hwnd, USHORT id)
 {
    WCNTL *wnd = getWidget(hwnd,id);
    int   ret  = 0;
    char *ptr;

    if(!wnd)
       return ret;

    switch(wnd->element->type)
    {
    case ICQTABLE_ENTRY:
       ret = strlen(gtk_entry_get_text(GTK_ENTRY(wnd->widget)));
       break;
       
    case ICQTABLE_MLE:
       ptr = gtk_editable_get_chars(GTK_EDITABLE(wnd->widget),0,-1);
       ret = strlen(ptr);
       g_free(ptr);
       break;
    }

    return ret;
 }

 static void changed(GtkWidget *wdg, TABLEDEF *element)
 {
    DLGDATA *dBlock = gtk_object_get_user_data(GTK_OBJECT(wdg));
    char	buffer[0x0100];
    USHORT   sz     = strlen(gtk_entry_get_text(GTK_ENTRY(wdg)));
    dBlock->mgr->changed(&icqgtk_tableHelper,dBlock->table,dBlock->icq, dBlock->uin,element->id,sz,buffer);
 }

 static int _System populateEventList(HWND hwnd, USHORT id)
 {
    char    buffer[0x0100];
    DLGDATA *dBlock = gtk_object_get_user_data(GTK_OBJECT(hwnd));
    WCNTL   *wnd    = getWidget(hwnd,id);
    FILE    *arq;
    char    *ptr;

    wnd->wdg = hwnd;

    icqQueryProgramPath(dBlock->icq,"events.dat",buffer,0xFF);

    arq = fopen(buffer,"r");    
    if(!arq)
       return -1;
    
    while(!feof(arq))
    {
       *buffer = 0;
       fgets(buffer,0xFF,arq);
       *(buffer+0xFF) = 0;
       for(ptr = buffer;*ptr && *ptr >= ' ';ptr++);
       *ptr = 0;
       if(strlen(buffer) > 6 && *buffer != ';')
          add2ListBox(wnd, *( (ULONG *) buffer ), buffer+5);
    }
    fclose(arq);

    return 0;    
 }
 
 struct browseData
 {
    HWND	  hwnd;
    USHORT    id;
    DLGDATA   *dBlock;
    GtkWidget *fileSel;
 };
 
 static void destroyFBrowser(GtkWidget *widget, struct browseData *d)
 {
    DBGMessage("Seletor de arquivos destruido");
    gtk_widget_set_sensitive(d->hwnd,TRUE); 
    free(d);
 }

 static void saveBFile(GtkWidget *widget, struct browseData *d)
 {
    setString(d->hwnd,d->id,gtk_file_selection_get_filename(GTK_FILE_SELECTION(d->fileSel)));
    gtk_widget_destroy(d->fileSel);
 }
 
 static int  _System browseFiles(HWND hwnd, USHORT id, BOOL save, const char *key, const char *masc, const char *title, char *vlr)
 {
    struct browseData *d = malloc(sizeof(struct browseData));
    
    if(!d)
       return -1;
       
    d->hwnd    = hwnd;
    d->id 	 = id;
    d->dBlock  = gtk_object_get_user_data(GTK_OBJECT(hwnd));
    d->fileSel = gtk_file_selection_new(title);

    gtk_signal_connect(GTK_OBJECT(d->fileSel),"destroy",GTK_SIGNAL_FUNC(destroyFBrowser),d);

    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(d->fileSel)->ok_button),
   			   "clicked", GTK_SIGNAL_FUNC(saveBFile), d);

    gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(d->fileSel)->cancel_button),
   					  "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
   					  (gpointer) d->fileSel);

    gtk_widget_set_sensitive(hwnd,FALSE); 
    gtk_widget_show(d->fileSel);

    return -1;
 }

/*
 gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->ok_button),
   			   "clicked", GTK_SIGNAL_FUNC (store_filename), NULL);
   			   
   // Ensure that the dialog box is destroyed when the user clicks a button. 
   
   gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->ok_button),
   					  "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
   					  (gpointer) file_selector);

   gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->cancel_button),
   					  "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
   					  (gpointer) file_selector);

*/
