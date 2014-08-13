/*
 * EVENT.C - Processa eventos de interface grafica
 */
 
 #include <unistd.h>
 
 #include <icqgtk.h>

/*---[ Prototipos ]------------------------------------------------------------------------*/

 
/*---[ Implementacao ]---------------------------------------------------------------------*/

 void EXPENTRY icqgtkEvent_Gui(HICQ icq, MAINWIN *wnd, ULONG uin, USHORT event)
 {
    switch(event)
    {
    case 0:	/* Pronto para iniciar */
       icqgtk_createMainWindow(icq, wnd);
       if(icqInitialize(icq,icqgtk_setStartupStage))
       {
          gtk_main_quit();
          return;
       }
       wnd->filter = icqLoadValue(icq,"wndFilterMode",0);
       gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wnd->filter ? wnd->online : wnd->all), TRUE);
       icqgtk_setMainWindowTitle(icq,wnd);
       icqgtk_RestoreWindow(wnd->main,wnd->icq,0,"mainWindowPos",150,375);
       icqReconnect(wnd->icq);
       break;
       
    }

 }

 int _System icqgtk_insertListener(HICQ icq, void (_System *callBack)(HICQ,ULONG,char,USHORT,ULONG,void *),void *dataBlock)
 {
    MAINWIN  *wnd =  icqGetSkinDataBlock(icq);
    LISTENER *l;
    
    if(!callBack)
       return -2;
    
    l = icqAddElement(wnd->listeners,sizeof(LISTENER));
    
    CHKPoint();
    
    if(!l)
    {
       icqWriteSysLog(icq,PROJECT,"Error adding event listener");
       return -1;
    }
    
    l->callBack  = callBack;
    l->dataBlock = dataBlock;
    l->flag      = 1;
    
    return 0;
 }
 
 int _System icqgtk_removeListener(HICQ icq, void (_System *callBack)(HICQ,ULONG,char,USHORT,ULONG,void *),void *dataBlock)
 {
    MAINWIN  *wnd =  icqGetSkinDataBlock(icq);
    LISTENER *l;

    CHKPoint();

    for(l=icqQueryFirstElement(wnd->listeners);l;l=icqQueryNextElement(wnd->listeners,l))
    {
       if(l->callBack == callBack && l->dataBlock == dataBlock)
       {
          l->flag = 2;
          return 0;
       }
    }
 
    return -1;
 }


