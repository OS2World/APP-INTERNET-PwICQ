/*
 * added.c - "You were added"  message processing
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

/*----------------------------------------------------------------------------------------------------*/

      /* Read default message dialog controls, format packet and send */
      /* Executes the SEND-TO button */
      /* Called when the window is receiving another message */
      /* Event processor for the message-edit window */


 MSGMGR icqmsg_added = {        sizeof(MSGMGR),
                                MSG_ADD,
                                MSG_NORMAL,                  /* type of reply for this kind of message      */
                                ICQMSGMF_LOADUSERFIELDS|ICQMSGMF_SYSTEMMESSAGE,     /* Flags                                       */
                                { 29,30 },                   /* Icons                                       */
                                NULL,                        /* Pre-Process message                         */
                                title,                       /* Query the message title                     */
                                formatLog,                   /* format the log entry for the message        */
                                NULL,
                                NULL,                        /* View/edit message, if not defined the internal MSG Edit box is called */
                                formatWindow,                /* Show/Hide default msg-edit window controls, fill values from the message */
                                NULL,                        /* Button override, return FALSE if the button wasn't processed */
                                send,                        /* Read default message dialog controls, format packet and send */
                                NULL,                        /* Send pre-formated text                                       */
                                NULL,                        /* Executes the SEND-TO button */
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
    strcpy(buffer, "You were added from ");
    strncat(buffer,icqQueryUserNick(usr),sz);
    return buffer;
 }

 static int _System formatWindow(const MSGEDITHELPER *dlg, MSGEDITWINDOW hwnd, HICQ icq, ULONG uin, USHORT type, BOOL out, HMSG msg)
 {
    static const USHORT hide[] = {       ICQMSGC_ENTRY,
                                         ICQMSGC_BROWSE,
                                         ICQMSGC_REJECT,
                                         ICQMSGC_STATICSUBJ,
                                         ICQMSGC_SUBJECT,
                                         ICQMSGC_SEND,
                                         ICQMSGC_ACCEPT,
                                         ICQMSGC_REFUSE,
                                         ICQMSGC_CHECKBOX,
                                         ICQMSGC_SENDTO,
                                         ICQMSGC_OPEN,
                                         0 };

    const USHORT        *wdg;
    BOOL                in       = !out;

    for(wdg = hide; *wdg; wdg++)
       dlg->setVisible(hwnd,*wdg,FALSE);

    dlg->setVisible(hwnd, ICQMSGC_ADD,   in);
    dlg->setVisible(hwnd, ICQMSGC_REPLY, in);
    dlg->setVisible(hwnd, ICQMSGC_NEXT,  in);
    dlg->setVisible(hwnd, ICQMSGC_SEND,  out);
    dlg->setString(hwnd,  ICQMSGC_TEXT,  "You were added");

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

    icqSendConfirmation(icq, uin, MSG_REQUEST, TRUE, text);

    free(text);
    return 0;
 }




