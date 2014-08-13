/*
 * os2static.c - OS/2 Static control
 */

 #pragma strings(readonly)

 #define INCL_WIN
 #define INCL_GPI

 #include <string.h>
 #include <stdlib.h>
 #include <malloc.h>
 #include <pwicqgui.h>

/*---[ Defines ]------------------------------------------------------------------------------------------*/

 #define ICQCLR_BORDER ICQCLR_SELECTEDBG
 #define GET_TEXT_POINTER(c) (((char *)c)+c->sz)
 #define TEXT_FLAGS(s) (s & (DT_LEFT|DT_CENTER|DT_RIGHT|DT_TOP|DT_VCENTER|DT_BOTTOM))

/*---[ Constants ]----------------------------------------------------------------------------------------*/

 const char *icqStaticWindowClass = ICQ_STATIC;

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static void destroy(HWND);
 static void create(HWND, PCREATESTRUCT);
 static void configure(HWND, USHORT, USHORT, PCREATESTRUCT);
 static void erase(HWND, HPS, PRECTL);
 static void clear(HWND, ICQSTATIC *);
 static void paint(HWND);
 static int  querySizes(HWND,USHORT *,USHORT *);
 static BOOL setparm(HWND,PWNDPARAMS);
 static BOOL getparm(HWND,PWNDPARAMS);
 static void ppchanged(HWND,ULONG);
 static void drawContents(HWND, HPS);
 static BOOL loadSkin(HWND,const char *,SKINFILESECTION *);
 static void setbackground(HWND,USHORT,const char *);
 static void resize(HWND,short,short);

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 MRESULT EXPENTRY icqStaticWindow(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    switch(msg)
    {
    case WM_CREATE:
       create(hwnd,(PCREATESTRUCT) mp2);
       break;

    case WMICQ_CONFIGURE:
       configure(hwnd, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1), (PCREATESTRUCT) mp2);
       break;

    case WMICQ_SETICON:
       DBGTrace(mp1);
       ((ICQSTATIC *) WinQueryWindowPtr(hwnd,0))->icon = SHORT1FROMMP(mp1);
       CHKPoint();
       WinInvalidateRect(hwnd,NULL,FALSE);
       break;

    case WMICQ_SETICQHANDLE:
       ((ICQSTATIC *) WinQueryWindowPtr(hwnd,0))->icq  = (HICQ) mp1;
       break;

    case WMICQ_SETMARGIN:
       ((ICQSTATIC *) WinQueryWindowPtr(hwnd,0))->xLeft  = (USHORT) mp1;
       WinInvalidateRect(hwnd,NULL,FALSE);
       break;

    case WMICQ_SETNAME:
       ((ICQSTATIC *) WinQueryWindowPtr(hwnd,0))->name =  (const char *) mp1;
       break;

    case WMICQ_DRAWCONTENTS:
       drawContents(hwnd,(HPS) mp1);
       break;

    case WM_DESTROY:
       destroy(hwnd);
       break;

    case WM_ERASEBACKGROUND:
       erase(hwnd,(HPS) mp1, (PRECTL) mp2);
       break;

    case WM_SETWINDOWPARAMS:
       return (MRESULT) setparm(hwnd, PVOIDFROMMP(mp1));

    case WM_QUERYWINDOWPARAMS:
        return (MRESULT) getparm(hwnd, PVOIDFROMMP(mp1));

    case WM_PRESPARAMCHANGED:
       ppchanged(hwnd,LONGFROMMP(mp1));
       break;

    case WM_PAINT:
       paint(hwnd);
       break;

    case WMICQ_QUERYPALLETE:
       return (MRESULT) ((ICQSTATIC *) WinQueryWindowPtr(hwnd,0))->pal;

    case WMICQ_QUERYSIZES:
       return (MRESULT) querySizes(hwnd, (USHORT *) mp1, (USHORT *) mp2);

    case WMICQ_LOADSKIN:
       return (MRESULT) loadSkin(hwnd,(const char *) mp1, (SKINFILESECTION *) mp2);

    case WMICQ_SETBGIMAGE:
       setbackground(hwnd,SHORT1FROMMP(mp1),(const char *) mp2);
       break;

   case WM_SIZE:
      resize(hwnd,SHORT1FROMMP(mp2),SHORT2FROMMP(mp2));
      break;

    default:
       return WinDefWindowProc(hwnd,msg,mp1,mp2);

    }

    return 0;
 }

 static void destroy(HWND hwnd)
 {
    ICQSTATIC *cfg = WinQueryWindowPtr(hwnd,0);

    if(!cfg || cfg->sz != sizeof(ICQSTATIC))
       return;


    cfg->sz = 0;
    free(cfg);
 }

 static void create(HWND hwnd, PCREATESTRUCT cr)
 {
    ICQSTATIC *cfg;
    int       szText    = 0;

    if(cr->pszText)
       szText = strlen(cr->pszText)+4;

    cfg = malloc(sizeof(ICQSTATIC)+szText);
    WinSetWindowPtr(hwnd,0,cfg);
    if(!cfg)
    {
       icqskin_closeWindow(hwnd);
       return;
    }

    DBGTrace(szText);

    WinSendMsg(hwnd,WMICQ_CONFIGURE,MPFROM2SHORT(sizeof(ICQSTATIC),szText), (MPARAM) cr);
 }

 static void configure(HWND hwnd, USHORT sz, USHORT szText, PCREATESTRUCT cr)
 {
    ICQSTATIC *cfg   = WinQueryWindowPtr(hwnd,0);
    int       f;

    memset(cfg,0,sz);

    DBGTrace(sz);
    DBGTrace(szText);

    cfg->sz      = sz;
    cfg->maxText = szText;

    if(cr->pszText)
    {
       szText = strlen(cr->pszText)+1;

       if(szText > cfg->maxText)
       {
          DBGMessage("*** Window structure needs resizing");
          cfg = realloc(cfg,cfg->sz+szText);
          if(!cfg)
          {
             WinPostMsg(hwnd,WM_CLOSE,0,0);
             return;
          }
          WinSetWindowPtr(hwnd,0,cfg);
          cfg->maxText = szText;
       }
       strcpy( GET_TEXT_POINTER(cfg),cr->pszText);
    }

    clear(hwnd, cfg);
    cfg->icon  = NO_ICON;

 }

 void icqskin_eraseRect(HWND hwnd, HPS hps, PRECTL rcl)
 {
    HWND       owner = WinQueryWindow(hwnd, QW_OWNER);
    HBITMAP    bg    = icqskin_queryBackground(owner);

    RECTL	   rMap;
    POINTL     p;

    icqskin_selectPallete(owner,hps);

    if(bg == NO_IMAGE)
    {
       WinFillRect(hps, rcl, ICQCLR_BACKGROUND);
    }
    else
    {
       rMap.xLeft   = rcl->xLeft;
       rMap.xRight  = rcl->xRight;
       rMap.yTop    = rcl->yTop;
       rMap.yBottom = rcl->yBottom;
       WinMapWindowPoints(hwnd, owner, (POINTL *) &rMap, 2);

       p.x = rcl->xLeft;
       p.y = rcl->yBottom;
       WinDrawBitmap(hps, bg, &rMap, &p, CLR_WHITE, CLR_BLACK, DBM_NORMAL);
    }

 }

 static void erase(HWND hwnd, HPS hps, PRECTL rcl)
 {
    hps = WinBeginPaint(hwnd,NULLHANDLE,rcl);
    icqskin_drawContents(hwnd,hps);
    WinEndPaint(hps);
 }

 static void drawContents(HWND hwnd, HPS hps)
 {
    ICQSTATIC  *cfg   = WinQueryWindowPtr(hwnd,0);
    RECTL      rcl;
    ULONG      style  = WinQueryWindowULong(hwnd,QWL_STYLE);
    POINTL     ptl;

    if(!cfg || cfg->sz != sizeof(ICQSTATIC))
    {
       DBGTracex(hwnd);
       DBGMessage("*** Invalid static window when drawing");
       return;
    }

    WinQueryWindowRect( hwnd, &rcl);

    icqskin_loadPallete(hps, 0, cfg->pal);

    if(cfg->bg != NO_IMAGE)
    {
       ptl.x = rcl.xLeft;
       ptl.y = rcl.yBottom;
       WinDrawBitmap(hps, cfg->bg, &rcl, &ptl, CLR_WHITE, CLR_BLACK, DBM_NORMAL);
    }
    else if(cfg->pal[ICQCLR_BACKGROUND] != 0xFFFFFFFF)
    {
       WinFillRect(hps, &rcl, ICQCLR_BACKGROUND);
    }
    else
    {
       icqskin_eraseRect(hwnd, hps, &rcl);
    }

    if((style & SS_FGNDFRAME) && (cfg->pal[ICQCLR_BORDER] != 0xFFFFFFFF))
    {
       rcl.xRight--;
       rcl.yTop--;
       GpiSetColor(hps,ICQCLR_BORDER);

       ptl.x = rcl.xLeft;
       ptl.y = rcl.yBottom;
       GpiMove(hps,&ptl);
       ptl.y = rcl.yTop;
       GpiLine(hps,&ptl);
       ptl.x = rcl.xRight;
       GpiLine(hps,&ptl);
       ptl.y = rcl.yBottom;
       GpiLine(hps,&ptl);
       ptl.x = rcl.xLeft;
       GpiLine(hps,&ptl);

       rcl.xLeft++;
       rcl.yBottom++;
    }

    rcl.xLeft += cfg->xLeft;

    if((style & SS_ICON) && cfg && cfg->icq && cfg->icon != NO_ICON)
       rcl.xLeft = (icqskin_drawIcon(icqskin_getDataBlock(cfg->icq), hps, cfg->icon, rcl.xLeft+2, 3))+3;

    if(style & SS_TEXT)
       WinDrawText(hps, -1, GET_TEXT_POINTER(cfg), &rcl, ICQCLR_FOREGROUND, ICQCLR_BACKGROUND, TEXT_FLAGS(style));

 }

 static void paint(HWND hwnd)
 {
    HPS   hps;
    RECTL rcl;
    erase(hwnd,hps,&rcl);
 }

 static int querySizes(HWND hwnd, USHORT *xPtr, USHORT *yPtr)
 {
    ICQSTATIC        *cfg   = WinQueryWindowPtr(hwnd,0);
    ULONG            style  = WinQueryWindowULong(hwnd,QWL_STYLE);
    int	  		     x      = cfg->xLeft;
    int              y      = 0;
    HPS              hps;
    char             *ptr;
    SKINDATA         *skn;
    POINTL           aptl[TXTBOX_COUNT];
    BITMAPINFOHEADER bmpData;

    if((style & SS_FGNDFRAME) && (cfg->pal[ICQCLR_BORDER] != 0xFFFFFFFF))
    {
       x += 4;
       y += 4;
    }

    if((style & SS_ICON) && cfg && cfg->icq && cfg->icon != NO_ICON)
    {
       skn  = icqskin_getDataBlock(cfg->icq);
       if(skn)
       {
          x += skn->iconSize+6;
          y += skn->iconSize+6;
       }
    }

    if(style & SS_TEXT)
    {
       ptr = GET_TEXT_POINTER(cfg);
       hps = WinGetPS(hwnd);
       GpiQueryTextBox(hps,strlen(ptr),(char *) ptr,TXTBOX_COUNT,aptl);
       WinReleasePS(hps);
       x += aptl[TXTBOX_TOPRIGHT].x;
       y += aptl[TXTBOX_TOPRIGHT].y;
    }

    if(cfg->baseImage != NO_IMAGE)
    {
       GpiQueryBitmapParameters(cfg->baseImage, &bmpData);
       y = bmpData.cy;
    }

    if(xPtr)
       *xPtr = x;

    if(yPtr)
       *yPtr = y;

    return x;
 }


 static BOOL getparm(HWND hwnd, PWNDPARAMS p)
 {
    ICQSTATIC *cfg = WinQueryWindowPtr(hwnd,0);

    if(!cfg || !(p->fsStatus & WPM_TEXT))
       return FALSE;

    if(p->fsStatus & WPM_CCHTEXT)
    {
       strncpy(p->pszText,GET_TEXT_POINTER(cfg),p->cchText-1);
       *(p->pszText+p->cchText) = 0;
    }
    else
    {
       strcpy(p->pszText,GET_TEXT_POINTER(cfg));
    }


    return TRUE;
 }

 static BOOL setparm(HWND hwnd, PWNDPARAMS p)
 {
   ICQSTATIC *cfg = WinQueryWindowPtr(hwnd,0);
   int       sz;

   if(!cfg || p->fsStatus != WPM_TEXT)
      return FALSE;

   if(!p->pszText)
   {
      *(GET_TEXT_POINTER(cfg)) = 0;
      return TRUE;
   }

   sz = strlen(p->pszText)+1;

   if(sz > cfg->maxText)
   {
      cfg = realloc(cfg,sz+sizeof(ICQSTATIC));
      if(!cfg)
         return FALSE;
      WinSetWindowPtr(hwnd,0,cfg);
      cfg->maxText = sz;
   }

   strcpy(GET_TEXT_POINTER(cfg),p->pszText);

   WinInvalidateRect(hwnd,NULL,FALSE);

   return TRUE;

 }

 static void ppchanged(HWND hwnd, ULONG id)
 {
   ICQSTATIC *cfg = WinQueryWindowPtr(hwnd,0);
   if(cfg)
     icqskin_updatePresParam(hwnd,id,cfg->pal);
 }

 static void clear(HWND hwnd, ICQSTATIC *cfg)
 {
    ULONG  *pal = (ULONG *) WinSendMsg(WinQueryWindow(hwnd,QW_OWNER),WMICQ_QUERYPALLETE,0,0);
    int    f;

    icqskin_deleteImage(cfg->baseImage);
    icqskin_deleteImage(cfg->bg);

    if(pal)
    {
       for(f=0;f<ICQCLR_COUNTER;f++)
          cfg->pal[f] = pal[f];
       pal[ICQCLR_BORDER] = 0xFFFFFFFF;
    }
 }

 static BOOL loadSkin(hWindow hwnd, const char *name, SKINFILESECTION *fl)
 {
    static const char *pallete[ICQCLR_COUNTER]    = { "Background", "Foreground", "Border", "DisabledForeground" };

    ICQSTATIC         *cfg                        = WinQueryWindowPtr(hwnd,0);
    const char        *type                       = "Static";

    int               f;
    int               c;
    ULONG             pal;
    const char        *ptr                        = NULL;
    char              buffer[0x0100];

    clear(hwnd,cfg);

    for(f=0;f<ICQCLR_COUNTER;f++)
    {
       sprintf(buffer,"%s.%s",cfg->name,pallete[f]);
       ptr = icqskin_getSkinFileEntry(fl,name,buffer);
       if(!ptr)
       {
          sprintf(buffer,"%s.%s",type,pallete[f]);
          ptr = icqskin_getSkinFileEntry(fl,name,buffer);
          if(!ptr)
             ptr = icqskin_getSkinFileEntry(fl,name,pallete[f]);
       }

       if(ptr)
       {
          strncpy(buffer,ptr,0xFF);

          pal = 0;
          ptr = strtok(buffer, ",");
          for(c=0;c<3&& ptr;c++)
          {
             pal <<= 8;
             pal  |=  (atoi(ptr) & 0x000000ff);
             ptr   = strtok(NULL,",");
          }
          cfg->pal[f] = pal;
       }
       else
       {
          cfg->pal[f] = 0xFFFFFFFF;
       }
    }

    /* Set font */

    sprintf(buffer,"%s.Font",cfg->name);
    ptr = icqskin_getSkinFileEntry(fl,name,buffer);
    if(!ptr)
    {
       sprintf(buffer,"%s.Font",type);
       ptr = icqskin_getSkinFileEntry(fl,name,buffer);
       if(!ptr)
          ptr = icqskin_getSkinFileEntry(fl,name,"Font");
    }

    if(ptr)
       icqskin_setButtonFont(hwnd,ptr);


    /* Set background image */

    sprintf(buffer,"%s.BgImage",cfg->name);
    ptr = icqskin_getSkinFileEntry(fl,name,buffer);
    if(!ptr)
    {
       sprintf(buffer,"%s.BgImage",type);
       ptr = icqskin_getSkinFileEntry(fl,name,buffer);
    }

    if(ptr)
       icqskin_setButtonBackground(hwnd,0,ptr);

    /* Set left Margim */
    sprintf(buffer,"%s.Margin",cfg->name);
    ptr = icqskin_getSkinFileEntry(fl,name,buffer);
    if(!ptr)
    {
       sprintf(buffer,"%s.Margin",type);
       ptr = icqskin_getSkinFileEntry(fl,name,buffer);
    }

    if(ptr)
       cfg->xLeft = atoi(ptr);
    else
       cfg->xLeft = 0;

    WinInvalidateRect(hwnd,NULL,FALSE);

    return TRUE;

 }

 static void setbackground(HWND hwnd, USHORT id, const char *filename)
 {
    ICQSTATIC *cfg = WinQueryWindowPtr(hwnd,0);

    icqskin_deleteImage(cfg->baseImage);
    icqskin_deleteImage(cfg->bg);

    cfg->baseImage = icqskin_loadBitmap(cfg->icq, filename);

    if(cfg->bg != NO_IMAGE)
       cfg->bg = icqskin_createBackground(hwnd,cfg->baseImage);

    WinInvalidateRect(hwnd,NULL,FALSE);
 }

 static void resize(HWND hwnd, short cx, short cy)
 {
    ICQSTATIC *cfg = WinQueryWindowPtr(hwnd,0);

    if(!cfg)
       return;

    icqskin_deleteImage(cfg->bg);

    if(cfg->baseImage != NO_IMAGE)
       cfg->bg = icqskin_createBackground(hwnd,cfg->baseImage);

 }

