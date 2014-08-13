/*
 * os2create.c - Create OS2 Windows
 */

 #pragma strings(readonly)

 #define INCL_WIN
 #define INCL_DOSRESOURCES

 #include <stdlib.h>
 #include <string.h>
 #include <pwicqgui.h>

/*---[ Structures ]---------------------------------------------------------------------------------------*/

 #pragma pack(1)

 typedef struct _icqdialog
 {
    ICQFRAME_STANDARD_PREFIX

    USHORT       szDataBlock;
    const DLGMGR *mgr;

 } ICQDIALOG;

 struct cparm
 {
    USHORT       sz;
    HICQ         icq;
    const DLGMGR *mgr;
 };


 #pragma pack()

/*---[ Constants ]----------------------------------------------------------------------------------------*/

 const char   *icqDialogWindowClass = "pwICQDialog";

 static PFNWP listBoxClassProcessor = NULL;

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static void create(HWND, PCREATESTRUCT);
 static void destroy(HWND);
 static void erase(HWND, HPS, PRECTL);
 static void paint(HWND);
 static void resize(HWND, short, short);
 static void initialize(HWND,ULONG);
 static void load(HWND,ULONG);
 static void save(HWND,ULONG);
 static int  control(HWND,USHORT, USHORT, ULONG);
 static void event(HWND, USHORT, USHORT, ULONG);
 static int  command(HWND, USHORT, USHORT, USHORT);

 MRESULT EXPENTRY icqListBoxWindow(HWND, ULONG, MPARAM, MPARAM);

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

/*

DLGTEMPLATE:

  USHORT       cbTemplate;          Length of template.
  USHORT       type;                Template format type.
  USHORT       codepage;            Code page.
  USHORT       offadlgti;           Offset to dialog items.
  USHORT       fsTemplateStatus;    Template status.
  USHORT       iItemFocus;          Index of item to receive focus initially.
  USHORT       coffPresParams;      Count of presentation-parameter offsets.
  DLGTITEM     adlgti[1];           Start of dialog items.

DLGITEM:

  USHORT     fsItemStatus;    Status.
  USHORT     cChildren;       Count of children to this dialog item.
  USHORT     cchClassLen;     Length of class name.
  USHORT     offClassName;    Offset to class name.
  USHORT     cchTextLen;      Length of text.
  USHORT     offText;         Offset to text.
  ULONG      flStyle;         Dialog item window style.
  SHORT      x;               X-coordinate of origin of dialog-item window.
  SHORT      y;               Y-coordinate of origin of dialog-item window.
  SHORT      cx;              Dialog-item window width.
  SHORT      cy;              Dialog-item window height.
  USHORT     id;              Identity.
  USHORT     offPresParams;   Reserved.
  USHORT     offCtlData;      Offset to control data.

  WC_FRAME             ((PSZ)0xffff0001L)
  WC_COMBOBOX          ((PSZ)0xffff0002L)
  WC_BUTTON            ((PSZ)0xffff0003L)
  WC_MENU              ((PSZ)0xffff0004L)
  WC_STATIC            ((PSZ)0xffff0005L)
  WC_ENTRYFIELD        ((PSZ)0xffff0006L)
  WC_LISTBOX           ((PSZ)0xffff0007L)
  WC_SCROLLBAR         ((PSZ)0xffff0008L)
  WC_TITLEBAR          ((PSZ)0xffff0009L)
  WC_MLE               ((PSZ)0xffff000AL)
  WC_APPSTAT           ((PSZ)0xffff0010L)
  WC_KBDSTAT           ((PSZ)0xffff0011L)
  WC_PECIC             ((PSZ)0xffff0012L)
  WC_DBE_KKPOPUP       ((PSZ)0xffff0013L)
  WC_SPINBUTTON        ((PSZ)0xffff0020L)
  WC_CONTAINER         ((PSZ)0xffff0025L)
  WC_SLIDER            ((PSZ)0xffff0026L)
  WC_VALUESET          ((PSZ)0xffff0027L)
  WC_NOTEBOOK          ((PSZ)0xffff0028L)
  WC_PENFIRST          ((PSZ)0xffff0029L)
  WC_PENLAST           ((PSZ)0xffff002CL)
  WC_MMPMFIRST         ((PSZ)0xffff0040L)
  WC_CIRCULARSLIDER    ((PSZ)0xffff0041L)
  WC_MMPMLAST          ((PSZ)0xffff004fL)
  WC_PRISTDDLGFIRST    ((PSZ)0xffff0050L)
  WC_PRISTDDLGLAST     ((PSZ)0xffff0057L)
  WC_PUBSTDDLGFIRST    ((PSZ)0xffff0058L)
  WC_PUBSTDDLGLAST     ((PSZ)0xffff005fL)

*/


 hWindow icqskin_loadDialog(HICQ icq, hWindow owner, HMODULE hmod, USHORT idName, const DLGMGR *mgr)
 {
    HAB                 hab   = icqQueryAnchorBlock(icq);
    HWND                hwnd  = NULLHANDLE;
    USHORT				cx    = 0;
    USHORT				cy    = 0;
    ULONG               *pal  = NULL;
    ULONG               bg    = 0x00D1D1D1;
    ULONG               fg    = 0x00000040;
    struct cparm        cParm = { sizeof(struct cparm), icq, mgr };

    APIRET              rc;
    HWND                h;
    char				text[0x0100];
    char				classBuffer[0x0100];
    PSZ					wndClass;
    PDLGTEMPLATE        res;
    PDLGTITEM           itn;
    CLASSINFO           cInfo;
    int                 szString;
    int                 qtd;
    POINTL				ptl[2];
    int                 f;

    rc = DosGetResource(hmod, RT_DIALOG, idName, (PPVOID) &res);

    DBGTrace(rc);

    if(rc)
    {
       icqWriteSysRC(icq, PROJECT, rc, "Failure loading resource information");
       return 0;
    }

    if(owner)
       pal = (ULONG *) WinSendMsg(owner,WMICQ_QUERYPALLETE,0,0);

    if(pal)
    {
       bg = pal[ICQCLR_BACKGROUND];
       fg = pal[ICQCLR_FOREGROUND];
    }

    if(!listBoxClassProcessor)
    {
       if(WinQueryClassInfo(WinQueryAnchorBlock(owner), WC_LISTBOX, &cInfo))
          listBoxClassProcessor = cInfo.pfnWindowProc;
    }

    DBGTracex(listBoxClassProcessor);

/*
    DBGTrace(res->cbTemplate);
    DBGTrace(res->type);
    DBGTrace(res->codepage);
    DBGTrace(res->offadlgti);
    DBGTrace(res->fsTemplateStatus);
    DBGTrace(res->iItemFocus);
    DBGTrace(res->coffPresParams);

#ifdef DEBUG
    icqDumpPacket(icq, NULL, "Dialog loaded", res->cbTemplate, (unsigned char *) res);
#endif
*/

    itn      = res->adlgti;
    qtd      = itn->cChildren;

    ptl[0].x = itn->x;
    ptl[0].y = itn->y;
    ptl[1].x = itn->cx;
    ptl[1].y = itn->cy;

/*
    DBGMessage("** WINDOW MAPPING **");
    DBGTrace(itn->cx);
    DBGTrace(itn->cy);
*/
    WinMapDlgPoints(owner, ptl, 2, TRUE);
/*
    DBGTrace(itn->cx);
    DBGTrace(itn->cy);
*/

    hwnd = WinCreateWindow( owner,                        // Parent window
                            (PSZ) icqDialogWindowClass,   // Class name
                            (PSZ) "",                     // Window text
                            0,                            // Window style
                            ptl[0].x, ptl[0].y,           // Position (x,y)
                            ptl[1].x, ptl[1].y,           // Size (width,height)
                            owner,                        // Owner window
                            HWND_BOTTOM,                  // Sibling window
                            2000,                         // Window id
                            &cParm,                       // Control data
                            NULL);                        // Pres parameters

    for(f=0;f<qtd;f++)
    {
       itn++;

       szString =  min(0xFF,itn->offText);
       strncpy(text,( (char *) res)+itn->offText,szString);
       *(text+szString) = 0;

       if(itn->cchClassName)
       {
          szString =  min(0xFF,itn->offClassName);
          strncpy(classBuffer,( (char *) res)+itn->offClassName,szString);
          *(classBuffer+szString) = 0;
          wndClass = classBuffer;
       }
       else
       {
          wndClass =  (PSZ) ( (0xffff0000L) | ((ULONG) itn->offClassName));
       }

       ptl[0].x = itn->x;
       ptl[0].y = itn->y;
       ptl[1].x = itn->cx;
       ptl[1].y = itn->cy;

       WinMapDlgPoints(hwnd, ptl, 2, TRUE);

       if(wndClass == WC_STATIC && (itn->flStyle & SS_BITMAP))
       {
          DBGMessage("Creating bitmap");
          h = WinCreateWindow(   hwnd,                       // Parent window
                                 (PSZ) icqBitmapWindowClass, // Class name
                                 "",                         // Window text
                                 itn->flStyle,               // Window style
                                 ptl[0].x, ptl[0].y,         // Position (x,y)
                                 ptl[1].x, ptl[1].y,         // Size (width,height)
                                 hwnd,                       // Owner window
                                 HWND_BOTTOM,                // Sibling window
                                 itn->id,                    // Window id
                                 NULL,                       // Control data
                                 NULL);                      // Pres parameters
          DBGTracex(h);
       }
       else
       {
          h = WinCreateWindow(   hwnd,                       // Parent window
                                 wndClass,                   // Class name
                                 (PSZ) text,                 // Window text
                                 itn->flStyle,               // Window style
                                 ptl[0].x, ptl[0].y,         // Position (x,y)
                                 ptl[1].x, ptl[1].y,         // Size (width,height)
                                 hwnd,                       // Owner window
                                 HWND_BOTTOM,                // Sibling window
                                 itn->id,                    // Window id
                                 NULL,                       // Control data
                                 NULL);                      // Pres parameters
      }

      if(!h)
      {
         icqPrintLog(icq,PROJECT,"Failure creating window class %08lx for dialog %04d:%04d in module %08lx",
         							(ULONG) wndClass,
         							(int)   idName, (int) itn->id,
         							(ULONG) hmod );
      }
      else if(wndClass == WC_LISTBOX)
      {
         if(WinSubclassWindow(h, icqListBoxWindow) != listBoxClassProcessor)
         {
            DBGMessage("******** ERROR, UNEXPECTED Listbox class Processor when subclassing");
            icqWriteSysLog(icq,PROJECT,"*** UNEXPECTED Listbox class Processor when subclassing");
         }
#ifdef DEBUG
         else
         {
            DBGMessage("Listbox subclassed");
         }

         *text = 0;
         icqskin_getWindowText(h,0xFF,text);
         DBGMessage(text);

         WinSetWindowText(h,"Teste1");
         icqskin_getWindowText(h,0xFF,text);
         DBGMessage(text);

#endif

      }

    }

    DosFreeResource(res);

    icqskin_setFont(hwnd,"8.Helv");
    WinSetPresParam(hwnd,PP_FOREGROUNDCOLOR,sizeof(ULONG),&fg);

    WinSetPresParam(hwnd,PP_BACKGROUNDCOLOR,sizeof(ULONG),&bg);
    WinSetPresParam(hwnd,PP_DISABLEDBACKGROUNDCOLOR,sizeof(ULONG),&bg);

    return hwnd;
 }

 MRESULT EXPENTRY icqDialogWindow(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    switch(msg)
    {
    case WM_CREATE:
       create(hwnd,(PCREATESTRUCT) mp2);
       break;

    case WM_DESTROY:
       destroy(hwnd);
       break;

    case WM_ERASEBACKGROUND:
       erase(hwnd,(HPS) mp1, (PRECTL) mp2);
       break;

    case WM_PAINT:
       paint(hwnd);
       break;

    case WM_SIZE:
      resize(hwnd,SHORT1FROMMP(mp2),SHORT2FROMMP(mp2));
      break;

    case WM_CONTROL:    /* Control event */
       control(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1),LONGFROMMP(mp2));
       break;

    case WMICQ_SETICQHANDLE:
       ((ICQDIALOG *) WinQueryWindowPtr(hwnd,0))->icq  = (HICQ) mp1;
       break;

    case WMICQ_SETDIALOG:
       ((ICQDIALOG *) WinQueryWindowPtr(hwnd,0))->mgr  = (const DLGMGR *) mp1;
       break;

    case WMICQ_INITIALIZE:
       initialize(hwnd,(ULONG) mp1);
       break;

    case WMICQ_LOAD:
       load(hwnd,(ULONG) mp1);
       break;

    case WMICQ_SAVE:
       save(hwnd,(ULONG) mp1);
       break;

    case WMICQ_DLGEVENT:
       event(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1),(ULONG) mp2);
       break;

    case WMICQ_GETUSERID:
       return WinSendMsg(WinQueryWindow(hwnd,QW_PARENT),msg,mp1,mp2);

    case WM_COMMAND:
       if(command(hwnd, SHORT1FROMMP(mp1), SHORT1FROMMP(mp2), SHORT2FROMMP(mp1)))
          return WinDefWindowProc(hwnd,msg,mp1,mp2);
       break;

    default:
       return WinDefWindowProc(hwnd,msg,mp1,mp2);

    }

    return 0;
 }

 static void erase(HWND hwnd, HPS hps, PRECTL rcl)
 {
    hps = WinBeginPaint(hwnd,NULLHANDLE,rcl);
    icqskin_eraseRect(hwnd, hps, rcl);
    WinEndPaint(hps);
 }

 static void paint(HWND hwnd)
 {
    RECTL rcl;
    HPS   hps   = WinBeginPaint(hwnd,NULLHANDLE,&rcl);
    icqskin_eraseRect(hwnd, hps, &rcl);
    WinEndPaint(hps);
 }

 static void destroy(HWND hwnd)
 {
    ICQDIALOG *cfg = WinQueryWindowPtr(hwnd,0);
    if(!cfg || cfg->sz < sizeof(ICQDIALOG))
       return;
    cfg->sz = 0;
    free(cfg);
 }

 static void resize(HWND hwnd, short cx, short cy)
 {

 }

 static void create(HWND hwnd, PCREATESTRUCT cr)
 {
    struct cparm *cParm             = (struct cparm *) cr->pCtlData;
    USHORT       szDataBlock        = 0;
    char         buffer[0x0100];
    ICQDIALOG    *cfg;

    DBGTracex(cParm);

    if(!cParm)
    {
       WinPostMsg(hwnd,WM_CLOSE,0,0);
       return;
    }

    if(cParm->sz != sizeof(struct cparm))
    {
       DBGMessage("************ INVALID CONTROL DATA");
       WinPostMsg(hwnd,WM_CLOSE,0,0);
       return;
    }

    if(cParm->mgr)
       szDataBlock = cParm->mgr->userDataSize;

    cfg = malloc(szDataBlock+sizeof(ICQDIALOG));

    if(!cfg)
    {
       icqWriteSysLog(cParm->icq,PROJECT,"Memory allocation error when creating dialog box");
       WinPostMsg(hwnd,WM_CLOSE,0,0);
       return;
    }

    memset(cfg,0,szDataBlock+sizeof(ICQDIALOG));
    cfg->sz          = sizeof(ICQDIALOG);
    cfg->ft          = 1;
    cfg->szDataBlock = szDataBlock;
    cfg->icq         = cParm->icq;
    cfg->mgr         = cParm->mgr;

    WinSetWindowPtr(hwnd,0,cfg);
    WinSendMsg(hwnd,WMICQ_CONFIGURE,MPFROM2SHORT(sizeof(ICQDIALOG),szDataBlock), (MPARAM) cr);

 }

 static void initialize(HWND hwnd, ULONG uin)
 {
    ICQDIALOG *cfg            = WinQueryWindowPtr(hwnd,0);
    char      buffer[0x0100];

    if(cfg && cfg->mgr && cfg->mgr->initialize)
    {
       if(cfg->mgr->initialize(&icqskin_dialogCalls,hwnd,cfg->icq,uin,buffer) < 0)
          cfg->mgr = NULL;
    }
 }

 static void load(HWND hwnd, ULONG uin)
 {
    ICQDIALOG *cfg            = WinQueryWindowPtr(hwnd,0);
    char      buffer[0x0100];

    WinEnableWindow(hwnd,TRUE);

    if(cfg && cfg->mgr)
    {
       if(cfg->mgr->load)
       {
          if(cfg->mgr->load(&icqskin_dialogCalls,hwnd,cfg->icq,uin,buffer) < 0)
             WinEnableWindow(hwnd,FALSE);
       }

       if(!cfg->mgr->save)
          WinEnableWindow(hwnd,FALSE);
    }


 }

 static void save(HWND hwnd, ULONG uin)
 {
    ICQDIALOG *cfg            = WinQueryWindowPtr(hwnd,0);
    char      buffer[0x0100];

    DBGPrint("Saving dialog information for ICQ#%ld...",uin);

    if(cfg && cfg->mgr && cfg->mgr->save)
    {
       CHKPoint();
       if(cfg->mgr->save(&icqskin_dialogCalls,hwnd,cfg->icq,uin,buffer) < 0)
          WinEnableWindow(hwnd,FALSE);
    }
 }

 static void event(HWND hwnd, USHORT id, USHORT event, ULONG parm)
 {
    ICQDIALOG      *cfg            = WinQueryWindowPtr(hwnd,QWL_USER);
    ULONG          uin             = (ULONG) WinSendMsg(WinQueryWindow(hwnd,QW_PARENT),WMICQ_GETUSERID,0,0);
    char           buffer[0x0100];

    if(cfg && cfg->mgr && cfg->mgr->event)
       cfg->mgr->event(&icqskin_dialogCalls, hwnd, cfg->icq, uin, (char) id, event, buffer);

 }


 static int control(HWND hwnd, USHORT id, USHORT code, ULONG parm)
 {
    /* Processa um controle */
    HWND           h               = WinWindowFromID(hwnd,id);
    ULONG          hdl             = 0;
    USHORT         pos             = 0xFFFF;
    ULONG          uin             = (ULONG) WinSendMsg(WinQueryWindow(hwnd,QW_PARENT),WMICQ_GETUSERID,0,0);
    ICQDIALOG      *cfg            = WinQueryWindowPtr(hwnd,QWL_USER);
    char           class[20];
    char           buffer[0x0100];

    if(!cfg)
       return 0;

    if(!cfg->mgr)
       return 0;

    if(!h)
       return 0;

    WinQueryClassName(h,19,class);

    if( !(strcmp(class,"#2") && strcmp(class,"#7")) && code == LN_SELECT)
    {
       /* DropDown/ListBox Select */
       pos = SHORT1FROMMR(WinSendMsg(h,LM_QUERYSELECTION,MPFROMSHORT(LIT_FIRST),0));

       if(pos != LIT_NONE)
          hdl = LONGFROMMR(WinSendMsg(h,LM_QUERYITEMHANDLE,MPFROMSHORT(pos),0));

       DBGTracex(hdl);

       if(cfg->mgr->selected)
          cfg->mgr->selected(&icqskin_dialogCalls,hwnd,cfg->icq,0,id,hdl,buffer);
    }
    else if( !strcmp(class,"#6") && code == EN_CHANGE )
    {
       if(cfg->mgr->changed)
          cfg->mgr->changed(&icqskin_dialogCalls,hwnd,cfg->icq,uin,id,(USHORT) WinQueryWindowTextLength(h),buffer);
    }

    return 0;
 }


 static int command(HWND hwnd, USHORT id, USHORT cmd, USHORT parm)
 {
    ICQDIALOG      *cfg            = WinQueryWindowPtr(hwnd,QWL_USER);
    ULONG          uin             = (ULONG) WinSendMsg(WinQueryWindow(hwnd,QW_PARENT),WMICQ_GETUSERID,0,0);
    char           buffer[0x0100];

    if(!cfg || cmd != CMDSRC_PUSHBUTTON)
       return -1;

    if(cfg->mgr->click)
       cfg->mgr->click(&icqskin_dialogCalls,hwnd,cfg->icq,uin,id,buffer);

    return 0;
 }

 MRESULT EXPENTRY icqListBoxWindow(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    PWNDPARAMS p;
    char       buffer[0x0100];
    short      pos;

    if(msg == WM_QUERYWINDOWPARAMS)
    {
       p = (PWNDPARAMS) mp1;

       if(!(p->fsStatus & WPM_TEXT))
          return listBoxClassProcessor(hwnd,msg,mp1,mp2);

       pos = SHORT1FROMMR( WinSendMsg(hwnd, LM_QUERYSELECTION, MPFROMSHORT(LIT_CURSOR), 0) );

       if(pos == LIT_NONE)
       {
          *p->pszText = 0;
       }
       else
       {
          if(p->fsStatus & WPM_CCHTEXT)
          {
             WinSendMsg(hwnd,LM_QUERYITEMTEXT,MPFROM2SHORT(pos,p->cchText),(MPARAM) p->pszText);
             *(p->pszText+p->cchText) = 0;
          }
          else
          {
             WinSendMsg(hwnd,LM_QUERYITEMTEXT,MPFROM2SHORT(pos,0xFF),(MPARAM) p->pszText);
          }
       }

       return (MRESULT) TRUE;

    }

    return listBoxClassProcessor(hwnd,msg,mp1,mp2);
 }


