/*
 * System event processor
 */

 #include <unistd.h>
  
 #include <icqgtk.h>
 
/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static void loadUserList(MAINWIN *);
 static void ajustSysButton(MAINWIN *);
 
/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 void EXPENTRY icqgtkEvent_System(HICQ icq, MAINWIN *wnd, USHORT event)
 {
    if(!wnd || wnd->sz != sizeof(MAINWIN))
       return;

    switch(event)
    {
    case ICQEVENT_ONLINE:
    case ICQEVENT_OFFLINE:
       wnd->aPos = 0xFFFE;		   // Will update mode button
       ICQREORDER(wnd);
       break;
       
    case ICQEVENT_CHANGED:		   // Mode changed
       wnd->aPos = 0xFFFE;		   // Will update mode button
       break;
    
    case ICQEVENT_ABEND:		     // Abend - Terminate program
       gtk_main_quit();
       break;
       
    case ICQEVENT_CONNECTING:
    case ICQEVENT_FINDINGSERVER:
       wnd->aPos = 0xFFFD;		   // Will update screen
       break;

    case ICQEVENT_MESSAGECHANGED:	// Ajust system message button
       ajustSysButton(wnd);
       break;
       
    case ICQEVENT_SECONDARY:	     // Background startup complete - Load user information
       loadUserList(wnd);
       break;

    }
 }

 static void addUserInListBox(MAINWIN *ctl, HUSER usr)
 {
    gchar	*tbl[] = { "A", "*Failed*" };
    int	  pos;

    tbl[1] = (char *) icqQueryUserNick(usr);
    pos = gtk_clist_append(GTK_CLIST(ctl->listBox),(gchar **) &tbl);
    gtk_clist_set_row_data(GTK_CLIST(ctl->listBox),pos,usr);
    gtk_clist_set_pixmap (GTK_CLIST(ctl->listBox),pos,0,ctl->iPixmap[usr->modeIcon],ctl->iBitmap[usr->modeIcon]);
    
 }

 void icqgtk_addUserInListBox(MAINWIN *ctl, HUSER usr)
 {
    ICQLOCK(ctl);
    addUserInListBox(ctl, usr);
    ICQUNLOCK(ctl);
 }

 static void loadUserList(MAINWIN *ctl)
 {
    HUSER	usr;

    DBGMessage("Preenchendo lista de usuarios");
    
    ICQLOCK(ctl);
    for(usr = icqQueryFirstUser(ctl->icq); usr; usr = icqQueryNextUser(ctl->icq,usr))
    {
       if(!(usr->flags & USRF_HIDEONLIST))
          addUserInListBox(ctl,usr);
    }
    ICQUNLOCK(ctl);
    
#ifdef DEBUG
//   icqInsertMessage(ctl->icq,  5496712, MSG_NORMAL, 0, 0, -1, "Teste de inclusao de mensagem");
//    icqInsertMessage(ctl->icq,  5496712, MSG_URL, 0, 0, -1, "Teste de inclusao de URLþhttp://www.os2brasil.com.br");
//    icqInsertMessage(icq,  9999, MSG_NORMAL, 0, 0, -1, "Teste de inclusao na fila do sistema");
//    icqInsertMessage(icq,  8888, MSG_URL, 0, 0, -1, "Teste de inclusao de URLþhttp://www.os2brasil.com.br");
//    icqInsertMessage(ctl->icq,  28986464, MSG_ADD, 0, 0, -1, "SalþRobertoþSalomonþsalomon@scr.com.brþ0");
//    icqInsertMessage(icq,  28986464, MSG_REQUEST, 0, 0, -1, "SalþRobertoþSalomonþsalomon@scr.com.brþ0þResolve, Pô!!!");
//    icqInsertMessage(icq,  27241234, MSG_AUTHORIZED, 0, 0, -1, "Autorizado!!");
//    icqInsertMessage(icq,  27241234, MSG_REFUSED, 0, 0, -1, "Voce nao e bom o bastante para entrar na minha lista");
#endif

    DBGMessage("Lista de usuarios preenchida");

    ctl->iTick  = 0;
    ctl->flags |= ICQGTK_SHOW|ICQGTK_FLAG_READY|ICQGTK_FLAG_REORDER;
    
 }

 static void ajustSysButton(MAINWIN *wnd)
 {
    HMSG msg = icqQueryMessage(wnd->icq,0);
    
    if(!msg)
    {
       wnd->eventIcon = wnd->readIcon;
    }
    else if(msg->mgr)
    {
       wnd->readIcon = msg->mgr->icon[1];
       wnd->eventIcon = msg->mgr->icon[0];
    }
    else
    {
       wnd->readIcon  = 1;
       wnd->eventIcon = 0;
    }
    wnd->sTick = 0;
  
 }

