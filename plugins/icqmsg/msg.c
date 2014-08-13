/*
 * MSG.C - Normal message processor
 */

 #include <string.h>
 #include <stdio.h>
 #include <malloc.h>
 #include <ctype.h>

 #include "icqmsg.h"

/*----------------------------------------------------------------------------------------------------*/

 static const char * _System title(HICQ, HUSER, ULONG, BOOL, ICQMSG *, char *, int);
 static void         _System formatLog(HICQ, BOOL, ICQMSG *, char *, int);
 static int          _System formatWindow(const MSGEDITHELPER *, MSGEDITWINDOW, HICQ, ULONG, USHORT, BOOL, HMSG);
 static int          _System send(const MSGEDITHELPER *, MSGEDITWINDOW, HICQ, ULONG);
 static int          _System getField(HICQ, ICQMSG *, int, int, char *);
 static int          _System sendText(HICQ,ULONG,const char *);

/*----------------------------------------------------------------------------------------------------*/

      /* Read default message dialog controls, format packet and send */
      /* Executes the SEND-TO button */
      /* Called when the window is receiving another message */
      /* Event processor for the message-edit window */

 MSGMGR icqmsg_normal = {       sizeof(MSGMGR),
 								MSG_NORMAL,                                      /* Message type */
                                MSG_NORMAL,                                      /* type of reply for this kind of message      */
                                ICQMSGMF_LOADUSERFIELDS|ICQMSGMF_USECHATWINDOW|ICQMSGMF_SENDTO,  /* Flags                                       */
                                { ICQICON_UNREADMESSAGE,ICQICON_READMESSAGE },   /* Icons                                       */
                                NULL,                        /* Pre-Process message                         */
                                title,                       /* Query the message title                     */
                                formatLog,                   /* format the log entry for the message        */
                                getField,                    /* Get message field information        */
                                NULL,                        /* View/edit message, if not defined the internal MSG Edit box is called */
                                formatWindow,                /* Show/Hide default msg-edit window controls, fill values from the message */
                                NULL,                        /* Button override, return FALSE if the button wasn't processed */
                                send,                        /* Read default message dialog controls, format packet and send */
                                sendText,                    /* Executes the SEND-TO button */
                                NULL,                        /* Send pre-formated text                                       */
                                NULL,                        /* Called when the window is receiving another message */
                                NULL
                          };

/*----------------------------------------------------------------------------------------------------*/

 static void _System formatLog(HICQ icq, BOOL out, ICQMSG *msg, char *buffer, int sz)
 {
    strncpy(buffer,(char *)(msg+1), sz);
 }

 static const char * _System title(HICQ icq, HUSER usr, ULONG uin, BOOL out, ICQMSG *msg, char *buffer, int sz)
 {
    sprintf(buffer,"Message %s %s (ICQ#%lu)",   out ? "to" : "from",
                                                icqQueryUserNick(usr),
                                                uin );
    return buffer;
 }

 static int _System getField(HICQ icq, ICQMSG *msg, int id, int sz, char *buffer)
 {
    const char *ptr = (char *) (msg+1);
    int        ret  = 0;

    if(!msg || id > 1)
       return -1;

    if(id)
    {
       /* Looking for an URL */
       ptr = icqQueryURL(icq, ptr);
       DBGTracex(ptr);
    }

    if(ptr && *ptr)
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



 static int _System formatWindow(const MSGEDITHELPER *dlg, MSGEDITWINDOW hwnd, HICQ icq, ULONG uin, USHORT type, BOOL out, HMSG msg)
 {
    static const USHORT hide[] = {       ICQMSGC_ENTRY,
                                         ICQMSGC_BROWSE,
                                         ICQMSGC_ACCEPT,
                                         ICQMSGC_REJECT,
                                         ICQMSGC_REFUSE,
                                         ICQMSGC_STATICSUBJ,
                                         ICQMSGC_SENDTO,
                                         ICQMSGC_SUBJECT,
                                         ICQMSGC_OPEN,
                                         0 };

    const USHORT           *wdg;
    BOOL                   in           = !out;
    char                   *ptr;
    char                   buffer[0x0100];

    for(wdg = hide; *wdg; wdg++)
       dlg->setVisible(hwnd,*wdg,FALSE);

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
       dlg->setString(hwnd,ICQMSGC_TEXT,(const char *)(msg+1));

       ptr = (char *) icqQueryURL(icq, (const char *)(msg+1));

       if(ptr)
       {
          dlg->setVisible(hwnd, ICQMSGC_ENTRY,  TRUE);
          dlg->setVisible(hwnd, ICQMSGC_SENDTO, TRUE);

          strncpy(buffer,ptr,0xFF);
          *(buffer+0xFF) = 0;

          for(ptr=buffer;*ptr && !isspace(*ptr);ptr++);
          *ptr = 0;

          dlg->setString(hwnd, ICQMSGC_ENTRY, buffer);
       }

    }
    else
    {
       dlg->setString(hwnd,ICQMSGC_TEXT,"");
    }

    return 0;
 }


 static int _System send(const MSGEDITHELPER *dlg, MSGEDITWINDOW hwnd, HICQ icq, ULONG uin)
 {
    char *text  = malloc(MAX_MSG_SIZE);

    if(!text)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when trying to send message");
       return -1;
    }

    dlg->getString(hwnd,ICQMSGC_TEXT,MAX_MSG_SIZE,text);

    sendText(icq,uin,text);

    free(text);
    return 0;
 }

 static int _System sendText(HICQ icq, ULONG uin, const char *text)
 {
    icqSendMessage(icq, uin, MSG_NORMAL, (char *) text);
    return 0;
 }




