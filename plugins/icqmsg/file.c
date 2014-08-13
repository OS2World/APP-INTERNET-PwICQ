/*
 * MSG.C - URL message processor
 */

 #include <string.h>
 #include <stdio.h>
 #include <malloc.h>

 #include "icqmsg.h"

 #define FIELD_COUNT  5

/*----------------------------------------------------------------------------------------------------*/

 static const char * _System title(HICQ, HUSER, ULONG, BOOL, ICQMSG *, char *, int);
 static void         _System formatLog(HICQ, BOOL, ICQMSG *, char *, int);
 static int          _System formatWindow(const MSGEDITHELPER *, MSGEDITWINDOW, HICQ, ULONG, USHORT, BOOL, HMSG);
 static void         _System event(const MSGEDITHELPER *, MSGEDITWINDOW, HICQ, ULONG, char, USHORT, ULONG);
 static int          _System send(const MSGEDITHELPER *, MSGEDITWINDOW, HICQ, ULONG);

/*----------------------------------------------------------------------------------------------------*/

 MSGMGR icqmsg_file = {         sizeof(MSGMGR),
                                MSG_FILE,
                                MSG_FILE,                    /* type of reply for this kind of message    */
                                ICQMSGMF_LOADUSERFIELDS|ICQMSGMF_REQUESTMESSAGE|ICQMSGMF_DISABLESEND,
                                { 47,48 },                   /* Icons                                     */
                                NULL,
                                title,                       /* Query the message title */
                                formatLog,                   /* format the log entry for the message */
                                NULL,
                                NULL,                        /* View/edit message, if not defined the internal MSG Edit box is called */
                                formatWindow,                /* Show/Hide default msg-edit window controls, fill values from the message */
                                NULL,                        /* Button override, return FALSE if the button wasn't processed */
                                send,                        /* Read default message dialog controls, format packet and send */
                                NULL,                        /* Send pre-formated text                                       */
                                NULL,                        /* Executes the SEND-TO button */
                                NULL,                        /* Called when the window is receiving another message */

                                event,                       /* Event processor for the message-edit window */
                      };

/*----------------------------------------------------------------------------------------------------*/

 static char *stripFields(const char *txt, char **buffer)
 {
    int           f;
    char          *ret = strdup(txt);
    unsigned char *ptr;

    for(f=0;f<FIELD_COUNT;f++)
       buffer[f] = "";

    if(ret)
    {
       f = 1;
       for(buffer[0] = ptr = ret;*ptr && f<FIELD_COUNT;ptr++)
       {
          if(*ptr == ICQ_FIELD_SEPARATOR || *ptr == 0xFE)
          {
             *(ptr++) = 0;
             buffer[f++] = ptr;
          }
       }
    }

    DBGMessage(buffer[0]);
    DBGMessage(buffer[1]);
    DBGMessage(buffer[2]);

    return ret;
 }

 static void _System formatLog(HICQ icq, BOOL out, ICQMSG *msg, char *buffer, int sz)
 {
    char *fields[FIELD_COUNT];
    char *fieldBuffer = stripFields((char *)(msg+1),fields);

    sprintf(buffer,"%s\n%s\n%s\n",fields[1],fields[2],fields[3]);

    if(fieldBuffer)
       free(fieldBuffer);
 }

 static const char * _System title(HICQ icq, HUSER usr, ULONG uin, BOOL out, ICQMSG *msg, char *buffer, int sz)
 {
    sprintf(buffer,"File %s %s (ICQ#%lu)",   out ? "to" : "from",
                                             icqQueryUserNick(usr),
                                             uin );
    return buffer;
 }

 static int _System formatWindow(const MSGEDITHELPER *dlg, MSGEDITWINDOW hwnd, HICQ icq, ULONG uin, USHORT type, BOOL out, HMSG msg)
 {
    static const USHORT hide[] = {       ICQMSGC_REFUSE,
                                         ICQMSGC_SENDTO,
                                         ICQMSGC_OPEN,
                                         ICQMSGC_REPLY,
                                         ICQMSGC_STATICSUBJ,
                                         ICQMSGC_SUBJECT,
                                         0 };
    const USHORT           *wdg;
    BOOL                   in           = !out;
    char 		   *fieldBuffer = NULL;
    HUSER                  usr          = icqQueryUserHandle(icq,uin);

    char		   *fields[FIELD_COUNT];

    for(wdg = hide; *wdg; wdg++)
       dlg->setVisible(hwnd,*wdg,FALSE);

    dlg->setVisible(hwnd,ICQMSGC_ENTRY,  TRUE);
    dlg->setVisible(hwnd,ICQMSGC_REJECT, in);
    dlg->setVisible(hwnd,ICQMSGC_ACCEPT, in);
    dlg->setVisible(hwnd,ICQMSGC_NEXT,   in);
    dlg->setVisible(hwnd,ICQMSGC_SEND,   out);
    dlg->setVisible(hwnd,ICQMSGC_BROWSE, out);

    if(out)
    {
       dlg->loadString(hwnd,ICQMSGC_STATICEMAIL, ICQMSGS_EMAIL);
       dlg->setEnabled(hwnd,ICQMSGC_SEND, usr && (usr->flags & USRF_CANRECEIVE));
    }
    else
    {
       dlg->loadString(hwnd, ICQMSGC_STATICEMAIL, ICQMSGS_DATE);

       if(msg && msg->msgTime)
          dlg->setTime(hwnd, ICQMSGC_EMAIL, msg->msgTime);

    }

    if(msg)
    {
       fieldBuffer = stripFields((char *)(msg+1),fields);

       if(fieldBuffer)
       {
          dlg->setString(hwnd,ICQMSGC_TEXT, fields[1]);
          dlg->setString(hwnd,ICQMSGC_ENTRY,fields[2]);
          free(fieldBuffer);
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

 static void _System event(const MSGEDITHELPER *dlg, MSGEDITWINDOW hwnd, HICQ icq, ULONG uin, char type, USHORT eventCode, ULONG parm)
 {
    HUSER usr;

    if(type != 'U')
       return;

    if(!dlg)
    {
       icqWriteSysLog(icq,PROJECT,"**** INTERNAL ERROR: Event code without edit helper!!!");
       return;
    }

    if(dlg->sz != sizeof(MSGEDITHELPER))
    {
       icqWriteSysLog(icq,PROJECT,"**** INTERNAL ERROR: Event code with invalid edit helper!!!");
       return;
    }

    if(!dlg->setEnabled)
    {
       icqWriteSysLog(icq,PROJECT,"**** INTERNAL ERROR: Event code with invalid edit helper entry!!!");
       return;
    }

    switch(eventCode)
    {
    case ICQEVENT_PEERBEGIN:
#ifdef EXTENDED_LOG
       icqWriteSysLog(icq,PROJECT,"Peer begin, correcting \"send\" button");
#endif
       usr = icqQueryUserHandle(icq,uin);
       DBGTracex(usr);
       DBGTracex(dlg);
       DBGTracex(dlg->setEnabled);
       if(usr)
       {
          if(usr->flags & USRF_CANRECEIVE)
          {
#ifdef EXTENDED_LOG
             icqWriteSysLog(icq,PROJECT,"User can receive files");
#endif
             dlg->setEnabled(hwnd, ICQMSGC_SEND, TRUE);
          }
          else
          {
#ifdef EXTENDED_LOG
             icqWriteSysLog(icq,PROJECT,"User can't receive files");
#endif
             CHKPoint();
             dlg->setEnabled(hwnd, ICQMSGC_SEND, FALSE);
          }
       }
       else
       {
          CHKPoint();
          dlg->setEnabled(hwnd, ICQMSGC_SEND, FALSE);
       }
       CHKPoint();
#ifdef EXTENDED_LOG
       icqWriteSysLog(icq,PROJECT,"Event processed");
#endif
       break;

    case ICQEVENT_PEEREND:
#ifdef EXTENDED_LOG
       icqWriteSysLog(icq,PROJECT,"Peer end, correcting \"send\" button");
#endif
       CHKPoint();
       dlg->setEnabled(hwnd,ICQMSGC_SEND, FALSE);
       break;
    }

 }

 static int _System send(const MSGEDITHELPER *dlg, MSGEDITWINDOW hwnd, HICQ icq, ULONG uin)
 {
    char fileName[0x0200];
    char *text                    = malloc(MAX_MSG_SIZE);
//    int  sz;

    if(!text)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when trying to send file request");
       return -1;
    }

    dlg->getString(hwnd,ICQMSGC_TEXT,MAX_MSG_SIZE,text);
    dlg->getString(hwnd,ICQMSGC_ENTRY,0x01FF,fileName);

    icqSendFile(icq, uin, fileName, text);

    free(text);
    return 0;
 }




