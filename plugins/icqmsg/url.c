/*
 * MSG.C - URL message processor
 */

 #include <string.h>
 #include <stdio.h>
 #include <malloc.h>

 #include "icqmsg.h"

/*----------------------------------------------------------------------------------------------------*/

 static const char * _System title(HICQ, HUSER, ULONG, BOOL, ICQMSG *, char *, int);
 static void         _System formatLog(HICQ, BOOL, ICQMSG *, char *, int);
 static int          _System formatWindow(const MSGEDITHELPER *, MSGEDITWINDOW, HICQ, ULONG, USHORT, BOOL, HMSG);
 static int          _System send(const MSGEDITHELPER *, MSGEDITWINDOW, HICQ, ULONG);
 static int          _System sendText(HICQ, ULONG, const char *);

/*----------------------------------------------------------------------------------------------------*/

      /* Read default message dialog controls, format packet and send */
      /* Executes the SEND-TO button */
      /* Called when the window is receiving another message */
      /* Event processor for the message-edit window */


 MSGMGR icqmsg_url = {          sizeof(MSGMGR),
                                MSG_URL,
                                MSG_NORMAL,                              /* type of reply for this kind of message    */
                                ICQMSGMF_LOADUSERFIELDS|ICQMSGMF_SENDTO, /* Flags                                     */
                                { ICQICON_UNREADURL,ICQICON_READURL },   /* Icons                                     */
                                icqmsg_filterURL,
                                title,                       /* Query the message title */
                                formatLog,                   /* format the log entry for the message */
                                icqmsg_getField,             /* Get message field information        */
                                NULL,                        /* View/edit message, if not defined the internal MSG Edit box is called */
                                formatWindow,                /* Show/Hide default msg-edit window controls, fill values from the message */
                                NULL,                        /* Button override, return FALSE if the button wasn't processed */
                                send,                        /* Read default message dialog controls, format packet and send */
                                sendText,                    /* Send pre-formated text                                       */
                                NULL,                        /* Executes the SEND-TO button */
                                NULL,                        /* Called when the window is receiving another message */
                                NULL
                      };

/*----------------------------------------------------------------------------------------------------*/

 void _System icqmsg_filterURL(HICQ icq, UCHAR *txt, ICQMSG *msg)
 {
    char *ptr = NULL;

    while(*txt)
    {
       if(*txt == 0xFE)
          ptr = txt;
       txt++;
    }

    if(ptr)
       *ptr = ICQ_FIELD_SEPARATOR;

 }

 int _System icqmsg_getField(HICQ icq, ICQMSG *msg, int id, int sz, char *buffer)
 {
    char *ptr = (char *) (msg+1);
    int  ret  = 0;

    if(!msg || id > 1)
       return -1;

    *(buffer+sz) = 0;

    if(id)
    {
       while(*ptr && *ptr != ICQ_FIELD_SEPARATOR)
          ptr++;
       if(*ptr)
          ptr++;
    }

    if(*ptr)
    {
       while(*ptr && *ptr != ICQ_FIELD_SEPARATOR)
       {
          if(sz > 0)
          {
             *(buffer++) = *ptr;
             sz--;
          }
          ret++;
          ptr++;
       }

       DBGTrace(sz);

       if(sz > 0)
          *buffer = 0;

    }

    return ret;

 }

 static void _System formatLog(HICQ icq, BOOL out, ICQMSG *msg, char *buffer, int sz)
 {
    char *ptr   = buffer;
    char *src   = (char *)(msg+1);

    for(src = (char *)(msg+1);*src && *src != ICQ_FIELD_SEPARATOR && sz;*(ptr++) = *(src++))
       sz--;

    if(*src)
       src++;

    *(ptr++) = '\n';

    while(*src && sz--)
       *(ptr++) = *(src++);

    *ptr = 0;
 }

 static const char * _System title(HICQ icq, HUSER usr, ULONG uin, BOOL out, ICQMSG *msg, char *buffer, int sz)
 {
    sprintf(buffer,"URL %s %s (ICQ#%lu)",   out ? "to" : "from",
                                                icqQueryUserNick(usr),
                                                uin );
    return buffer;
 }

 static int _System formatWindow(const MSGEDITHELPER *dlg, MSGEDITWINDOW hwnd, HICQ icq, ULONG uin, USHORT type, BOOL out, HMSG msg)
 {
    static const USHORT hide[] = {       ICQMSGC_BROWSE,
                                         ICQMSGC_SENDTO,
                                         ICQMSGC_ACCEPT,
                                         ICQMSGC_REJECT,
                                         ICQMSGC_REFUSE,
                                         ICQMSGC_STATICSUBJ,
                                         ICQMSGC_SUBJECT,
                                         0 };

    const USHORT           *wdg;
    BOOL                   in           = !out;
    char                   *ptr;

    for(wdg = hide; *wdg; wdg++)
       dlg->setVisible(hwnd,*wdg,FALSE);

    dlg->loadString(hwnd,ICQMSGC_OPEN,  ICQMSGS_OPEN);
    dlg->setVisible(hwnd,ICQMSGC_OPEN,  in && icqLoadValue(icq, "urlOpen", TRUE));

    dlg->setVisible(hwnd,ICQMSGC_ENTRY, TRUE);
    dlg->setVisible(hwnd,ICQMSGC_REPLY, in);
    dlg->setVisible(hwnd,ICQMSGC_NEXT,  in);
    dlg->setVisible(hwnd,ICQMSGC_SEND,  out);

    if(out)
    {
       dlg->loadString(hwnd, ICQMSGC_STATICEMAIL, ICQMSGS_EMAIL);
    }
    else
    {
       dlg->loadString(hwnd, ICQMSGC_STATICEMAIL, ICQMSGS_DATE);

       if(msg && msg->msgTime)
          dlg->setTime(hwnd, ICQMSGC_EMAIL, msg->msgTime);

    }

    if(msg)
    {
       for(ptr = (char *)(msg+1);*ptr && *ptr != ICQ_FIELD_SEPARATOR;ptr++);
       if(*ptr == ICQ_FIELD_SEPARATOR)
       {
          *ptr = 0;
          dlg->setString(hwnd,ICQMSGC_TEXT,(char *)(msg+1));
          *ptr = ICQ_FIELD_SEPARATOR;
          dlg->setString(hwnd,ICQMSGC_ENTRY,(char *)(ptr+1));
       }
       else
       {
          dlg->setString(hwnd,ICQMSGC_TEXT,(char *)(msg+1));
          dlg->setString(hwnd,ICQMSGC_ENTRY,"");
       }

    }
    else
    {
       dlg->setString(hwnd,ICQMSGC_TEXT,"");
       dlg->setString(hwnd,ICQMSGC_ENTRY,"");
    }

    return 0;
 }


 static int _System send(const MSGEDITHELPER *dlg, MSGEDITWINDOW hwnd, HICQ icq, ULONG uin)
 {
    char *text  = malloc(MAX_MSG_SIZE);
    int  sz;

    if(!text)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when trying to send URL");
       return -1;
    }

    dlg->getString(hwnd,ICQMSGC_TEXT,MAX_MSG_SIZE,text);
    sz = strlen(text);

    *(text+sz) = ICQ_FIELD_SEPARATOR;
    sz++;

    if(sz < MAX_MSG_SIZE)
       dlg->getString(hwnd,ICQMSGC_ENTRY,MAX_MSG_SIZE-sz,text+sz);

    sendText(icq,uin,text);

    free(text);
    return 0;
 }

 static int _System sendText(HICQ icq, ULONG uin, const char *text)
 {
    icqSendMessage(icq, uin, MSG_URL, (char *) text);
    return 0;
 }



