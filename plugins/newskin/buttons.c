/*
 * button.c - Message window buttons
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_WIN
#endif

 #include <malloc.h>
 #include <string.h>
 #include <pwicqgui.h>

/*---[ Prototipes ]-----------------------------------------------------------*/

 static void ICQCALLBACK closeButton(hWindow, hWindow);
 static void ICQCALLBACK sendButton(hWindow, hWindow);
 static void ICQCALLBACK sendToButton(hWindow, hWindow);
 static void ICQCALLBACK aboutButton(hWindow, hWindow);
 static void ICQCALLBACK infoButton(hWindow, hWindow);
 static void ICQCALLBACK addButton(hWindow, hWindow);
 static void ICQCALLBACK replyButton(hWindow, hWindow);
 static void ICQCALLBACK acceptButton(hWindow, hWindow);
 static void ICQCALLBACK refuseButton(hWindow, hWindow);
 static void ICQCALLBACK browseButton(hWindow, hWindow);
 static void ICQCALLBACK nextButton(hWindow, hWindow);

/*---[ Implementation ]-------------------------------------------------------*/

 void icqskin_setMsgButtonCallbacks(hWindow hwnd)
 { 	
	CHKPoint();
	
    icqskin_setMsgButtonCallback(hwnd, MSGID_CLOSEBUTTON, closeButton);
    icqskin_setMsgButtonCallback(hwnd, MSGID_SENDBUTTON,  sendButton);
    icqskin_setMsgButtonCallback(hwnd, MSGID_ABOUTBUTTON, aboutButton);
    icqskin_setMsgButtonCallback(hwnd, MSGID_INFOBUTTON,  infoButton);
    icqskin_setMsgButtonCallback(hwnd, MSGID_ADDBUTTON,   addButton);
    icqskin_setMsgButtonCallback(hwnd, MSGID_SENDTO,      sendToButton);
	
	if( (((ICQMSGDIALOG *) icqskin_getWindowDataBlock(hwnd))->flags & ICQMSGDIALOG_CHAT) == 0)
	{
	   DBGMessage("Standard window, configure it");
       icqskin_setMsgButtonCallback(hwnd, MSGID_REPLYBUTTON,  replyButton);
       icqskin_setMsgButtonCallback(hwnd, MSGID_ACCEPTBUTTON, acceptButton);
       icqskin_setMsgButtonCallback(hwnd, MSGID_REFUSEBUTTON, refuseButton);
       icqskin_setMsgButtonCallback(hwnd, MSGID_BROWSEBUTTON, browseButton);
       icqskin_setMsgButtonCallback(hwnd, MSGID_NEXTBUTTON,   nextButton);
    }		
 }


 static void ICQCALLBACK closeButton(hWindow btn, hWindow hwnd)
 {
    icqskin_closeWindow(hwnd);
 }

 static void ICQCALLBACK aboutButton(hWindow btn, hWindow hwnd)
 {
	ICQMSGDIALOG *cfg = icqskin_getWindowDataBlock(hwnd);

    cfg->idleSeconds = 0;

	if( !(cfg&&cfg->uin&&cfg->fr.icq) )
	   return;
    icqskin_openAboutUser(cfg->fr.icq, cfg->uin, 0, 0);
 }

 static void ICQCALLBACK infoButton(hWindow btn, hWindow hwnd)
 {
	ICQMSGDIALOG *cfg = icqskin_getWindowDataBlock(hwnd);
	
	CHKPoint();

    cfg->idleSeconds = 0;
	
	if( !(cfg&&cfg->uin&&cfg->fr.icq) )
	   return;

    icqOpenConfigDialog (cfg->fr.icq, cfg->uin, 0);

 }

 static void ICQCALLBACK addButton(hWindow btn, hWindow hwnd)
 {
	ICQMSGDIALOG *cfg = icqskin_getWindowDataBlock(hwnd);
    cfg->idleSeconds = 0;

	CHKPoint();
 }

 static void setMessageType(hWindow hwnd, ICQMSGDIALOG *cfg, USHORT type)
 {
	DBGTracex(type);
	
	cfg->flags |= ICQMSGDIALOG_OUT;
	
    icqskin_setMessageManager(hwnd, icqQueryMessageManager(cfg->fr.icq,type));
	
    if(cfg->mgr && (cfg->mgr->flags & ICQMSGMF_USECHATWINDOW) && cfg->mgr->sendText && icqLoadValue(cfg->fr.icq,"chatmode",1))
	{
	   DBGMessage("Opening a new chat window");
       icqNewMessageWithManager(cfg->fr.icq, cfg->uin, cfg->mgr);
	   icqskin_closeWindow(hwnd);
	}
	else
	{
	   DBGMessage("Setting new message type");
       icqskin_setMessageHandle(hwnd, NULL);
	}
	
 }

 static void ICQCALLBACK replyButton(hWindow btn, hWindow hwnd)
 {
	ICQMSGDIALOG *cfg = icqskin_getWindowDataBlock(hwnd);
	
	if(cfg->mgr && cfg->mgr->reply)
       setMessageType(hwnd,cfg,cfg->mgr->reply);	
	else
       setMessageType(hwnd,cfg,MSG_NORMAL);	
	
 }

 static void sendOk(hWindow hwnd, ICQMSGDIALOG *cfg)
 {
    CHKPoint();
 }

 static void ICQCALLBACK acceptButton(hWindow btn, hWindow hwnd)
 {
	ICQMSGDIALOG *cfg      = icqskin_getWindowDataBlock(hwnd);

    CHKPoint();

    if(cfg->mgr && (cfg->mgr->flags & ICQMSGMF_REQUESTMESSAGE))
    {
       DBGTracex(cfg->sequenceNumber);

#ifdef EXTENDED_LOG
       sprintf(cfg->buffer,"Request %08lx accepted",cfg->sequenceNumber);
       icqWriteSysLog(cfg->fr.icq,PROJECT,cfg->buffer);
#endif
       if(!icqAcceptRequest(cfg->fr.icq, cfg->sequenceNumber, ""))
          icqskin_closeWindow(hwnd);
    }
    else
    {
       CHKPoint();

       if(!icqSendMessage(cfg->fr.icq, cfg->uin, MSG_AUTHORIZED, ""))
       {
          sendOk(hwnd,cfg);
          icqskin_setEnabled(btn,FALSE);
       }
    }
	
 }

 static void ICQCALLBACK refuseButton(hWindow btn, hWindow hwnd)
 {
	ICQMSGDIALOG *cfg      = icqskin_getWindowDataBlock(hwnd);
    cfg->idleSeconds = 0;

    setMessageType(hwnd, cfg, MSG_REFUSED);

 }

 static void ICQCALLBACK browseButton(hWindow btn, hWindow hwnd)
 {
	ICQMSGDIALOG *cfg      = icqskin_getWindowDataBlock(hwnd);
    cfg->idleSeconds = 0;

	CHKPoint();
 }

 static void ICQCALLBACK nextButton(hWindow btn, hWindow hwnd)
 {
	ICQMSGDIALOG *cfg      = icqskin_getWindowDataBlock(hwnd);
    cfg->idleSeconds = 0;

	CHKPoint();
 }

 static void ICQCALLBACK sendButton(hWindow btn, hWindow hwnd)
 {
	char         hdr[100];
	time_t       ltime;
	char         *txt;
	
	ICQMSGDIALOG *cfg      = icqskin_getWindowDataBlock(hwnd);
	int          rc        = -1;

    cfg->idleSeconds = 0;
    	
    if(cfg->mgr)
    {
	   if(cfg->mgr->sendText)
	   {
		  txt = malloc(MAX_MSG_SIZE+5);
		
	      if(!txt)
	      {
	         icqWriteSysLog(cfg->fr.icq,PROJECT,"Memory allocation error when loading message text");
	         icqAbend(cfg->fr.icq,0);
	         return;
	      }
		
          *txt = 0;
          icqskin_getMsgWindowText(hwnd,MSGID_EDITBOX,MAX_MSG_SIZE,txt);
		
		  if(*txt)
             rc = cfg->mgr->sendText(cfg->fr.icq,cfg->uin,txt);
		
	      if(!rc && cfg->flags & ICQMSGDIALOG_CHAT)
	      {
             time(&ltime);
             strftime(hdr, 69, "%m/%d/%Y %H:%M:%S ", localtime(&ltime));
             icqLoadString(cfg->fr.icq, "NickName", "", hdr+strlen(hdr), 50);
             icqskin_getMsgWindowText(hwnd,MSGID_EDITBOX,MAX_MSG_SIZE,txt);
             icqskin_insertMessageHistory(hwnd, 1, hdr, txt);
	      }
		
		  free(txt);
       }
	   else if(cfg->mgr->send)
	   {
		  rc = cfg->mgr->send(&icqskin_dialogCalls,hwnd,cfg->fr.icq,cfg->uin);
       }
	
    }
	
	if(rc)
	{
	   sprintf(hdr,"Error sending message (rc=%d)",rc);
	   icqWriteSysLog(cfg->fr.icq,PROJECT,hdr);
	   return;
	}

	if( cfg->flags & ICQMSGDIALOG_CHAT )
	{
       icqskin_setMsgWindowText(hwnd,MSGID_EDITBOX,"");
	}
	else
	{
	   icqskin_closeWindow(hwnd);
	}
	
 }

 static void ICQCALLBACK sendToButton(hWindow btn, hWindow hwnd)
 {
    ICQMSGDIALOG *cfg = icqskin_getWindowDataBlock(hwnd);

    CHKPoint();

    if(!cfg->sendToMenu)
       return;

    icqskin_showMenu(cfg->sendToMenu,(ULONG) hwnd,0,0);
    	
 }


