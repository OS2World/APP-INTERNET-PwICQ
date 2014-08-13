/*
 * user.c - User event processor
 */

 #include <stdio.h>
 #include <icqgtk.h>

/*---[ Prototipos ]------------------------------------------------------------------------*/

 static int  updateUserIcon(HICQ, MAINWIN *, HUSER);
 static int  removeUserFromList(HICQ, MAINWIN *, HUSER);
 static void addUser(HICQ, MAINWIN *, HUSER );
 static void setOffline(HICQ, MAINWIN *, HUSER);
 static gint locateUser(HICQ, MAINWIN *, HUSER, const char *);
 
/*---[ Implementacao ]---------------------------------------------------------------------*/

 void EXPENTRY icqgtkEvent_User(HICQ icq, MAINWIN *wnd, ULONG uin, USHORT event, HUSER usr)
 {
    if(!(wnd->flags & ICQGTK_FLAG_READY))
       return;

    switch(event)
    {
    case ICQEVENT_ONLINE:
    case ICQEVENT_OFFLINE:
       updateUserIcon(icq, wnd, usr);
       ICQREORDER(wnd);
       break;
       
    case ICQEVENT_USERADDED:
    case ICQEVENT_SHOW:
    case ICQEVENT_UPDATED:
       addUser(icq, wnd, usr);
       break;
       
    case ICQEVENT_ICONCHANGED:
       updateUserIcon(icq, wnd, usr);
       break;
    
    case ICQEVENT_POSCHANGED:   
       ICQREORDER(wnd);
       break;
       
    case ICQEVENT_HIDE:
    case ICQEVENT_WILLCHANGE:
    case ICQEVENT_DELETED:
       removeUserFromList(icq,wnd,usr);
       break;

    case ICQEVENT_SETOFFLINE:
       setOffline(icq,wnd,usr);
       break;
       
    case ICQEVENT_MESSAGECHANGED:
       if(wnd->iTick == 0xFFFF)
          wnd->iTick = 0;
       addUser(icq, wnd, usr);
       break;
    }
 }

 static void setOffline(HICQ icq, MAINWIN *wnd, HUSER usr)
 {
    gint  pos;

    if(!usr)
       return;

    ICQLOCK(wnd);

    pos = gtk_clist_find_row_from_data(GTK_CLIST(wnd->listBox),usr);
    
    if(pos != -1)
       gtk_clist_set_pixmap(GTK_CLIST(wnd->listBox),pos,0,ICQPIXMAP(wnd,usr->offlineIcon));
    
/*    
    {
       if(usr->flags & USRF_TEMPORARY)
          gtk_clist_remove(GTK_CLIST(wnd->listBox),pos);
       else
          gtk_clist_set_pixmap(GTK_CLIST(wnd->listBox),pos,0,ICQPIXMAP(wnd,usr->offlineIcon));
       
    }
*/
    
    ICQUNLOCK(wnd);
 
 }
 
 static gint locateUser(HICQ icq, MAINWIN *wnd, HUSER usr, const char *log)
 {
    gint pos;
#ifdef EXTENDED_LOG
    char logBuffer[0x0100];
#endif
    
    if(!usr)
    	return -1;

    pos = gtk_clist_find_row_from_data(GTK_CLIST(wnd->listBox),usr);
    
    if(pos == -1)
    {
#ifdef EXTENDED_LOG
        sprintf(logBuffer,"Handle isn't in list (ICQ# %ld User: %s)",usr->uin,icqQueryUserNick(usr));
        icqWriteSysLog(icq,PROJECT,logBuffer);
#endif
       usr = icqQueryUserHandle(icq,usr->uin);
       pos = gtk_clist_find_row_from_data(GTK_CLIST(wnd->listBox),usr);
       
       if(pos != -1)
       	   icqWriteSysLog(icq,PROJECT,"Invalid user handle received");
    }
    
    if(pos == -1)
    	icqWriteSysLog(icq,PROJECT,log);

    return pos;    
 }
 
 static int updateUserIcon(HICQ icq, MAINWIN *wnd, HUSER usr)
 {
    gint  pos;

    if(!usr)
       return -1;

    if(usr->flags & USRF_HIDEONLIST)
       return -1;

    ICQLOCK(wnd);
    
    pos = locateUser(icq,wnd,usr,"Unexpected user update icon request");

    if(pos != -1)
       gtk_clist_set_pixmap(GTK_CLIST(wnd->listBox),pos,0,ICQPIXMAP(wnd,usr->modeIcon));

    ICQUNLOCK(wnd);
    
    return pos;
 }

 static int removeUserFromList(HICQ icq, MAINWIN *wnd, HUSER usr)
 {
    gint  pos;

    CHKPoint();
    DBGTracex(usr);
    
    if(!usr)
    {
       icqWriteSysLog(icq,PROJECT,"User remove request without handler");
       return -1;
    }

    ICQLOCK(wnd);

    pos = gtk_clist_find_row_from_data(GTK_CLIST(wnd->listBox),usr);
    
    if(pos == -1)
       icqWriteSysLog(icq,PROJECT,"Unexpected user remove request");
    else
       gtk_clist_remove(GTK_CLIST(wnd->listBox),pos);

    ICQUNLOCK(wnd);
    
    return pos;
 }

 static void addUser(HICQ icq, MAINWIN *wnd, HUSER usr)
 {
    gint  pos;

    if(!usr)
       return;
       
    pos = gtk_clist_find_row_from_data(GTK_CLIST(wnd->listBox),usr);
    
    if(pos == -1)
       icqgtk_addUserInListBox(wnd, usr);

    ICQREORDER(wnd);

 }
 
