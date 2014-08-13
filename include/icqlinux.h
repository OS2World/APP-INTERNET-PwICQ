/*
 * icqlinux.h - Configuration dialog boxes for pwICQ2-Linux
 */

 #ifdef GtkWidget
    #define WIDGET GtkWidget *
 #else
    #define WIDGET void *
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

/*---[ Callback tables ]-------------------------------------------------------------------------*/

 #pragma pack(1)

 typedef struct tabledef
 {
    USHORT	 id;

    USHORT	 top;
    USHORT	 left;
    USHORT	 bottom;
    USHORT	 right;

    USHORT     type;
    #define ICQTABLE_LABEL	     1
    #define ICQTABLE_ENTRY	     2
    #define ICQTABLE_BUTTON        3
    #define ICQTABLE_CHECKBOX      4
    #define ICQTABLE_SPIN		  5
    #define ICQTABLE_RADIOBUTTON   6
    #define ICQTABLE_MLE		   7
    #define ICQTABLE_DROPBOX	   8

    #define ICQTABLE_BEGIN	0xFFFE
    #define ICQTABLE_END	  0xFFFF

    ULONG	  parm1;
    ULONG	  parm2;

    const char *text;

 } TABLEDEF;

 typedef struct dlghelper
 {
    USHORT   sz;

    /* Simple control management */
    int      (* _System setString)(HWND,USHORT,const char *);
    int      (* _System getString)(HWND,USHORT,int,char *);
    int      (* _System setTextLimit)(HWND,USHORT,int);
    int      (* _System getLength)(HWND,USHORT);

    int      (* _System setCheckBox)(HWND,USHORT,BOOL);
    int      (* _System getCheckBox)(HWND,USHORT);

    int      (* _System setSpinButton)(HWND,USHORT,int);
    int      (* _System getSpinButton)(HWND,USHORT);

    int	  (* _System setRadioButton)(HWND,USHORT,int);
    int	  (* _System getRadioButton)(HWND,USHORT,int);

    int      (* _System setEnabled)(HWND,USHORT,BOOL);

    /* List-box management */
    int       (* _System listBoxInsert)(HWND,USHORT,ULONG,const char *,BOOL);

    /* Miscellaneous */
    int       (* _System populateEventList)(HWND,USHORT);
    int       (* _System browseFiles)(HWND,USHORT, BOOL, const char *, const char *, const char *, char *, int (* _System)(const struct dlghelper *,HWND,HICQ,char *));

    /* Direct interaction with Controls and pwICQ's configuration file */
    int      (* _System loadString)(HWND, ULONG, USHORT, const char *, int, const char *);
    int      (* _System saveString)(HWND,ULONG,USHORT,const char *,int);

    int      (* _System loadCheckBox)(HWND,ULONG,USHORT,const char *,BOOL);
    int      (* _System saveCheckBox)(HWND,ULONG,USHORT,const char *);

    int      (* _System loadRadioButton)(HWND,ULONG,USHORT,const char *, int, int);
    int      (* _System saveRadioButton)(HWND,ULONG,USHORT,const char *, int);

    int      (* _System loadSpinButton)(HWND,ULONG,USHORT,const char *, int, int, int);
    int      (* _System saveSpinButton)(HWND,ULONG,USHORT,const char *);

    /* Data area Allocated based on the configure() return value */
    void *   (* _System getConfig)(HWND);

 } DLGHELPER;

 typedef struct dlgmgr
 {
    USHORT    sz;

    int       (* _System configure)(const DLGHELPER *,HWND,HICQ,ULONG,  char *);
    int       (* _System load)(     const DLGHELPER *,HWND,HICQ,ULONG,  char *);
    int       (* _System save)(     const DLGHELPER *,HWND,HICQ,ULONG,  char *);
    int       (* _System cancel)(   const DLGHELPER *,HWND,HICQ,ULONG,  char *);
    void      (* _System event)(    const DLGHELPER *,HWND,HICQ,ULONG,  char,USHORT,char *);
    void      (* _System selected)( const DLGHELPER *,HWND,HICQ,ULONG,  USHORT, ULONG, char *);
    int       (* _System click)(    const DLGHELPER *,HWND,HICQ,ULONG,  USHORT, char * );
    int       (* _System changed)(  const DLGHELPER *,HWND,HICQ,ULONG,  USHORT, USHORT, char * );
    int	      (* _System sysButton)( const DLGHELPER *, HWND, HICQ, ULONG, HWND, USHORT, SEARCHCALLBACK, char *);

 } DLGMGR;


