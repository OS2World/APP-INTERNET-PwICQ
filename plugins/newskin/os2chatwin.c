/*
 * os2chatwin.c - OS/2 Chat Window
 */


 #pragma strings(readonly)

 #include <string.h>
 #include <malloc.h>
 #include <ctype.h>

 #define INCL_GPIPRIMITIVES
 #include <pwicqgui.h>

/*---[ Defines ]------------------------------------------------------------------------------------------*/

 #define MSGID_NAMEBOX    MSG_WINDOW_COMPONENTS
 #define MSGID_TEXTBOX    MSG_WINDOW_COMPONENTS+1
 #define HISTORY_SELECTED CRA_CURSORED

 #define CHILD_WINDOW(h,i) WinWindowFromID(h,DLG_ELEMENT_ID+i)

/*---[ Structures ]---------------------------------------------------------------------------------------*/

typedef struct _record
{
  RECORDCORE  	recordCore;
  ULONG         flags;
  HPOINTER      icon;
  PSZ			msgText;
  char          text[1];
} RECORD, *PRECORD;


/*---[ Constants ]----------------------------------------------------------------------------------------*/

 const char *icqChatWindowClass = "pwICQChatWindow";

 static const MSGCHILD childlist[] =
 {

    { "Static",       MSGID_NICK,        ICQ_STATIC,   "Nick:",     "8.Helv",      SS_TEXT|DT_LEFT|DT_VCENTER                                              },
    { "Field",        MSGID_NICKENTRY,   ICQ_STATIC,   "",          "8.Helv.Bold", SS_TEXT|DT_LEFT|DT_VCENTER                                              },
    { "Static",       MSGID_NAME,        ICQ_STATIC,   "Name:",     "8.Helv",      SS_TEXT|DT_LEFT|DT_VCENTER                                              },
    { "Field",        MSGID_NAMEENTRY,   ICQ_STATIC,   "",          "8.Helv.Bold", SS_TEXT|DT_LEFT|DT_VCENTER                                              },

    { "Edit",         MSGID_EDITBOX,     WC_MLE,       "",          "8.Helv",      MLS_BORDER|MLS_WORDWRAP|MLS_VSCROLL|MLS_IGNORETAB|WS_VISIBLE|WS_TABSTOP },
    { "History",      MSGID_HISTORY,     WC_CONTAINER, "",          "8.Helv",      WS_VISIBLE|WS_TABSTOP|CCS_READONLY                                      },


    { "CloseButton",  MSGID_CLOSEBUTTON, ICQ_BUTTON,   "~Close",    "8.Helv",      0                                                                       },
    { "SendButton",   MSGID_SENDBUTTON,  ICQ_BUTTON,   "~Send",     "8.Helv",      0                                                                       },
    { "SendToButton", MSGID_SENDTO,      ICQ_BUTTON,   "Send ~To",  "8.Helv",      0                                                                       },
    { "AddButton",    MSGID_ADDBUTTON,   ICQ_BUTTON,   "~Add",      "8.Helv",      0                                                                       },

    { "AboutButton",  MSGID_ABOUTBUTTON, ICQ_BUTTON,   "",          "8.Helv",      BS_NOBORDER                                                             },
    { "InfoButton",   MSGID_INFOBUTTON,  ICQ_BUTTON,   "",          "8.Helv",      BS_NOBORDER                                                             },

    { "UserMode",     MSGID_USERMODE,    ICQ_STATIC,   "",          "",            SS_ICON                                                                 },

    { "Textbox",      MSGID_TEXTBOX,     WC_STATIC,    "",          "8.Helv",      SS_GROUPBOX|DT_LEFT|DT_VCENTER                                          },
    { "Namebox",      MSGID_NAMEBOX,     WC_STATIC,    "",          "8.Helv",      SS_GROUPBOX|DT_LEFT|DT_VCENTER                                          }

 };


 static const USHORT userBox[] =   {	MSGID_NICK,
   										MSGID_NICKENTRY,
   										MSGID_NAME,
   										MSGID_NAMEENTRY,
   										MSGID_USERMODE
   							       };

 static const USHORT buttonBar[] = { 	MSGID_CLOSEBUTTON,
                                        MSGID_SENDBUTTON,
                                        MSGID_SENDTO,
                                        MSGID_ADDBUTTON
   							        };


 #pragma pack()

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static void   resize(HWND, short, short);
 static USHORT setControlPos(HWND, USHORT, USHORT, USHORT);
 static void   destroy(HWND);
 static void   configure(HWND, const MSGCHILD *, USHORT);
 static USHORT resizeUserInfo(HWND, short, short);
 static USHORT sizeButtonBar(HWND, short, USHORT, const USHORT *);
 static void   control(HWND, USHORT, USHORT, MPARAM);

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 hWindow icqskin_createChatWindow(HICQ icq)
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
    							  (PSZ) icqChatWindowClass,
    							  (PSZ) "pwICQ Message",
    							  WS_VISIBLE,
    							  (HMODULE) module,
    							  1000,
    							  &hwnd
    						  );
    							
    if(!hwnd)
       return hwnd;

    DBGTracex(icq);

    cfg = WinQueryWindowPtr(hwnd,0);
    WinSetWindowPtr(frame,QWL_USER,cfg);

    /* Configure message icons */
    DBGTracex(icq);
    icqskin_setICQHandle(hwnd,icq);
    icqskin_setFrameName(hwnd,"Chat");
    icqskin_createChilds(hwnd);

#ifdef DEBUG
    if(cfg->fr.icq != icq)
    {
       DBGMessage("****************** INVALID ICQ ID SET");
       DBGTracex(cfg->fr.icq);
    }
#endif

    cfg->flags  |= ICQMSGDIALOG_CHAT;
    cfg->msgIn   = icqskin_CreatePointer(skn, ICQICON_MESSAGERECEIVED);
    cfg->msgOut  = icqskin_CreatePointer(skn, ICQICON_MESSAGESENT);

    /* Configure the user history window */
    h = CHILD_WINDOW(hwnd,MSGID_HISTORY);
    WinSetWindowPtr(h,QWL_USER,cfg);

    DBGTracex(h);

    memset(&cnrinfo,0,sizeof(cnrinfo));
    cnrinfo.cb                = sizeof( CNRINFO );
    cnrinfo.flWindowAttr      = CV_DETAIL|CV_MINI; // CV_TEXT; // |CA_TREELINE CV_TREE| |CA_CONTAINERTITLE; // |CA_TITLESEPARATOR;
    cnrinfo.cxTreeIndent      = 1;
    cnrinfo.cxTreeLine        = -1;
    cnrinfo.cyLineSpacing     = 10;
    cnrinfo.slBitmapOrIcon.cx =
    cnrinfo.slBitmapOrIcon.cy = skn->iconSize;
    cnrinfo.pszCnrTitle       = "";

    WinSendMsg(h, CM_SETCNRINFO, MPFROMP( &cnrinfo ),MPFROMLONG(CMA_FLWINDOWATTR|CMA_CXTREEINDENT|CMA_LINESPACING|CMA_SLBITMAPORICON)); //); |CMA_CNRTITLE)); // |CMA_LINESPACING));

    if(!WinSendMsg(h, CM_SETCNRINFO, &cnrinfo, MPFROMLONG(CMA_FLWINDOWATTR|CMA_CNRTITLE)))
    {
       DBGMessage("Error in container configuration");
       DBGTrace(WinGetLastError(WinQueryAnchorBlock(h)));
       icqskin_closeWindow(hwnd);
    }
    else
    {
       firstFieldInfo         =
       pFieldInfo             = WinSendMsg(h, CM_ALLOCDETAILFIELDINFO, MPFROMLONG(2), NULL);

       DBGTracex(pFieldInfo);

       pFieldInfo->cb         = sizeof(FIELDINFO);
       pFieldInfo->flData     = CFA_BITMAPORICON  | CFA_TOP | CFA_CENTER; // | CFA_SEPARATOR;
       pFieldInfo->flTitle    = CFA_CENTER;
       pFieldInfo->pTitleData = (PVOID) "";
       pFieldInfo->offStruct  = FIELDOFFSET(RECORD,icon);

       pFieldInfo             = pFieldInfo->pNextFieldInfo;
       pFieldInfo->cb         = sizeof(FIELDINFO);
       pFieldInfo->flData     = CFA_STRING | CFA_LEFT; // | CFA_SEPARATOR;
       pFieldInfo->flTitle    = CFA_LEFT;
       pFieldInfo->pTitleData = (PVOID) "";
       pFieldInfo->offStruct  = FIELDOFFSET(RECORD,msgText);

       memset(&fieldInfoInsert,0,sizeof(FIELDINFOINSERT));
       fieldInfoInsert.cb                   = (ULONG)(sizeof(FIELDINFOINSERT));
       fieldInfoInsert.pFieldInfoOrder      = (PFIELDINFO)CMA_FIRST;
       fieldInfoInsert.cFieldInfoInsert     = 2;
       fieldInfoInsert.fInvalidateFieldInfo = TRUE;

       if(!WinSendMsg(h, CM_INSERTDETAILFIELDINFO,firstFieldInfo,&fieldInfoInsert))
       {
          DBGMessage("******** CM_INSERTDETAILFIELDINFO has failed!");
          DBGTrace(WinGetLastError(WinQueryAnchorBlock(h)));
          icqskin_closeWindow(hwnd);
       }
    }

    WinEnableWindow(CHILD_WINDOW(hwnd,MSGID_SENDBUTTON), FALSE);

    return hwnd;
 }

 static void containerAction(HWND hwnd, USHORT id, void *parm)
 {
    PRECORD       rec;
    ICQMSGDIALOG  *cfg   = WinQueryWindowPtr(hwnd,0);
    const char    *text  = NULL;
    int           f      = 0;

    DBGTracex(cfg);

    if(id != CN_EMPHASIS)
       return;

    DBGTracex(((PNOTIFYRECORDEMPHASIS) parm)->fEmphasisMask & CRA_SELECTED);

    if( !(((PNOTIFYRECORDEMPHASIS) parm)->fEmphasisMask & CRA_SELECTED) )
       return;

    rec = (PRECORD) ((PNOTIFYRECORDEMPHASIS) parm)->pRecord;
    DBGTracex(rec);
    DBGMessage(rec->text);

    text = (char *) icqQueryURL(cfg->fr.icq, rec->text);

    DBGTracex(text);

    if(text)
    {
       for(f=0;*text && !isspace(*text) && f < 0xFE;f++)
          cfg->sendToParm[f] = *(text++);
    }

    cfg->sendToParm[f] = 0;

    DBGMessage(cfg->sendToParm);
#ifndef DEBUG
    WinEnableControl(hwnd, DLG_ELEMENT_ID+MSGID_SENDTO, f > 0);
#endif

 }

 MRESULT EXPENTRY icqChatWindow(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    switch(msg)
    {
    case WM_CHAR:
       ((ICQMSGDIALOG *) WinQueryWindowPtr(hwnd,0))->idleSeconds = 0;
       return icqFrameWindow(hwnd, msg, mp1, mp2);

    case WM_CREATE:
       icqskin_cfgWindow(hwnd,ICQMSGDIALOG,0);
       break;

    case WMICQ_CREATECHILDS:
       CHKPoint();
       icqskin_setMsgWindowChilds(hwnd,childlist);
       CHKPoint();
       break;

    case WMICQ_LOADSKIN:
       icqFrameWindow(hwnd, msg, mp1, mp2);
       WinSendMsg(hwnd,WMICQ_SKINCHILDS,mp1,mp2);
       break;

    case WM_CONTROL:
       control(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1),mp2);
       break;

    case WM_SIZE:
       resize(hwnd,SHORT1FROMMP(mp2),SHORT2FROMMP(mp2));
       return icqFrameWindow(hwnd, msg, mp1, mp2);

    case WM_DESTROY:
       destroy(hwnd);
       return icqFrameWindow(hwnd, msg, mp1, mp2);

    case WM_USER+10:
        WinInvalidateRect(hwnd,NULL,TRUE);
        break;

    case WMICQ_SETMSGCHILD:
        configure(hwnd, (const MSGCHILD *) mp1, SHORT1FROMMP(mp2));
        break;

    case WMICQ_SIZEMSGBOX:
       return (MRESULT) resizeUserInfo(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1));

    case WMICQ_SIZEBUTTONS:
       return (MRESULT) sizeButtonBar(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1),(const USHORT *) mp2);

    default:
       return icqFrameWindow(hwnd, msg, mp1, mp2);

    }

    return 0;
 }

 static void destroy(HWND hwnd)
 {
    ICQMSGDIALOG *cfg = WinQueryWindowPtr(hwnd,0);

	icqskin_destroyMsgWindow(cfg);
    icqskin_terminateMessageEditor(cfg->fr.icq, hwnd);

    DBGTracex(cfg->msgIn);

    if(cfg->msgIn)
       WinDestroyPointer(cfg->msgIn);

    if(cfg->msgOut)
       WinDestroyPointer(cfg->msgOut);

 }

 static void configure(HWND hwnd, const MSGCHILD *def, USHORT qtd)
 {
    int                 f;
    HWND                h;
    ICQMSGDIALOG        *cfg        = WinQueryWindowPtr(hwnd,0);

#ifdef STANDARD_GUI

    ULONG               bg          = 0x00D1D1D1;

    static const USHORT stsFields[] = { MSGID_NICK,
    			                        MSGID_NICKENTRY,
    			                        MSGID_NAME,
    			                        MSGID_NAMEENTRY
    			                      };

#endif

    DBGMessage("Creating child windows");
    DBGTracex(cfg->fr.icq);

    cfg->fr.childCount = qtd;
    cfg->fr.childList  = def;

    for(f=0;f < qtd;f++)
    {
       h = WinCreateWindow(   hwnd,                     /* Parent window             */
                              (PSZ) def->wClass,        /* Class name                */
                              (PSZ) def->wText,         /* Window text               */
                              def->wStyle|WS_VISIBLE,   /* Window style              */
                               0, 0,                    /* Position (x,y)            */
                              10,10,                    /* Size (width,height)       */
                              hwnd,                     /* Owner window              */
                              HWND_BOTTOM,              /* Sibling window            */
                              DLG_ELEMENT_ID+def->id,   /* Window id                 */
                              NULL,                     /* Control data              */
                              NULL);                    /* Pres parameters           */

       icqskin_setICQHandle(h,cfg->fr.icq);
       icqskin_setFont(h,def->font);
       icqskin_setButtonName(h,def->name);

       def++;
    }

#ifdef STANDARD_GUI
    WinSetPresParam(hwnd,PP_BACKGROUNDCOLOR,sizeof(bg),&bg);
    WinSetPresParam(hwnd,PP_DISABLEDBACKGROUNDCOLOR,sizeof(bg),&bg);

    for(f=0;f< sizeof(stsFields)/sizeof(USHORT);f++)
    {
       h = CHILD_WINDOW(hwnd,stsFields[f]);
       if(h)
       {
          WinSetPresParam(h,PP_BACKGROUNDCOLOR,sizeof(bg),&bg);
          WinSetPresParam(h,PP_DISABLEDBACKGROUNDCOLOR,sizeof(bg),&bg);
       }
    }

    h = CHILD_WINDOW(hwnd,MSGID_HISTORY);

    if(h)
    {
       bg = 0x00FFFFFF;
       WinSetPresParam(h,PP_BACKGROUNDCOLOR,sizeof(bg),&bg);
       WinSetPresParam(h,PP_DISABLEDBACKGROUNDCOLOR,sizeof(bg),&bg);
    }

#endif

    /* Set button icons */
    icqskin_setButtonIcon(CHILD_WINDOW(hwnd,MSGID_ABOUTBUTTON), ICQICON_ABOUT);
    icqskin_setButtonIcon(CHILD_WINDOW(hwnd,MSGID_INFOBUTTON),  ICQICON_USERCONFIG);
    icqskin_setButtonIcon(CHILD_WINDOW(hwnd,MSGID_USERMODE),    ICQICON_OFFLINE);

    DBGTracex(CHILD_WINDOW(hwnd,MSGID_EDITBOX));

    DBGMessage("Child creation complete");

 }

 static USHORT setControlPos(HWND hwnd, USHORT x, USHORT y, USHORT ySize)
 {
    char   buffer[0x0100];
    POINTL aptl[TXTBOX_COUNT];
    HPS    hps;

    ULONG  flag        = SWP_SIZE;

    WinQueryWindowText(hwnd,0xFF,buffer);
    *(buffer+0xFF) = 0;

    hps = WinGetPS(hwnd);

    GpiQueryTextBox(      hps,
                          strlen(buffer),
                          (char *) buffer,
                          TXTBOX_COUNT,
                          aptl);
    WinReleasePS(hps);

    if(x != 0xFFFF && y != 0xFFFF)
       flag |= SWP_MOVE|SWP_SHOW;

    WinSetWindowPos(hwnd, 0, x, y, aptl[TXTBOX_TOPRIGHT].x, ySize, flag);

    return x+aptl[TXTBOX_TOPRIGHT].x;

 }

 static USHORT getControlSize(HWND hwnd)
 {
    USHORT ret;

    icqskin_querySizes(hwnd,0,&ret);

    return ret;

/*
    char   buffer[0x0100];
    POINTL aptl[TXTBOX_COUNT];
    HPS    hps         =  WinGetPS(hwnd);

    CHKPoint();
    WinQueryWindowText(hwnd,0xFF,buffer);
    *(buffer+0xFF) = 0;
    DBGMessage(buffer);

    hps = WinGetPS(hwnd);

    GpiQueryTextBox(      hps,
                          strlen(buffer),
                          (char *) buffer,
                          TXTBOX_COUNT,
                          aptl);
    WinReleasePS(hps);

    return (aptl[TXTBOX_TOPRIGHT].y - aptl[TXTBOX_BOTTOMRIGHT].y)+2;
*/
 }

 static USHORT resizeUserInfo(HWND hwnd, short cx, short cy)
 {
    int     f;
    int     ySize;
    int		iconSize;
    USHORT	top;
    int     pos;
    USHORT	y;

    iconSize =
    ySize    = (int) icqskin_querySizes(CHILD_WINDOW(hwnd,MSGID_ABOUTBUTTON),0,0);

    for(f=0;f < (sizeof(userBox)/sizeof(USHORT)); f++)
    {
       pos = getControlSize(CHILD_WINDOW(hwnd,userBox[f]));
       if(pos > ySize)
          ySize = pos;
    }

    top = y = (cy-(ySize+8));
    WinSetWindowPos(CHILD_WINDOW(hwnd,MSGID_NAMEBOX), 0, 0, y, cx, ySize+12, SWP_SIZE|SWP_MOVE|SWP_SHOW);

    y   += 3;

    WinSetWindowPos(CHILD_WINDOW(hwnd,MSGID_USERMODE), 0, 3, y, iconSize, iconSize, SWP_SIZE|SWP_MOVE|SWP_SHOW);

    pos  = setControlPos(CHILD_WINDOW(hwnd,MSGID_NICK),6+ySize, y, ySize);
    WinSetWindowPos(CHILD_WINDOW(hwnd,MSGID_NICKENTRY), 0, pos+4, y, (cx/2)-(pos+8), ySize, SWP_SIZE|SWP_MOVE|SWP_SHOW);

    pos  = setControlPos(CHILD_WINDOW(hwnd,MSGID_NAME),cx/2,y,ySize);

    f    = cx-((iconSize*2)+6);
    WinSetWindowPos(CHILD_WINDOW(hwnd,MSGID_NAMEENTRY), 0, pos+4, y, f-(pos+6), ySize, SWP_SIZE|SWP_MOVE|SWP_SHOW);

    f   += 1;
    WinSetWindowPos(CHILD_WINDOW(hwnd,MSGID_ABOUTBUTTON), 0, f, y, iconSize, iconSize, SWP_SIZE|SWP_MOVE|SWP_SHOW);

    f   += iconSize+1;
    WinSetWindowPos(CHILD_WINDOW(hwnd,MSGID_INFOBUTTON), 0, f, y, iconSize, iconSize, SWP_SIZE|SWP_MOVE|SWP_SHOW);

    return top;
 }

 static USHORT sizeButtonBar(HWND hwnd, short cx, USHORT btnCount, const USHORT *btnlist)
 {
    USHORT              pos;
    int                 ySize = 0;
    USHORT			    xSize;
    int                 f;
    USHORT				y     = 0;
    HWND                h;

    for(f=0;f < btnCount; f++)
    {
       h = CHILD_WINDOW(hwnd,btnlist[f]);
       if(WinIsWindowVisible(h))
       {
          icqskin_querySizes(h,0,&y);
          if(y > ySize)
             ySize = y;
       }
    }

    pos = cx;

    for(f=0;f < btnCount; f++)
    {
       h = CHILD_WINDOW(hwnd,btnlist[f]);
       if(WinIsWindowVisible(h))
       {
          xSize  = (USHORT) icqskin_querySizes(h,0,0);
          pos   -= xSize;
          WinSetWindowPos(h, 0, pos, 0, xSize, ySize, SWP_SIZE|SWP_MOVE);
          pos   -= 4;
       }
    }

    return ySize;
 }

 static void resize(HWND hwnd, short cx, short cy)
 {
    USHORT    ySize;
    USHORT	  top;
    USHORT    fator;
    int       pos;
    HWND	  h;

    top = icqskin_resizeUserInfoBox(hwnd,cx,cy);

    /* Button bar */
    ySize = icqskin_resizeButtonBox(hwnd,cx,buttonBar);

    /* Text box */
    pos = ySize+2;
    WinSetWindowPos(CHILD_WINDOW(hwnd,MSGID_TEXTBOX), 0, 0, pos, cx, (top-pos)+2, SWP_SIZE|SWP_MOVE|SWP_SHOW);

    /* Edit area */
    pos += 6;
    top -= 10;

    h = CHILD_WINDOW(hwnd,MSGID_HISTORY);

    if(h && WinIsWindowVisible(h))
    {
       fator = (top-pos) / 3;

       if(fator < (ySize * 3))
          ySize = fator;
       else
          ySize *= 3;

       WinSetWindowPos(h, 0, 4, pos+ySize, cx-8, top-(pos+ySize), SWP_SIZE|SWP_MOVE|SWP_SHOW);
       top = (pos+ySize)-4;
    }

    WinSetWindowPos(CHILD_WINDOW(hwnd,MSGID_EDITBOX), 0, 4, pos, cx-8, top-pos, SWP_SIZE|SWP_MOVE|SWP_SHOW);
 }

 int icqskin_logErrorInfo(HICQ icq, hWindow hwnd, BOOL close, char *msg)
 {
    char      buffer[0x0100];
    PERRINFO  perriErrorInfo  = WinGetErrorInfo(WinQueryAnchorBlock(hwnd));

    if(perriErrorInfo)
    {
       DBGTracex(perriErrorInfo->offaoffszMsg);

       sprintf(buffer,"%s (rc=%d/0x%04x)",perriErrorInfo->idError,perriErrorInfo->idError);
       DBGMessage(buffer);

       if(icq)
       {
          icqWriteSysLog(icq,PROJECT,buffer);
       }

       WinFreeErrorInfo(perriErrorInfo);
    }

    if(close)
    {
       icqskin_closeWindow(hwnd);
    }

    return -1;
 }

 int icqskin_insertMessageHistory(HWND hwnd, ULONG flags, const char *hdr, const char *text)
 {
    ICQMSGDIALOG    *cfg           = WinQueryWindowPtr(hwnd,0);
    ULONG		    bytes          = sizeof(RECORD)+strlen(hdr)+strlen(text)+5;
    HWND 		    h 		       = CHILD_WINDOW(hwnd,MSGID_HISTORY);
    PRECORD		    rec;
    RECORDINSERT    recordInsert;
    PRECORD         last           = NULL;
    char            *ptr;

    DBGTracex(h);

    last = (PRECORD) WinSendMsg(h,CM_QUERYRECORD, 0, MPFROM2SHORT(CMA_LAST,CMA_ITEMORDER));
    if( ((long) last) == -1)
       last = NULL;

    rec = WinSendMsg(h,CM_ALLOCRECORD,MPFROMLONG(bytes),MPFROMSHORT(1));
    DBGTracex(rec);

    if(!rec)
      return icqskin_logErrorInfo(((ICQFRAME *) WinQueryWindowPtr(hwnd,0))->icq,hwnd,TRUE,"CM_ALLOCRECORD has failed");

    memset(rec,0,sizeof(RECORD));

    rec->recordCore.cb              = sizeof(RECORDCORE);

    sprintf(rec->text,"%s\n%s",hdr,text);

    rec->recordCore.pszText 	    =
    rec->recordCore.pszIcon 	    =
    rec->recordCore.pszName 	    =
    rec->msgText                    = rec->text;

    rec->flags                      = flags;

    rec->icon                       =
    rec->recordCore.hptrIcon        = (flags & 1) ? cfg->msgOut : cfg->msgIn;

    for(ptr = rec->text;*ptr;ptr++)
    {
       if(*ptr == ICQ_FIELD_SEPARATOR)
          *ptr = '\n';
    }

    memset(&recordInsert,0,sizeof(RECORDINSERT));
    recordInsert.cb 			    = sizeof(RECORDINSERT);
    recordInsert.pRecordParent		= NULL;
    recordInsert.pRecordOrder 		= (PRECORDCORE)CMA_END;
    recordInsert.zOrder    		    = CMA_TOP;
    recordInsert.cRecordsInsert 	= 1;
    recordInsert.fInvalidateRecord 	= TRUE;

    if(!WinSendMsg(h, CM_INSERTRECORD, (PRECORDCORE) rec, &recordInsert))
       return icqskin_logErrorInfo(((ICQFRAME *) WinQueryWindowPtr(hwnd,0))->icq,hwnd,TRUE,"CM_INSERTRECORD has failed");

    if(last)
    {
       if(last->recordCore.flRecordAttr & HISTORY_SELECTED)
          WinSendMsg(h,CM_SETRECORDEMPHASIS,(MPARAM) last, MPFROM2SHORT(FALSE, HISTORY_SELECTED));
       else
          return 0;
    }

    WinSendMsg(h,CM_SETRECORDEMPHASIS,(MPARAM) rec,  MPFROM2SHORT(TRUE,  HISTORY_SELECTED));

    WinPostMsg(WinWindowFromID(h,CID_VSCROLL),WM_CHAR,MPFROMSH2CH(KC_VIRTUALKEY,0,0),MPFROM2SHORT(0,VK_END));
    WinPostMsg(hwnd,WM_USER+10,0,0);

    return 0;

 }

 static void control(HWND hwnd, USHORT id, USHORT notify, MPARAM mp2)
 {
    HWND h = WinWindowFromID(hwnd,id);

    id -= DLG_ELEMENT_ID;

    switch(id)
    {
    case MSGID_HISTORY:
       containerAction(hwnd,notify,(void *) mp2);
       break;

    case MSGID_EDITBOX:
       if(notify == MLN_CHANGE)
          WinEnableWindow(CHILD_WINDOW(hwnd,MSGID_SENDBUTTON), (WinQueryWindowTextLength(h) > 0) );
       break;
    }
 }



