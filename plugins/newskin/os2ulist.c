/*
 * os2ulist.c - OS2 User list message processing
 */

 #pragma strings(readonly)

 #define INCL_WIN
 #define INCL_GPI

 #include <string.h>
 #include <malloc.h>
 #include <stdlib.h>
 #include <pwicqgui.h>

/*---[ Defines ]------------------------------------------------------------------------------------------*/

 #define NORMAL_USERLIST

 #define ID_CONTAINER 1000

/*---[ Structures ]---------------------------------------------------------------------------------------*/

 #pragma pack(1)

 struct group_record
 {
    UCHAR  type;
    USHORT id;
    char   text[1];
 };

 struct user_record
 {
    UCHAR  type;
    HUSER  usr;
    char   text[1];
 };

/*---[ control data structure ]---------------------------------------------------------------------------*/

 #pragma pack(1)

 #define ICQCLR_ONLINE      ICQCLR_COUNTER
 #define ICQCLR_OFFLINE     ICQCLR_COUNTER+1
 #define ICQCLR_ONLINEWAIT  ICQCLR_COUNTER+2
 #define ICQCLR_OFFINEWAIT  ICQCLR_COUNTER+3

 #define ICQCLR_CTLCOLORS   ICQCLR_COUNTER+4

 typedef struct _icquserlist
 {
    USHORT     sz;
    HICQ       icq;

    const char *name;

    /* Window information  */
    HWND       owner;
    HWND       base;
    HUSER      current;
    HWND       hBar;
    HWND       vBar;
    PFNWP      hBarProc;
    PFNWP      oldCntProc;
    ULONG      flags;

    /* Vertical Scroll Bar */
    PFNWP      vBarProc;
    HWND       externVBar;

    /* Window size         */
    short      xSize;
    short      ySize;

    /* Window Positioning  */
    short      x;
    short      y;
    short      cx;
    short      cy;
    short	   xLeft;
    short      xRight;
    USHORT     yDesloc;

    /* Skin system */
    ULONG   pal[ICQCLR_CTLCOLORS];

    short   treeIconSize;
    HBITMAP treeImg;
    HBITMAP treeMsk;
    HBITMAP imgSelected;
    HBITMAP mskSelected;

 } ICQULIST;


// int icqskin_loadImage(HICQ icq, const char *fileName, HBITMAP *img, HBITMAP *msk)

 struct creation_data
 {
    USHORT  cb;
    HICQ    icq;
    HWND    owner;
 };

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static RECORDCORE       *insertGroup(HWND, USHORT, const char *);
 static int              loadusers(HWND);
 static BOOL             drawElement(HWND, USHORT, POWNERITEM);
 static void             create(HWND,struct creation_data *);
 static void             destroy(HWND);
 static void             erase(HWND, HPS, PRECTL);
 static void             setsize(HWND,short,short,short,short);
 static void             autosize(HWND);
 static void             resize(HWND,short,short);
 static void             adduser(HWND,HUSER);
 static void             deluser(HWND,ULONG);
 static void             control(HWND,ICQULIST *, USHORT, PRECORDCORE);
 static void             treeIcons(HWND, const char *);
 static void             keyboardAction(HWND,USHORT);
 static void             setselectbar(HWND, const char *);
 static int              paintBackground(HWND, ICQULIST *, HPS, PRECTL);
 static void             adjustClientSize(HWND);
 static void             loadSkin(HWND,const char *,SKINFILESECTION *);
 static void             refreshUser(HWND, ULONG);
 static PRECORDCORE      findUser(HWND, ULONG);
 static HUSER			 queryUser(HWND,ULONG);
 static void             clearSkin(ICQULIST *);
 static HUSER            updateUser(HWND, ULONG);

 static SHORT EXPENTRY   usrListSort(PRECORDCORE, PRECORDCORE, ICQULIST *);

 static MRESULT EXPENTRY vBarProc(HWND, ULONG, MPARAM, MPARAM);
 static MRESULT EXPENTRY hBarProc(HWND, ULONG, MPARAM, MPARAM);
 static MRESULT EXPENTRY cntProc(HWND, ULONG, MPARAM, MPARAM);

/*---[ Constants ]----------------------------------------------------------------------------------------*/

 const char *icqUserListWindowClass = "pwICQUserlist";

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 hWindow icqSkin_createStandardUserList(HICQ icq, hWindow hwnd, USHORT id)
 {
    HWND 					ret;
    struct creation_data    parm;

    DBGTracex(hwnd);

    memset(&parm,0,sizeof(parm));
    parm.cb    = sizeof(parm);
    parm.icq   = icq;
    parm.owner = hwnd;

    ret = WinCreateWindow(hwnd, (PSZ) icqUserListWindowClass, "", WS_VISIBLE, 0,0,10,10,hwnd, HWND_TOP, id, &parm, NULL);

    DBGTracex(ret);

//    icqskin_setTreeIcons(ret,"treeicons.gif");

    return ret;
 }


 MRESULT EXPENTRY icqUserListWindow(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    switch(msg)
    {
    case WM_CLOSE:
       if( ((ICQULIST *) WinQueryWindowPtr(hwnd,0))->flags & ICQULIST_POPUP )
       {
          WinDestroyWindow(WinQueryWindow(hwnd,QW_PARENT));
          return 0;
       }
       return WinDefWindowProc(hwnd,msg,mp1,mp2);

    case WM_ERASEBACKGROUND:
       erase(hwnd,(HPS) mp1, (PRECTL) mp2);
       break;

    case WM_CREATE:
       create(hwnd,(struct creation_data *) mp1);
       break;

    case WM_DESTROY:
       destroy(hwnd);
       break;

    case WM_SIZE:
       resize(hwnd,SHORT1FROMMP(mp2),SHORT2FROMMP(mp2));
       break;

    case WM_CONTROL:
       if(SHORT1FROMMP(mp1) == ID_CONTAINER)
          control( hwnd, ((ICQULIST *) WinQueryWindowPtr(hwnd,0)), SHORT2FROMMP(mp1), (PRECORDCORE) mp2);
       else
          return WinDefWindowProc(hwnd,msg,mp1,mp2);
       break;

    case WM_PRESPARAMCHANGED:
       CHKPoint();
       icqskin_updatePresParam(hwnd,LONGFROMMP(mp1),((ICQULIST *) WinQueryWindowPtr(hwnd,0) )->pal);
       break;

    case WMICQ_SETSIZE:
       setsize(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1),SHORT1FROMMP(mp2),SHORT2FROMMP(mp2));
       break;

    case WMICQ_AUTOSIZE:
       autosize(hwnd);
       break;

    case WMICQ_INSERTGROUP:
       insertGroup(WinWindowFromID(hwnd,ID_CONTAINER), (USHORT) mp1, (const char *) mp2);
       break;

    case WMICQ_ADDUSER:
       adduser(hwnd,(HUSER) mp1);
       break;

    case WMICQ_DELUSER:
       deluser(hwnd,(ULONG) mp1);
       break;

    case WM_DRAWITEM:
       return (MRESULT) drawElement(hwnd,SHORT1FROMMP(mp1),(POWNERITEM) mp2);
       break;

    case WMICQ_SORTLIST:
       DBGMessage("Sorting user list");
       ((ICQULIST *) WinQueryWindowPtr(hwnd,0))->flags &= ~ICQULIST_SORT;
       WinSendDlgItemMsg(hwnd,ID_CONTAINER,CM_SORTRECORD,(MPARAM) usrListSort, (MPARAM) WinQueryWindowPtr(hwnd,0));
       break;

    case WMICQ_SETICQHANDLE:
       ((ICQULIST *) WinQueryWindowPtr(hwnd,0))->icq = (HICQ) mp1;
       break;

    case WMICQ_SETSELECTBAR:
       setselectbar(hwnd,(const char *) mp1);
       break;

    case WMICQ_SETTREEIMAGE:
       treeIcons(hwnd,(const char *) mp1);
       break;

    case WMICQ_QUERYPALLETE:
       return (MRESULT) ((ICQULIST *) WinQueryWindowPtr(hwnd,0))->pal;

    case WMICQ_SETFLAG:
      ((ICQULIST *) WinQueryWindowPtr(hwnd,0))->flags |= (ULONG) mp1;
      break;

    case WMICQ_GETFLAG:
       return (MRESULT) ((ICQULIST *) WinQueryWindowPtr(hwnd,0))->flags;

    case WMICQ_GETFLAGADDR:
       return (MRESULT) & ((ICQULIST *) WinQueryWindowPtr(hwnd,0))->flags;

    case WMICQ_SWITCHFLAG:
      ((ICQULIST *) WinQueryWindowPtr(hwnd,0))->flags ^= (ULONG) mp1;
      break;

    case WMICQ_LOADSKIN:
       loadSkin(hwnd,(const char *) mp1, (SKINFILESECTION *) mp2);
       break;

    case WMICQ_BLINK:
    case WMICQ_UPDATEICON:
       refreshUser(WinWindowFromID(hwnd,ID_CONTAINER),(ULONG) mp1);
       break;

    case WMICQ_QUERYUSER:
       return (MRESULT) queryUser(WinWindowFromID(hwnd,ID_CONTAINER), (ULONG) mp1);

    case WMICQ_CLEAR:
       clearSkin( (ICQULIST *) WinQueryWindowPtr(hwnd,0));
       break;

    case WMICQ_UPDATEUSER:
       return (MRESULT) updateUser(WinWindowFromID(hwnd,ID_CONTAINER), (ULONG) mp1);

    case WMICQ_CONNECT:
       ((ICQULIST *) WinQueryWindowPtr(hwnd,0))->externVBar = (HWND) mp1;
       break;

    case WM_USER+50:		// Keyboard action on Container (MP1=Code)
       keyboardAction(hwnd,SHORT1FROMMP(mp1));
       break;

    case WM_USER+51:
       adjustClientSize(hwnd);
       break;

    default:
       return WinDefWindowProc(hwnd,msg,mp1,mp2);
    }

    return 0;
 }

 static void create(HWND hwnd,struct creation_data *cd)
 {
    ICQULIST *cfg          = malloc(sizeof(ICQULIST));
    CNRINFO  cnrinfo;
    HWND     h;
    SKINDATA *skn          = NULL;
    int      f;
    ULONG    *pal;
    HENUM    hEnum;
    HWND     hwndNext;
    char     buffer[0x0100];

    WinSetWindowPtr(hwnd,0,cfg);

    if(!cfg)
    {
       icqskin_closeWindow(hwnd);
       return;
    }

    memset(cfg,0,sizeof(ICQULIST));
    cfg->sz              = sizeof(ICQULIST);

    cfg->treeImg         =
    cfg->treeMsk         =
    cfg->imgSelected     =
    cfg->mskSelected     = NO_IMAGE;
    cfg->base            = hwnd;
    cfg->owner           = cd->owner;
    cfg->icq             = cd->icq;

    /* Get pallete information from the owner window */
    pal = (ULONG *) WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),WMICQ_QUERYPALLETE,0,0);

    if(pal && pal != cfg->pal)
    {
       for(f=0;f<ICQCLR_COUNTER;f++)
          cfg->pal[f] = pal[f];
    }
    else
    {
       cfg->pal[ICQCLR_FOREGROUND] = 0x00000000;
       cfg->pal[ICQCLR_BACKGROUND] = 0x00D1D1D1;
    }

    cfg->pal[ICQCLR_SELECTEDBG] = 0x00000080;
    cfg->pal[ICQCLR_SELECTEDFG] = 0x00FFFFFF;
    cfg->pal[ICQCLR_ONLINE]     = 0x00000080;
    cfg->pal[ICQCLR_OFFLINE]    = 0x00800000;
    cfg->pal[ICQCLR_ONLINEWAIT] = 0x00000040;
    cfg->pal[ICQCLR_OFFINEWAIT] = 0x00400000;


    /* Create container window */
    h = WinCreateWindow(hwnd,WC_CONTAINER,"",WS_VISIBLE|WS_TABSTOP|CCS_AUTOPOSITION|CCS_READONLY,0,0,0,0,hwnd,HWND_TOP,ID_CONTAINER,NULL,NULL );
    WinSetWindowPtr(h,QWL_USER,cfg);

    DBGTracex(h);

    memset(&cnrinfo,0,sizeof(cnrinfo));
// CV_ICON
    cnrinfo.cb                = sizeof( CNRINFO );
    cnrinfo.flWindowAttr      = CV_TREE|CA_TREELINE|CA_OWNERDRAW|CA_OWNERPAINTBACKGROUND; // |CMA_SLBITMAPORICON ; //  |CA_CONTAINERTITLE; // |CA_TITLESEPARATOR;
    cnrinfo.cxTreeIndent      = 1;
    cnrinfo.cxTreeLine        = -1;
    cnrinfo.pszCnrTitle       = "";
    cnrinfo.cyLineSpacing     = 1;

    if(cfg->icq)
       skn = icqskin_getDataBlock(cfg->icq);

    if(skn)
    {
       cnrinfo.slBitmapOrIcon.cx =
       cnrinfo.slBitmapOrIcon.cy = skn->iconSize+2;
    }
    else
    {
       cnrinfo.slBitmapOrIcon.cx =
       cnrinfo.slBitmapOrIcon.cy = 18;
    }

    if(!WinSendMsg(h, CM_SETCNRINFO, MPFROMP( &cnrinfo ),MPFROMLONG(CMA_FLWINDOWATTR|CMA_LINESPACING|CMA_SLBITMAPORICON|CMA_CXTREEINDENT)))
    {
       DBGMessage("Error in container configuration");
       if(cd && cd->icq)
       {
          icqWriteSysLog(cd->icq,PROJECT,"Failure in user list container configuration");
       }
       icqskin_closeWindow(hwnd);
    }

    WinSetPresParam(h, PP_BACKGROUNDCOLOR,(ULONG) sizeof(ULONG), cfg->pal+ICQCLR_BACKGROUND);
    WinSetPresParam(h, PP_FOREGROUNDCOLOR,(ULONG) sizeof(ULONG), cfg->pal+ICQCLR_FOREGROUND);

    icqskin_initializeUserList(cfg->icq, hwnd);

    /* Search for the scroll bar */
    hEnum  = WinBeginEnumWindows(h);

    while((hwndNext = WinGetNextWindow(hEnum)) != NULLHANDLE && !(cfg->hBar&cfg->vBar))
    {
       DBGTracex(hwndNext);
       *buffer = 0;
       if(WinQueryClassName(hwndNext, 0xFF, buffer) > 0)
       {
          if( (!strcmp(buffer,"#8")) )
          {
             if(WinQueryWindowULong(hwndNext,QWL_STYLE) & SBS_VERT)
                cfg->vBar = hwndNext;
             else
                cfg->hBar = hwndNext;
          }
       }
    }

    WinEndEnumWindows(hEnum);

    DBGTracex(cfg->vBar);
    DBGTrace(WinQueryWindowUShort(cfg->vBar,QWS_ID));

    DBGTracex(cfg->hBar);
    DBGTrace(WinQueryWindowUShort(cfg->hBar,QWS_ID));

    if(cfg->vBar)
       cfg->vBarProc = WinSubclassWindow(cfg->vBar, vBarProc);

    if(cfg->hBar)
    {
       WinShowWindow(cfg->hBar,FALSE);
       cfg->hBarProc = WinSubclassWindow(cfg->hBar, hBarProc);
    }

    cfg->oldCntProc = WinSubclassWindow(h, cntProc);

 }

 static void destroy(HWND hwnd)
 {
    ICQULIST *cfg = WinQueryWindowPtr(hwnd,0);

    DBGTracex(cfg);

    if(!cfg || cfg->sz != sizeof(ICQULIST))
       return;

    icqskin_terminateUserList(cfg->icq, hwnd);

    clearSkin(cfg);

    cfg->sz = 0;
    free(cfg);
 }

 static void clearSkin(ICQULIST *cfg)
 {
    cfg->flags &= ~ICQULIST_SKINNED;
    icqskin_deleteImage(cfg->treeImg);
    icqskin_deleteImage(cfg->treeMsk);
    icqskin_deleteImage(cfg->imgSelected);
    icqskin_deleteImage(cfg->mskSelected);
 }

 static void paint(HWND hwnd)
 {
    HPS      hps;
    RECTL    rcl;
    ICQULIST *cfg = WinQueryWindowPtr(hwnd,0);

    hps = WinBeginPaint(hwnd,NULLHANDLE,&rcl);
    icqskin_loadPallete(hps, ICQCLR_CTLCOLORS, cfg->pal);
    WinFillRect(hps, &rcl, ICQCLR_BACKGROUND);
    WinEndPaint(hps);
 }

 static void erase(HWND hwnd, HPS hps, PRECTL rcl)
 {
    ICQULIST *cfg = WinQueryWindowPtr(hwnd,0);

    hps = WinBeginPaint(hwnd,NULLHANDLE,rcl);

    icqskin_loadPallete(hps, ICQCLR_CTLCOLORS, cfg->pal);
    WinFillRect(hps, rcl, ICQCLR_BACKGROUND);

    WinEndPaint(hps);
 }

 static void setsize(HWND hwnd, short x, short y, short cx, short cy)
 {
    ICQULIST *cfg = WinQueryWindowPtr(hwnd,0);

    cfg->x     = x;
    cfg->y     = y;
    cfg->cx    = cx;
    cfg->cy    = cy;

    WinPostMsg(hwnd,WMICQ_AUTOSIZE,0,0);
 }

 static void autosize(HWND hwnd)
 {
    ICQULIST *cfg = WinQueryWindowPtr(hwnd,0);
    if(icqskin_autoSize(hwnd,cfg->x,cfg->y,cfg->cx,cfg->cy,0))
       WinPostMsg(WinQueryWindow(hwnd, QW_OWNER),WMICQ_SIZECHANGED,(MPARAM) WinQueryWindowUShort(hwnd,QWS_ID),(MPARAM) hwnd);
 }

 static void resize(HWND hwnd, short cx, short cy)
 {
    ICQULIST *cfg = WinQueryWindowPtr(hwnd,0);

    if(!cfg || cfg->sz != sizeof(ICQULIST))
       return;

    cfg->xSize  = cx;
    cfg->ySize  = cy;

    adjustClientSize(hwnd);

 }

 static void adjustClientSize(HWND hwnd)
 {
    ICQULIST *cfg     = WinQueryWindowPtr(hwnd,0);
    CNRINFO  cnrinfo;
    SKINDATA *skn     = icqskin_getDataBlock(cfg->icq);
    HWND     h        = WinWindowFromID(hwnd,ID_CONTAINER);
    int      xSize    = cfg->xSize;
    RECTL    rcl;

    cfg->xRight = cfg->xLeft + cfg->xSize; // - SV_CXVSCROLL;

    memset(&cnrinfo,0,sizeof(cnrinfo));
    cnrinfo.cb                = sizeof( CNRINFO );

    cnrinfo.slBitmapOrIcon.cx  = (cfg->xRight - cfg->xLeft);
    cnrinfo.slBitmapOrIcon.cx -= SV_CXVSCROLL;
    cnrinfo.slBitmapOrIcon.cx -= 5;

    if(skn)
       cnrinfo.slBitmapOrIcon.cy = skn->iconSize+2;
    else
       cnrinfo.slBitmapOrIcon.cy = 18;

    WinSendMsg(h, CM_SETCNRINFO, MPFROMP( &cnrinfo ),MPFROMLONG(CMA_SLBITMAPORICON));

    WinQueryWindowRect(cfg->hBar,&rcl);
    cfg->yDesloc = (rcl.yTop - rcl.yBottom);

    if(cfg->flags & ICQULIST_SKINNED)
       xSize += SV_CXVSCROLL;

    //                    X  Y              CX     CY
    WinSetWindowPos(h, 0, 0, -cfg->yDesloc, xSize, cfg->ySize+cfg->yDesloc, SWP_SIZE|SWP_MOVE);

    WinShowWindow(cfg->hBar,FALSE);

 }


 static RECORDCORE *insertGroup(HWND h, USHORT id, const char *text)
 {
    RECORDCORE          *core;
    RECORDINSERT        recordInsert;
    ICQULIST            *cfg             = WinQueryWindowPtr(h,QWL_USER);
    struct group_record *p;
    int                 sz               = sizeof(struct group_record)+strlen(text)+1;
    char			    buffer[0x0101];

    core = PVOIDFROMMR(WinSendMsg(h,CM_ALLOCRECORD,MPFROMLONG(sz),MPFROMSHORT(1)));
    DBGTracex(core);

    if(!core)
       return NULL;

    core->cb    = sizeof(RECORDCORE);
    memset(core,0,core->cb);

    p           = (struct group_record *) (core+1);
    p->type     = 0;
    p->id       = id;
    strcpy(p->text,text);

    DBGTrace(p->id);
    DBGMessage(p->text);

    icqLoadString(cfg->icq, "TreeElements", "", buffer, 0xFF);
    DBGMessage(buffer);

    if(strlen(buffer) >= id)
    {
       if(buffer[id] == '1')
          core->flRecordAttr |= CRA_EXPANDED;
       else
          core->flRecordAttr |= CRA_COLLAPSED;
    }

//    core->pszIcon  =
//    core->pszName  =
//    core->pszText  = NULL; // p->text;

    memset(&recordInsert,0,sizeof(RECORDINSERT));

    recordInsert.cb                  = sizeof(RECORDINSERT);
    recordInsert.pRecordOrder        = (PRECORDCORE)CMA_END;
    recordInsert.zOrder              = CMA_TOP;
    recordInsert.cRecordsInsert      = 1;
    recordInsert.fInvalidateRecord   = TRUE;


    WinSendMsg(h,CM_INSERTRECORD,MPFROMP(core),MPFROMP(&recordInsert));

    return core;
 }

 static RECORDCORE *findGroup(HWND container, USHORT group)
 {
    RECORDCORE           *node = WinSendMsg(container,CM_QUERYRECORD,0,MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
    struct group_record  *p;

    while(node)
    {
       p = (struct group_record *) (node+1);
       if(!p->type && p->id == group)
          return node;
       node = WinSendMsg(container,CM_QUERYRECORD,MPFROMP(node),MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
    }
    return NULL;
 }

 static void adduser(HWND hwnd, HUSER usr)
 {
    ICQULIST            *cfg = WinQueryWindowPtr(hwnd,0);
    RECORDCORE          *node;
    RECORDCORE          *core;
    RECORDINSERT        recordInsert;
    char                key[20];
    char                buffer[0x0100];
	char                *ptr;
    struct user_record  *p;
    int                 sz;

    if(cfg->sz != sizeof(ICQULIST))
    {
       DBGMessage("Invalid userlist data block");
       return;
    }

    hwnd = WinWindowFromID(hwnd,ID_CONTAINER);
    node = findGroup(hwnd,usr->c2sGroup);

//    DBGTracex(node);

    if(!node)
    {
//       DBGTracex(usr->c2sGroup);

	   if(usr->c2sGroup == 0x0F00)
	   {
		  ptr = "Not in list";
	   }
	   else
	   {
          sprintf(key,"UserGroup%02d",usr->c2sGroup);
//          DBGMessage(key)
//          DBGTracex(cfg->icq);
          icqLoadString(cfg->icq, key, "Unknown", buffer, 0xFF);
//          DBGMessage(buffer);
          ptr = strstr(buffer, ",");

          DBGTracex(ptr);

	      if(!ptr)
		     ptr = buffer;
	      else
		     ptr++;
		
	   }
//	   DBGMessage(ptr);
       node = insertGroup(hwnd, usr->c2sGroup, ptr);
//       DBGTracex(node);
    }

    if(!node)
    {
       DBGMessage("Can't add user -> No group!!!");
       icqWriteSysLog(cfg->icq,PROJECT,"Failure on UserGroup management");
       return;
    }

    /* Add user in the container */
    sz = sizeof(struct group_record)+strlen(icqQueryUserNick(usr))+1;

    core = PVOIDFROMMR(WinSendMsg(hwnd,CM_ALLOCRECORD,MPFROMLONG(sz),MPFROMSHORT(1)));

    if(!core)
       return;

    core->cb    = sizeof(RECORDCORE);
    memset(core,0,core->cb);

    p           = (struct user_record *) (core+1);
    p->type     = 1;
    p->usr      = usr;
    strcpy(p->text,icqQueryUserNick(usr));

    memset(&recordInsert,0,sizeof(RECORDINSERT));

    recordInsert.cb                  = sizeof(RECORDINSERT);
    recordInsert.pRecordParent       = node;
    recordInsert.pRecordOrder        = (PRECORDCORE)CMA_END;
    recordInsert.zOrder              = CMA_TOP;
    recordInsert.cRecordsInsert      = 1;
    recordInsert.fInvalidateRecord   = TRUE;
    WinSendMsg(hwnd,CM_INSERTRECORD,MPFROMP(core),MPFROMP(&recordInsert));

    return;

 }

 static SHORT EXPENTRY usrListSort(PRECORDCORE r1, PRECORDCORE r2, ICQULIST *cfg)
 {
    struct user_record *user1 = (struct user_record *) (r1+1);
    struct user_record *user2 = (struct user_record *) (r2+1);

    if(!user1->type)
       return ((struct group_record *) user1)->id - ((struct group_record *) user2)->id;

    if(user1->usr->index != user2->usr->index)
       return user1->usr->index - user2->usr->index;

    return stricmp(icqQueryUserNick(user1->usr),icqQueryUserNick(user2->usr));
 }

 static int drawTreeIcon(ICQULIST *cfg, HPS hps, PRECTL rcl, USHORT id)
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
    PRECORDCORE rec;
    ICQULIST    *cfg = WinQueryWindowPtr(hwnd,0);
    SKINDATA    *skn = icqskin_getDataBlock(cfg->icq);
    HUSER       usr;
    ULONG       fg   = ICQCLR_FOREGROUND;
    ULONG       bg   = ICQCLR_BACKGROUND;
    USHORT      xLeft;

/*
typedef struct _OWNERITEM {
  HWND      hwnd;            //  Window handle.
  HPS       hps;             //  Presentation-space handle.
  ULONG     fsState;         //  State.
  ULONG     fsAttribute;     //  Attribute.
  ULONG     fsStateOld;      //  Old state.
  ULONG     fsAttributeOld;  //  Old attribute.
  RECTL     rclItem;         //  Item rectangle.
  LONG      idItem;          //  Item identity.
  ULONG     hItem;           //  Item.
} OWNERITEM;
*/
    if(id != ID_CONTAINER)
       return FALSE;

    xLeft = cfg->xLeft;
    icqskin_loadPallete(itn->hps, ICQCLR_CTLCOLORS, cfg->pal);

    if(itn->rclItem.xRight > cfg->xRight)
       itn->rclItem.xRight = cfg->xRight;

    rec = ((CNRDRAWITEMINFO *)(itn->hItem))->pRecord;

    if(rec->flRecordAttr & CRA_SELECTED)
    {
       bg = ICQCLR_SELECTEDBG;
       fg = ICQCLR_SELECTEDFG;
    }

    switch(*((char *)(rec+1)))
    {
    case 0:		// ******** TREE ********
       switch(itn->idItem)
       {
       case CMA_TREEICON:

          if(cfg->treeImg == NO_IMAGE)
             return FALSE;

          paintBackground(itn->hwnd, cfg, itn->hps, &itn->rclItem);
          drawTreeIcon(cfg, itn->hps, &itn->rclItem, (rec->flRecordAttr & CRA_COLLAPSED) ? 0 : 1);
          break;

       case CMA_ICON:

          xLeft = itn->rclItem.xLeft;

          if(rec->flRecordAttr & CRA_SELECTED)
             icqskin_drawSelected(hwnd,itn->hps,&itn->rclItem,cfg->imgSelected,cfg->mskSelected);
          else
             paintBackground(itn->hwnd, cfg, itn->hps, &itn->rclItem);

          itn->rclItem.xLeft += 2;
          WinDrawText(itn->hps, -1, (PSZ) ((struct group_record *)(rec+1))->text, &itn->rclItem, fg, bg, DT_VCENTER);
          break;
       }
       break;

    case 1:		// ******** RECORD ********
       usr = ( (struct user_record *)(rec+1) )->usr;

       switch(itn->idItem)
       {
       case CMA_ICON:

          xLeft = itn->rclItem.xLeft;

          if(rec->flRecordAttr & CRA_SELECTED)
             icqskin_drawSelected(hwnd,itn->hps,&itn->rclItem,cfg->imgSelected,cfg->mskSelected);
          else
             paintBackground(itn->hwnd, cfg, itn->hps, &itn->rclItem);

          icqskin_drawIcon(skn, itn->hps, usr->modeIcon, itn->rclItem.xLeft+1, itn->rclItem.yBottom+1);

          if((cfg->flags & ICQULIST_BLINK) && (usr->eventIcon != usr->modeIcon) )
             icqskin_drawIcon(skn, itn->hps, usr->eventIcon, itn->rclItem.xLeft+1, itn->rclItem.yBottom+1);

          itn->rclItem.xLeft += (skn->iconSize+4);

          if(usr && ! (rec->flRecordAttr & CRA_SELECTED))
          {
             if(usr->flags & USRF_BLINK)
                fg = (cfg->flags & ICQULIST_BLINK) ? ICQCLR_ONLINE : ICQCLR_OFFLINE;
             else
                fg = (usr->flags & USRF_ONLINE) ? ICQCLR_ONLINE : ICQCLR_OFFLINE;

             if(usr->flags & USRF_WAITING)
                fg += 2;
          }

          WinDrawText(itn->hps, -1, (PSZ) icqQueryUserNick(usr), &itn->rclItem, fg, bg, DT_VCENTER);
          return TRUE;
       }

    }

    if(xLeft != cfg->xLeft)
    {
       DBGMessage("Left margin was changed");
       cfg->xLeft = xLeft;
       WinPostMsg(cfg->base,WM_USER+51,0,0);
    }

    return TRUE;

 }

 static HUSER selectFromMouse(HWND hwnd)
 {
    POINTL              p;
    QUERYRECFROMRECT    r;
    PRECORDCORE         rec;
    struct user_record  *uRec;

    hwnd = WinWindowFromID(hwnd,ID_CONTAINER);

    WinQueryPointerPos(HWND_DESKTOP, &p);
    WinMapWindowPoints(HWND_DESKTOP, hwnd, &p, 1);

    memset(&r,0,sizeof(QUERYRECFROMRECT));
    r.cb = sizeof(QUERYRECFROMRECT);
    r.rect.xLeft   = p.x;
    r.rect.xRight  = p.x+1;

    r.rect.yBottom = p.y;
    r.rect.yTop    = p.y+1;

    r.fsSearch     = CMA_PARTIAL|CMA_ITEMORDER;

    rec = (PRECORDCORE) WinSendMsg(hwnd,CM_QUERYRECORDFROMRECT,(MPARAM) CMA_FIRST, MPFROMP(&r) );

    if(!rec)
       return NULL;

    WinSendMsg(hwnd,CM_SETRECORDEMPHASIS,MPFROMP(rec), MPFROM2SHORT(TRUE,CRA_SELECTED));

    uRec = (struct user_record *) (rec+1);

    if(uRec->type != 1)
       return NULL;

    DBGPrint("Found user %s",icqQueryUserNick(uRec->usr));
    return uRec->usr;

 }

 static void keyboardAction(HWND hwnd, USHORT key)
 {
    ICQULIST *cfg = WinQueryWindowPtr(hwnd,0);
    if(key == ' ' && cfg->current)
       icqskin_userContextMenu(cfg->icq, cfg->current);
 }

 static void setTreeStatus(HICQ icq, UCHAR pos, BOOL flag)
 {
    char buffer[0x0101];
    int  f;

    icqLoadString(icq, "TreeElements", "", buffer, 0xFF);

    f = strlen(buffer);
    while(f<pos)
       buffer[f++] = '0';
    buffer[f]   = 0;
    buffer[pos] = flag ? '1' : '0';

    DBGMessage(buffer);

    icqSaveString(icq,"TreeElements",buffer);

 }

 static void control(HWND hwnd, ICQULIST *cfg, USHORT id, PRECORDCORE r)
 {
    PNOTIFYRECORDEMPHASIS enf;
    RECTL                 rcl;
    struct user_record    *rec;
    QUERYRECORDRECT       query;

    switch(id)
    {
    case CN_EXPANDTREE:
       DBGMessage("CN_EXPANDTREE");
       if( ((struct group_record *) (r+1))->type )
          return;
       DBGTrace( ((struct group_record *) (r+1))->id   );
       setTreeStatus(cfg->icq,((struct group_record *) (r+1))->id, TRUE);
       break;

    case CN_COLLAPSETREE:
       DBGMessage("CN_COLLAPSETREE");
       if( ((struct group_record *) (r+1))->type )
          return;
       DBGTrace( ((struct group_record *) (r+1))->id   );
       setTreeStatus(cfg->icq,((struct group_record *) (r+1))->id, FALSE);
       break;

    case CN_EMPHASIS:
       enf = (PNOTIFYRECORDEMPHASIS) r;
       if(enf->fEmphasisMask & CRA_SELECTED)
       {
          rec = ((struct user_record *) (enf->pRecord+1));
          if(rec->type == 1)
             cfg->current = rec->usr;
          else
             cfg->current = NULL;
       }
       break;

    case CN_ENTER:
       if(cfg->current)
          icqskin_userlistClick(cfg->icq, cfg->current);
       break;

    case CN_SCROLL:
       WinInvalidateRect(hwnd,NULL,TRUE);
       break;

    case CN_CONTEXTMENU:
       icqskin_userContextMenu(cfg->icq, selectFromMouse(hwnd));
       break;

//#ifdef DEBUG
//    default:
//       DBGTrace(id);
//#endif
    }
 }


 static void treeIcons(HWND hwnd, const char *filename)
 {
    ICQULIST           *cfg    = WinQueryWindowPtr(hwnd,0);
    BITMAPINFOHEADER   bmpData;
    CNRINFO            cnrinfo;

    DBGMessage(filename);
    icqskin_loadImage(cfg->icq, filename, &cfg->treeImg, &cfg->treeMsk);

    if(cfg->treeImg != NO_IMAGE)
    {
       GpiQueryBitmapParameters(cfg->treeImg, &bmpData);
       cfg->treeIconSize = bmpData.cy;

       memset(&cnrinfo,0,sizeof(CNRINFO));
       cnrinfo.cb                    = sizeof(CNRINFO);

       cnrinfo.slTreeBitmapOrIcon.cx =
       cnrinfo.slTreeBitmapOrIcon.cy = cfg->treeIconSize;

       WinSendDlgItemMsg(hwnd, ID_CONTAINER, CM_SETCNRINFO, MPFROMP(&cnrinfo), MPFROMLONG(CMA_SLTREEBITMAPORICON));

    }
 }

 static MRESULT EXPENTRY vBarProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    ICQULIST *cfg = WinQueryWindowPtr(WinQueryWindow(hwnd, QW_OWNER),QWL_USER);

    if(!cfg || cfg->sz != sizeof(ICQULIST))
    {
       DBGMessage("Invalid vertical scroll-bar subclassing");
       icqskin_closeWindow(hwnd);
       return WinDefWindowProc(hwnd,msg,mp1,mp2);
    }

    if(cfg->externVBar)
    {
       if( msg == SBM_SETSCROLLBAR || msg == SBM_SETPOS || msg == SBM_SETTHUMBSIZE)
          WinPostMsg(cfg->externVBar,msg,mp1,mp2);
    }

    return cfg->vBarProc(hwnd,msg,mp1,mp2);
 }

 static MRESULT EXPENTRY hBarProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    ICQULIST *cfg = WinQueryWindowPtr(WinQueryWindow(hwnd, QW_OWNER),QWL_USER);

    if(!cfg || cfg->sz != sizeof(ICQULIST))
    {
       DBGMessage("Invalid horizontal scroll-bar subclassing");
       icqskin_closeWindow(hwnd);
       return WinDefWindowProc(hwnd,msg,mp1,mp2);
    }

    return cfg->hBarProc(hwnd,msg,mp1,mp2);
 }

 static int paintBackground(HWND hwnd, ICQULIST *cfg, HPS hps, PRECTL rclBackground)
 {
    RECTL   rcl;
    HBITMAP bgImage  = icqskin_queryBackground(cfg->owner);
    POINTL  p;

    if(bgImage == NO_IMAGE)
    {
       icqskin_loadPallete(hps, ICQCLR_CTLCOLORS, cfg->pal);
       WinFillRect(hps, rclBackground, ICQCLR_BACKGROUND);
    }
    else
    {
       p.x         = rclBackground->xLeft;
       p.y         = rclBackground->yBottom;

       rcl.xLeft   = rclBackground->xLeft;
       rcl.xRight  = rclBackground->xRight;

       rcl.yTop    = rclBackground->yTop;
       rcl.yBottom = rclBackground->yBottom;
       WinMapWindowPoints(hwnd, cfg->owner, (POINTL *) &rcl, 2);

//       rcl.yBottom = (p.y - 16);
//       rcl.yTop    = rcl.yBottom + (rclBackground->yTop-rclBackground->yBottom);

       WinDrawBitmap(hps, bgImage, &rcl, &p, CLR_WHITE, CLR_BLACK, DBM_NORMAL);

    }

    return TRUE;
 }

 static void chkKeyboard(HWND hwnd, USHORT fsflags, USHORT code)
 {
    if(fsflags & KC_CHAR)
       WinPostMsg(hwnd,WM_USER+50,MPFROMSHORT(code),0);
 }


 static MRESULT EXPENTRY cntProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    ICQULIST *cfg = WinQueryWindowPtr(hwnd, QWL_USER);

    if(!cfg || cfg->sz != sizeof(ICQULIST))
    {
       DBGMessage("Invalid container subclassing");
       icqskin_closeWindow(hwnd);
       return WinDefWindowProc(hwnd,msg,mp1,mp2);
    }

    switch(msg)
    {
    case CM_PAINTBACKGROUND:
       return (MRESULT)  paintBackground(hwnd, cfg, ((POWNERBACKGROUND) mp1)->hps, &((POWNERBACKGROUND) mp1)->rclBackground);

    case WM_CHAR:
       chkKeyboard( WinQueryWindow(hwnd,QW_OWNER),SHORT1FROMMP(mp1),SHORT1FROMMP(mp2));
       break;

    case WM_PRESPARAMCHANGED:
       // icqskin_updatePresParam(hwnd,LONGFROMMP(mp1),cfg->pal);
       WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),msg,mp1,mp2);
       break;
    }

    return cfg->oldCntProc(hwnd,msg,mp1,mp2);

 }

 static void setselectbar(HWND hwnd, const char *filename)
 {
    ICQULIST *cfg = WinQueryWindowPtr(hwnd,0);
    icqskin_loadImage(cfg->icq, filename, &cfg->imgSelected, &cfg->mskSelected);
 }

 static void loadSkin(HWND hwnd, const char *name, SKINFILESECTION *fl)
 {
    static const char *pallete[ICQCLR_CTLCOLORS] =
    {
       STANDARD_SKIN_PALLETE_NAMES,

       "Online",
       "Offline",
       "OnlineWaiting",
       "OfflineWaiting"
    };

    ICQULIST   *cfg            = WinQueryWindowPtr(hwnd,0);
    int        pos[]           = { 0,0,0,0 };
    const char *ptr            = NULL;

    char       buffer[0x0100];
    char       *tok;
    int        f;

    DBGMessage("Loading skin for the user list control");

    ptr = icqskin_getSkinFileEntry(fl, name, "UserList");

    if(ptr)
    {
       cfg->flags |= ICQULIST_SKINNED;

       strncpy(buffer,ptr,0xFF);
       DBGMessage(buffer);

       tok = strtok(buffer, ",");
       for(f=0;f<4 && tok;f++)
       {
          pos[f] = atoi(tok);
          tok    = strtok(NULL, ",");
       }

       DBGTrace(pos[0]);
       DBGTrace(pos[1]);
       DBGTrace(pos[2]);
       DBGTrace(pos[3]);

       icqskin_setButtonSize(hwnd,pos[0],pos[1],pos[2],pos[3]);

       if(tok)
       {
          DBGMessage(tok);
          icqskin_setTreeIcons(hwnd,tok);
          tok = strtok(NULL,",");
       }
       else
       {
          icqskin_setTreeIcons(hwnd,NULL);
       }

       if(tok)
       {
          icqskin_setSelectBar(hwnd,tok);
          DBGMessage(tok);
       }
       else
       {
          icqskin_setSelectBar(hwnd,NULL);
       }

    }
    else
    {
       clearSkin(cfg);
    }

    ptr = icqskin_getSkinFileEntry(fl, name, "UserList.Font");

    if(ptr)
       icqskin_setButtonFont(WinWindowFromID(hwnd,ID_CONTAINER),ptr);

    icqskin_loadSkinPallete(hwnd, name, fl, pallete, ICQCLR_CTLCOLORS, cfg->pal);

    WinPostMsg(hwnd,WMICQ_AUTOSIZE,0,0);
    WinInvalidateRect(hwnd,NULL,FALSE);

 }

 static PRECORDCORE searchTree(HWND hwnd, PRECORDCORE root, ULONG uin)
 {
    PRECORDCORE          node;
    PRECORDCORE          ret   = NULL;
    struct user_record   *p;

     node = WinSendMsg(hwnd,CM_QUERYRECORD,(MPARAM) root,MPFROM2SHORT(CMA_FIRSTCHILD,CMA_ITEMORDER));
//       node = WinSendMsg(hwnd,CM_QUERYRECORD,0,MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));

    while(node && !ret)
    {
       p = (struct user_record *) (node+1);

       switch(p->type)
       {
       case 0:
//          DBGMessage( ((struct group_record *) p)->text );
          ret = searchTree(hwnd, node, uin);
          break;

       case 1:
//          DBGMessage(icqQueryUserNick(p->usr));
          if(p->usr && p->usr->uin == uin)
             return node;
          break;

       }

       node = WinSendMsg(hwnd,CM_QUERYRECORD,MPFROMP(node),MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
    }

    return ret;

 }

 static PRECORDCORE findUser(HWND hwnd, ULONG uin)
 {
    PRECORDCORE ret;

//#ifdef DEBUG
//    ICQULIST *cfg = (ICQULIST *) WinQueryWindowPtr(hwnd,QWL_USER);
//    DBGPrint("*********[ %s ]*********",icqQueryUserNick(icqQueryUserHandle(cfg->icq,uin)));
//#endif

    ret = searchTree(hwnd,NULL,uin);

//    DBGTracex(ret);

    return ret;
 }

 static HUSER updateUser(HWND hwnd, ULONG uin)
 {
    PRECORDCORE        rec[1] = { findUser(hwnd, uin) };
    struct user_record *p;
    HUSER              usr = icqQueryUserHandle( ((ICQULIST *) WinQueryWindowPtr(hwnd,QWL_USER))->icq,uin);

//    DBGTracex(rec);

    if(!rec[0])
       return NULL;

    p = (struct user_record *) (rec+1);

    if(p->type != 1)
       return NULL;

    if(p->usr == usr)
       return usr;

    WinSendMsg(hwnd, CM_INVALIDATERECORD,(MPARAM) rec, MPFROM2SHORT(1,CMA_NOREPOSITION));

    return usr;
 }

 static HUSER queryUser(HWND hwnd, ULONG uin)
 {
    PRECORDCORE        rec;
    struct user_record *p;

    rec = findUser(hwnd,uin);
    DBGTracex(rec);

    if(!rec)
       return NULL;

    p = (struct user_record *) (rec+1);

    DBGTrace(p->type);
    DBGTracex(p->usr);
    DBGTracex(p->usr->uin);

    if(p->type == 1 && p->usr && p->usr->uin == uin)
       return p->usr;

    return NULL;
 }


 static void refreshUser(HWND hwnd, ULONG uin)
 {
    PRECORDCORE rec[1] = { findUser(hwnd, uin) };

    if(rec[0])
       WinSendMsg(hwnd, CM_INVALIDATERECORD,(MPARAM) rec, MPFROM2SHORT(1,CMA_NOREPOSITION));

 }

 static void deluser(HWND hwnd, ULONG uin)
 {
    PRECORDCORE rec[1] = { findUser(hwnd, uin) };

    if(rec[0])
       WinSendMsg(hwnd, CM_REMOVERECORD,(MPARAM) rec, MPFROM2SHORT(1,CMA_FREE|CMA_INVALIDATE));

 }


