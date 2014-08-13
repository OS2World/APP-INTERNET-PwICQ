/*
 * os2tools.c - OS2 Implementation of pwICQGUI Functions
 */

 #pragma strings(readonly)

 #define INCL_WIN
 #define INCL_GPI
 #define INCL_GPILOGCOLORTABLE

 #include <string.h>
 #include <stdlib.h>
 #include <malloc.h>
 #include <ctype.h>
 #include <pwicqgui.h>

/*---[ Defines ]------------------------------------------------------------------------------------------*/

 #define ICONBAR_ROWS 16

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 int icqskin_setVisible(SKINDATA *cfg, BOOL visible)
 {
    DBGTrace(visible);
    DBGTracex(cfg->frame);
    WinShowWindow(cfg->frame,visible);
    return 0;
 }

#ifdef ICONTABLE16

 static int loadAlternativeIconTable(SKINDATA *wnd)
 {
    SIZEL              sizl            = { 0, 0 };  /* use same page size as device */
    DEVOPENSTRUC       dop             = { 0, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};
    HAB                hab             = icqQueryAnchorBlock(wnd->icq);
    HDC                hdc;
    HPS                hps;

    CHKPoint();

    icqskin_deleteImage( wnd->iconImage );
    icqskin_deleteImage( wnd->iconMasc  );

    hdc  = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);
    hps  = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);

    if(hps)
    {
       wnd->iconImage = GpiLoadBitmap(hps, module, 101, ICONBAR_ROWS * PWICQICONBARSIZE, ICONBAR_ROWS);
       wnd->iconMasc  = GpiLoadBitmap(hps, module, 102, ICONBAR_ROWS * PWICQICONBARSIZE, ICONBAR_ROWS);
       GpiDestroyPS(hps);
    }

    DevCloseDC(hdc);

    return ICONBAR_ROWS;
 }

#endif

 int icqskin_loadIcons(SKINDATA *wnd, const char *fileName)
 {
    int iconSize;

#ifdef ICONTABLE16

    loadAlternativeIconTable(wnd);
    wnd->iconSize = ICONBAR_ROWS;

    if(wnd->iconImage != NO_IMAGE)
       return 0;

#endif

    iconSize = icqskin_loadImageFromResource(wnd->icq, 100, (ICONBAR_ROWS * PWICQICONBARSIZE), ICONBAR_ROWS, &wnd->iconImage, &wnd->iconMasc);

    if(iconSize < 0 || (wnd->iconImage == NO_IMAGE))
       return -1;

    wnd->iconSize = iconSize;

    DBGTrace(wnd->iconSize);
    return 0;
 }

 int icqskin_setFrameIcon(hWindow hwnd, SKINDATA *skn, USHORT id)
 {
    ICQFRAME *cfg = WinQueryWindowPtr(hwnd, QWL_USER);

    if(!cfg || cfg->sz < sizeof(ICQFRAME))
    {
       DBGMessage("Invalid frame window when setting icon");
       icqWriteSysLog(skn->icq,PROJECT,"Invalid frame window when setting icon");
       return -1;
    }

    DBGTrace(cfg->iconID);
    DBGTrace(id);

    if(cfg->iconID == id)
       return 0;

    DBGTrace(id);

    if(cfg->icon)
    {
       WinDestroyPointer(cfg->icon);
       cfg->icon = NULLHANDLE;
    }

    cfg->icon   = icqskin_CreatePointer(skn, id);
    cfg->iconID = id;

    WinSendMsg(hwnd,WM_SETICON,(MPARAM) cfg->icon, 0);

    return 0;
 }

 int icqskin_setFrameTitle(hWindow hwnd, const char *title)
 {
    WinSetWindowText(hwnd, (PSZ) title);
    return 0;
 }

 int icqskin_autoSize(HWND hwnd, short x, short y, short cx, short cy, short baseCy)
 {
    RECTL rcl;
    WinQueryWindowRect( WinQueryWindow(hwnd, QW_OWNER), &rcl);

    if(cx < 0)
       cx = rcl.xRight + cx;

    if( ((USHORT) cy) == 0xFFFF)
       cy = baseCy;
    else if(cy < 0)
       cy = rcl.yTop + cy;

    if(x < 0)
       x = (rcl.xRight + x) - cx;

    if(y < 0)
       y = (rcl.yTop + y) - cy;

    if( cx != (rcl.xLeft - rcl.xRight) || cy != (rcl.yTop - rcl.yBottom))
    {
       WinSetWindowPos(hwnd, 0, x, y, cx, cy, SWP_SIZE|SWP_MOVE);
       WinInvalidateRect(hwnd,NULL,FALSE);
       return TRUE;
    }

    return FALSE;
 }

 int icqskin_drawImage(HPS hps, PRECTL rcl, HBITMAP image, HBITMAP mask)
 {
    POINTL             aptl[4];
    BITMAPINFOHEADER   bmpData;
    int                x,y;

    GpiQueryBitmapParameters(image, &bmpData);

    x = ((rcl->xRight - rcl->xLeft)/2)   - (bmpData.cx/2);
    y = ((rcl->yTop   - rcl->yBottom)/2) - (bmpData.cy/2);

    aptl[0].x = x+rcl->xLeft;
    aptl[0].y = y+rcl->yBottom;
    aptl[1].x = (aptl[0].x+bmpData.cx)-1;
    aptl[1].y = (aptl[0].y+bmpData.cy)-1;

    aptl[2].x =
    aptl[2].y = 0;
    aptl[3].x = bmpData.cx;
    aptl[3].y = bmpData.cy;

    GpiSetColor(hps,CLR_WHITE);
    GpiSetBackColor(hps,CLR_BLACK);

    GpiWCBitBlt( hps, mask,  4, aptl, ROP_SRCAND,   BBO_IGNORE);
    GpiWCBitBlt( hps, image, 4, aptl, ROP_SRCPAINT, BBO_IGNORE);

    return bmpData.cy;
 }

 USHORT icqskin_drawIcon(SKINDATA *s, HPS hps, USHORT id, USHORT x, USHORT y)
 {
    POINTL               aptl[4];
    USHORT               fator;

    aptl[0].x = x;
    aptl[0].y = y;
    aptl[1].x = x+s->iconSize-1;
    aptl[1].y = y+s->iconSize-1;

    fator = s->iconSize * id;

    aptl[2].x = fator;
    aptl[2].y = 0;
    aptl[3].x = fator + s->iconSize;
    aptl[3].y = s->iconSize;

    GpiSetColor(hps,CLR_WHITE);
    GpiSetBackColor(hps,CLR_BLACK);
    GpiWCBitBlt( hps,s->iconMasc,4,aptl,ROP_SRCAND,BBO_IGNORE);
    GpiWCBitBlt( hps,s->iconImage,4,aptl,ROP_SRCPAINT,BBO_IGNORE);

    return s->iconSize;
 }

 int icqskin_createButtons(HWND hwnd, SKINDATA *cfg, const struct icqButtonTable *btn, int elements)
 {
    HWND temp;

    DBGTrace(elements);

    while(elements--)
    {
       temp = icqbutton_new(hwnd, btn->id, btn->name, btn->icon, btn->text);
       icqskin_setButtonCallback(temp,btn->cbk,cfg);

//       temp = icqskin_createButton(hwnd,btn->id,cfg,btn->icon,btn->text, btn->name, btn->cbk,cfg);

       icqskin_setButtonSize(temp,btn->x,btn->y,btn->cx,btn->cy);
       if(btn->font)
          icqskin_setButtonFont(temp,btn->font);
       btn++;
    }

    return 0;
 }

 void icqskin_loadPallete(HPS hps, int size, const ULONG *clr)
 {
    int  f;
    LONG *alTable;

    if(!size)
       size = ICQCLR_COUNTER;

    alTable = malloc(size * sizeof(ULONG));
    if(!alTable)
       return;

    GpiQueryLogColorTable(hps, 0L, 0L, size, alTable);
    for(f=0;f<size;f++)
    {
       if(clr[f] != 0xFFFFFFFF)
          alTable[f] = clr[f];
    }
    GpiCreateLogColorTable(hps,0L,LCOLF_CONSECRGB,0L,size,alTable);

    free(alTable);

 }

 void icqskin_updatePresParam(HWND hwnd, ULONG id, ULONG *pal)
 {
    static const ULONG idList[] = { PP_BACKGROUNDCOLOR, PP_FOREGROUNDCOLOR };
    ULONG              clr;
    int                rc;

    rc = WinQueryPresParam(hwnd,id,0,NULL,sizeof(ULONG),&clr,QPF_PURERGBCOLOR);
    if(rc != 4)
       return;

    for(rc = 0; rc < sizeof(idList)/sizeof(ULONG);rc++)
    {
       if(idList[rc] == id)
       {
          DBGTrace(rc);
          pal[rc] = clr;
          WinInvalidateRect(hwnd,NULL,TRUE);
          return;
       }
    }
 }

 ULONG icqskin_loadPalleteFromString(const char *key)
 {
    ULONG      pal = 0;
    int        c;
    char       buffer[0x0100];
    char       *ptr;

    strncpy(buffer,key,0xFF);
    *(buffer+0xFF) = 0;

    ptr = strtok(buffer, ",");
    for(c=0;c<3&& ptr;c++)
    {
       pal <<= 8;
       pal  |=  (atoi(ptr) & 0x000000ff);
       ptr   = strtok(NULL,",");
    }
    return pal;
 }

 void icqskin_loadSkinPallete(hWindow hwnd, const char *name, SKINFILESECTION *fl, const char **tbl, int sz, ULONG *pTable)
 {
    int        f;
    const char *ptr;

    DBGTrace(sz);

    for(f=0;f<sz && *tbl;f++)
    {
       ptr = icqskin_getSkinFileEntry(fl, name, *tbl);

       if(ptr)
          *pTable = icqskin_loadPalleteFromString(ptr);

       tbl++;
       pTable++;
    }
 }

 int icqskin_queryClipboard(HICQ icq, int sz, char *buffer)
 {
    HAB  hab   = icqQueryAnchorBlock(icq);
    char *ptr;

    *buffer = 0;

    if(!WinOpenClipbrd(hab))
       return -1;

    ptr = (char *) WinQueryClipbrdData(hab , CF_TEXT);

    if(ptr)
    {
       strncpy(buffer,ptr,sz);
       *(buffer+sz) = '0';
    }

    WinCloseClipbrd(hab);

    return strlen(buffer);
 }

 int icqskin_registerWindowClass(HICQ icq)
 {
    HAB      hab  = icqQueryAnchorBlock(icq);
    int      f;
    char     buffer[0x0100];

    #pragma pack(1)

    static const struct wnd
    {
       const char **Name;
       PFNWP      Proc;
       const char *log;
    } wndList[] =
    {
    	  { &icqFrameWindowClass,    icqFrameWindow,     "frame"         },
    	  { &icqChatWindowClass,     icqChatWindow,      "chat"          },
          { &icqPagerWindowClass,    icqPagerWindow,     "alert"         },
    	  { &icqSCBarWindowClass,    icqSCBarWindow,     "scroll-bar"    },
    	  { &icqButtonWindowClass,   icqButtonWindow,    "button"        },
    	  { &icqUserListWindowClass, icqUserListWindow,  "userlist"      },
    	  { &icqStaticWindowClass,   icqStaticWindow,    "static"        },
    	  { &icqMenuWindowClass,     icqMenuListWindow,  "menu"          },
    	  { &icqDialogWindowClass,   icqDialogWindow,    "dialog"        },
    	  { &icqUPopupWindowClass,   icqUserPopupWindow, "userpopup"     },
    	  { &icqBitmapWindowClass,   icqBitmapWindow,    "bitmap"        },
    	  { &icqConfigWindowClass,   icqConfigWindow,    "configuration" }
    };

    #pragma pack()

    /* Register Window Classes */

    DBGTrace(sizeof(wndList)/sizeof(struct wnd));

    for(f=0;f < sizeof(wndList)/sizeof(struct wnd);f++ )
    {
       DBGPrint("Registering \"%s\"",*wndList[f].Name);

       if(!WinRegisterClass(hab,(PSZ) *wndList[f].Name, wndList[f].Proc,0,sizeof(PVOID)))
       {
          sprintf(buffer,"Failure registering %s window class",wndList[f].log);
          icqWriteSysLog(icq,PROJECT,buffer);
          return -1;
       }
    }
    return 0;
 }

 int EXPENTRY icqskin_selectFile(HWND hwnd, HICQ icq, USHORT id, BOOL save, const char *key, const char *masc, const char *title, char *vlr, int (* _System callBack)(const DIALOGCALLS *,HWND,HICQ,char *))
 {
    char                temp[80];
    FILEDLG             fild;
    char                *ptr;
    char                *pos;

    CHKPoint();
    memset(&fild,0,sizeof(FILEDLG));

    fild.cbSize      = sizeof(FILEDLG);
    fild.fl          = FDS_CENTER|FDS_OPEN_DIALOG;
    fild.pszTitle    = (PSZ) title;

    if(id)
       WinQueryDlgItemText(hwnd,id,CCHMAXPATH,fild.szFullFile);
    else if(vlr)
       strncpy(vlr,fild.szFullFile,CCHMAXPATH);

    DBGMessage(fild.szFullFile);
    DBGMessage(fild.pszTitle);

    if(WinFileDlg(HWND_DESKTOP, hwnd, &fild) && fild.lReturn == DID_OK)
    {
       if(callBack)
          callBack(&icqskin_dialogCalls,hwnd,icq,fild.szFullFile);

       if(id)
          WinSetDlgItemText(hwnd,id,fild.szFullFile);

       if(vlr)
          strncpy(fild.szFullFile,vlr,CCHMAXPATH);

       for(pos = ptr = fild.szFullFile;*ptr;ptr++)
       {
          if(*ptr == '\\' || *ptr == ':')
              pos = ptr;

       }
       *pos = 0;

       DBGMessage(temp);
       DBGMessage(fild.szFullFile);

       if(key)
          icqSaveString(icq, key, fild.szFullFile);

       return strlen(fild.szFullFile);

    }

    return 0;

 }


