/*
 * os2config.c - OS2 Configuration window Processing
 */

 #pragma strings(readonly)

 #define INCL_WIN
 #define INCL_GPI

 #include <string.h>
 #include <stdlib.h>
 #include <ctype.h>
 #include <malloc.h>
 #include <pwicqgui.h>

/*---[ Defines ]------------------------------------------------------------------------------------------*/

//  #define CONTAINER_IS_NORMAL_CHILD /* Don't know why this is not working */

 #define CHILD_WINDOW(h,i) WinWindowFromID(h,DLG_ELEMENT_ID+i)

 #define MSGID_NAMEBOX    MSG_WINDOW_COMPONENTS

/*---[ Structures ]---------------------------------------------------------------------------------------*/

 #pragma pack(1)

 typedef struct _record
 {
    RECORDCORE  r;
    HWND        hwnd;
    USHORT      id;
    char		text[1];
 } RECORD;

 #pragma pack()

/*---[ Constants ]----------------------------------------------------------------------------------------*/

 const char *icqConfigWindowClass = "pwICQConfig";

 static const MSGCHILD childlist[] =
 {
    { "InfoBox",       MSGID_INFOBOX,     WC_STATIC,    NULL,        "8.Helv",      SS_GROUPBOX|DT_LEFT|DT_VCENTER                                          },
    { "Namebox",       MSGID_NAMEBOX,     WC_STATIC,    "",          "8.Helv",      SS_GROUPBOX|DT_LEFT|DT_VCENTER                                          },

    { "Static",        MSGID_NICK,        ICQ_STATIC,   "Nick:",     "8.Helv",      SS_TEXT|DT_LEFT|DT_VCENTER                                              },
    { "Field",         MSGID_NICKENTRY,   ICQ_STATIC,   "",          "8.Helv.Bold", SS_TEXT|DT_LEFT|DT_VCENTER                                              },
    { "Static",        MSGID_NAME,        ICQ_STATIC,   "Name:",     "8.Helv",      SS_TEXT|DT_LEFT|DT_VCENTER                                              },
    { "Field",         MSGID_NAMEENTRY,   ICQ_STATIC,   "",          "8.Helv.Bold", SS_TEXT|DT_LEFT|DT_VCENTER                                              },

    { "Title",         MSGID_TITLE,       ICQ_STATIC,   "About...",  "8.Helv.Bold", SS_TEXT|DT_LEFT|DT_VCENTER|SS_FGNDFRAME                                 },

    { "CloseButton",   MSGID_CLOSEBUTTON, ICQ_BUTTON,   "~Close",    "8.Helv",      0                                                                       },
    { "SaveButton",    MSGID_SAVE,        ICQ_BUTTON,   "~Save",     "8.Helv",      0                                                                       },
    { "ReloadButton",  MSGID_RELOAD,      ICQ_BUTTON,   "Re~load",   "8.Helv",      0                                                                       },
    { "RefreshButton", MSGID_ADDBUTTON,   ICQ_BUTTON,   "~Refresh",  "8.Helv",      0                                                                       },

    { "MsgButton",     MSGID_MESSAGE,     ICQ_BUTTON,   "",          "8.Helv",      BS_NOBORDER                                                             },
    { "AboutButton",   MSGID_ABOUTBUTTON, ICQ_BUTTON,   "",          "8.Helv",      BS_NOBORDER                                                             },

#ifdef CONTAINER_IS_NORMAL_CHILD
    { "Tree",          MSGID_OPTIONS,     WC_CONTAINER, "",          "8.Helv",      WS_VISIBLE|WS_TABSTOP|CCS_READONLY                                      },
#endif

    { "Static",        MSGID_NICK,        ICQ_STATIC,   "Nick:",     "8.Helv",      SS_TEXT|DT_LEFT|DT_VCENTER                                              },

    { "UserMode",      MSGID_USERMODE,    ICQ_STATIC,   "",          "",            SS_ICON                                                                 }

 };

 static const USHORT userBox[] =   {	MSGID_NICK,
   										MSGID_NICKENTRY,
   										MSGID_NAME,
   										MSGID_NAMEENTRY,
   										MSGID_USERMODE
   							       };

 static const USHORT buttonBar[] = {    MSGID_CLOSEBUTTON,		/* Close button                 */
                                        MSGID_SENDBUTTON,		/* Save configuration           */
                                        MSGID_SENDTO,           /* Reload configuration         */
                                        MSGID_ADDBUTTON         /* Refresh information from srv */
   							        };


/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static void    resize(HWND, short, short);
 static USHORT  resizeUserInfo(HWND, short, short);
 static void    configureContainer(HWND);
 static MRESULT containerAction(HWND, USHORT, PNOTIFYRECORDEMPHASIS);
 static void    loadSkin(HWND, const char *, SKINFILESECTION *);
 static void    destroy(HWND);
 static void    drawContents(HWND, HPS);
 static void    setWindowType(HWND,USHORT);
 static void    autosize(HWND);
 static void    configure(HWND);
 static void    treeIcons(HWND, const char *);
 static int     drawTreeIcon(ICQCONFIGDIALOG *, HPS, PRECTL, USHORT);
 static BOOL    drawElement(HWND, USHORT, POWNERITEM);
 static void    cfigTree(HWND, ICQCONFIGDIALOG *);
 static void    broadcast(HWND, ULONG, MPARAM, MPARAM);

 static void _System thdInitialize(ICQTHREAD *);
 static void _System thdLoad(ICQTHREAD *);

#ifndef CONTAINER_IS_NORMAL_CHILD
 static void   SkinContainer(HWND, const char *, SKINFILESECTION *);
#endif

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 hWindow icqskin_createConfigWindow(HICQ icq, ULONG uin, int userBCount, const ICQUSERBUTTON *userButtons)
 {
    ULONG               ulFrameFlags = FCF_TITLEBAR|FCF_SIZEBORDER|FCF_MINMAX|FCF_TASKLIST|FCF_SYSMENU|FCF_NOMOVEWITHOWNER|FCF_ACCELTABLE;
    HWND                frame;
    HWND                hwnd;
    HWND                h;
    int                 f;
    ICQCONFIGDIALOG     *cfg;

    frame = WinCreateStdWindow(   HWND_DESKTOP,
    						      0,
  							      &ulFrameFlags,
    							  (PSZ) icqConfigWindowClass,
    							  (PSZ) "pwICQ",
    							  WS_VISIBLE,
    							  (HMODULE) module,
    							  1001,
    							  &hwnd
    						  );
    							
    if(!hwnd)
       return hwnd;

    cfg = WinQueryWindowPtr(hwnd,0);
    WinSetWindowPtr(frame,QWL_USER,cfg);

    icqskin_setICQHandle(hwnd,icq);

    icqskin_setWindowUserID(hwnd,uin);
    icqskin_createChilds(hwnd);

    for(f=0;f<userBCount;f++)
    {
       h = CHILD_WINDOW(hwnd,userButtons->id);
       if(h)
          icqskin_setButtonIcon(h,userButtons->icon);
       userButtons++;
    }

    return hwnd;
 }

 MRESULT EXPENTRY icqConfigWindow(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    switch(msg)
    {
    case WM_CONTROL:
       if(SHORT1FROMMP(mp1) == (DLG_ELEMENT_ID+MSGID_OPTIONS))
          return containerAction(hwnd,SHORT2FROMMP(mp1), (PNOTIFYRECORDEMPHASIS) mp2);
       return icqFrameWindow(hwnd, msg, mp1, mp2);

    case WM_CREATE:
       icqskin_cfgWindow(hwnd,ICQCONFIGDIALOG,0);
       break;

    case WM_DESTROY:
       destroy(hwnd);
       return icqFrameWindow(hwnd, msg, mp1, mp2);

    case WMICQ_SETUSERID:
       ((ICQCONFIGDIALOG *) WinQueryWindowPtr(hwnd,0))->uin = (ULONG) mp1;
       break;

    case WMICQ_CREATECHILDS:
       icqskin_setMsgWindowChilds(hwnd,childlist);
       break;

    case WMICQ_SETMSGCHILD:
       icqFrameWindow(hwnd, msg, mp1, mp2);
       configureContainer(hwnd);
       break;

    case WMICQ_LOADSKIN:
       icqFrameWindow(hwnd, msg, mp1, mp2);
       WinSendMsg(hwnd,WMICQ_SKINCHILDS,mp1,mp2);
       break;

    case WMICQ_SETTYPE:
       setWindowType(hwnd,(USHORT) mp1);
       break;

    case WMICQ_SETTREEIMAGE:
       treeIcons(hwnd,(const char *) mp1);
       break;

    case WMICQ_CONFIGURE:
       icqFrameWindow(hwnd, msg, mp1, mp2);
       configure(hwnd);
       break;

    case WMICQ_DRAWCONTENTS:
       drawContents(hwnd,(HPS) mp1);
       break;

    case WMICQ_SKINCHILDS:
       icqFrameWindow(hwnd, msg, mp1, mp2);
#ifndef CONTAINER_IS_NORMAL_CHILD
       SkinContainer(hwnd,(const char *) mp1, (SKINFILESECTION *) mp2);
#endif
       loadSkin(hwnd,(const char *) mp1, (SKINFILESECTION *) mp2);
       break;

    case WM_SIZE:
       resize(hwnd,SHORT1FROMMP(mp2),SHORT2FROMMP(mp2));
       return icqFrameWindow(hwnd, msg, mp1, mp2);

    case WM_DRAWITEM:
       return (MRESULT) drawElement(hwnd,SHORT1FROMMP(mp1),(POWNERITEM) mp2);
       break;

    case WMICQ_SIZEMSGBOX:
       return (MRESULT) resizeUserInfo(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1));

    case WMICQ_AUTOSIZE:
       autosize(hwnd);
       break;

    case WMICQ_INITIALIZE:
       icqCreateThread(	((ICQCONFIGDIALOG *) WinQueryWindowPtr(hwnd,0))->fr.icq,
       					(void (* _System)(ICQTHREAD *)) &thdInitialize,
       					32768, 0, (void *) hwnd, "CWI" );
       break;

    case WMICQ_LOAD:
       icqskin_setEnabled(WinWindowFromID(hwnd,DLG_ELEMENT_ID+MSGID_RELOAD),FALSE);
       icqCreateThread(	((ICQCONFIGDIALOG *) WinQueryWindowPtr(hwnd,0))->fr.icq,
       					(void (* _System)(ICQTHREAD *)) &thdLoad,
       					32768, 0, (void *) hwnd, "CWL" );
       break;

    case WMICQ_SAVE:

       CHKPoint();

       icqskin_setEnabled(WinWindowFromID(hwnd,DLG_ELEMENT_ID+MSGID_RELOAD),FALSE);
       icqskin_setEnabled(WinWindowFromID(hwnd,DLG_ELEMENT_ID+MSGID_SAVE),FALSE);

       broadcast(hwnd, WMICQ_SAVE, (MPARAM) ((ICQCONFIGDIALOG *) WinQueryWindowPtr(hwnd, 0))->uin, 0);

       icqskin_setEnabled(WinWindowFromID(hwnd,DLG_ELEMENT_ID+MSGID_RELOAD),TRUE);
       icqskin_setEnabled(WinWindowFromID(hwnd,DLG_ELEMENT_ID+MSGID_SAVE),TRUE);

       break;

    case WMICQ_DLGEVENT:
       broadcast(hwnd, msg, mp1, mp2);
       break;

    case WMICQ_GETUSERID:
       return (MRESULT) ((ICQCONFIGDIALOG *) WinQueryWindowPtr(hwnd,0))->uin;

    default:
       return icqFrameWindow(hwnd, msg, mp1, mp2);
    }

    return 0;
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
    char   buffer[0x0100];
    POINTL aptl[TXTBOX_COUNT];

    HPS    hps         =  WinGetPS(hwnd);

    WinQueryWindowText(hwnd,0xFF,buffer);
    *(buffer+0xFF) = 0;

    hps = WinGetPS(hwnd);

    GpiQueryTextBox(      hps,
                          strlen(buffer),
                          (char *) buffer,
                          TXTBOX_COUNT,
                          aptl);
    WinReleasePS(hps);

    return (aptl[TXTBOX_TOPRIGHT].y - aptl[TXTBOX_BOTTOMRIGHT].y)+2;
 }

 static USHORT resizeUserInfo(HWND hwnd, short cx, short cy)
 {
    HWND    h;
    int     f;
    int     qtd         = 0;
    int     ySize;
    int		iconSize;
    USHORT	top;
    int     pos;
    USHORT	y;

    iconSize =
    ySize    = (int) icqskin_querySizes(CHILD_WINDOW(hwnd,MSGID_ABOUTBUTTON),0,0);

    for(f=0;f < (sizeof(userBox)/sizeof(USHORT)); f++)
    {
       h = CHILD_WINDOW(hwnd,userBox[f]);

       if( h && WinIsWindowVisible(h))
       {
          qtd++;
          pos = getControlSize(h);
          if(pos > ySize)
             ySize = pos;
       }

    }

    if(!qtd)
       return 0;

    top = y = (cy-(ySize+8));
    WinSetWindowPos(CHILD_WINDOW(hwnd,MSGID_NAMEBOX), 0, 0, y, cx, ySize+12, SWP_SIZE|SWP_MOVE);

    y   += 3;

    WinSetWindowPos(CHILD_WINDOW(hwnd,MSGID_USERMODE), 0, 3, y, iconSize, iconSize, SWP_SIZE|SWP_MOVE);

    pos  = setControlPos(CHILD_WINDOW(hwnd,MSGID_NICK),6+ySize, y, ySize);
    WinSetWindowPos(CHILD_WINDOW(hwnd,MSGID_NICKENTRY), 0, pos+4, y, (cx/2)-(pos+8), ySize, SWP_SIZE|SWP_MOVE);

    pos  = setControlPos(CHILD_WINDOW(hwnd,MSGID_NAME),cx/2,y,ySize);

    f    = cx-((iconSize*2)+6);
    WinSetWindowPos(CHILD_WINDOW(hwnd,MSGID_NAMEENTRY), 0, pos+4, y, f-(pos+6), ySize, SWP_SIZE|SWP_MOVE);

    f   += 1;
    WinSetWindowPos(CHILD_WINDOW(hwnd,MSGID_ABOUTBUTTON), 0, f, y, iconSize, iconSize, SWP_SIZE|SWP_MOVE);

    f   += iconSize+1;
    WinSetWindowPos(CHILD_WINDOW(hwnd,MSGID_INFOBUTTON), 0, f, y, iconSize, iconSize, SWP_SIZE|SWP_MOVE);

    return top;
 }


 static RECORD * insertRecord(HWND h, RECORD *base, USHORT id, HWND dialog, const char *text)
 {
    ICQFRAME            *cfg            = WinQueryWindowPtr(h,QWL_USER);
    int                 sz              = strlen(text)+sizeof(RECORD);
    RECORD				*rec;
    RECORDINSERT        recordInsert;
    char                *ptr;

    rec  = PVOIDFROMMR(WinSendMsg(h,CM_ALLOCRECORD,MPFROMLONG(sz+1),MPFROMSHORT(1)));
    DBGTracex(rec);

    if(!rec)
    {
       icqWriteSysLog(cfg->icq,PROJECT,"Failure adding new record in container");
       return NULL;
    }

    memset(rec,0,sz);
    rec->r.cb = sz;
    rec->hwnd = dialog;
    rec->id   = id;

    strcpy(rec->text,text);

    rec->r.pszIcon  =
    rec->r.pszName  =
    rec->r.pszText  = rec->text;

    DBGMessage(rec->text);

    for(ptr = rec->text;*ptr && *ptr != '\n';ptr++);
    *ptr = 0;

    DBGMessage(rec->text);

    memset(&recordInsert,0,sizeof(RECORDINSERT));

    recordInsert.cb                  = sizeof(RECORDINSERT);
    recordInsert.pRecordParent       = (PRECORDCORE) base;
    recordInsert.pRecordOrder        = (PRECORDCORE) CMA_END;
    recordInsert.zOrder              = CMA_TOP;
    recordInsert.cRecordsInsert      = 1;
    recordInsert.fInvalidateRecord   = TRUE;

    if(!WinSendMsg(h, CM_INSERTRECORD, (PRECORDCORE) rec, &recordInsert))
    {
       DBGMessage("CM_INSERTRECORD has failed");
       icqskin_logErrorInfo(cfg->icq,h,TRUE,"CM_INSERTRECORD has failed");
       return NULL;
    }

    return rec;

 }



 static void configureContainer(HWND hwnd)
 {
#ifdef DEBUG
    RECORD           *rec;
#endif
#ifdef STANDARD_GUI
    ULONG            *pal;
    HPS              hps;
#endif
    HWND             h;
    CNRINFO          cnrinfo;
    ULONG            clr;
    ICQCONFIGDIALOG  *cfg = WinQueryWindowPtr(hwnd,0);

#ifdef CONTAINER_IS_NORMAL_CHILD
    h = CHILD_WINDOW(hwnd,MSGID_OPTIONS);
#else
    h = WinCreateWindow(        hwnd,
                                WC_CONTAINER,
                                "",
                                WS_VISIBLE|WS_TABSTOP|CCS_READONLY, // |CCS_AUTOPOSITION
                                2,2,
                                119,428,
                                hwnd,
                                HWND_TOP,
                                DLG_ELEMENT_ID+MSGID_OPTIONS,
                                NULL,
                                NULL );
#endif

    WinSetWindowPtr(h,QWL_USER,cfg);

    clr = 0x00002000;
    WinSetPresParam(h,PP_BACKGROUNDCOLOR,sizeof(clr),&clr);

    clr = 0x0000FF00;
    WinSetPresParam(h,PP_FOREGROUNDCOLOR,sizeof(clr),&clr);

    icqskin_setFont(h, "8.Helv");

    memset(&cnrinfo,0,sizeof(cnrinfo));

    cnrinfo.cb                = sizeof( CNRINFO );
    cnrinfo.flWindowAttr      = CV_TREE|CV_TEXT|CA_TREELINE|CA_OWNERDRAW;
    cnrinfo.cxTreeIndent      = 1;
    cnrinfo.cxTreeLine        = -1;
    cnrinfo.pszCnrTitle       = "";
    cnrinfo.cyLineSpacing     = 1;

    WinSendMsg(h, CM_SETCNRINFO,
    				 MPFROMP( &cnrinfo ),
    				 MPFROMLONG(CMA_FLWINDOWATTR|CMA_CXTREEINDENT) // |CMA_CNRTITLE)
    		  );


#ifdef STANDARD_GUI

    h = CHILD_WINDOW(hwnd,MSGID_TITLE);

    pal = (ULONG *) WinSendMsg(h,WMICQ_QUERYPALLETE,0,0);

    if(pal)
    {
       pal[ICQCLR_BACKGROUND] = 0x00000040;
       pal[ICQCLR_FOREGROUND] = 0x00FFFFFF;
       pal[ICQCLR_SELECTEDBG] = 0x00000000;
    }

    icqstatic_setMargin(h,4);

    h = CHILD_WINDOW(hwnd,MSGID_OPTIONS);
    icqskin_loadImageFromResource(cfg->fr.icq, 1002, 22, 11, &cfg->treeImg, &cfg->treeMsk);
    cfigTree(hwnd,cfg);

#endif

 }

 static void autosize(HWND hwnd)
 {
    SWP  swp;

    WinQueryWindowPos(hwnd,&swp);
    resize(hwnd,swp.cx,swp.cy);
    WinPostMsg(hwnd,WMICQ_INVALIDATE,0,0);
 }

 static void resize(HWND hwnd, short cx, short cy)
 {
    ICQCONFIGDIALOG  *cfg    = WinQueryWindowPtr(hwnd,0);
    USHORT           yTop    = icqskin_resizeUserInfoBox(hwnd,cx,cy)+2;
    USHORT           yBottom = icqskin_resizeButtonBox(hwnd,cx,buttonBar)+2;
    USHORT           xLeft   = 4;
    USHORT           ySize   = 0;
    HWND             h;

    if(yTop < yBottom)
       yTop = cy;

    WinSetWindowPos(CHILD_WINDOW(hwnd,MSGID_INFOBOX), 0, 0, yBottom, cx, yTop-yBottom, SWP_SIZE|SWP_MOVE);
    cx      -=  4;
    yTop    -= 10;
    yBottom +=  4;

    h = CHILD_WINDOW(hwnd,MSGID_OPTIONS);

    if(h &&  WinIsWindowVisible(h))
    {
       WinSetWindowPos(h, 0, xLeft, yBottom, 119, yTop-yBottom, SWP_SIZE|SWP_MOVE);
       xLeft += 121;
    }

    cfg->yTop = (yTop-yBottom);

    h = CHILD_WINDOW(hwnd,MSGID_TITLE);

    icqskin_querySizes(h,0,&ySize);

    yTop -= ySize;
    WinSetWindowPos(h, 0, xLeft, yTop, cx-xLeft, ySize, SWP_SIZE|SWP_MOVE|SWP_SHOW);
    yTop -= 2;

    cfg->dlg.x  = xLeft;
    cfg->dlg.y  = yBottom;
    cfg->dlg.cx = cx-xLeft;
    cfg->dlg.cy = yTop-yBottom;

    if(cfg && cfg->dialog)
    {
       WinShowWindow(CHILD_WINDOW(hwnd,MSGID_TITLE),TRUE);
       WinSetWindowPos(cfg->dialog, 0, cfg->dlg.x, cfg->dlg.y, cfg->dlg.cx, cfg->dlg.cy, SWP_SIZE|SWP_MOVE|SWP_SHOW);
    }
    else
    {
       WinShowWindow(CHILD_WINDOW(hwnd,MSGID_TITLE),FALSE);
    }

 }

#ifndef CONTAINER_IS_NORMAL_CHILD
 static void SkinContainer(HWND hwnd, const char *name, SKINFILESECTION *fl)
 {
    const char *parms[] = { name, "Tree", "8.Helv", (const char *) fl };
    WinSendMsg(hwnd,WMICQ_SKINELEMENT,(MPARAM) CHILD_WINDOW(hwnd,MSGID_OPTIONS),(MPARAM) parms);
 }
#endif

 static void destroy(HWND hwnd)
 {
    ICQCONFIGDIALOG *cfg = WinQueryWindowPtr(hwnd,0);
    icqskin_deleteImage(cfg->noDialog);
    icqskin_deleteImage(cfg->treeImg);
    icqskin_deleteImage(cfg->treeMsk);
    icqskin_destroyCfgWindow(hwnd);
 }

 static void loadSkin(HWND hwnd, const char *name, SKINFILESECTION *fl)
 {
    char             buffer[80];
    ICQCONFIGDIALOG *cfg = WinQueryWindowPtr(hwnd,0);
    const    char   *ptr;

    if(icqskin_getSkinFileSection(fl, cfg->fr.name))
       name = cfg->fr.name;

    DBGMessage(name);

    /* Load base image */
    icqskin_deleteImage(cfg->noDialog);

    sprintf(buffer,"%s.BgImage",cfg->fr.name);
    DBGMessage(buffer);
    ptr = icqskin_getSkinFileEntry(fl, name, buffer);

    DBGTracex(ptr);

    if(!ptr)
       ptr = icqskin_getSkinFileEntry(fl, name, "BgImage");

    DBGTracex(ptr);

    if(ptr)
    {
       DBGMessage(ptr);
       cfg->noDialog = icqskin_loadBitmap(cfg->fr.icq, ptr);
       DBGTracex(cfg->noDialog);
    }

    icqskin_setTreeIcons(hwnd,icqskin_getSkinFileEntry(fl, name, "Tree.Icons"));

 }

 RECORD * getRecord(HWND hwnd, USHORT id)
 {
    RECORD *node = WinSendMsg(hwnd,CM_QUERYRECORD,0,MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
    while(node && node->id != id)
       node = WinSendMsg(hwnd,CM_QUERYRECORD,MPFROMP(node),MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
    return node;
 }

 int icqskin_queryConfigSection(hWindow hwnd, USHORT id)
 {
    return getRecord(CHILD_WINDOW(hwnd,MSGID_OPTIONS),id) != NULL;
 }

 int _System icqskin_insertConfigPage(hWindow hwnd, USHORT parent, hWindow dialog, USHORT id, const char *text)
 {
    RECORD *rec;

    hwnd = CHILD_WINDOW(hwnd,MSGID_OPTIONS);

    if(dialog)
       WinEnableWindow(dialog,FALSE);

    rec = getRecord(hwnd,parent);

    DBGTracex(rec);

    if(parent && !rec)
       rec = insertRecord(hwnd,NULL,parent,0,icqskin_configsections[parent]);

    DBGTracex(rec);

    insertRecord(hwnd,rec,id,dialog,text);

    return 0;

 }

 void icqskin_selectConfigPage(hWindow hwnd, hWindow dialog)
 {
    ICQCONFIGDIALOG  *cfg    = WinQueryWindowPtr(hwnd,0);
    SWP				 swp;
    WinQueryWindowPos(hwnd, &swp);

    if(cfg->dialog == dialog)
       return;

    if(cfg->dialog)
       WinShowWindow(cfg->dialog,FALSE);

    cfg->dialog = dialog;

    DBGTracex(cfg->dialog);

    WinPostMsg(hwnd,WMICQ_AUTOSIZE,0,0);

 }

 static MRESULT containerAction(HWND hwnd, USHORT action, PNOTIFYRECORDEMPHASIS enf)
 {
    RECORD *rec;
    char   *ptr;

    if(action != CN_EMPHASIS)
       return 0;

    if(enf->fEmphasisMask & CRA_SELECTED)
    {
       rec = (RECORD *) enf->pRecord;
       if(rec)
       {
          for(ptr = rec->text; *ptr && *ptr != '\n';ptr++);
          ptr++;
          icqstatic_setText(CHILD_WINDOW(hwnd,MSGID_TITLE),ptr);
          icqskin_selectConfigPage(hwnd, rec->hwnd);
       }
    }

    return 0;
 }

 static void drawContents(HWND hwnd, HPS hps)
 {
    POINTL           p;
    ICQCONFIGDIALOG  *cfg = WinQueryWindowPtr(hwnd,0);
    BITMAPINFOHEADER bmp;
    RECTL            rcl;
    USHORT           aj;

    if(cfg->dialog)
       return;

     p.x = cfg->dlg.x;
     p.y = cfg->dlg.y;

     if(cfg->noDialog != NO_IMAGE)
     {
        rcl.xLeft   =
        rcl.yBottom = 0;

        GpiQueryBitmapParameters(cfg->noDialog, &bmp);

        rcl.xRight  = bmp.cx;
        rcl.yTop    = bmp.cy;


        if(bmp.cx > cfg->dlg.cx)
        {
           aj           = (bmp.cx - cfg->dlg.cx)/2;
           rcl.xLeft   += aj;
           rcl.xRight  -= aj;
        }
        else
        {
           p.x        += ( (cfg->dlg.cx/2) - (bmp.cx/2) );
        }

        if(bmp.cy > cfg->yTop)
        {
           aj           = (bmp.cy - cfg->yTop)/2;
           rcl.yBottom += aj;
           rcl.yTop    -= aj;
        }
        else
        {
           p.y        += ( (cfg->yTop/2) - (bmp.cy/2) );
        }

        WinDrawBitmap(hps, cfg->noDialog, &rcl, &p, CLR_WHITE, CLR_BLACK, DBM_NORMAL);

     }

 }

 static void setWindowType(HWND hwnd, USHORT type)
 {
    ICQCONFIGDIALOG  *cfg = WinQueryWindowPtr(hwnd,0);

    HPS              hps;

    if(cfg->noDialog == NO_IMAGE)
    {
       hps = WinGetPS(hwnd);
       cfg->noDialog = GpiLoadBitmap(hps, module, 200+type, 448, 369);
       if(cfg->noDialog == GPI_ERROR)
          cfg->noDialog = NO_IMAGE;
       WinReleasePS(hps);
    }			

    WinSendMsg(hwnd,WMICQ_AUTOSIZE,0,0);

 }

 static void configure(HWND hwnd)
 {
    ICQCONFIGDIALOG *cfg = WinQueryWindowPtr(hwnd,0);

    cfg->treeImg  =
    cfg->treeMsk  =
    cfg->noDialog = NO_IMAGE;

 }

 static void cfigTree(HWND hwnd, ICQCONFIGDIALOG *cfg)
 {
    BITMAPINFOHEADER   bmpData;
    CNRINFO            cnrinfo;

    if(cfg->treeImg != NO_IMAGE)
    {
       GpiQueryBitmapParameters(cfg->treeImg, &bmpData);
       cfg->treeIconSize = bmpData.cy;

       DBGTrace(cfg->treeIconSize);

       memset(&cnrinfo,0,sizeof(CNRINFO));
       cnrinfo.cb                    = sizeof(CNRINFO);

       cnrinfo.slTreeBitmapOrIcon.cx =
       cnrinfo.slTreeBitmapOrIcon.cy = cfg->treeIconSize;

       WinSendDlgItemMsg(hwnd, DLG_ELEMENT_ID+MSGID_OPTIONS, CM_SETCNRINFO, MPFROMP(&cnrinfo), MPFROMLONG(CMA_SLTREEBITMAPORICON));

    }
 }

 static void treeIcons(HWND hwnd, const char *filename)
 {
    ICQCONFIGDIALOG    *cfg    = WinQueryWindowPtr(hwnd,0);
    icqskin_loadImage(cfg->fr.icq, filename, &cfg->treeImg, &cfg->treeMsk);
    cfigTree(hwnd,cfg);
 }

 static int drawTreeIcon(ICQCONFIGDIALOG *cfg, HPS hps, PRECTL rcl, USHORT id)
 {
    POINTL             aptl[4];
    USHORT             fator;

    aptl[0].x = rcl->xLeft;
    aptl[0].y = rcl->yBottom;
    aptl[1].x = aptl[0].x+cfg->treeIconSize-1;
    aptl[1].y = aptl[0].y+cfg->treeIconSize-1;

    fator = cfg->treeIconSize * id;

    aptl[2].x = fator;
    aptl[2].y = 0;
    aptl[3].x = fator + cfg->treeIconSize;
    aptl[3].y = cfg->treeIconSize;

    GpiSetColor(hps,CLR_WHITE);
    GpiSetBackColor(hps,CLR_BLACK);

    GpiWCBitBlt( hps, cfg->treeMsk, 4, aptl, ROP_SRCAND,   BBO_IGNORE);
    GpiWCBitBlt( hps, cfg->treeImg, 4, aptl, ROP_SRCPAINT, BBO_IGNORE);

    return 0;
 }

 static BOOL drawElement(HWND hwnd, USHORT id, POWNERITEM itn)
 {
    RECORD           *rec;
    ICQCONFIGDIALOG  *cfg = WinQueryWindowPtr(hwnd,0);
    SKINDATA         *skn = icqskin_getDataBlock(cfg->fr.icq);

    if((id != (DLG_ELEMENT_ID+MSGID_OPTIONS)) || (itn->idItem != CMA_TREEICON))
       return FALSE;

    if((cfg->treeImg == NO_IMAGE) || (id != DLG_ELEMENT_ID+MSGID_OPTIONS) || (itn->idItem != CMA_TREEICON))
       return FALSE;

    rec = (RECORD *) ((CNRDRAWITEMINFO *)(itn->hItem))->pRecord;

//    DBGTrace( (itn->rclItem.xRight - itn->rclItem.xLeft)   - cfg->treeIconSize);
//    DBGTrace( (itn->rclItem.yTop   - itn->rclItem.yBottom) - cfg->treeIconSize);

    drawTreeIcon(cfg, itn->hps, &itn->rclItem, (rec->r.flRecordAttr & CRA_COLLAPSED) ? 0 : 1);

    return TRUE;

 }

 hWindow icqconfig_getNextPage(HWND hwnd, void **node)
 {
 /*
    RECORD *rec = NULL;

    if(node)
       rec = *node;

    DBGTracex(rec);

    hwnd  = CHILD_WINDOW(hwnd,MSGID_OPTIONS);
    DBGTracex(hwnd);

    rec   = WinSendMsg(hwnd,CM_QUERYRECORD,MPFROMP(rec),MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));

    if(node)
       *node = rec;

    if(!rec)
       return 0;

    return rec->hwnd;
*/
    return 0;
 }

 static void broadcast(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    RECORD  *node;
    RECORD  *item;

    hwnd    = CHILD_WINDOW(hwnd,MSGID_OPTIONS);

    for( node = WinSendMsg(hwnd,CM_QUERYRECORD,0,MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
         node;
         node = WinSendMsg(hwnd,CM_QUERYRECORD,MPFROMP(node),MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER)) )
    {
       for( item = WinSendMsg(hwnd,CM_QUERYRECORD,MPFROMP(node),MPFROM2SHORT(CMA_FIRSTCHILD,CMA_ITEMORDER));
            item;
            item = WinSendMsg(hwnd,CM_QUERYRECORD,MPFROMP(item),MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER)) )
       {
          if(item->hwnd)
             WinSendMsg(item->hwnd,msg,mp1,mp2);
       }
    }

 }

 static void thdService(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    RECORD           *node;
    ICQCONFIGDIALOG  *cfg = WinQueryWindowPtr(hwnd,0);

    HWND             h;
    HAB              hab;
    QMSG             qmsg;
    HMQ              hmq;

    hab = WinInitialize(0);

    if(hab)
    {
       hmq = WinCreateMsgQueue(hab,0);
       if(hmq)
       {
          icqskin_setEnabled(WinWindowFromID(hwnd,DLG_ELEMENT_ID+MSGID_RELOAD),FALSE);
          icqskin_setEnabled(WinWindowFromID(hwnd,DLG_ELEMENT_ID+MSGID_SAVE),FALSE);

          broadcast(hwnd,msg,mp1,mp2);

          icqskin_setEnabled(WinWindowFromID(hwnd,DLG_ELEMENT_ID+MSGID_RELOAD),TRUE);
          icqskin_setEnabled(WinWindowFromID(hwnd,DLG_ELEMENT_ID+MSGID_SAVE),TRUE);

          WinDestroyMsgQueue(hmq);
       }
       WinTerminate(hab);
    }
 }


 static void _System thdInitialize(ICQTHREAD *thd)
 {
    DBGMessage("Dialog initialization thread started");
    thdService((HWND) thd->parm, WMICQ_INITIALIZE, (MPARAM) ((ICQCONFIGDIALOG *) WinQueryWindowPtr((HWND) thd->parm,0))->uin, 0);
    icqconfig_load( (HWND) thd->parm );
    DBGMessage("Dialog initialization thread terminated");
 }

 static void _System thdLoad(ICQTHREAD *thd)
 {
    DBGMessage("Dialog load thread started");

    thdService((HWND) thd->parm, WMICQ_LOAD, (MPARAM) ((ICQCONFIGDIALOG *) WinQueryWindowPtr((HWND) thd->parm,0))->uin, 0);

    DBGMessage("Dialog load thread terminated");
 }


