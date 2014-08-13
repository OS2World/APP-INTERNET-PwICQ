/*
 * os2pagewin.c - OS/2 Message Window
 */


 #pragma strings(readonly)

 #include <stdlib.h>
 #include <string.h>
 #include <malloc.h>

 #define INCL_GPIPRIMITIVES
 #include <pwicqgui.h>

/*---[ Defines ]------------------------------------------------------------------------------------------*/

 #define MSGID_NAMEBOX    MSG_WINDOW_COMPONENTS
 #define MSGID_TEXTBOX    MSG_WINDOW_COMPONENTS+1
 #define HISTORY_SELECTED CRA_CURSORED

 #define CHILD_WINDOW(h,i) WinWindowFromID(h,DLG_ELEMENT_ID+i)

/*---[ Structures ]---------------------------------------------------------------------------------------*/


/*---[ Constants ]----------------------------------------------------------------------------------------*/

 const char *icqPagerWindowClass = "pwICQPagerWindow";

 static const MSGCHILD childlist[] =
 {
    { "Textbox",      MSGID_TEXTBOX,      WC_STATIC,     "",         "8.Helv",      SS_GROUPBOX|DT_LEFT|DT_VCENTER                                          },
    { "Namebox",      MSGID_NAMEBOX,      WC_STATIC,     "",         "8.Helv",      SS_GROUPBOX|DT_LEFT|DT_VCENTER                                          },

    { "Static",       MSGID_NICK,         ICQ_STATIC,    "Nick:",    "8.Helv",      SS_TEXT|DT_LEFT|DT_VCENTER                                              },
    { "Field",        MSGID_NICKENTRY,    ICQ_STATIC,    "",         "8.Helv.Bold", SS_TEXT|DT_LEFT|DT_VCENTER                                              },
    { "Static",       MSGID_NAME,         ICQ_STATIC,    "Name:",    "8.Helv",      SS_TEXT|DT_LEFT|DT_VCENTER                                              },
    { "Field",        MSGID_NAMEENTRY,    ICQ_STATIC,    "",         "8.Helv.Bold", SS_TEXT|DT_LEFT|DT_VCENTER                                              },

    { "CloseButton",  MSGID_CLOSEBUTTON,  ICQ_BUTTON,    "~Close",   "8.Helv",      0                                                                       },
    { "NextButton",   MSGID_NEXTBUTTON,   ICQ_BUTTON,    "~Next",    "8.Helv",      0                                                                       },
    { "SendButton",   MSGID_SENDBUTTON,   ICQ_BUTTON,    "~Send",    "8.Helv",      0                                                                       },
    { "AddButton",    MSGID_ADDBUTTON,    ICQ_BUTTON,    "~Add",     "8.Helv",      0                                                                       },
    { "SendToButton", MSGID_SENDTO,       ICQ_BUTTON,    "Send ~To", "8.Helv",      0                                                                       },
    { "ReplyButton",  MSGID_REPLYBUTTON,  ICQ_BUTTON,    "~Reply",   "8.Helv",      0                                                                       },
    { "AcceptButton", MSGID_ACCEPTBUTTON, ICQ_BUTTON,    "Acce~pt",  "8.Helv",      0                                                                       },
    { "RefuseButton", MSGID_REFUSEBUTTON, ICQ_BUTTON,    "Re~fuse",  "8.Helv",      0                                                                       },
    { "BrowseButton", MSGID_BROWSEBUTTON, ICQ_BUTTON,    "~Browse",  "8.Helv",      0                                                                       },

    { "Entry",        MSGID_ENTRY,        WC_ENTRYFIELD, "",         "8.Helv",      ES_LEFT|ES_AUTOSCROLL|ES_MARGIN|WS_TABSTOP                              },

    { "Edit",         MSGID_EDITBOX,      WC_MLE,        "",         "8.Helv",      MLS_BORDER|MLS_WORDWRAP|MLS_VSCROLL|MLS_IGNORETAB|WS_VISIBLE|WS_TABSTOP },

    { "AboutButton",  MSGID_ABOUTBUTTON,  ICQ_BUTTON,    "",         "8.Helv",      BS_NOBORDER                                                             },
    { "InfoButton",   MSGID_INFOBUTTON,   ICQ_BUTTON,    "",         "8.Helv",      BS_NOBORDER                                                             },

    { "UserMode",     MSGID_USERMODE,     ICQ_STATIC,    "",         "",            SS_ICON                                                                 }

 };

 static const USHORT userBox[] =   {	MSGID_NICK,
   										MSGID_NICKENTRY,
   										MSGID_NAME,
   										MSGID_NAMEENTRY,
   										MSGID_USERMODE
   							       };

 static const USHORT buttonBar[] = {    MSGID_CLOSEBUTTON,		/* Close button                 */
                                        MSGID_SENDBUTTON,		/* Send message                 */
                                        MSGID_NEXTBUTTON,		/* Read next message            */
                                        MSGID_REPLYBUTTON,      /* Reply button                 */
                                        MSGID_ACCEPTBUTTON,     /* Accept button                */
                                        MSGID_REFUSEBUTTON,     /* Refuse button                */
                                        MSGID_ADDBUTTON 		/* Add user in contact-list     */
   							        };


 #pragma pack()

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static void resize(HWND, short, short);

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 hWindow icqskin_createPagerWindow(HICQ icq)
 {
    ULONG               ulFrameFlags = FCF_TITLEBAR|FCF_SIZEBORDER|FCF_MINMAX|FCF_TASKLIST|FCF_SYSMENU|FCF_NOMOVEWITHOWNER|FCF_ACCELTABLE;
    HWND                frame;
    HWND                hwnd;
    HWND                h;
    SKINDATA            *skn               = icqskin_getDataBlock(icq);
    CNRINFO             cnrinfo;
    PFIELDINFO          pFieldInfo,
                        firstFieldInfo;
    FIELDINFOINSERT     fieldInfoInsert;
    ICQMSGDIALOG        *cfg;

    frame = WinCreateStdWindow(   HWND_DESKTOP,
    						      0,
  							      &ulFrameFlags,
    							  (PSZ) icqPagerWindowClass,
    							  (PSZ) "pwICQ Alert",
    							  WS_VISIBLE,
    							  (HMODULE) module,
    							  1000,
    							  &hwnd
    						  );
    							
    if(!hwnd)
       return hwnd;

    cfg = WinQueryWindowPtr(hwnd,0);
    WinSetWindowPtr(frame,QWL_USER,cfg);

    icqskin_setICQHandle(hwnd,icq);
    icqskin_setFrameName(hwnd,"Pager");
    icqskin_createChilds(hwnd);

    return hwnd;
 }

 MRESULT EXPENTRY icqPagerWindow(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    switch(msg)
    {
    case WM_SIZE:
       resize(hwnd,SHORT1FROMMP(mp2),SHORT2FROMMP(mp2));
       return icqFrameWindow(hwnd, msg, mp1, mp2);

    case WMICQ_CREATECHILDS:
       icqskin_setMsgWindowChilds(hwnd,childlist);
       WinSendMsg(CHILD_WINDOW(hwnd,MSGID_ENTRY),EM_SETTEXTLIMIT,MPFROMSHORT(0xFF), 0);
       WinSendMsg(CHILD_WINDOW(hwnd,MSGID_EDITBOX),MLM_SETTEXTLIMIT,MPFROMLONG(MAX_MSG_SIZE), 0);
       break;

    default:
       return icqChatWindow(hwnd, msg, mp1, mp2);
    }

    return 0;
 }

 static void resize(HWND hwnd, short cx, short cy)
 {
    USHORT			    top;
    USHORT              ySize;
    USHORT				xSize;
    USHORT              yButton;
    USHORT              pos;
    HWND                h;

    top = icqskin_resizeUserInfoBox(hwnd,cx,cy);

    /* Button bar */
    ySize = icqskin_resizeButtonBox(hwnd,cx,buttonBar);

    /* Text box */
    pos = ySize+2;
    WinSetWindowPos(CHILD_WINDOW(hwnd,MSGID_TEXTBOX), 0, 0, pos, cx, (top-pos)+2, SWP_SIZE|SWP_MOVE|SWP_SHOW);

    /* URL/File entry field */
    pos += 3;

    h = CHILD_WINDOW(hwnd,MSGID_ENTRY);

    if(WinIsWindowVisible(h))
    {
       ySize -= 5;

       h = CHILD_WINDOW(hwnd,MSGID_BROWSEBUTTON);

       if(h && WinIsWindowVisible(h))
       {
          icqskin_querySizes(h,&xSize,&yButton);

          if(ySize > yButton)
          {
             WinSetWindowPos(h,0,cx-(xSize+3),pos + ((ySize/2)-(yButton/2)),xSize,yButton,SWP_SIZE|SWP_MOVE|SWP_SHOW);
          }
          else
          {
             WinSetWindowPos(h,0,cx-(xSize+3),pos,xSize,yButton,SWP_SIZE|SWP_MOVE|SWP_SHOW);
          }

          xSize  = cx - (xSize+2);
       }
       else
       {
          xSize   = cx;
       }
       WinSetWindowPos(CHILD_WINDOW(hwnd,MSGID_ENTRY),0,6,pos+3,xSize-12,ySize,SWP_SIZE|SWP_MOVE|SWP_SHOW);

       pos += max(yButton,ySize);
       pos += 5;

    }

    WinSetWindowPos(CHILD_WINDOW(hwnd,MSGID_EDITBOX), 0, 3, pos, cx-6, (top-pos)-6, SWP_SIZE|SWP_MOVE|SWP_SHOW);

 }



