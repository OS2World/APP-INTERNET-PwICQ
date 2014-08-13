/*
 * os2button.c - OS2 Button Message Processing
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


/*---[ Button control data structure ]--------------------------------------------------------------------*/

 #pragma pack(1)

 enum BUTTON_BITMAPS
 {
    BITMAP_NORMAL,
    BITMAP_PRESSED,
    BITMAP_MOUSEOVER,
    BITMAP_DISABLED,

    BITMAP_COUNTER
 };

 #define ICQCLR_DISABLEDBG ICQCLR_SELECTEDBG
 #define ICQCLR_DISABLEDFG ICQCLR_SELECTEDFG

 #define SIZE_FROM_IMAGE 0x8000

 typedef struct _icqbutton
 {
    ICQGUI_STANDARD_PREFIX

    ULONG      flags;
    #define    BTNFLAG_PRESSED      0x00000001  // The mouse is pressed
    #define    BTNFLAG_OVER         0x00000002  // The mouse is over the control

    #define    BTNFLAG_REDRAW	    0x80000000	 // Regenerate background bitmap
    #define    BTNFLAG_PARMCHANGED  0x40000000  // Parameter was changed, base size needs to be recalculated

    /* Callback functions */
    void       *clickedParm;
    void       (ICQCALLBACK *clicked)(hWindow,void *);

    /* Window size */
    short      xSize;
    short      ySize;

    /* Window Positioning */
    short      x;
    short      y;
    short      cx;
    short      cy;
    short      minCy;

    /* Images */
    USHORT     icon;				    // Current button icon
    HBITMAP    bg;                      // Current button background
    HBITMAP    imgBase;                 // Current button image
    HBITMAP    imgMask;

    /* Skin system */
    HBITMAP    baseImage[BITMAP_COUNTER];	// Button base bitmaps
    ULONG      pal[ICQCLR_COUNTER];

    UCHAR      maxText;
    char       text[1];

 } ICQBUTTON;



/*---[ Constants ]----------------------------------------------------------------------------------------*/

 const char *icqButtonWindowClass = ICQ_BUTTON;

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static void erase(HWND, HPS, PRECTL);
 static void create(HWND, PCREATESTRUCT);
 static void destroy(HWND);
 static void setsize(HWND,short,short,short,short);
 static void autosize(HWND);
 static void resize(HWND,short,short);
 static void paint(HWND);
 static void click(HWND);
 static void seticon(HWND,USHORT);
 static void mouseover(HWND,BOOL);
 static void createDefaultBackground(HWND, ICQBUTTON *);
 static void setcallback(HWND, void (ICQCALLBACK *)(hWindow,void *), void *);
 static BOOL setparm(HWND,PWNDPARAMS);
 static BOOL getparm(HWND,PWNDPARAMS);
 static void ppchanged(HWND,ULONG);
 static void drawContents(HWND, ICQBUTTON *, HPS);
 static void drawBackground(HWND, HPS, PRECTL);
 static void setbackground(HWND,USHORT,const char *);
 static void loadSkin(HWND,const char *,SKINFILESECTION *);

 static int  querySizes(HWND,USHORT *,USHORT *);

 static int  setimage(HWND,const char *);

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 hWindow EXPENTRY icqbutton_new(hWindow owner, USHORT id, const char *name, USHORT icon, const char *text)
 {
    char buffer[0x0100];

    struct _ownerinfo
    {
       ICQGUI_STANDARD_PREFIX
    } *cfg = WinQueryWindowPtr(owner,0);

    HWND ret = WinCreateWindow(owner, (PSZ) icqButtonWindowClass, (PSZ) text, BS_AUTOSIZE|WS_VISIBLE, 0,0,0,0, owner, HWND_TOP, id, NULL, NULL);

    icqskin_setICQHandle(ret,cfg->icq);
    icqskin_setButtonIcon(ret,icon);
    icqskin_setButtonName(ret,name);

    if(icon == 0xFFFF)
    {
       strncpy(buffer,name,0xFF);
       strncat(buffer,".gif",0xFF);
       DBGMessage(buffer);
       icqskin_setButtonImage(ret,buffer);
    }

    return ret;

 }

/*
 hWindow icqskin_createButton(hWindow hwnd, USHORT id, SKINDATA *cfg, USHORT icon, const char *text, const char *name, void (ICQCALLBACK *cbk)(hWindow,void *), void *parm)
 {
    char buffer[0x0100];

    HWND ret = WinCreateWindow(hwnd, (PSZ) icqButtonWindowClass, (PSZ) text, BS_AUTOSIZE|WS_VISIBLE, 0,0,0,0, hwnd, HWND_TOP, id, NULL, NULL);

    icqskin_setButtonCallback(ret,cbk,parm);
    icqskin_setICQHandle(ret,cfg->icq);
    icqskin_setButtonIcon(ret,icon);
    icqskin_setButtonName(ret,name);

    if(icon == 0xFFFF)
    {
       strncpy(buffer,name,0xFF);
       strncat(buffer,".gif",0xFF);
       DBGMessage(buffer);
       icqskin_setButtonImage(ret,buffer);
    }

    return ret;
 }
*/

 MRESULT EXPENTRY icqButtonWindow(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    switch(msg)
    {
    case WM_CREATE:
       create(hwnd,(PCREATESTRUCT) mp2);
       break;

    case WM_DESTROY:
       destroy(hwnd);
       break;

    case WM_SIZE:
       resize(hwnd,SHORT1FROMMP(mp2),SHORT2FROMMP(mp2));
       break;

    case WM_SHOW:
       WinPostMsg(WinQueryWindow(hwnd, QW_OWNER),WMICQ_AUTOSIZE,0,0);
       break;

    case WM_ERASEBACKGROUND:
       erase(hwnd,(HPS) mp1, (PRECTL) mp2);
       break;

    case WM_PAINT:
       paint(hwnd);
       break;

    case WM_ENABLE:
       DBGTracex(hwnd);
       DBGTracex(mp1);
       DBGTracex(WinIsWindowEnabled(hwnd));
       ((ICQBUTTON *) WinQueryWindowPtr(hwnd,0))->flags |= BTNFLAG_REDRAW;
       WinInvalidateRect(hwnd,NULL,FALSE);
       break;

    case WM_BUTTON1DOWN:
       ((ICQBUTTON *) WinQueryWindowPtr(hwnd,0))->flags |= (BTNFLAG_PRESSED|BTNFLAG_REDRAW);
       WinInvalidateRect(hwnd,NULL,FALSE);
       break;

    case WM_BUTTON1UP:
       WinPostMsg(hwnd,WMICQ_CLICKED,0,0);
       break;

    case WMICQ_CLICKED:
       click(hwnd);
       break;

    case WM_PRESPARAMCHANGED:
       ppchanged(hwnd,LONGFROMMP(mp1));
       break;

    case WM_SETWINDOWPARAMS:
       return (MRESULT) setparm(hwnd, PVOIDFROMMP(mp1));

    case WM_CONTEXTMENU:
       DBGMessage("WM_CONTEXTMENU");
       break;

    case WM_QUERYWINDOWPARAMS:
        return (MRESULT) getparm(hwnd, PVOIDFROMMP(mp1));

    case 0x041E:
       mouseover(hwnd,TRUE);
       return WinDefWindowProc(hwnd, msg, mp1, mp2);

    case 0x041F:
       mouseover(hwnd,FALSE);
       return WinDefWindowProc(hwnd, msg, mp1, mp2);

    case WMICQ_SETBGIMAGE:
       setbackground(hwnd,SHORT1FROMMP(mp1),(const char *) mp2);
       break;

    case WMICQ_SETSIZE:
       setsize(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1),SHORT1FROMMP(mp2),SHORT2FROMMP(mp2));
       break;

    case WMICQ_AUTOSIZE:
       if(WinQueryWindowULong(hwnd,QWL_STYLE) & BS_AUTOSIZE)
          autosize(hwnd);
       break;

    case WMICQ_SETNAME:
       ((ICQBUTTON *) WinQueryWindowPtr(hwnd,0))->name = (const char *) mp1;
       break;

    case WMICQ_SETICQHANDLE:
       ((ICQBUTTON *) WinQueryWindowPtr(hwnd,0))->icq = (HICQ) mp1;
       break;

    case WMICQ_SETCALLBACK:
       setcallback(hwnd, (BUTTON_CALLBACK) mp1, (void *) mp2);
       break;

    case WMICQ_SETICON:
       seticon(hwnd,SHORT1FROMMP(mp1));
       break;

    case WMICQ_SETIMAGE:
       return (MRESULT) setimage(hwnd,(const char *) mp1);

    case WMICQ_QUERYPALLETE:
       return (MRESULT) ((ICQBUTTON *) WinQueryWindowPtr(hwnd,0))->pal;

    case WMICQ_LOADSKIN:
       loadSkin(hwnd,(const char *) mp1, (SKINFILESECTION *) mp2);
       return (MRESULT) TRUE;

    case WMICQ_QUERYSIZES:
       return (MRESULT) querySizes(hwnd, (USHORT *) mp1, (USHORT *) mp2);

    default:
       return WinDefWindowProc(hwnd,msg,mp1,mp2);
    }
    return 0;
 }

 static void drawBackground(HWND hwnd, HPS hps, PRECTL rcl)
 {
    ICQBUTTON *btn   = WinQueryWindowPtr(hwnd,0);
    POINTL    p;

    if(!btn || btn->sz != sizeof(ICQBUTTON))
    {
       WinFillRect(hps, rcl, CLR_RED);
       return;
    }

    if(btn->flags & BTNFLAG_REDRAW)
    {
       if(btn->bg != NO_IMAGE)
       {
          GpiDeleteBitmap(btn->bg);
          btn->bg = NO_IMAGE;
       }

       if(!WinIsWindowEnabled(hwnd))
       {
          if(btn->baseImage[BITMAP_DISABLED] != NO_IMAGE)
             btn->bg = icqskin_createBackground(hwnd,btn->baseImage[BITMAP_DISABLED]);
          else if(btn->baseImage[BITMAP_NORMAL] != NO_IMAGE)
             btn->bg = icqskin_createBackground(hwnd,btn->baseImage[BITMAP_NORMAL]);
          else
             createDefaultBackground(hwnd, btn);
       }
       else
       {
          if( (btn->flags & BTNFLAG_PRESSED) && (btn->baseImage[BITMAP_PRESSED] != NO_IMAGE))
             btn->bg = icqskin_createBackground(hwnd,btn->baseImage[BITMAP_PRESSED]);
          else if( (btn->flags & BTNFLAG_OVER) && (btn->baseImage[BITMAP_MOUSEOVER] != NO_IMAGE))
             btn->bg = icqskin_createBackground(hwnd,btn->baseImage[BITMAP_MOUSEOVER]);
          else if(btn->baseImage[BITMAP_NORMAL] != NO_IMAGE)
             btn->bg = icqskin_createBackground(hwnd,btn->baseImage[BITMAP_NORMAL]);
          else
             createDefaultBackground(hwnd, btn);
       }

       btn->flags &= ~(BTNFLAG_REDRAW|BTNFLAG_PARMCHANGED);

    }


    if(btn->bg != NO_IMAGE)
    {
       /* Draw background image */
       p.x = rcl->xLeft;
       p.y = rcl->yBottom;
       WinDrawBitmap(hps, btn->bg, rcl, &p, CLR_WHITE, CLR_BLACK, DBM_NORMAL);
    }
    else
    {
       /* No background */
       WinFillRect(hps, rcl, CLR_BLUE);
    }
 }

 static void erase(HWND hwnd, HPS hps, PRECTL rcl)
 {
    hps = WinBeginPaint(hwnd,NULLHANDLE,rcl);
    drawBackground(hwnd,hps,rcl);
    drawContents(hwnd,WinQueryWindowPtr(hwnd,0),hps);
    WinEndPaint(hps);
 }

 static void paint(HWND hwnd)
 {
    HPS   hps;
    RECTL rcl;
    erase(hwnd,hps,&rcl);
 }

 static void create(HWND hwnd, PCREATESTRUCT cr)
 {
    ICQBUTTON *btn;
    char      *txt = "";
    ULONG     *pal;
    int       f;

    if(cr && cr->pszText)
       txt = cr->pszText;

    btn = malloc(sizeof(ICQBUTTON)+strlen(txt)+1);
    DBGTracex(btn);

    WinSetWindowPtr(hwnd,0,btn);

    if(!btn)
    {
       DBGMessage("Memory allocation error when creating button");
       icqskin_closeWindow(hwnd);
       return;
    }

    CHKPoint();
    memset(btn,0,sizeof(ICQBUTTON));
    btn->sz = sizeof(ICQBUTTON);

    /* Initialize the button structure */
    btn->bg            =
    btn->imgBase       =
    btn->imgMask       = NO_IMAGE;
    btn->icon          = NO_ICON;

    btn->maxText       = strlen(txt);
    strcpy(btn->text,txt);

    for(f=0;f<BITMAP_COUNTER;f++)
       btn->baseImage[f] = NO_IMAGE;

    /* Get pallete information from the owner window */
    pal = (ULONG *) WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),WMICQ_QUERYPALLETE,0,0);

    if(pal)
    {
       for(f=0;f<ICQCLR_COUNTER;f++)
          btn->pal[f] = pal[f];
    }
#ifdef DEBUG
    else
    {
       DBGMessage("********* FAILURE GETTING PALLETE INFORMATION *********************");
    }
#endif


 }

 static void destroy(HWND hwnd)
 {
    ICQBUTTON *btn = WinQueryWindowPtr(hwnd,0);

//    DBGTracex(btn);

    if(!btn || btn->sz != sizeof(ICQBUTTON))
       return;

    btn->sz = 0;
    free(btn);
 }


 static void clear(HWND hwnd, ICQBUTTON *btn)
 {
    /* Clear bitmapped resources */

    int f;

    if(btn->bg != NO_IMAGE)
       GpiDeleteBitmap(btn->bg);

    if(btn->imgBase != NO_IMAGE)
    {
       GpiDeleteBitmap(btn->imgBase);
       btn->imgBase = NO_IMAGE;
    }

    if(btn->imgMask != NO_IMAGE)
    {
       GpiDeleteBitmap(btn->imgMask);
       btn->imgBase = NO_IMAGE;
    }

    for(f=0;f<BITMAP_COUNTER;f++)
    {
       if(btn->baseImage[f] != NO_IMAGE)
       {
          GpiDeleteBitmap(btn->baseImage[f]);
          btn->baseImage[f] = NO_IMAGE;
       }
    }
 }

 static void setsize(HWND hwnd, short x, short y, short cx, short cy)
 {
    ICQBUTTON        *btn = WinQueryWindowPtr(hwnd,0);
    BITMAPINFOHEADER bmpData;
    int				 f;

    memset(&bmpData,0,sizeof(bmpData));

    for(f=0;f<BITMAP_COUNTER && btn->baseImage[f] == NO_IMAGE;f++);

    if(f < BITMAP_COUNTER)
       GpiQueryBitmapParameters(btn->baseImage[f], &bmpData);

    btn->x     = x;
    btn->y     = y;

    if( ((USHORT) cx) == SIZE_FROM_IMAGE)
    {
       btn->cx = bmpData.cx;
    }
    else
    {
       btn->cx    = cx;
    }

    if( ((USHORT) cy) == SIZE_FROM_IMAGE)
    {
       btn->cy = bmpData.cy;
    }
    else
    {
       btn->cy    = cy;
    }

    if(btn->minCy < 1)
       btn->minCy = 4;   // FIXME: The correct way is recalculate the text/icon size

    WinPostMsg(hwnd,WMICQ_AUTOSIZE,0,0);
 }

 static void autosize(HWND hwnd)
 {
    ICQBUTTON *btn = WinQueryWindowPtr(hwnd,0);
    if(icqskin_autoSize(hwnd,btn->x,btn->y,btn->cx,btn->cy,btn->minCy))
       WinPostMsg(WinQueryWindow(hwnd, QW_OWNER),WMICQ_SIZECHANGED,(MPARAM) WinQueryWindowUShort(hwnd,QWS_ID),(MPARAM) hwnd);
 }

 static void resize(HWND hwnd, short cx, short cy)
 {
    ICQBUTTON *btn = WinQueryWindowPtr(hwnd,0);

    if(!btn || btn->sz != sizeof(ICQBUTTON))
       return;

    btn->flags |= BTNFLAG_REDRAW;
    btn->xSize  = cx;
    btn->ySize  = cy;

 }

 static void mouseover(HWND hwnd, BOOL mode)
 {
    ICQBUTTON *btn = WinQueryWindowPtr(hwnd,0);

    if(mode)
       btn->flags |= BTNFLAG_OVER;
    else
       btn->flags &= ~(BTNFLAG_OVER|BITMAP_PRESSED);

    btn->flags |= BTNFLAG_REDRAW;

    WinInvalidateRect(hwnd,NULL,FALSE);

 }

 void icqskin_drawBorder(HPS hps, PRECTL r, LONG clr1, LONG clr2)
 {
    POINTL ptl;
    GpiSetColor(hps,clr1);
    ptl.x = r->xRight-1;
    ptl.y = r->yTop-1;
    GpiMove(hps,&ptl);
    ptl.x = r->xLeft;
    GpiLine(hps,&ptl);
    ptl.y = r->yBottom;
    GpiLine(hps,&ptl);
    GpiSetColor(hps,clr2);
    ptl.x = r->xRight-1;
    GpiLine(hps,&ptl);
    ptl.y = r->yTop-1;
    GpiLine(hps,&ptl);
 }

 static void drawContents(HWND hwnd, ICQBUTTON *cfg, HPS hps)
 {
    RECTL      rcl;
    short      minCy   = 0;
    ULONG      fg      = ICQCLR_FOREGROUND;
    ULONG      bg      = ICQCLR_BACKGROUND;
    SKINDATA   *skn;
    ULONG	   flags;
    int        x;
    int        y;

    if(!WinIsWindowEnabled(hwnd))
    {
       fg = ICQCLR_DISABLEDFG;
       bg = ICQCLR_DISABLEDBG;
    }

    icqskin_loadPallete(hps, 0, cfg->pal);

    x           =
    y           =
    rcl.xLeft   =
    rcl.yBottom = 0;

    rcl.xRight  = cfg->xSize;
    rcl.yTop    = cfg->ySize;

    /* Draw Images/Text */

    if(cfg->icon != NO_ICON)
    {
       /* Draw pwICQ icon */
       skn  = icqskin_getDataBlock(cfg->icq);
       x   += 4;
       if(skn)
       {
          if(minCy < (skn->iconSize+6))
             minCy = skn->iconSize+6;

          y = (cfg->ySize/2) - (skn->iconSize/2);
          icqskin_drawIcon(skn, hps, cfg->icon, x, y);
          x += skn->iconSize+2;
       }
    }

    if(cfg->imgBase != NO_IMAGE)
    {
       rcl.xLeft   = x;
       rcl.yBottom = 0;
       rcl.xRight  = cfg->xSize;
       rcl.yTop    = cfg->ySize;
       y           = icqskin_drawImage(hps, &rcl, cfg->imgBase, cfg->imgMask)+4;
       if(minCy < y)
          minCy = y;
    }
    else if( *cfg->text)
    {
       /* Draw button text */
       flags = DT_VCENTER|DT_MNEMONIC;

       if(cfg->icon == NO_ICON)
          flags |= DT_CENTER;

       if(cfg->flags & BTNFLAG_PARMCHANGED)
       {
          WinDrawText(hps, -1, cfg->text, &rcl, fg, bg, flags|DT_QUERYEXTENT);
          rcl.yTop -= rcl.yBottom;
          rcl.yTop += 6;
          if(minCy < rcl.yTop)
             minCy = rcl.yTop;
       }

       rcl.xLeft   = x;
       rcl.yBottom = 0;
       rcl.xRight  = cfg->xSize;
       rcl.yTop    = cfg->ySize;

       WinDrawText(hps, -1, cfg->text, &rcl, fg, bg, flags);
    }

    if(minCy > cfg->minCy)
    {
       cfg->minCy = minCy;
       WinPostMsg(hwnd,WMICQ_AUTOSIZE,0,0);
    }

 }

 static void DrawBorder(HPS hps, PRECTL r, LONG clr1, LONG clr2)
 {
    POINTL ptl;
    GpiSetColor(hps,clr1);
    ptl.x = r->xRight-1;
    ptl.y = r->yTop-1;
    GpiMove(hps,&ptl);
    ptl.x = r->xLeft;
    GpiLine(hps,&ptl);
    ptl.y = r->yBottom;
    GpiLine(hps,&ptl);
    GpiSetColor(hps,clr2);
    ptl.x = r->xRight-1;
    GpiLine(hps,&ptl);
    ptl.y = r->yTop-1;
    GpiLine(hps,&ptl);
 }

 static void DrawBorder2(HPS hps, PRECTL r, LONG clr1, LONG clr2)
 {
    POINTL ptl;

    // Primeiro quadrado
    GpiSetColor(hps,clr2);
    ptl.x = r->xLeft-1;
    ptl.y = r->yTop;
    GpiMove(hps,&ptl);
    ptl.x = r->xRight-1;
    GpiLine(hps,&ptl);
    ptl.y = r->yBottom;
    GpiLine(hps,&ptl);
    ptl.x = r->xLeft-1;
    GpiLine(hps,&ptl);
    ptl.y = r->yTop-1;
    GpiLine(hps,&ptl);

    // Segundo quadrado
    GpiSetColor(hps,clr1);
    ptl.y = r->yTop-1;
    ptl.x = r->xRight;
    GpiMove(hps,&ptl);
    ptl.y = r->yBottom-1;
    GpiLine(hps,&ptl);
    ptl.x = r->xLeft;
    GpiLine(hps,&ptl);
    ptl.y = r->yTop-1;
    GpiLine(hps,&ptl);
    ptl.x = r->xRight-2;
    GpiLine(hps,&ptl);


 }

 static void createDefaultBackground(HWND hwnd, ICQBUTTON *cfg)
 {

    HPS                  hps;
    HDC                  hdc;
    SIZEL                sizl    = { 0, 0 };  /* use same page size as device */
    DEVOPENSTRUC         dop     = {0L, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};
    HAB                  hab     = WinQueryAnchorBlock(hwnd);
    BITMAPINFOHEADER2    bmih;
    BITMAPINFOHEADER     bmpData; /* bit-map information header     */
    RECTL                rcl;

    hdc = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);
    hps = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);

    memset(&bmih,0, sizeof(BITMAPINFOHEADER2));
    bmih.cbFix        = sizeof(bmih);
    bmih.cx           = cfg->xSize;
    bmih.cy           = cfg->ySize;
    bmih.cPlanes      = 1;
    bmih.cBitCount    = 24;

    cfg->bg = GpiCreateBitmap(hps,&bmih,0L,NULL,NULL);
    GpiSetBitmap(hps, cfg->bg);

    icqskin_loadPallete(hps, 0, cfg->pal);

//    drawElements(hwnd,cfg,hps);

    /* Draw inside the bitmap */

    GpiSetColor(hps,CLR_BLACK);
    GpiSetBackColor(hps,ICQCLR_BACKGROUND);

    rcl.xLeft   =
    rcl.yBottom = 0;
    rcl.xRight  = cfg->xSize;
    rcl.yTop    = cfg->ySize;
    WinFillRect(hps, &rcl, ICQCLR_BACKGROUND);

    /* Draw borders */
    if(WinIsWindowEnabled(hwnd))
    {
       if( !(WinQueryWindowULong(hwnd,QWL_STYLE)&BS_NOBORDER) && (cfg->flags&BTNFLAG_OVER) == BTNFLAG_OVER)
          DrawBorder(hps,&rcl,CLR_BLACK,CLR_BLACK);

       rcl.xRight--;
       rcl.yTop--;
       rcl.xLeft++;
       rcl.yBottom++;

       switch(cfg->flags & 0x03)
       {
       case BTNFLAG_OVER:
          DrawBorder(hps,&rcl,CLR_WHITE,CLR_DARKGRAY);
          break;

       case BTNFLAG_PRESSED:
       case BTNFLAG_PRESSED|BTNFLAG_OVER:
          DrawBorder(hps,&rcl,CLR_DARKGRAY,CLR_WHITE);
          break;

       default:
          if( !(WinQueryWindowULong(hwnd,QWL_STYLE)&BS_NOBORDER) )
             DrawBorder2(hps,&rcl,CLR_WHITE,CLR_DARKGRAY);
       }

    }

    /* Release resources */
    GpiSetBitmap(hps, NULLHANDLE);
    GpiDestroyPS(hps);
    DevCloseDC(hdc);

 }

 static void click(HWND hwnd)
 {
    ICQBUTTON *btn = WinQueryWindowPtr(hwnd,0);

    CHKPoint();

    btn->flags &= ~BTNFLAG_PRESSED;
    btn->flags |= BTNFLAG_REDRAW;

    WinInvalidateRect(hwnd,NULL,FALSE);

    if(WinIsWindowEnabled(hwnd))
    {

       if(btn->clicked)
          btn->clicked(hwnd,btn->clickedParm);

       WinPostMsg(	WinQueryWindow(hwnd, QW_OWNER),
                    WM_COMMAND,
   		            (MPARAM) WinQueryWindowUShort(hwnd,QWS_ID),
   		            MPFROM2SHORT((short) CMDSRC_PUSHBUTTON, (short) TRUE));
    }

 }

 static void setcallback(HWND hwnd, void (ICQCALLBACK *cbk)(hWindow,void *), void *parm)
 {
    ICQBUTTON *btn = WinQueryWindowPtr(hwnd,0);

    btn->clicked     = cbk;
    btn->clickedParm = parm;

 }

 static BOOL getparm(HWND hwnd, PWNDPARAMS p)
 {
    ICQBUTTON *btn = WinQueryWindowPtr(hwnd,0);
    int       sz;

    if(!btn || p->fsStatus != WPM_TEXT)
       return FALSE;

    strcpy(p->pszText,btn->text);

    return TRUE;

 }

 static BOOL setparm(HWND hwnd, PWNDPARAMS p)
 {
   ICQBUTTON *btn = WinQueryWindowPtr(hwnd,0);
   int       sz;

   if(!btn || p->fsStatus != WPM_TEXT)
      return FALSE;

   if(!p->pszText)
   {
      *(btn->text) = 0;
      return TRUE;
   }

   DBGMessage(p->pszText);
   sz = strlen(p->pszText)+1;

   if(sz > btn->maxText)
   {
      btn = realloc(btn,sz+sizeof(ICQBUTTON));
      if(!btn)
         return FALSE;
      WinSetWindowPtr(hwnd,0,btn);
      btn->maxText = sz;
   }

   strcpy(btn->text,p->pszText);

   btn->flags |= (BTNFLAG_REDRAW|BTNFLAG_PARMCHANGED);
   WinInvalidateRect(hwnd,NULL,FALSE);

   return TRUE;

 }

 static void seticon(HWND hwnd, USHORT icon)
 {
   ICQBUTTON *btn = WinQueryWindowPtr(hwnd,0);
   btn->icon   = icon;
   btn->flags |= (BTNFLAG_REDRAW|BTNFLAG_PARMCHANGED);
   WinInvalidateRect(hwnd,NULL,FALSE);
 }

 static void ppchanged(HWND hwnd, ULONG id)
 {
   ICQBUTTON *btn = WinQueryWindowPtr(hwnd,0);
   if(btn)
   {
      btn->flags |= (BTNFLAG_REDRAW|BTNFLAG_PARMCHANGED);
      icqskin_updatePresParam(hwnd,id,btn->pal);
   }
 }

 static int setimage(HWND hwnd, const char *img)
 {
    ICQBUTTON *btn = WinQueryWindowPtr(hwnd,0);

    if(icqskin_loadImage(btn->icq, img, &btn->imgBase, &btn->imgMask) < 1)
    {
       btn->imgBase = btn->imgMask = NO_IMAGE;
#ifndef USE_MMPM
       icqskin_loadImageFromResource(btn->icq, 1000+WinQueryWindowUShort(hwnd,QWS_ID), 37, 13, &btn->imgBase, &btn->imgMask);
#endif
    }

    btn->flags |= (BTNFLAG_REDRAW|BTNFLAG_PARMCHANGED);
    WinInvalidateRect(hwnd,NULL,FALSE);

    return btn->imgBase != NO_IMAGE;
 }

 static void setbackground(HWND hwnd, USHORT id, const char *filename)
 {
    ICQBUTTON *btn = WinQueryWindowPtr(hwnd,0);

//    DBGTrace(id);

    if(btn->baseImage[id] != NO_IMAGE)
    {
       GpiDeleteBitmap(btn->baseImage[id]);
       btn->baseImage[id] = NO_IMAGE;
    }

    btn->baseImage[id] = icqskin_loadBitmap(btn->icq, filename);

    btn->flags |= (BTNFLAG_REDRAW|BTNFLAG_PARMCHANGED);
    WinInvalidateRect(hwnd,NULL,FALSE);
 }

 const char * getSkinEntry(HWND hwnd, ICQBUTTON *cfg, const char *key, const char *name, SKINFILESECTION *fl)
 {
    const char *ptr;
    char       buffer[0x0100];

    sprintf(buffer,"%s.%s",cfg->name,key);
    ptr = icqskin_getSkinFileEntry(fl,name,buffer);

    if(ptr || (WinQueryWindowULong(hwnd,QWL_STYLE)&BS_NOBORDER))
       return ptr;

    sprintf(buffer,"Button.%s",key);
    return icqskin_getSkinFileEntry(fl,name,buffer);

 }

 static void loadSkin(hWindow hwnd, const char *name, SKINFILESECTION *fl)
 {
//    static const char *pallete[ICQCLR_COUNTER]    = { STANDARD_SKIN_PALLETE_NAMES };

    static const char *pallete[ICQCLR_COUNTER]    = { "Background", "Foreground", "DisabledBackground", "DisabledForeground" };
    static const char *imageName[BITMAP_COUNTER]  = { "Normal", "Pressed", "MouseOver", "Disabled" };

    char       buffer[0x0100];
    ICQBUTTON  *cfg = WinQueryWindowPtr(hwnd,0);
    const char *ptr;
    char       *tok;
    int        f;
    int        c;
    int        pos[]           = { 0,0,0,0 };
    ULONG      pal;

//    DBGMessage(name);
//    DBGMessage(cfg->name);

    clear(hwnd, cfg);

    ptr = icqskin_getSkinFileEntry(fl,name,cfg->name);

    if(!ptr && (WinQueryWindowULong(hwnd,QWL_STYLE) & BS_AUTOSIZE) )
    {
       icqskin_hide(hwnd);
    }
    else
    {
       icqskin_show(hwnd);

       sprintf(buffer,"%s.Image",cfg->name);
       icqskin_setButtonImage(hwnd,icqskin_getSkinFileEntry(fl,name,buffer));

       if(ptr)
       {
          strncpy(buffer,ptr,0xFF);
          DBGMessage(buffer);

          tok = strtok(buffer, ",");
          for(f=0;f<4 && tok;f++)
          {
             while(*tok && isspace(*tok))
                tok++;

             if(*tok == 'R')
                pos[f] = SIZE_FROM_IMAGE;
             else
                pos[f] = atoi(tok);
             tok    = strtok(NULL, ",");
          }

          for(f=0;f<BITMAP_COUNTER;f++)
          {
             if(tok)
             {
                DBGMessage(tok);
                icqskin_setButtonBackground(hwnd,f,tok);
                tok = strtok(NULL,",");
             }
          }

          icqskin_setButtonSize(hwnd,pos[0],pos[1],pos[2],pos[3]);

          DBGTracex(cfg->baseImage[BITMAP_NORMAL] == NO_IMAGE);
          DBGTracex(cfg->baseImage[BITMAP_PRESSED] == NO_IMAGE);
          DBGTracex(cfg->baseImage[BITMAP_MOUSEOVER] == NO_IMAGE);

       }

       for(f=0;f<ICQCLR_COUNTER;f++)
       {
          ptr = getSkinEntry(hwnd, cfg, pallete[f], name, fl);

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
       }

       for(f=0;f<BITMAP_COUNTER;f++)
       {
          if(cfg->baseImage[f] == NO_IMAGE)
          {
             ptr = getSkinEntry(hwnd, cfg, imageName[f], name, fl);
             if(ptr)
                icqskin_setButtonBackground(hwnd,f,ptr);

          }
       }

       ptr = getSkinEntry(hwnd, cfg, "Font", name, fl);

       if(ptr)
          icqskin_setButtonFont(hwnd,ptr);

       WinPostMsg(hwnd,WMICQ_AUTOSIZE,0,0);
       WinInvalidateRect(hwnd,NULL,FALSE);

    }
 }

 static int querySizes(HWND hwnd, USHORT *xPtr, USHORT *yPtr)
 {
    POINTL           aptl[TXTBOX_COUNT];
    ICQBUTTON        *cfg = WinQueryWindowPtr(hwnd,0);
    SKINDATA         *skn;
    HPS              hps;
    USHORT	         x    = 0;
    USHORT           y    = 0;
    BITMAPINFOHEADER bmpData;
    int				 f;

    if(cfg->icon != NO_ICON && cfg->icq)
    {
       skn  = icqskin_getDataBlock(cfg->icq);
       if(skn)
          x = skn->iconSize+6;
    }

    if( *cfg->text )
    {
       hps = WinGetPS(hwnd);

       GpiQueryTextBox(      hps,
                             strlen(cfg->text),
                             (char *) cfg->text,
                             TXTBOX_COUNT,
                             aptl);

       WinReleasePS(hps);

       f = aptl[TXTBOX_TOPRIGHT].x + 12;

       if(f>x)
          x=f;

       f = (aptl[TXTBOX_TOPRIGHT].y - aptl[TXTBOX_BOTTOMRIGHT].y)+6;

       if(f>y)
          y = f;

    }

    for(f=0;f<BITMAP_COUNTER && cfg->baseImage[f] == NO_IMAGE;f++);

    if(f < BITMAP_COUNTER)
    {
       memset(&bmpData,0,sizeof(bmpData));
       GpiQueryBitmapParameters(cfg->baseImage[f], &bmpData);
       y = bmpData.cy;
    }

/*
#ifdef DEBUG
    if(y < 1)
    {
       DBGTrace(y);
       DBGTrace(WinQueryWindowUShort(hwnd,QWS_ID))
    }
#endif
*/

    if(xPtr)
       *xPtr = x;

    if(yPtr)
       *yPtr = y;

    return x;
 }



