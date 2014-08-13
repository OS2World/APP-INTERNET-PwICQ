/*
 * Timer
 */
 
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 
 #include <icqgtk.h>
 
/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static void ajustMode(HICQ, MAINWIN *);
 static void blinkMessage(MAINWIN *);
 static void blinkSysButton(MAINWIN *);
 static void internalEvent(HICQ,EVENT *);
 
/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 int _System icqgtk_guiTimer(HICQ icq, MAINWIN *wnd, char flag)
 {
    /* Fast timer */ 
    EVENT       *e;
    LISTENER	*l;
    LISTENER    *n;

    if(!wnd)
       return 0;

    if(wnd->flags & ICQGTK_LOCK)
       return 0;

    /* Processa eventos pendentes */

    e = icqQueryFirstElement(wnd->events);
    
    while(e && e->type)
    {
       switch(e->type)
       {
       case 'S':
          icqgtkEvent_System(icq, wnd, e->event);
          break;
    
       case 'U':
          if(e->parm && * ((USHORT *) e->parm) == sizeof(ICQUSER))
             icqgtkEvent_User(icq, wnd, e->uin, e->event,(HUSER) e->parm);
          else
             icqgtkEvent_User(icq, wnd, e->uin, e->event,icqQueryUserHandle(icq,e->uin));
          break;
          
       case 'G':
          internalEvent(icq,e);
          break;
       }
       
       l = icqQueryFirstElement(wnd->listeners);
       while(l)
       {
          n = icqQueryNextElement(wnd->listeners,l);
       
          switch(l->flag)
          {
          case 1:
             l->callBack(wnd->icq,e->uin,e->type,e->event,e->parm,l->dataBlock);
             break;
             
          case 2:
             DBGMessage("Listener removido");
             icqRemoveElement(wnd->listeners,l);
          }
          
          l = n;
       }
       
       icqRemoveElement(wnd->events,e);
       e = icqQueryFirstElement(wnd->events);
       
    }
    
    if(!(wnd->flags & ICQGTK_FLAG_READY))
       return 0;

    /* Processa flags */
                  
    switch(wnd->aPos)
    {
    case 0xFFFF:
       break;
       
    case 0xFFFE:
       ICQLOCK(wnd);
       ajustMode(wnd->icq,wnd);
       ICQUNLOCK(wnd);
       break;
       
    case 0xFFFD:
       ICQLOCK(wnd);
       wnd->aTick = wnd->aPos = 0;
       gtk_label_set_text(GTK_LABEL(wnd->modeText),"Connecting");
       ICQUNLOCK(wnd);
       break;
              
    default:
       if(wnd->aTick++ > 5)
       {
          wnd->aTick = 0;
          wnd->aPos++;
          wnd->aPos &= 0x07;
          ICQLOCK(wnd);
          gtk_pixmap_set(GTK_PIXMAP(wnd->modePix),wnd->iPixmap[18+wnd->aPos],wnd->iBitmap[18+wnd->aPos]);
          ICQUNLOCK(wnd);
       }   
    }

    if(wnd->fTick++ > 15)
    {
       wnd->fTick = 0;
       if(wnd->flags & ICQGTK_FLAG_REORDER)
       {
          DBGMessage("Reordernando a lista...");
          ICQLOCK(wnd);
          wnd->flags &= ~ICQGTK_FLAG_REORDER;
          gtk_clist_sort(GTK_CLIST(wnd->listBox));
          if(wnd->flags & ICQGTK_SHOW)
          {
             DBGMessage("Mostrar a janela");
             gtk_widget_show_all(wnd->main);
             wnd->flags &= ~ICQGTK_SHOW;
          }
          ICQUNLOCK(wnd);
       }
       
       if(wnd->flags & ICQGTK_SAVE)
       {
          wnd->flags &= ~ICQGTK_SAVE;
          icqgtk_StoreWindow(wnd->main, wnd->icq, 0, "mainWindowPos");
       }
          
    }
    
    if(wnd->iTick != 0xFFFF && wnd->iTick++ > 10)
       blinkMessage(wnd);

    if(wnd->sTick != 0xFFFF && wnd->sTick++ > 10)
       blinkSysButton(wnd);

    return 0;
 }

 static void ajustMode(HICQ icq, MAINWIN *wnd)
 {
    ULONG 			 current = icqQueryOnlineMode(icq);
    const ICQMODETABLE *mode;
    
    for(mode = icqQueryModeTable(icq); mode && mode->mode != current && mode->descr; mode++);
    
    wnd->aPos = 0xFFFF;		// Disable connecting button

    if(!mode || !mode->descr)
       return;

    DBGTrace(mode->icon);
    DBGMessage(mode->descr);
    
    gtk_pixmap_set(GTK_PIXMAP(wnd->modePix),ICQPIXMAP(wnd,mode->icon));
    gtk_label_set_text(GTK_LABEL(wnd->modeText),mode->descr);
 }    

 static void blinkMessage(MAINWIN *wnd)
 {
    HUSER 	usr;
    gint  	pos;
    USHORT	icon;

    wnd->flags ^= ICQGTK_FLAG_MSGICON;

    wnd->iTick = 0xFFFF;

    for(usr = icqQueryFirstUser(wnd->icq);usr;usr=icqQueryNextUser(wnd->icq,usr))
    {
       if(usr->modeIcon != usr->eventIcon)
       {
          wnd->iTick = 0;
          icon       = (wnd->flags & ICQGTK_FLAG_MSGICON) ? usr->modeIcon : usr->eventIcon;
          
          ICQLOCK(wnd);
          pos  = gtk_clist_find_row_from_data(GTK_CLIST(wnd->listBox),usr);

          if(pos != -1)
          {
             SETLOCK(wnd);
             gtk_clist_set_pixmap(GTK_CLIST(wnd->listBox),pos,0,ICQPIXMAP(wnd,icon));
          }
          ICQUNLOCK(wnd);
       }
    }
 }

 static void blinkSysButton(MAINWIN *wnd)
 {
    USHORT	icon;
    
    wnd->flags ^= ICQGTK_FLAG_SYSICON;
    
    ICQLOCK(wnd);
    if(wnd->eventIcon == wnd->readIcon)
    {
       wnd->sTick  = 0xFFFF;
       wnd->flags &= ~ICQGTK_FLAG_SYSICON;
       gtk_pixmap_set (GTK_PIXMAP(wnd->sysMsgPix),ICQPIXMAP(wnd,wnd->readIcon));
    }
    else
    {
       wnd->sTick = 0;
       icon = (wnd->flags & ICQGTK_FLAG_SYSICON) ? wnd->readIcon : wnd->eventIcon;
       gtk_pixmap_set (GTK_PIXMAP(wnd->sysMsgPix),ICQPIXMAP(wnd,icon));
    }
    ICQUNLOCK(wnd);
    
 
 }

 static void internalEvent(HICQ icq, EVENT *e)
 {
    switch(e->event)
    {
    case 1:
       icqgtk_openMessageWindow(icq,e->uin, ((HMSG) e->parm)->type,FALSE,(HMSG) e->parm);
       break;
       
    case 2:
       icqgtk_openMessageWindow(icq, e->uin, (USHORT) e->parm, TRUE, NULL);
       break;
       
    default:
       DBGPrint("***************** CODIGO DE EVENTO INVALIDO: %d",e->event);
       icqWriteSysLog(icq,PROJECT,"Unexpected internal event request");
    }
    
 }
