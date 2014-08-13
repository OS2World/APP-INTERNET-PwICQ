/*
 * pwICQGUI.H - Definitions for the common pwICQ GUI system
 */

#ifndef ICQGUI_INCLUDED

   #define ICQGUI_INCLUDED "2.0"

   #if defined(STANDARD_GUI) && defined(SKINNED_GUI)
      #define SKIN_FLAG ""
   #else
      #ifdef STANDARD_GUI
	     #define SKIN_FLAG "-std"
      #else
	     #define SKIN_FLAG "-skn"
	  #endif
   #endif

   #ifdef GTK2
      #include <gtk/gtk.h>
      #define PLUGIN_ID "gtk2" SKIN_FLAG
   #endif

   #ifdef GTK1
      #include <gtk/gtk.h>
	  #include <gdk-pixbuf/gdk-pixbuf.h>
      #define PLUGIN_ID "gtk1" SKIN_FLAG
   #endif

   #if defined(GTK1) || defined(GTK2)

   //   #define USE_PIXBUF_BACKGROUND
	
	  #define FONT_NORMAL        "8.Helvetica"
	  #define FONT_BOLD          "8.Helvetica.Bold"
      #define hWindow            GtkWidget *
	  #define MPARAM             void *
	  #define MRESULT            void *
	  #define WM_USER            0x0400
   #endif

   #ifdef __OS2__
      #define INCL_WIN
      #define hWindow            HWND
      #define hBitmap			 HBITMAP
	  #define FONT_NORMAL        "8.Helv"
	  #define FONT_BOLD          "8.Helv.Bold"
   #endif

   #ifdef GTK1	
      #define icqskin_setWindowManagerName(h,n) gtk_window_set_wmclass(GTK_WINDOW(h),"pwICQ", n)
 	  #define icqskin_setEditable(h)			gtk_text_set_editable(GTK_TEXT(h),TRUE)
	  #define icqskin_setReadOnly(h)            gtk_text_set_editable(GTK_TEXT(h),FALSE)

   #endif	

   #ifdef GTK2
      #define icqskin_setWindowManagerName(h,n) gtk_window_set_role(GTK_WINDOW(h),n)
	  #define icqskin_setEditable(h)            gtk_text_view_set_editable(GTK_TEXT_VIEW(h),TRUE)
	  #define icqskin_setReadOnly(h)            gtk_text_view_set_editable(GTK_TEXT_VIEW(h),FALSE)
   #endif	

   #ifdef __OS2__
      #define icqskin_queryTaskICON(i)    icqLoadValue(i,"TaskICON",1)
      #define icqskin_queryTaskTITLE(i)   icqLoadValue(i,"TaskTITLE",1)
   #else
      #define icqskin_queryTaskICON(i)    icqLoadValue(i,"TaskICON",2)
      #define icqskin_queryTaskTITLE(i)   icqLoadValue(i,"TaskTITLE",2)
   #endif

/*---[ Includes ]-------------------------------------------------------------*/

   #include <icqtlkt.h>
   #include <stdio.h>

/*---[ Defines ]--------------------------------------------------------------*/

   #pragma pack(1)

#ifdef SKINNED_GUI
   enum ICQFRAME_BITMAPS
   {
      ICQFRAME_TITLELEFT,
      ICQFRAME_TITLESTRETCH,
      ICQFRAME_TITLERIGHT,

      ICQFRAME_TOPLEFT,
      ICQFRAME_TOPRIGHT,

      ICQFRAME_MIDLEFT,
      ICQFRAME_MIDSTRETCH,
      ICQFRAME_MIDRIGHT,

      ICQFRAME_BOTTOMLEFT,
      ICQFRAME_BOTTOMMIDLEFT,
      ICQFRAME_BOTTOMSTRETCH,
      ICQFRAME_BOTTOMMIDRIGHT,
      ICQFRAME_BOTTOMRIGHT,

      ICQFRAME_SKINBITMAPS
   };
#endif


   enum MAIN_WINDOW_IDS
   {
      ID_SYSMSG,	 	 	/* System message button   		*/
      ID_CONFIG,         	/* Configuration button    		*/
      ID_MODE,           	/* Mode selection          		*/
      ID_SEARCH,         	/* Search button           		*/
      ID_USERLIST,       	/* User's list             		*/
      ID_SCROLLBAR,	     	/* User's list scrollbar   		*/

      MAIN_WINDOW_COMPONENTS
   };

   enum MSG_WINDOW_IDS
   {
      MSGID_CLOSEBUTTON,		/*  0 Close button               */
      MSGID_SENDBUTTON,			/*  1 Send message               */
      MSGID_ADDBUTTON,			/*  2 Add user in contact-list   */
      MSGID_ABOUTBUTTON,		/*  3 About user button          */
      MSGID_INFOBUTTON,         /*  4 User info button           */
      MSGID_REPLYBUTTON,        /*  5 Reply button               */
      MSGID_ACCEPTBUTTON,       /*  6 Accept button              */
      MSGID_REFUSEBUTTON,       /*  7 Refuse button              */
      MSGID_BROWSEBUTTON,       /*  8 Browse button              */
      MSGID_NEXTBUTTON,         /*  9 Read next message          */

      MSGID_USERMODE,           /* 10 User mode icon             */
      MSGID_NICK,				/* 11 Static field nickname:	 */
      MSGID_NICKENTRY,			/* 12 Entry field nickname		 */
      MSGID_NAME,				/* 13 Static field name: 		 */
      MSGID_NAMEENTRY,			/* 14 Entry field name   		 */
      MSGID_SENDTO,             /* 15 Send to button             */

      MSGID_EDITBOX,			/* 16 Edit box                   */
      MSGID_HISTORY,			/* 17 History list box			 */
	  MSGID_NAMEBOX,            /* 18 User Information box       */
	
#ifdef linux
      MSGID_PANEL,              /* 19 Panel                      */
#endif
   	
      MSG_WINDOW_COMPONENTS
   };

   #define MSGID_ENTRY   MSGID_HISTORY

   #define MSGID_SAVE    MSGID_SENDBUTTON
   #define MSGID_RELOAD  MSGID_SENDTO
   #define MSGID_REFRESH MSGID_ADDBUTTON
   #define MSGID_MESSAGE MSGID_INFOBUTTON
   #define MSGID_INFOBOX MSGID_EDITBOX
   #define MSGID_OPTIONS MSGID_HISTORY

   #define MSGID_TITLE   MSGID_BROWSEBUTTON

   #define STANDARD_FRAME_ELEMENTS     "TitleLeft","TitleStretch","TitleRight","TopLeft","TopRight","MidLeft","MidStretch","MidRight","BottomLeft","BottomMidLeft","BottomStretch","BottomMidRight","BottomRight"
   #define STANDARD_SKIN_PALLETE_NAMES "Background", "Foreground", "SelectedBackground", "SelectedForeground"
   #define STANDARD_MENU_NAMES         "Menu.User", "Menu.OnlineMode", "Menu.Configuration"

   enum WINDOW_COLORS
   {
      ICQCLR_BACKGROUND, /* Window background color */
      ICQCLR_FOREGROUND, /* Window foreground color */
      ICQCLR_SELECTEDBG,
      ICQCLR_SELECTEDFG,

      ICQCLR_COUNTER
   };

   #define icqDeclareUserlistEventProcessor(e)     void icqUserlistEvent_##e(HICQ,HWND,ULONG)
   #define icqUserlistEventProcessor(t,e)          { t, e, (void(*)(HICQ,HWND,ULONG)) icqUserlistEvent_##e }
   #define icqBeginUserlistEventProcessor(m,h,i,u) void icqUserlistEvent_##m(HICQ i, HWND h,ULONG u)

   #define ICQULIST_SORT    0x00000001
   #define ICQULIST_BLINK   0x00000002
   #define ICQULIST_SKINNED 0x00000004
   #define ICQULIST_POPUP   0x00000008

   #if defined(GTK1) || defined(GTK2)
      #define DECLARE_WND_PROC(x) MRESULT _System x(HWND, ULONG, MPARAM, MPARAM);
      #define ICQGUI_STANDARD_PREFIX    USHORT sz;HICQ icq;MRESULT (_System *msg)(HWND,ULONG,MPARAM,MPARAM);const char *name;
      #define ICQFRAME_STANDARD_PREFIX  ICQGUI_STANDARD_PREFIX UCHAR ft;
   #endif

   #ifdef __OS2__
      #define DECLARE_WND_PROC(x) MRESULT EXPENTRY x(HWND, ULONG, MPARAM, MPARAM);
      #define ICQGUI_STANDARD_PREFIX   USHORT sz;HICQ icq;const char *name;
      #define ICQFRAME_STANDARD_PREFIX ICQGUI_STANDARD_PREFIX UCHAR ft;
   #endif

   #ifdef WM_USER

      #define WMICQ_SETSIZE      WM_USER+1000
      #define WMICQ_AUTOSIZE     WM_USER+1001
      #define WMICQ_SETCALLBACK  WM_USER+1002
      #define WMICQ_SETICQHANDLE WM_USER+1003
      #define WMICQ_SETICON      WM_USER+1004
      #define WMICQ_SETIMAGE     WM_USER+1005
      #define WMICQ_SIZECHANGED  WM_USER+1006
      #define WMICQ_SETNAME      WM_USER+1007
      #define WMICQ_SETBGIMAGE   WM_USER+1008
      #define WMICQ_SETBUTTONTBL WM_USER+1009
      #define WMICQ_SETPALLETE   WM_USER+1010
      #define WMICQ_QUERYPALLETE WM_USER+1011
      #define WMICQ_INSERTGROUP  WM_USER+1012
      #define WMICQ_ADDUSER      WM_USER+1013
      #define WMICQ_SORTLIST     WM_USER+1014
      #define WMICQ_SETTREEIMAGE WM_USER+1015
      #define WMICQ_MEASURE      WM_USER+1016
      #define WMICQ_SETSELECTBAR WM_USER+1017
      #define WMICQ_LOADSKIN     WM_USER+1018
      #define WMICQ_QUERYBGIMAGE WM_USER+1019
      #define WMICQ_PAINTBG      WM_USER+1020
      #define WMICQ_SETFLAG      WM_USER+1021
      #define WMICQ_GETFLAG      WM_USER+1022
      #define WMICQ_UPDATEICON   WM_USER+1023
      #define WMICQ_GETFLAGADDR  WM_USER+1024
      #define WMICQ_SWITCHFLAG   WM_USER+1025
      #define WMICQ_BLINK        WM_USER+1026
      #define WMICQ_CLEAR        WM_USER+1027
      #define WMICQ_QUERYUSER    WM_USER+1028
      #define WMICQ_UPDATEUSER   WM_USER+1029
      #define WMICQ_DELUSER      WM_USER+1030
      #define WMICQ_CONNECT      WM_USER+1031
      #define WMICQ_RESTORE      WM_USER+1032
      #define WMICQ_STORE        WM_USER+1033
      #define WMICQ_CONFIGURE    WM_USER+1034
      #define WMICQ_QUERYSIZES   WM_USER+1035
      #define WMICQ_CLICKED      WM_USER+1036
      #define WMICQ_SELECTPAL    WM_USER+1037
      #define WMICQ_SETMSGCHILD  WM_USER+1038
      #define WMICQ_SIZEMSGBOX   WM_USER+1039
      #define WMICQ_SIZEBUTTONS  WM_USER+1040
      #define WMICQ_SHOWMENU     WM_USER+1041
      #define WMICQ_CREATECHILDS WM_USER+1042
      #define WMICQ_SETUSERID    WM_USER+1043
      #define WMICQ_SKINCHILDS   WM_USER+1044
      #define WMICQ_SKINELEMENT  WM_USER+1045
      #define WMICQ_DRAWCONTENTS WM_USER+1046
      #define WMICQ_SETMARGIN    WM_USER+1047
      #define WMICQ_GETICQHANDLE WM_USER+1048
      #define WMICQ_SETDIALOG    WM_USER+1049
      #define WMICQ_SETTYPE      WM_USER+1050
      #define WMICQ_INITIALIZE   WM_USER+1051
      #define WMICQ_LOAD         WM_USER+1052
      #define WMICQ_INVALIDATE   WM_USER+1053
      #define WMICQ_SAVE         WM_USER+1054
      #define WMICQ_REDRAW		 WM_USER+1055
      #define WMICQ_CHANGED      WM_USER+1056
      #define WMICQ_DLGEVENT     WM_USER+1057
      #define WMICQ_GETUSERID    WM_USER+1058
	
   #endif

/*---[ Structures ]-----------------------------------------------------------*/

#ifdef __cplusplus
   extern "C" {
#endif

   struct icqButtonTable			/* Button description table */
   {
	  UCHAR      section;
      USHORT     id;
      USHORT     icon;
      short      x;
      short      y;
      short      cx;
      short      cy;
      const char *name;
      void       (ICQCALLBACK *cbk)(hWindow,void *);
      const char *text;
      const char *font;
   };

   typedef struct _skinfilerecord
   {
      struct _skinfilerecord *up;
      struct _skinfilerecord *down;
      char                   key[1];
   } SKINFILERECORD;

   typedef struct _skinfilesection
   {
      struct _skinfilesection *up;
      struct _skinfilesection *down;

      SKINFILERECORD *first;
      SKINFILERECORD *last;

      char                    tag[1];

   } SKINFILESECTION;

#if defined(GTK1) || defined(GTK2)

   typedef struct _icqframe				/* Frame window data (GTK) */
   {
      ICQFRAME_STANDARD_PREFIX
	
	  /* Flags */
	  ULONG   flags;
	  #define ICQFRAME_FLAG_TOPWINDOW 0x00000001
	  #define ICQFRAME_RESIZED        0x00000002
	  #define ICQFRAME_FLAG_BORDER    0x00000004
	
      /* Window Positioning */
      short   x;
      short   y;
      short   cx;
      short   cy;
	
      /* Window information */	
#ifdef SKINNED_GUI
	  hWindow   hwnd;
	  GdkGC     *gc;
	  GdkPixmap *bg;
      GdkPixbuf *base[ICQFRAME_SKINBITMAPS];
#endif
	
   } ICQFRAME;

   typedef struct _buttonlist
   {
      USHORT     id;
      const char *wText;
   } BUTTONLIST;

#endif

#ifdef __OS2__

   typedef  struct _msgchild
   {
      const char *name;
      USHORT     id;
      const char *wClass;
      const char *wText;
      const char *font;
      ULONG      wStyle;
   } FRAMECHILD;

   #define  MSGCHILD FRAMECHILD

   typedef struct _icqframe			
   {
      ICQFRAME_STANDARD_PREFIX

      // Window buttons
      const struct icqButtonTable *btn;
      int                         btnCount;

      // Sizing information
      USHORT                      xPos;
      USHORT                      yPos;
      USHORT                      maxCx;
      USHORT                      maxCy;
      USHORT                      minCx;
      USHORT                      minCy;

      short  					  xMouse;
      short                       yMouse;
      ULONG                       timer;

      // Frame information
      ULONG                       flags;
      #define					  ICQFRAME_FLAG_SETCX 0x00000001
      #define				      ICQFRAME_FLAG_SETCY 0x00000002
      #define                     ICQFRAME_FLAG_MOVEY 0x00000004
      #define                     ICQFRAME_FLAG_MOVEX 0x00000008

      USHORT					  iconID;
      HPOINTER					  icon;

      // Skin information
      ULONG                       pal[ICQCLR_COUNTER];

#ifdef SKINNED_GUI
      HBITMAP                     bg;
      HBITMAP                     img[ICQFRAME_SKINBITMAPS];
      HBITMAP                     msk[ICQFRAME_SKINBITMAPS];
#endif
      int                         childCount;
      const FRAMECHILD            *childList;


   } ICQFRAME;

#endif


   typedef struct _icqconfigdialog
   {
      ICQFRAME                fr;

      struct _icqconfigdialog *up;
      struct _icqconfigdialog *down;

      hWindow                 hwnd;
      hWindow				  dialog;

      USHORT                  yTop;

      struct _icqconfigchild
      {
         USHORT x;
         USHORT y;
         USHORT cx;
         USHORT cy;
      }                       dlg;

#ifdef __OS2__
      USHORT                  treeIconSize;
      HBITMAP                 noDialog;
      HBITMAP 				  treeImg;
      HBITMAP 				  treeMsk;
#endif

#if defined(GTK1) || defined(GTK2)
	  hWindow                 noDialog;
	  hWindow                 dlgBox;
	  GdkGC                   *gc;
#endif	
	
      ULONG                   uin;
      USHORT                  type;

   } ICQCONFIGDIALOG;

   typedef struct _icqmsgdialog
   {
      ICQFRAME        fr;

      struct          _icqmsgdialog *up;
      struct          _icqmsgdialog *down;

	  hWindow         hwnd;
	  hWindow         sendToMenu;
	  ULONG           uin;
	
	  ULONG           flags;
                	  #define ICQMSGDIALOG_OUT  0x00000001
                	  #define ICQMSGDIALOG_LOAD 0x00000002
                	  #define ICQMSGDIALOG_CHAT 0x00000004
                	  #define ICQMSGDIALOG_HIDE 0x00000008
                	  #define ICQMSGDIALOG_SHOW 0x00000010
	
	  ULONG           sequenceNumber;
	  USHORT          idleSeconds;

#ifdef __OS2__
      HPOINTER        msgIn;
      HPOINTER        msgOut;
#endif
	
#if defined(GTK1) || defined(GTK2)
	  hWindow           hstLast;
	  hWindow           hstSelected;
	  GtkScrolledWindow *scBar;
#endif
	
	  const MSGMGR    *mgr;
	  char            sendToParm[0x0100];
	  char            buffer[0x0100];
	
   } ICQMSGDIALOG;

   typedef struct _skindata			/* Skin definition */
   {
      USHORT           sz;
      HICQ             icq;

	  ULONG            flags;
	  #define          SKINDATA_FLAGS_LOADED     0x00000001
	  #define          SKINDATA_FLAGS_CONNECTING 0x00000002
	  #define          SKINDATA_FLAGS_BLINK      0x00000004
	  #define          SKINDATA_FLAGS_HOOKED     0x00000008
	
	  USHORT           animate;
	
	  USHORT           iconSize;
	  USHORT           eventIcon;
	  USHORT           readIcon;
	
	  hWindow          frame;
      hWindow          mainWindow;
	  hWindow          userPopup;
	  hWindow          menu[ICQMNU_COUNTER];
	
	  ICQMSGDIALOG     *firstMsgWin;
	  ICQMSGDIALOG     *lastMsgWin;
	
	  ICQCONFIGDIALOG  *firstCfgWin;
	  ICQCONFIGDIALOG  *lastCfgWin;
	
	  SKINFILESECTION  *skin;

#ifdef __OS2__
      USHORT           tick;
      HACCEL           accTable;
      HBITMAP          iconImage;
      HBITMAP          iconMasc;
      hWindow          TrayServer;
#endif

#if defined(GTK1) || defined(GTK2)
      USHORT           tick;
	  HLIST            pendingEvents;
      GdkPixbuf        *icons[PWICQICONBARSIZE];
      GdkPixmap	       *iPixmap[PWICQICONBARSIZE];
      GdkBitmap        *iBitmap[PWICQICONBARSIZE];
#endif
	
   } SKINDATA;

   typedef struct _userbutton
   {
      USHORT id;
      USHORT icon;
   } ICQUSERBUTTON;

   typedef struct _icqstatic
   {
      ICQGUI_STANDARD_PREFIX

#if defined(GTK1) || defined(GTK2)
      GdkPixbuf  *baseImage;
      GdkPixbuf  *bg;

#ifdef SKINNED_GUI
	  USHORT     cx;
	  USHORT     cy;
	  GdkGC      *gc;
#endif	
	
#endif	
	
#ifdef __OS2__
      USHORT     icon;
      ULONG      pal[ICQCLR_COUNTER];
      HBITMAP    baseImage;
      HBITMAP    bg;
      USHORT     xLeft;
      USHORT     maxText;
#endif	

  } ICQSTATIC;

   /*---[ Public information blocks ]-------------------------------------------------------------------*/

   extern const struct icqButtonTable icqgui_MainButtonTable[];
   extern const int                   icqgui_MainButtonTableElements;

   extern const DIALOGCALLS           icqskin_dialogCalls;
   extern HMODULE                     module;

   extern const ICQUSERBUTTON         icqgui_msgUserBoxButtonTable[];
   extern const int                   icqgui_msgUserBoxButtonTableElements;

   extern const ICQUSERBUTTON         icqgui_cfgUserBoxButtonTable[];
   extern const int                   icqgui_cfgUserBoxButtonTableElements;

   extern const char                  *icqskin_configsections[CFGWIN_USER];

   #ifdef MAKELIB
      extern SKINDATA 				  icqskin_dataBlock;
      #define icqskin_getDataBlock(i) &icqskin_dataBlock
   #else
      #define icqskin_getDataBlock(i) icqGetSkinDataBlock(i)
      extern const SKINMGR            icqgui_descr;
   #endif

   /*---[ Skin Manager entry points ]-------------------------------------------------------------------*/

   SKINDATA * _System icqskin_Start(HICQ, ULONG);
   int        _System icqskin_Stop(HICQ, SKINDATA *);
   #ifdef STANDARD_GUI
      int _System icqskin_CreateStandardWindow(HICQ, SKINDATA *);
   #endif

   #ifdef SKINNED_GUI
      int _System icqskin_CreateSkinnedWindow(HICQ, SKINDATA *);
   #endif

   /*---[ Common functions ]--------------------------------------------------*/

   void             icqskin_Initialize(HICQ, SKINDATA *);
   int              icqskin_loadIcons(SKINDATA *, const char *);
   int              icqskin_event(HICQ, ULONG, UCHAR, USHORT, ULONG);
   void             icqskin_loadUsersList(hWindow, HICQ);
   int              icqskin_queryImage(HICQ, const char *, int, char *);

   /* Frame window */
   int              icqskin_setFrameIcon(hWindow, SKINDATA *, USHORT);
   int              icqskin_setFrameTitle(hWindow, const char *);

   /* Message processing */
   void ICQCALLBACK icqskin_viewMessage(HICQ, ULONG, HMSG);
   int  ICQCALLBACK icqskin_newMessage(HICQ, ULONG, USHORT, const char *, const char *);
   int  ICQCALLBACK icqskin_newWithMgr(HICQ, ULONG, const MSGMGR *, HMSG);

   /* Message dialog */
   hWindow          icqskin_createChatWindow(HICQ);
   hWindow          icqskin_createPagerWindow(HICQ);
   void             icqskin_setMessageManager(hWindow, const MSGMGR *);
   void             icqskin_setMessageHandle(hWindow, HMSG);
   void             icqskin_setMessageUser(hWindow, ULONG);
   int              icqskin_insertMessageHistory(HWND, ULONG, const char *, const char *);
   void             icqskin_setMsgButtonCallbacks(hWindow);
   int              icqskin_destroyMsgWindow(ICQMSGDIALOG *);

   /* Buttons */
   int              icqskin_createButtons(hWindow, SKINDATA *, const struct icqButtonTable *, int);

   /* User list */
   hWindow          icqSkin_createStandardUserList(HICQ, hWindow, USHORT);

   /* Misc window */
   hWindow          icqSkin_createStandardScrollBar(HICQ, hWindow, USHORT);
   int              icqskin_queryClipboard(HICQ, int, char *);
   int              icqskin_postEvent(HICQ, ULONG, char, USHORT, ULONG);
   int              icqskin_setVisible(SKINDATA *, BOOL);

   /* Menu */
   hWindow          icqskin_CreateMenu(HICQ, USHORT, SKINDATA *);
   void             icqskin_setMenuTitle(HICQ, USHORT, const char *);

   /* Entry points */
   int     _System  icqskin_insertMenu(HICQ, USHORT, const char *, USHORT, MENUCALLBACK *, ULONG);
   hWindow _System  icqskin_openConfig(HICQ, ULONG, USHORT);

   /*---[ OS Dependent functions ]--------------------------------------------*/

   #ifdef GTK2
      #define icqskin_activate(h)                gtk_window_present(GTK_WINDOW(h))
   #else
      #define icqskin_activate(h)				 /* h */
   #endif

   #if defined(GTK1) || defined(GTK2)

      #define DLG_ELEMENT_ID     0
	
	  /* Generic window */
      #define icqskin_terminate(i)                gtk_main_quit()
      #define icqskin_setEnabled(h,m)             gtk_widget_set_sensitive(h,m)


      #define icqskin_show(h)                     gtk_widget_show(h)
      #define icqskin_hide(h)                     gtk_widget_hide(h)
	  #define icqskin_closeWindow(h)              gtk_widget_destroy(h)
	  #define icqskin_getVisibility(h)            GTK_WIDGET_VISIBLE(h)
	
	  /* Frame */
      #define icqskin_showFrame(h)                gtk_widget_show(h)
      #define icqskin_hideFrame(h)                gtk_widget_hide(h)
      #define icqskin_setChildEnabled(h,i,f)      gtk_widget_set_sensitive(icqskin_getDlgItemHandle(h,i),f)
      #define icqskin_setMsgButtonCallback(h,i,c) gtk_signal_connect(GTK_OBJECT(icqskin_getDlgItemHandle(h,i)), "clicked", GTK_SIGNAL_FUNC(c), h)
      #define icqframe_getHandle(h)               ((ICQFRAME *) icqskin_getWindowDataBlock(h))->icq

      /* User list */	
      #define icqskin_sortUserList(h)             gtk_clist_sort(GTK_CLIST(h))
	  #define icqskin_switchBlinkStatus(h)        icqskin_switchFlag(h,ICQULIST_BLINK)
	
	  /* Button */
      #define icqskin_setButtonCallback(b,c,p)    gtk_signal_connect(GTK_OBJECT(b), "clicked", GTK_SIGNAL_FUNC(c), p)
      #define icqskin_setButtonFont(b,n)          /* */
      #define icqskin_setButtonName(b,n)          /* */
      #define icqskin_queryCheckstate(h)          0

      /* Resource management */
      #define icqskin_loadString(i,m,n,b,s)       icqGetString(i,m,n,b,s)

      /* Configuration */
      #define icqskin_createChilds(h)            /* */

      /* Functions */
	
      GdkPixbuf * icqskin_loadImage(HICQ, const char *);
      GtkWidget * gtk_pixmap_new_from_pixbuf(GdkPixbuf *);

/*      hWindow       icqskin_getDlgItemHandle(hWindow,USHORT); */

      #define icqskin_setDlgItemHandle(f, i, h) icqgui_setWindowID(h,i)
/*      int           icqskin_setDlgItemHandle(hWindow, USHORT, hWindow); */

      void          icqskin_insertUserGroup(hWindow,int,const char *);
	  void          icqskin_addUser(hWindow,HUSER);
      int           icqskin_setButtonText(hWindow, const char *);
      int           icqskin_setButtonIcon(hWindow, USHORT);
	
      ULONG *       icqskin_getUserListFlagPointer(hWindow);
      void          icqskin_setUserListFlag(hWindow,ULONG);
	  void          icqskin_switchFlag(hWindow,ULONG);
      void          icqskin_blinkUser(hWindow,ULONG,BOOL,BOOL);
      void          icqskin_updateUserIcon(hWindow,ULONG);

      HUSER         icqskin_queryUserListEntry(hWindow,ULONG);
      HUSER         icqskin_updateUserHandle(hWindow,ULONG);
      void          icqskin_delUser(hWindow,ULONG);

      int           icqskin_setICQHandle(hWindow, HICQ);

      void          icqskin_restoreFrame(hWindow, const char *, USHORT, USHORT);
	  void          icqskin_storeFrame(hWindow, const char *);

      int           icqskin_setFrameName(hWindow, const char *);

      /* Generic Window management */
      void *        icqskin_getWindowDataBlock(hWindow);
	  int           icqskin_setWindowDataBlock(hWindow,void *);
      void          icqskin_setFont(hWindow, const char *);
      void          icqskin_setDlgItemIcon(hWindow,USHORT,USHORT);
      void          icqskin_getWindowText(hWindow, int, char *);
      void          icqskin_setWindowText(hWindow,const char *);
	  void          icqskin_setChecked(hWindow);
	  void          icqskin_setUnchecked(hWindow);
	
	  /* Message Window */
	  int           icqskin_setMsgWindowText(hWindow,USHORT,const char *);
      void          icqskin_getMsgWindowText(hWindow,USHORT,int, char *);

      #define icqskin_getDlgItemHandle(h,i) icqframe_queryChild(h,i)

      hWindow       icqskin_createUserBox(hWindow, SKINDATA *, int, const ICQUSERBUTTON *);
      hWindow       icqskin_createButtonBar(HICQ, hWindow, int, const BUTTONLIST *);

      /* Configuration Window */
      void          icqskin_setWindowUserID(hWindow, ULONG);
      void          icqconfig_setWindowType(hWindow,USHORT);
      void          icqconfig_initialize(hWindow);
	  void          icqconfig_load(hWindow);
      void          icqconfig_save(hWindow);

      /* Menus */
      void          icqskin_showMenu(hWindow,ULONG,USHORT,USHORT);

      /* Static */
      #define icqstatic_setText(h,t)   gtk_label_set(GTK_LABEL(h), t)


   #endif

   #ifdef __OS2__

      #define DLG_ELEMENT_ID     500

      #define NO_ICON            0xFFFF

      #define WM_TRAYADDME       (WM_USER+1)
      #define WM_TRAYDELME       (WM_USER+2)
      #define WM_TRAYICON        (WM_USER+3)
      #define WM_TRAYEXIT        (0xCD20)

#ifdef MAKELIB
      #define WMICQ_SYSTEMEVENT  WM_USER+3000
      #define WMICQ_USEREVENT    WM_USER+3001
      #define WMICQ_GUIEVENT     WM_USER+3002
#endif

      /* Frame */
      #define icqskin_setFrameName(b,n)          WinSendMsg(b,WMICQ_SETNAME,(MPARAM) n, 0)
      #define icqskin_restoreFrame(h,n,x,y)      WinSendMsg(h,WMICQ_RESTORE,(MPARAM) n,MPFROM2SHORT(x,y))
      #define icqskin_storeFrame(h,n)            WinSendMsg(h,WMICQ_STORE,(MPARAM) n, 0)
      #define icqskin_createChilds(h)            WinSendMsg(h,WMICQ_CREATECHILDS,0,0)
      #define icqframe_getHandle(h)              ((HICQ) WinSendMsg(h,WMICQ_GETICQHANDLE,0,0))

      /* Window */
      #define icqskin_cfgWindow(h,i,p)           WinSetWindowPtr(h,0,malloc(sizeof(i))); WinSendMsg(h,WMICQ_CONFIGURE,MPFROM2SHORT(sizeof(i),0), mp2);

#ifdef DEBUG
      #define icqskin_closeWindow(h)             WinPostMsg(h,WM_CLOSE,0,0);DBGMessage("Posting WM_CLOSE");
#else
      #define icqskin_closeWindow(h)             WinPostMsg(h,WM_CLOSE,0,0);
#endif
      #define icqskin_queryBackground(h)         ((HBITMAP) WinSendMsg(h,WMICQ_QUERYBGIMAGE,0,0))
      #define icqskin_loadSkin(h,n,s)            WinSendMsg(h,WMICQ_LOADSKIN,(MPARAM) n, (MPARAM) s)

      #define icqskin_show(h)                    WinShowWindow(h,TRUE)
      #define icqskin_hide(h)                    WinShowWindow(h,FALSE)

      #define icqskin_showFrame(h)               WinSetWindowPos(WinQueryWindow(h,QW_PARENT), HWND_TOP, 0, 0, 0, 0, SWP_ZORDER|SWP_SHOW)
      #define icqskin_hideFrame(h)               WinShowWindow(WinQueryWindow(h,QW_PARENT),FALSE)
      #define icqskin_activate(h)                /* */

      #define icqskin_getWindowDataBlock(h)      WinQueryWindowPtr(h,0)
      #define icqskin_setWindowDataBlock(h,d)    WinSetWindowPtr(h,0,d)
      #define icqskin_querySizes(h,x,y)          WinSendMsg(h,WMICQ_QUERYSIZES,(MPARAM) x, (MPARAM) y)
      #define icqskin_setFont(h,f)               WinSetPresParam(h,PP_FONTNAMESIZE,(ULONG) strlen(f)+1,(PVOID) f);
      #define icqskin_selectPallete(h,p)         WinSendMsg(h, WMICQ_SELECTPAL,(MPARAM) p,0)
      #define icqskin_setDlgItemIcon(b,i,n)      WinSendDlgItemMsg(b,DLG_ELEMENT_ID+i,WMICQ_SETICON,MPFROMSHORT(n), 0)
      #define icqskin_setMsgWindowChilds(h,x)    WinSendMsg(h,WMICQ_SETMSGCHILD,(MPARAM) x, MPFROMSHORT((sizeof(x)/sizeof(MSGCHILD))));
      #define icqskin_setEnabled(h,m)            WinEnableWindow(h,m);
	  #define icqskin_getVisibility(h)           WinIsWindowVisible(h)

      /* Message window */
      #define icqskin_setMsgWindowText(h,i,t)     WinSetWindowText(WinWindowFromID(h,DLG_ELEMENT_ID+i),(PSZ) t)
      #define icqskin_getMsgWindowText(h,i,s,t)   WinQueryDlgItemText(h,DLG_ELEMENT_ID+i,s,t)
      #define icqskin_setMsgButtonCallback(h,i,c) WinPostMsg(WinWindowFromID(h,DLG_ELEMENT_ID+i),WMICQ_SETCALLBACK,(MPARAM) c, (MPARAM) h)
      #define icqskin_resizeUserInfoBox(h,x,y)    ((USHORT) WinSendMsg(h,WMICQ_SIZEMSGBOX,MPFROM2SHORT(x,y),0))
      #define icqskin_setMsgWindowChilds(h,x)     WinSendMsg(h,WMICQ_SETMSGCHILD,(MPARAM) x, MPFROMSHORT((sizeof(x)/sizeof(MSGCHILD))));
      #define icqskin_resizeButtonBox(h,x,l)      ((USHORT) WinSendMsg(h,WMICQ_SIZEBUTTONS,MPFROM2SHORT(x,(sizeof(l)/sizeof(USHORT))),(MPARAM) l))
      #define icqskin_setChildEnabled(h,i,f)      WinEnableControl(h, DLG_ELEMENT_ID+i, f)
 	  #define icqskin_setEditable(h)			  WinPostMsg(h,MLM_SETREADONLY,0,0)
	  #define icqskin_setReadOnly(h)              WinPostMsg(h,MLM_SETREADONLY,MPFROMSHORT(TRUE),0)

      /* Configuration window */
      #define icqskin_setWindowUserID(h,u)        WinSendMsg(h,WMICQ_SETUSERID,(MPARAM) u, 0)
      #define icqskin_loadString(i,m,n,b,s)       WinLoadString(icqQueryAnchorBlock(i),m,n,s,b)
      #define icqconfig_setWindowType(h,t)        WinSendMsg(h,WMICQ_SETTYPE,(MPARAM) t, 0)
      #define icqconfig_initialize(h)             WinPostMsg(h,WMICQ_INITIALIZE,0,0)
      #define icqconfig_load(h)                   WinPostMsg(h,WMICQ_LOAD,0,0)
      #define icqconfig_save(h)                   WinPostMsg(h,WMICQ_SAVE,0,0)

      /* Button */
      #define icqskin_setButtonCallback(b,c,p)    WinPostMsg(b,WMICQ_SETCALLBACK,(MPARAM) c, (MPARAM) p)
      #define icqskin_setButtonIcon(b,i)          WinSendMsg(b,WMICQ_SETICON, MPFROMSHORT(i), 0)
      #define icqskin_setButtonImage(b,i)         WinSendMsg(b,WMICQ_SETIMAGE,(MPARAM) i, 0)
      #define icqskin_setButtonFont(b,n)          WinSetPresParam(b,PP_FONTNAMESIZE,(ULONG) strlen(n)+1,(PVOID) n);
      #define icqskin_setButtonName(b,n)          WinSendMsg(b,WMICQ_SETNAME,(MPARAM) n, 0)
      #define icqskin_setButtonBackground(b,m,n)  WinSendMsg(b,WMICQ_SETBGIMAGE,MPFROMSHORT(m), (MPARAM) n)
      #define icqskin_setChecked(h)               WinSendMsg(h,BM_SETCHECK,MPFROMSHORT(1),0)
      #define icqskin_setUnchecked(h)             WinSendMsg(h,BM_SETCHECK,MPFROMSHORT(0),0)
      #define icqskin_queryCheckstate(h)          ((int) WinSendMsg(h,BM_QUERYCHECK,0,0))

      /* Menus */
      #define icqskin_showMenu(h,u,x,y)           WinPostMsg(h,WMICQ_SHOWMENU,(MPARAM) u,MPFROM2SHORT(x,y));

      /* Static */
      #define icqskin_drawContents(h,p)           WinSendMsg(h,WMICQ_DRAWCONTENTS,(MPARAM) p, 0)
      #define icqstatic_setText(h,t)              WinSetWindowText(h,(PSZ) t)
      #define icqstatic_setMargin(h,m)            WinSendMsg(h,WMICQ_SETMARGIN,(MPARAM) m, 0)

      /* Dialog */
      #define icqskin_setDialogManager(h,m)		  WinSendMsg(h,WMICQ_SETDIALOG,(MPARAM) m, 0)

      /* Generic */
      #define icqskin_setButtons(h,t,s)           WinSendMsg(h,WMICQ_SETBUTTONTBL,(MPARAM) t, (MPARAM) s)
      #define icqskin_paintBackground(h,s,r)      WinSendMsg(h,WMICQ_PAINTBG,(MPARAM) s, (MPARAM) r)
      #define icqskin_connectScrollBar(h,s)       WinSendMsg(h,WMICQ_CONNECT, (MPARAM) s, 0)

      #define icqskin_setTreeIcons(h,f)           WinSendMsg(h,WMICQ_SETTREEIMAGE,(MPARAM) f, 0);
      #define icqskin_setSelectBar(h,f)           WinSendMsg(h,WMICQ_SETSELECTBAR, (MPARAM) f, 0);

      #define icqskin_insertUserGroup(h,i,t)      WinSendMsg(h,WMICQ_INSERTGROUP, (MPARAM) i, (MPARAM) t)
      #define icqskin_getDlgItemHandle(h,i)       WinWindowFromID(h,i)
      #define icqskin_sortUserList(h)             WinPostMsg(h,WMICQ_SORTLIST,0,0)
      #define icqskin_setUserListFlag(h,f)        WinPostMsg(h,WMICQ_SETFLAG,(MPARAM) f, 0)
      #define icqskin_getUserListFlag(h)          ((ULONG) WinSendMsg(h,WMICQ_GETFLAG,0, 0))
      #define icqskin_getUserListFlagPointer(h)   ((ULONG *) WinSendMsg(h,WMICQ_GETFLAGADDR,0, 0))

      #define icqskin_updateUserIcon(h,u)         WinPostMsg(h,WMICQ_UPDATEICON,(MPARAM) u, 0)
      #define icqskin_switchBlinkStatus(h)        WinSendMsg(h,WMICQ_SWITCHFLAG,(MPARAM) ICQULIST_BLINK,0);
      #define icqskin_blinkUser(h,u,i,t)          WinSendMsg(h,WMICQ_BLINK,(MPARAM) u,MPFROM2SHORT(i,t))
      #define icqskin_clearSkin(h)                WinSendMsg(h,WMICQ_CLEAR,0,0)

      #define icqskin_addUser(h,u)                WinSendMsg(h,WMICQ_ADDUSER, (MPARAM) u, 0)
      #define icqskin_delUser(h,u)                WinSendMsg(h,WMICQ_DELUSER, (MPARAM) u, 0)

      #define icqskin_queryUserListEntry(h,u)     ((HUSER) WinSendMsg(h,WMICQ_QUERYUSER,(MPARAM) u, 0))
      #define icqskin_updateUserHandle(h,u)       WinSendMsg(hwnd,WMICQ_UPDATEUSER,(MPARAM) u,0)

      #define icqskin_setWindowText(h,t)          WinSetWindowText(h,(PSZ) t)
      #define icqskin_getWindowText(h,s,t)        WinQueryWindowText(h,s,t)

      #define ICQ_BUTTON "pwICQButton"
      #define ICQ_STATIC "pwICQStatic"

      extern const char *icqButtonWindowClass;
      extern const char *icqUserListWindowClass;
      extern const char *icqMenuWindowClass;
      extern const char *icqSCBarWindowClass;
      extern const char *icqStaticWindowClass;
      extern const char *icqDialogWindowClass;
      extern const char *icqUPopupWindowClass;
      extern const char *icqBitmapWindowClass;

      extern const char *icqFrameWindowClass;
      extern const char *icqConfigWindowClass;
      extern const char *icqChatWindowClass;
      extern const char *icqPagerWindowClass;

      MRESULT EXPENTRY icqUserListWindow(  HWND, ULONG, MPARAM, MPARAM );
      MRESULT EXPENTRY icqUserPopupWindow( HWND, ULONG, MPARAM, MPARAM );
      MRESULT EXPENTRY icqMenuListWindow(  HWND, ULONG, MPARAM, MPARAM );
      MRESULT EXPENTRY icqSCBarWindow(     HWND, ULONG, MPARAM, MPARAM );	
      MRESULT EXPENTRY icqDialogWindow(    HWND, ULONG, MPARAM, MPARAM );
      MRESULT EXPENTRY icqBitmapWindow(    HWND, ULONG, MPARAM, MPARAM );

      MRESULT EXPENTRY icqChatWindow(      HWND, ULONG, MPARAM, MPARAM );
      MRESULT EXPENTRY icqPagerWindow(     HWND, ULONG, MPARAM, MPARAM );
      MRESULT EXPENTRY icqConfigWindow(    HWND, ULONG, MPARAM, MPARAM );

      int              icqskin_autoSize(HWND,short,short,short,short,short);

      USHORT           icqskin_drawIcon(SKINDATA *, HPS, USHORT, USHORT, USHORT);
      void             icqskin_drawSelected(HWND, HPS, PRECTL, HBITMAP, HBITMAP);
      int              icqskin_drawImage(HPS, PRECTL, HBITMAP, HBITMAP);
      HBITMAP          icqskin_loadBitmap(HICQ, const char *);
      HBITMAP          icqskin_createBackground(HWND, HBITMAP);
      HPOINTER         icqskin_CreatePointer(SKINDATA *, USHORT);
      void             icqskin_drawFrameBackground(HWND, HPS, USHORT, USHORT, ULONG);
      int              icqskin_MakeTransparent(HPS, HBITMAP, HBITMAP *, HBITMAP *);

      void             icqskin_loadPallete(HPS, int, const ULONG *);
      void             icqskin_updatePresParam(HWND, ULONG, ULONG *);

      void             icqskin_loadSkinPallete(hWindow, const char *, SKINFILESECTION *, const char **, int, ULONG *);

      #define NO_IMAGE 0xFFFFFFFF
      int icqskin_loadImage(HICQ, const char *, HBITMAP *, HBITMAP *);
      int icqskin_loadImageFromResource(HICQ, USHORT, USHORT, USHORT, HBITMAP *, HBITMAP *);

      #define icqskin_logError(i,m) icqWriteSysLog(i,PROJECT,m)
      #define icqskin_setButtonSize(h,x,y,cx,cy) WinSendMsg(h,WMICQ_SETSIZE,MPFROM2SHORT(x,y),MPFROM2SHORT(cx,cy))
      #define icqskin_setICQHandle(h,i) WinSendMsg(h,WMICQ_SETICQHANDLE,(MPARAM) i, 0)
      #define icqskin_deleteImage(x) if(x != NO_IMAGE) { GpiDeleteBitmap(x); x = NO_IMAGE; }
      #define icqskin_setButtonText(h,t) WinSetWindowText(h,(PSZ) t)
      #define icqskin_terminate(i) WinPostMsg( ((SKINDATA *)icqskin_getDataBlock(icq))->mainWindow,WM_CLOSE,0,0)

      void    icqskin_loadPallete(HPS, int, const ULONG *);
      ULONG   icqskin_loadPalleteFromString(const char *);

      void    icqskin_drawBorder(HPS, PRECTL, LONG, LONG);
      void    icqskin_eraseRect(HWND, HPS, PRECTL);

      HBITMAP icqskin_createFrameBackground(HWND hwnd, HBITMAP, short, short, HBITMAP *, HBITMAP *);

      int     icqskin_logErrorInfo(HICQ, hWindow, BOOL, char *);

      int     icqskin_registerWindowClass(HICQ);

      HWND    EXPENTRY icqDDEQueryServer(HICQ, const char *, const char *);
      HWND    EXPENTRY icqQuerySystray(HICQ);
      int     EXPENTRY icqDisableSysTray(HICQ);

      void    EXPENTRY icqDDEData(HWND, PDDESTRUCT);
      BOOL    EXPENTRY icqDDEInitiateAck(HWND, HWND, PDDEINIT);

   #endif

   int _System icqskin_popupMenu(HICQ, ULONG, USHORT, USHORT, USHORT);
   int _System icqskin_setItemBitmap(HICQ, HWND, HMODULE, USHORT, const char *);

	
   /* User list (Common) */
   void    icqskin_initializeUserList(HICQ,hWindow);
   void    icqskin_terminateUserList(HICQ,hWindow);
   void    icqskin_popupUserList(HICQ);

   /* Message editor (Common) */
   void                    icqskin_initializeMessageEditor(HICQ, hWindow);
   void                    icqskin_terminateMessageEditor(HICQ, hWindow);

   int                     icqskin_openSkinFile(SKINDATA *);
   int                     icqskin_closeSkinFile(SKINDATA *);
   const char *            icqskin_getSkinFileEntry(SKINFILESECTION *, const char *, const char *);
   const SKINFILESECTION * icqskin_getSkinFileSection(SKINFILESECTION *, const char *);

   /*---[ Misc windows (Common) ]---------------------------------------------*/

   void    EXPENTRY icqskin_logonDialog(HICQ);
   hWindow EXPENTRY icqskin_loadDialog(HICQ,hWindow,HMODULE,USHORT, const DLGMGR *);
   int     EXPENTRY icqskin_selectFile(HWND, HICQ, USHORT, BOOL, const char *, const char *, const char *, char *, int (* _System)(const DIALOGCALLS *,HWND,HICQ,char *));

   /*---[ Configuration dialog (Common) ]-------------------------------------*/

   hWindow                 icqskin_createConfigWindow(HICQ, ULONG, int, const ICQUSERBUTTON *);
   void                    icqskin_destroyCfgWindow(hWindow);
   void  _System           icqskin_cfgEventProcessor(HICQ,ULONG,UCHAR,USHORT,ULONG,hWindow);
   int   _System           icqskin_insertConfigPage(hWindow, USHORT, hWindow, USHORT, const char *);
   void                    icqskin_selectConfigPage(hWindow, hWindow);
   hWindow                 icqconfig_getNextPage(HWND, void **);
   int                     icqskin_setBitmap(HICQ, hWindow, const char *);

   /*---[ Menu (Common) ]-----------------------------------------------------*/

   int                     icqskin_insertMenuOption(HWND, const char *, USHORT, MENUCALLBACK *, ULONG);

   /*---[ Actions ]-----------------------------------------------------------*/

   #define BUTTON_CALLBACK void (ICQCALLBACK *)(hWindow,void *)
   void ICQCALLBACK icqskin_configButton(hWindow, SKINDATA *);
   void ICQCALLBACK icqskin_modeButton(hWindow,   SKINDATA *);
   void ICQCALLBACK icqskin_sysmsgButton(hWindow, SKINDATA *);
   void ICQCALLBACK icqskin_searchButton(hWindow, SKINDATA *);
   void ICQCALLBACK icqskin_userlistClick(HICQ, HUSER);
   void ICQCALLBACK icqskin_userContextMenu(HICQ, HUSER);
   void ICQCALLBACK icqskin_msgWindowAction(hWindow, USHORT);
   int  ICQCALLBACK icqskin_openUserInfo(HICQ, ULONG, USHORT, ULONG);
   int  ICQCALLBACK icqskin_openAboutUser(HICQ, ULONG, USHORT, ULONG);

   /*---[ Advanced Skin Toolkit ]---------------------------------------------*/

   DECLARE_WND_PROC(icqFrameWindow);
   DECLARE_WND_PROC(icqStaticWindow);
   DECLARE_WND_PROC(icqButtonWindow);

   void        EXPENTRY icqgui_setWindowID(hWindow,USHORT);

   /* Frame Window */
   hWindow     EXPENTRY icqframe_new(HICQ,const char *,int,int,BOOL,BOOL);
   ICQFRAME *  EXPENTRY icqframe_createDataBlock(HICQ, int, const char *);
   void        EXPENTRY icqframe_initialize(hWindow);

   int         EXPENTRY icqframe_loadSkin(hWindow, const char *, SKINFILESECTION *);
   hWindow     EXPENTRY icqframe_queryChild(hWindow, USHORT);
   void        EXPENTRY icqframe_destroy(hWindow, ICQFRAME *);

   /* Configuration window */

   /* Label window */
   hWindow     EXPENTRY icqlabel_new(hWindow, USHORT, const char *, const char *);
   ICQSTATIC * EXPENTRY icqlabel_createDataBlock(hWindow, int, const char *);
   void        EXPENTRY icqlabel_createBackground(hWindow, ICQSTATIC *, int, int);
   void        EXPENTRY icqlabel_destroy(hWindow, ICQSTATIC *);
   void        EXPENTRY icqlabel_setForegroundRGB(hWindow, USHORT, USHORT, USHORT);

   /* Message dialog */
   hWindow     EXPENTRY icqmsgdialog_new(HICQ, const char *, USHORT, USHORT);

   /* Button */
   hWindow     EXPENTRY icqbutton_new(hWindow, USHORT, const char *, USHORT, const char *);

#if defined(GTK1) || defined(GTK2)
   void        EXPENTRY icqlabel_setBackground(hWindow, GdkPixbuf *);
   gboolean             icqlabel_expose(GtkWidget *, GdkEventExpose *, ICQSTATIC *);
#endif

   #pragma pack()

#ifdef __cplusplus
   }
#endif

#endif
