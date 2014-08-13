/*
 * MSG.C - Normal message processor
 */

 #include <string.h>
 #include <stdio.h>
 #include <malloc.h>
 #include <ctype.h>

 #include "ishared.h"
 #include <msgdlg.h>

/*----------------------------------------------------------------------------------------------------*/

 static const char * _System title(HICQ, HUSER, ULONG, BOOL, ICQMSG *, char *, int);
 static void         _System formatLog(HICQ, BOOL, ICQMSG *, char *, int);
 static int          _System formatWindow(const DIALOGCALLS *, hWindow, HICQ, ULONG, USHORT, BOOL, HMSG);
 static int          _System send(const DIALOGCALLS *, hWindow, HICQ, ULONG);
// static int          _System sendText(HICQ,ULONG,const char *);

/*----------------------------------------------------------------------------------------------------*/

 const MSGMGR pwICQMessageManager = {   sizeof(MSGMGR),
         								0,                           /* Message type */
                                        MSG_NORMAL,                  /* type of reply for this kind of message      */
                                        ICQMSGMF_URLCOMPATIBLE,      /* Flags                                       */
                                        { 0,1 },                     /* Icons                                       */
                                        NULL,                        /* Pre-Process message                         */
                                        title,                       /* Query the message title                     */
                                        formatLog,                   /* format the log entry for the message        */
                                        NULL,                        /* Get message field information        */
                                        NULL,                        /* View/edit message, if not defined the internal MSG Edit box is called */
                                        formatWindow,                /* Show/Hide default msg-edit window controls, fill values from the message */
                                        NULL,                        /* Button override, return FALSE if the button wasn't processed */
                                        send,                        /* Read default message dialog controls, format packet and send */
                                        NULL, // sendText,                    /* Executes the SEND-TO button */
                                        NULL,                        /* Called when the window is receiving another message */
                                        NULL
                          };

/*----------------------------------------------------------------------------------------------------*/

 static void _System formatLog(HICQ icq, BOOL out, ICQMSG *msg, char *buffer, int sz)
 {
    CHKPoint();
    strncpy(buffer,(char *)(msg+1), sz);
    CHKPoint();
 }

 static const char * _System title(HICQ icq, HUSER usr, ULONG uin, BOOL out, ICQMSG *msg, char *buffer, int sz)
 {
    CHKPoint();
    strcpy(buffer,"I-Share broadcast message");
    CHKPoint();

    return buffer;
 }

 static int _System formatWindow(const DIALOGCALLS *dlg, hWindow hwnd, HICQ icq, ULONG uin, USHORT type, BOOL out, HMSG msg)
 {
    static const USHORT hide[] = {       ICQMSGC_ENTRY,
                                         ICQMSGC_BROWSE,
                                         ICQMSGC_ACCEPT,
                                         ICQMSGC_REJECT,
                                         ICQMSGC_REFUSE,
                                         ICQMSGC_STATICSUBJ,
                                         ICQMSGC_SENDTO,
                                         ICQMSGC_ADD,
                                         ICQMSGC_SUBJECT,
                                         ICQMSGC_OPEN,
                                         ICQMSGC_REPLY,
                                         ICQMSGC_NEXT,
                                         ICQMSGC_CHECKBOX,
                                         0 };

    const USHORT           *wdg;
//    BOOL                   in           = !out;
    const unsigned char    *ptr;
    char                   buffer[0x0100];

    CHKPoint();

    for(wdg = hide; *wdg; wdg++)
       dlg->setVisible(hwnd,*wdg,FALSE);

    CHKPoint();

    dlg->setVisible(hwnd,ICQMSGC_SEND, TRUE);
    dlg->setString(hwnd,ICQMSGC_TEXT,"");

    CHKPoint();
    dlg->setString(hwnd, ICQMSGC_UIN,      "N/A");

    CHKPoint();
    dlg->setString(hwnd, ICQMSGC_NICKNAME, "Intranet");

    CHKPoint();
    dlg->setString(hwnd, ICQMSGC_NAME,     "I-Share users");

    CHKPoint();
    dlg->setString(hwnd, ICQMSGC_EMAIL,    "N/A");

    CHKPoint();
	
    *buffer = 0;
    dlg->queryClipboard(icq,0xFF,buffer);

    CHKPoint();

    ptr = icqQueryURL(icq,buffer);

    CHKPoint();

    if(ptr)
    {
       dlg->setString(hwnd,ICQMSGC_ENTRY,ptr);
       dlg->setVisible(hwnd,ICQMSGC_ENTRY,TRUE);
    }

    CHKPoint();

    return 0;
 }


 static int _System send(const DIALOGCALLS *dlg, hWindow hwnd, HICQ icq, ULONG uin)
 {
    ISHARE_UDP_MESSAGE *msg = malloc(sizeof(ISHARE_UDP_MESSAGE)+MAX_MSG_SIZE+5);
    ISHARED_CONFIG     *cfg = icqGetPluginDataBlock(icq, module);
    int                szText;

    if(!msg)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when generating message");
       return -1;
    }

    DBGMessage("Sending I-Share broadcast message");

    memset(msg,0,sizeof(ISHARE_UDP_MESSAGE));

    msg->h.type  = ISHAREUDP_MESSAGE;
    msg->from    = icqQueryUIN(icq);
    msg->msgType = MSG_NORMAL;
    msg->id      = cfg->msgSequence++;

    dlg->getString(hwnd,ICQMSGC_TEXT,MAX_MSG_SIZE,msg->text);

    if(dlg->getVisible(hwnd,ICQMSGC_ENTRY))
    {
       icqWriteSysLog(cfg->icq,PROJECT,"Sending Broadcast URL");
       msg->msgType = MSG_URL;
       strncat(msg->text,"\n\x01",MAX_MSG_SIZE);
       szText = strlen(msg->text);
       dlg->getString(hwnd,ICQMSGC_ENTRY,ICQMSGC_ENTRY-szText,msg->text+szText);
    }
    else
    {
       icqWriteSysLog(cfg->icq,PROJECT,"Sending Broadcast Message");
    }

    icqWriteSysLog(cfg->icq,PROJECT,msg->text);

    DBGMessage(msg->text);
    ishare_send(cfg, IPADDR_BROADCAST, FALSE, sizeof(ISHARE_UDP_MESSAGE)+strlen(msg->text), msg);

    free(msg);
    return 0;
 }

/*
 static int _System sendText(HICQ icq, ULONG uin, const char *text)
 {
    ISHARE_UDP_MESSAGE *msg = malloc(sizeof(ISHARE_UDP_MESSAGE)+strlen(text)+5);
    ISHARED_CONFIG     *cfg = icqGetPluginDataBlock(icq, module);

    if(!msg)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when generating message");
       return -1;
    }

    DBGMessage("Sending I-Share broadcast message");

    memset(msg,0,sizeof(ISHARE_UDP_MESSAGE));

    msg->h.type  = ISHAREUDP_MESSAGE;
    msg->from    = icqQueryUIN(icq);
    msg->msgType = MSG_NORMAL;
    msg->id      = cfg->msgSequence++;

    strcpy(msg->text,text);

    DBGMessage(msg->text);
    ishare_send(cfg, IPADDR_BROADCAST, FALSE, sizeof(ISHARE_UDP_MESSAGE)+strlen(msg->text), msg);

    free(msg);
    return 0;

 }
*/

 BEGIN_UDP_PACKET_PROCESSOR( MESSAGE )
 {
    CHKPoint();

    if(pkt->from && pkt->from == queryUIN(cfg))
       return 0;

    ishare_cacheUser(cfg, fromIP, fromPort);

    DBGMessage(pkt->text);
    icqInsertMessage(cfg->icq,  pkt->from, 0, pkt->msgType, 0, 0, -1, pkt->text);

    return 0;
 }



