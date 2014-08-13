/*
 * msg.c - Common message window processing
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_WIN
#endif

 #include <malloc.h>
 #include <string.h>
 #include <time.h>
 #include <pwicqgui.h>

#ifdef __OS2__
   #define CHILD_WINDOW(h,i) WinWindowFromID(h,DLG_ELEMENT_ID+i)
#else
   #define CHILD_WINDOW(h,i) icqskin_getDlgItemHandle(h,i)
#endif

/*---[ Structures ]-----------------------------------------------------------*/

 #pragma pack(1)

 struct parm
 {
	void         (* ICQCALLBACK callback)(struct parm *);
	USHORT       sz;
	HICQ         icq;
	ULONG        uin;
	ULONG		 flags;
	HMSG         msg;
	const MSGMGR *mgr;
 };

 #pragma pack()

/*---[ Prototipes ]-----------------------------------------------------------*/

 static void ICQCALLBACK serialize(struct parm *);
 static void             configureMessage(hWindow,ULONG);

/*---[ Implementation ]-------------------------------------------------------*/

 static int openMsgWindow(HICQ icq, ULONG flags, ULONG uin, const MSGMGR *mgr, HMSG msg, const char *text, const char *url)
 {
	int         sz  = sizeof(struct parm);
	struct parm *p;
	char        *ptr;

#ifdef DEBUG
    if(text)
       DBGMessage(text);
    if(url)
       DBGMessage(url);
#endif

	if(text)
	   sz += strlen(text);
	
	if(url)
	   sz += strlen(url);
	
	p = malloc(sz+5);
	
	if(!p)
	{
	   icqWriteSysLog(icq,PROJECT,"Memory allocation error when setting message window callback");
	   icqAbend(icq,0);
	   return -1;
	}

	DBGTracex(p);
	
    memset(p,0,sz);
	
	DBGTracex(serialize);
	
	p->callback = serialize;
	p->sz       = sizeof(struct parm);
	p->icq      = icq;
	p->uin      = uin;
	p->mgr      = mgr;
	p->msg      = msg;
	p->flags    = flags;
	
	ptr = (char *) (p+1);

    if(text)
    {	
	   strcpy(ptr,text);
	   ptr += strlen(ptr)+1;
	}
	else
	{
	   *(ptr++) = 0;
	}

    if(url)
    {	
	   strcpy(ptr,url);
	}
	else
	{
	   *(ptr++) = 0;
	}

    return icqskin_postEvent(icq, 0, 'g', ICQEVENT_CALLBACK, (ULONG) p);

 }

 static int _System sendToCallback(HICQ icq, hWindow hwnd, USHORT icon, SENDTOCALLBACK *callback)
 {
	ICQMSGDIALOG *cfg  = icqskin_getWindowDataBlock(hwnd);

    DBGTracex(hwnd);
    DBGTracex(cfg);
	
	if(!cfg || cfg->fr.sz < sizeof(ICQMSGDIALOG))
	{
	   icqWriteSysLog(icq,PROJECT,"Invalid window ID in send-to callback");
	   return -1;
	}

    if(!callback)
       return -2;

    icqskin_getMsgWindowText(hwnd,MSGID_EDITBOX,0xFF,cfg->buffer);

    DBGMessage(cfg->buffer);
    DBGMessage(cfg->sendToParm);

    return callback(icq,cfg->uin,cfg->mgr,cfg->buffer,cfg->sendToParm);

 }

 static int _System addSendTo(hWindow hwnd, USHORT icon, const char *text, SENDTOCALLBACK *callback)
 {
	ICQMSGDIALOG *cfg = icqskin_getWindowDataBlock(hwnd);
	
	DBGTracex(cfg->sendToMenu);
    icqskin_insertMenuOption(cfg->sendToMenu, text, 0x8000|icon, (MENUCALLBACK *) &sendToCallback, (ULONG) callback);
    return 0;
 }

#ifdef DEBUG

 static int _System testMenu(HICQ icq, ULONG uin, const MSGMGR *mgr, const char *text, const char *url)
 {
    DBGMessage("Teste de menu");
    DBGMessage(text);
    DBGMessage(url);
    return 0;
 }

#endif


 static void ICQCALLBACK serialize(struct parm *p)
 {
    SKINDATA     *skn            = icqskin_getDataBlock(p->icq);
	ICQMSGDIALOG *cfg;
    char         *text           = (char *) (p+1);
    char         *url            = NULL;
    hWindow      hwnd;

    CHKPoint();
    for(cfg = skn->firstMsgWin;cfg;cfg = cfg->down)
    {
 	   if(cfg->uin == p->uin)
	   {
	      if(cfg->flags & ICQMSGDIALOG_HIDE)
          {
			 cfg->idleSeconds   = 0;
             cfg->flags        &= ~(ICQMSGDIALOG_HIDE|ICQMSGDIALOG_SHOW);
             icqskin_showFrame(cfg->hwnd);
          }
	      free(p);
		  return;
  	   }
    }
    CHKPoint();

    url = (text + strlen(text) + 1);

    DBGMessage(text);
    DBGMessage(url);
    DBGTrace(p->uin);
    DBGTracex(p->mgr);

    if(p->uin && p->mgr && (p->mgr->flags & ICQMSGMF_USECHATWINDOW) && p->mgr->sendText && icqLoadValue(p->icq,"chatmode",1))
       hwnd = icqskin_createChatWindow(p->icq);
    else
       hwnd = icqskin_createPagerWindow(p->icq);

    DBGTracex(hwnd);
	
    if(hwnd)
    {
	   DBGMessage("Message dialog created");
       icqskin_setICQHandle(hwnd,p->icq);
#ifndef DEBUG
       icqskin_setChildEnabled(hwnd,MSGID_SENDTO,FALSE);
#endif
       icqskin_setMsgButtonCallbacks(hwnd);
       icqskin_setMessageUser(hwnd,p->uin);

#ifdef DEBUG
       icqSaveString(p->icq, "msgWindow", NULL);
       CHKPoint();
#endif

#ifdef SKINNED_GUI
       icqskin_loadSkin(hwnd,"Message",skn->skin);
#endif
       icqskin_setMessageManager(hwnd,p->mgr);
	
       configureMessage(hwnd,p->flags);

       icqskin_initializeMessageEditor(p->icq, hwnd);

       DBGTracex(text);
       DBGTracex(p->msg);
	
       icqskin_setMessageHandle(hwnd, p->msg);

	   CHKPoint();
       icqskin_restoreFrame(hwnd,NULL,366,250);
	   CHKPoint();

	   cfg             = icqskin_getWindowDataBlock(hwnd);
	   cfg->hwnd       = hwnd;
  	   cfg->up         = skn->lastMsgWin;
       cfg->sendToMenu = icqskin_CreateMenu(p->icq, 10, skn);
	
       icqskin_setButtonFont(cfg->sendToMenu,FONT_NORMAL);
       icqskin_setButtonName(cfg->sendToMenu,"Menu.SendTo");

#ifdef SKINNED_GUI
       icqskin_loadSkin(cfg->sendToMenu,"Menu.Dialog",skn->skin);
#endif

	   if(cfg->up)
		  cfg->up->down = cfg;
	   else
		  skn->firstMsgWin = cfg;
	   skn->lastMsgWin = cfg;

       if( p->mgr && (p->mgr->flags & ICQMSGMF_SENDTO) )
       {
          DBGTracex(p->mgr->type);
          DBGTracex(p->msg);
		
#ifdef DEBUG
          addSendTo(hwnd, 1, "T1", testMenu);
          addSendTo(hwnd, 1, "T2", testMenu);
#endif
		
          CHKPoint();
          icqInsertSendToOptions(p->icq, hwnd, p->mgr->type, p->msg == NULL, p->mgr, (int ( * _System)(HWND, USHORT, const char *, SENDTOCALLBACK *)) addSendTo);
          CHKPoint();
       }
	
	   CHKPoint();
       icqskin_activate(hwnd);
   	   CHKPoint();

    }

    free(p);
 }

 void ICQCALLBACK icqskin_viewMessage(HICQ icq, ULONG uin, HMSG msg)
 {
	const MSGMGR *mgr               = NULL;
	char         *text				= NULL;
	const char   *url				= NULL;
	char         urlBuffer[0x0100];
	
    DBGTracex(msg);
    DBGTrace(uin);

	if(msg)
	{
	   if(msg->mgr)
	      mgr = msg->mgr;
	   else
	      mgr = icqQueryMessageManager(icq,msg->type);
	
	}
	
    CHKPoint();

	if(!mgr)
	   mgr = icqQueryMessageManager(icq,0);
	
    CHKPoint();

	if(!mgr)
	   mgr = icqQueryMessageManager(icq,MSG_NORMAL);
	
    *urlBuffer = 0;	

    CHKPoint();
	
	if(msg && mgr && mgr->getField)
	{
       CHKPoint();

	   if(mgr->getField(icq,msg,1,0xFF,urlBuffer) >= 0)
	      url = urlBuffer;
	
       CHKPoint();

	   text = malloc(MAX_MSG_SIZE+10);
	

	   if(text)
	   {
	      *text = 0;
	      mgr->getField(icq,msg,0,MAX_MSG_SIZE,text);
	   }
	
       CHKPoint();

    }	
	
    CHKPoint();

    openMsgWindow(icq,0,uin,mgr,msg,text,url);

    CHKPoint();

    if(text)
       free(text);

    DBGTracex(icqQueryNextMessage(icq, uin, NULL));
	
 }

 int ICQCALLBACK icqskin_newWithMgr(HICQ icq, ULONG uin, const MSGMGR *mgr, HMSG msg)
 {
	char  	    buffer[0x0100];
	const char  *txt            = NULL;
	const char  *url            = NULL;
	const char	*cbURL			= NULL;
	int         pos;
	
	if(!mgr || mgr->sz != sizeof(MSGMGR))
	{
	   icqWriteSysLog(icq,PROJECT,"Invalid message manager");
	   return -1;
	}

    icqskin_queryClipboard(icq, 0xFF, buffer);
	
    if(*buffer && (cbURL = icqQueryURL(icq, buffer)) != NULL)
    {
       url  = cbURL;
       pos  = (cbURL - buffer)-1;

       DBGTrace(pos);

       if(!txt && pos > 1 && pos < 0xF0)
          txt = buffer;
       else
          txt = url;
    }

    if(msg)
       return openMsgWindow(icq,ICQMSGDIALOG_OUT,uin,mgr,msg,txt,url);
    else
       return openMsgWindow(icq,ICQMSGDIALOG_OUT,uin,mgr,NULL,txt,url);

 }

 int ICQCALLBACK icqskin_newMessage(HICQ icq, ULONG uin, USHORT type, const char *txt, const char *url)
 {
	const MSGMGR 	*mgr;
	char  			buffer[0x0100];
	const char		*cbURL			= NULL;
	int			    pos;
	
	if(!type)
	{
	   if(url)
	   {
		  type = MSG_URL;
	   }
	   else if(icqLoadValue(icq,"Chk4URL",0))
	   {
		  /* Check for URL in clipboard */
          icqskin_queryClipboard(icq, 0xFF, buffer);

          if(*buffer && (cbURL = icqQueryURL(icq, buffer)) != NULL)
          {
             type = MSG_URL;
             url  = cbURL;
             pos  = (cbURL - buffer)-1;

             DBGTrace(pos);

             if(!txt && pos > 1 && pos < 0xF0)
             {
                *(buffer+pos) = 0;
                txt           = buffer;
             }
          }
		  else
		  {
			 type = MSG_NORMAL;
		  }
	   }
	   else
	   {
          type = MSG_NORMAL;
	   }
	}

	DBGTrace(type);
	mgr = icqQueryMessageManager(icq,type);
	
    if(!mgr)
	{
	   sprintf(buffer,"Can't fint a valid message processor for type %04x",type);
	   DBGMessage(buffer);
	   icqWriteSysLog(icq,PROJECT,buffer);
	   return 2;
	}
		
    openMsgWindow(icq,ICQMSGDIALOG_OUT,uin,mgr,NULL,txt,url);

    return 0;
 }

 void icqskin_setMessageManager(hWindow hwnd, const MSGMGR *mgr)
 {
	ICQMSGDIALOG *cfg = icqskin_getWindowDataBlock(hwnd);
	
	if(!cfg)
	   return;
	
	if(mgr && mgr->sz != sizeof(MSGMGR))
	{
	   if(cfg->fr.icq)
		  icqWriteSysLog(cfg->fr.icq,PROJECT,"Invalid message manager set");
	   return;
	}
	
	if(cfg->fr.sz == sizeof(ICQMSGDIALOG))
	   cfg->mgr = mgr;

	if(cfg->mgr)
       icqskin_setFrameIcon(hwnd, (SKINDATA *) icqskin_getDataBlock(cfg->fr.icq), cfg->mgr->icon[0]);

 }

 void icqskin_setMessageUser(hWindow hwnd, ULONG uin)
 {
	char         buffer[0x0100];
	ICQMSGDIALOG *cfg = icqskin_getWindowDataBlock(hwnd);
	HUSER        usr  = NULL;
	
	if(cfg && cfg->fr.sz == sizeof(ICQMSGDIALOG))
	{
	   DBGMessage("Message user was set");
	
	   cfg->uin = uin;
	
	   if(uin)
          usr = icqQueryUserHandle(cfg->fr.icq,uin);

//       DBGTracex(icqskin_getDlgItemHandle(hwnd,DLG_ELEMENT_ID+MSGID_ABOUTBUTTON));

       if(usr)
       {
          icqskin_setMsgWindowText(hwnd,MSGID_NICKENTRY,icqQueryUserNick(usr));
          strncpy(buffer,icqQueryUserFirstName(usr),0xFF);
          strncat(buffer," ",0xFF);
          strncat(buffer,icqQueryUserLastName(usr),0xFF);
          *(buffer+0xFF) = 0;
          icqskin_setMsgWindowText(hwnd,MSGID_NAMEENTRY,buffer);

          if(usr->modeIcon == usr->offlineIcon)
             icqskin_setDlgItemIcon(hwnd,MSGID_USERMODE,ICQICON_OFFLINE);
          else
             icqskin_setDlgItemIcon(hwnd,MSGID_USERMODE,usr->modeIcon);

          if(usr->flags & USRF_ONLIST)
             icqskin_setChildEnabled(hwnd,MSGID_ADDBUTTON,FALSE);
          else
             icqskin_setChildEnabled(hwnd,MSGID_ADDBUTTON,TRUE);

          icqskin_setEnabled(icqskin_getDlgItemHandle(hwnd,DLG_ELEMENT_ID+MSGID_ABOUTBUTTON),TRUE);
          icqskin_setEnabled(icqskin_getDlgItemHandle(hwnd,DLG_ELEMENT_ID+MSGID_INFOBUTTON),TRUE);

       }
       else
       {
          icqskin_setMsgWindowText(hwnd,MSGID_NICKENTRY,"N/A");
          icqskin_setMsgWindowText(hwnd,MSGID_NAMEENTRY,"N/A");

          icqskin_setDlgItemIcon(hwnd,MSGID_USERMODE,ICQICON_OFFLINE);
          icqskin_hide(icqskin_getDlgItemHandle(hwnd,MSGID_ADDBUTTON));

          icqskin_setEnabled(icqskin_getDlgItemHandle(hwnd,DLG_ELEMENT_ID+MSGID_ABOUTBUTTON),FALSE);
          icqskin_setEnabled(icqskin_getDlgItemHandle(hwnd,DLG_ELEMENT_ID+MSGID_INFOBUTTON),FALSE);

       }

    }
 }

 static void configureMessage(hWindow hwnd, ULONG flags)
 {
	ICQMSGDIALOG *cfg = icqskin_getWindowDataBlock(hwnd);
	
	if(!cfg || cfg->fr.sz < sizeof(ICQMSGDIALOG))
	   return;
	
    cfg->flags |= flags;

    CHKPoint();

 }

 void icqskin_setMessageHandle(hWindow hwnd, HMSG msg)
 {
	ICQMSGDIALOG *cfg = icqskin_getWindowDataBlock(hwnd);
	HUSER        usr;
	
    CHKPoint();
    usr = icqQueryUserHandle(cfg->fr.icq,cfg->uin);	

    *cfg->buffer = 0;

    if(cfg->flags & ICQMSGDIALOG_CHAT)
    {
       sprintf(cfg->buffer,"Session with %s",icqQueryUserNick(usr));
    }
	else if(cfg->mgr && cfg->mgr->title)
	{
	   cfg->mgr->title(cfg->fr.icq,usr,cfg->uin,(cfg->flags & ICQMSGDIALOG_OUT) != 0, NULL, cfg->buffer,0xFF);
    }
    else
    {
       sprintf(cfg->buffer,"Message %s %s", (cfg->flags & ICQMSGDIALOG_OUT) ? "to" : "from", icqQueryUserNick(usr));
    }

    DBGMessage(cfg->buffer);

    if(*cfg->buffer)
       icqskin_setFrameTitle(hwnd, cfg->buffer);

	if(cfg->flags & ICQMSGDIALOG_CHAT)
	   return;

    if(msg)
	{
       /* Incoming message */
       cfg->sequenceNumber = msg->sequenceNumber;

	   icqskin_setReadOnly(CHILD_WINDOW(hwnd,MSGID_EDITBOX));
	
	   icqskin_setChildEnabled(hwnd,MSGID_NEXTBUTTON,FALSE);

	   if(msg->mgr && cfg->mgr != msg->mgr)
          icqskin_setMessageManager(hwnd, msg->mgr);

	   if(cfg->mgr && cfg->mgr->formatWindow)
	      cfg->mgr->formatWindow(&icqskin_dialogCalls,hwnd,cfg->fr.icq,cfg->uin,cfg->mgr->type,FALSE,msg);

       icqRemoveMessage(cfg->fr.icq, cfg->uin, msg);
	
       cfg->flags |= ICQMSGDIALOG_LOAD; /* To verify the "next" button */
	
	}
	else
	{
	   /* Outgoing message */
       cfg->sequenceNumber = 0;

	   icqskin_setEditable(CHILD_WINDOW(hwnd,MSGID_EDITBOX));
	
	   if(cfg->mgr && cfg->mgr->formatWindow)
	      cfg->mgr->formatWindow(&icqskin_dialogCalls,hwnd,cfg->fr.icq,cfg->uin,cfg->mgr->type,TRUE,NULL);
	}
	
 }

 int icqskin_destroyMsgWindow(ICQMSGDIALOG *cfg)
 {
    SKINDATA  *skn;

	if(!cfg)
       return 0;

	DBGMessage("Cleaning up message window");
	
	skn = icqskin_getDataBlock(cfg->fr.icq);
	
	if(cfg->up)
	   cfg->up->down = cfg->down;
	else
	   skn->firstMsgWin = cfg->down;
	
	if(cfg->down)
	   cfg->down->up = cfg->up;
	else
	   skn->lastMsgWin = cfg->up;
	
	CHKPoint();
    	
    return 1;
 }

#ifdef MAKELIB
 APIRET EXPENTRY icqOpenMsgWindow(HICQ icq, ULONG uin, USHORT type, HMSG msg, const MSGMGR *mgr, const char *txt, const char *url)
 {

    if(msg)
       icqskin_viewMessage(icq, uin, msg);
    else if(mgr)
       icqskin_newWithMgr(icq, uin, mgr, msg);
    else
       icqskin_newMessage(icq, uin, type, txt, url);

    return 0;
 }
#endif


