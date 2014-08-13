/*
 * OS2CONFIG.H - Configuration dialogs for OS/2
 */

 #error ICQOS2.H was deprecated

/*---[ Messages sent from the main Config to dialog boxes ]-------------------------------------*/

 #ifdef WM_USER

    #define WMICQ_EVENT          WM_USER+5000    /* pwICQ event                 	*/
    #define WMICQ_LOAD           WM_USER+5001    /* Load information            	*/
    #define WMICQ_SAVE           WM_USER+5002    /* Save information            	*/
    #define WMICQ_SELECTED       WM_USER+5003    /* Dialog was selected         	*/
    #define WMICQ_DDEOPENURL     WM_USER+5004    /* Execute DDE Call            	*/
    #define WMICQ_SEARCH	     WM_USER+5005	 /* Search Button (MP1=id, MP2=HWND)	*/


    #ifndef WS_TOPMOST
       #define WS_TOPMOST  0x00200000
    #endif

 #endif

/*---[ Configuration window elements ]----------------------------------------------------------*/

#ifndef ICQGUI_INCLUDED

     #define CFGWIN_ROOT		 0
     #define CFGWIN_NETWORK		 1
     #define CFGWIN_MODE		 2
     #define CFGWIN_MULTIMEDIA		 3
     #define CFGWIN_USERS	 	 4
     #define CFGWIN_OPTIONS		 5
     #define CFGWIN_SECURITY		 6
     #define CFGWIN_MISC		 7
     #define CFGWIN_ADVANCED		 8
     #define CFGWIN_ABOUT		 9
     #define CFGWIN_INFO		10
     #define CFGWIN_EVENTS		11

     #define CFGWIN_USER

#endif

/*---[ Callback tables ]-----------------------------------------------------------------------*/

   typedef struct dlghelper
   {
      USHORT    sz;

      /* Simple control management */

      int       (* _System setString)(HWND,USHORT,const char *);
      int       (* _System getString)(HWND,USHORT,int,char *);

      int       (* _System setCheckBox)(HWND,USHORT,BOOL);
      int       (* _System getCheckBox)(HWND,USHORT);

      int       (* _System setRadioButton)(HWND,USHORT,int);
      int       (* _System getRadioButton)(HWND,USHORT,short);

      int       (* _System getLength)(HWND,USHORT);

      int       (* _System setEnabled)(HWND,USHORT,BOOL);

      int       (* _System setTextLimit)(HWND,USHORT,short);

      /* List-box management */
      int       (* _System listBoxInsert)(HWND,USHORT,ULONG,const char *,BOOL);
      int       (* _System listBoxQueryText)(HWND,USHORT,ULONG, int, char *);

      /* Miscellaneous */
      int       (* _System populateEventList)(HWND,USHORT);
      int       (* _System browseFiles)(HWND,USHORT, BOOL, const char *, const char *, const char *, char *, int (* _System)(const struct dlghelper *,HWND,HICQ,char *));

      /* Direct interaction with Controls and pwICQ's configuration file */
      int       (* _System loadCheckBox)(HWND,ULONG,USHORT,const char *,BOOL);
      int       (* _System loadString)(HWND,ULONG,USHORT,const char *,int,const char *);
      int       (* _System loadRadioButton)(HWND,ULONG,USHORT,const char *, int, int);
      int       (* _System loadSpinButton)(HWND,ULONG,USHORT,const char *, int, int, int);

      int       (* _System saveCheckBox)(HWND,ULONG,USHORT,const char *);
      int       (* _System saveString)(HWND,ULONG,USHORT,const char *, int);
      int       (* _System saveRadioButton)(HWND,ULONG,USHORT,const char *, int);
      int       (* _System saveSpinButton)(HWND,ULONG,USHORT,const char *);

      /* Data area (Allocated based on the configure() return value */
      void *    (* _System getConfig)(HWND);

   } DLGHELPER;


