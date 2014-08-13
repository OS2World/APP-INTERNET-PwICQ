/*
 * SKIN.H - Definicoes para a classe de teste de skinning
 */

#ifndef SKINKRNL_INCLUDED

 #define INCL_WIN
 #include <os2.h>
 #include <pwMacros.h>

 #define SKINKRNL_INCLUDED

 #include <skindefs.h>

/*---[ Definicoes ]---------------------------------------------------------------*/

 #define POS_CENTRALIZE         0xFFFF

 /* Bitmap id's */
 #define BITMAP_TOPLEFT         b[0]
 #define BITMAP_TOPCENTER       b[1]
 #define BITMAP_TOPRIGHT        b[2]
 #define BITMAP_MIDLEFT         b[3]
 #define BITMAP_MIDCENTER       b[4]
 #define BITMAP_MIDRIGHT        b[5]
 #define BITMAP_BOTTOMLEFT      b[6]
 #define BITMAP_BOTTOMCENTER    b[7]
 #define BITMAP_BOTTOMRIGHT     b[8]

 #define BITMAP_BOTTOMMIDLEFT   b[9]
 #define BITMAP_BOTTOMMIDRIGHT  b[10]

 #define BITMAP_TITLELEFT       b[11]
 #define BITMAP_TITLECENTER     b[12]
 #define BITMAP_TITLERIGHT      b[13]

 #define BITMAP_MENU            b[14]

 #define BITMAP_BACKGROUND      b[15]

 #define BITMAPS_MAINWIN        16

 #define ICONBARS               1

 #define XBITMAP_LOGO           xbm[0]
 #define XBITMAP_LOGO_POS       pxbm[0]
 #define XBITMAPS               1

 #define BTNPOS_XCLOSE          d[0]
 #define BTNPOS_YCLOSE          d[1]
 #define BTNPOS_MAINWIN         2

 #pragma pack(1)

 typedef struct _xbitmap
 {
    HBITMAP     masc;
    HBITMAP     image;
 } XBITMAP;

 typedef struct _skinconfig
 {

    BOOL        loaded;

    USHORT      cx;
    USHORT      cy;

    ULONG       color[SKC_MAXCOLORS];

    USHORT      d[BTNPOS_MAINWIN];

    POINTL      maxSize;
    POINTL      minSize;

    UCHAR       drgMode;
    POINTL      drgPos;

    XBITMAP     iconbar[ICONBARS];

    XBITMAP     xbm[XBITMAPS];
    POINTL      pxbm[XBITMAPS];

    HBITMAP     b[BITMAPS_MAINWIN];

    char        *menuFont;

 } SKINCONFIG;

 typedef SKINCONFIG * HSKIN;

 #define BITMAPS_BUTTON 4
 typedef struct _skinbutton
 {
    XBITMAP     btm[BITMAPS_BUTTON];
    HBITMAP     bg;                     /* Background */
    CHAR        current;                /* Current selected bitmap */
    ULONG       dragPos;
    ULONG       flags;                  /* Button Flags */
    SHORT       x;
    SHORT       y;
    SHORT       cx;

    SHORT       repeatTime;

    ULONG       hItem;                  /* Item Handle */
    UCHAR       *text;                  /* Button text */

    HWND        hMenu;                  /* Popup menu  */

    HWND        hConnect;
    ULONG       mConnect;

 } SKINBUTTON;

 #define BITMAPS_SLIDER 4
 typedef struct _sliderdata
 {
    ULONG       pos;
    ULONG       items;
    SHORT       x;
    SHORT       y;
    SHORT       cx;
    SHORT       cy;
    SHORT       yCursor;

    BOOL        drag;

    HBITMAP     bg;                     /* Background */
    XBITMAP     btm[BITMAPS_SLIDER];

    LONG        minVal;
    LONG        maxVal;
    LONG        selVal;

    HWND        hConnect;
    ULONG       mConnect;

    CHAR        current;                /* Current selected bitmap */

 } SLIDERDATA;

 typedef struct _itnData
 {
    ULONG       hItem;                          /* Item handle  */
    ULONG       handler;                        /* Menu handler */
    ULONG       itemID;                         /* Item ID      */
    USHORT      yTop;
    USHORT      yBottom;
    USHORT      xLeft;
    USHORT      xRight;
 } ITNDATA;

 typedef struct _lbdata
 {
    SHORT       itens;                          /* Used Itens      */
    SHORT       tSize;                          /* Table size      */
    ITNDATA     *list;                          /* Itens list      */
    SHORT       selected;                       /* Selected item   */
    SHORT       top;                            /* Top item        */
    SHORT       bottom;                         /* Bottom item     */

    ULONG       drawFlags;                      /* Drawing Flags   */

    ULONG       flags;                          /* Flags           */
    #define LBF_BUTTONDOWN      0x00000001
    #define LBF_BUTTON2         0x00000002      /* Button 2 active */
    #define LBF_REDRAW0         0x00000004      /* Recria imagem 0 */
    #define LBF_REDRAW1         0x00000008      /* Recria imagem 1 */
    #define LBF_DRAG		0x00000010	/* Drag ativo	   */

    SHORT       szImage;                        /* Size of the image part */
    SHORT       x;
    SHORT       y;
    SHORT       cx;
    SHORT       cy;

    HWND        hConnect;
    ULONG       mConnect;

    HWND        hMenu;                          /* Popup menu */

    int         (* _System reorder)(ULONG, ULONG);

    HBITMAP     bg;
    HBITMAP     cache[2];

 } LBDATA;

 typedef struct _mnudata
 {
    SHORT       itens;                          /* Used Itens           */
    SHORT       tSize;                          /* Table size           */
    ITNDATA     *list;                          /* Itens list           */
    SHORT       selected;                       /* Selected item        */

    ULONG       flags;                          /* Flags */

    HWND        owner;
    USHORT      id;

    HBITMAP     bg;

 } MNUDATA;

/*---[ Flags de botao ]-----------------------------------------------------------*/

#define BTNFLAG_RESIZEABLE      0x00000001

/*---[ Constantes ]---------------------------------------------------------------*/

 extern const char              *buttonWindowName;
 extern const char              *scbarWindowName;
 extern const char              *listboxWindowName;
 extern const char              *menuWindowName;

/*---[ Processamento de janelas ]-------------------------------------------------*/

 MRESULT EXPENTRY       btnProcedure(HWND, ULONG, MPARAM, MPARAM);
 MRESULT EXPENTRY       scbProcedure(HWND, ULONG, MPARAM, MPARAM);
 MRESULT EXPENTRY       lbsProcedure(HWND, ULONG, MPARAM, MPARAM);
 MRESULT EXPENTRY       mnuProcedure(HWND, ULONG, MPARAM, MPARAM);

 HWND                   createMainWindow(const char *);

/*---[ Prototipos ]---------------------------------------------------------------*/

 #include <skinapi.h>

 HBITMAP LoadBitmap( HAB, HDC, HPS, PSZ );
 HBITMAP createBaseImage(HWND, USHORT, USHORT);

 void    paintWindow(HWND, HBITMAP, HBITMAP);

 void    loadTransparent( HAB, HDC, HPS, XBITMAP *, PSZ );
 BOOL    validXBitmap(XBITMAP *);
 USHORT  drawIcon(HPS, XBITMAP *, USHORT, USHORT, USHORT);
 void    drawTransparent(HPS, PPOINTL, XBITMAP *);
 void    createControlBackground(HWND, PSWP, HBITMAP *, XBITMAP *);
 int     queryItemSize(HPS, HWND, USHORT, const char *, short, USHORT *);
 HWND    CreatePopupMenu(HWND, USHORT, ULONG);
 void    notifyClients(HWND, const SKINELEMENT *, ULONG, MPARAM, MPARAM);
 void    paintMainBackGround(HSKIN, HPS, USHORT, USHORT);
 void    ajustPresParam(HWND,ULONG);
 void    destroyImages(HSKIN);


#endif



