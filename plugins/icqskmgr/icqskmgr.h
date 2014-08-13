/*
 * wintlkt.h - Prototipos e definicoes para o gerenciador de janelas
 */

 #define INCL_WIN
 #include <os2.h>

 #include <skinapi.h>
 #include <icqtlkt.h>
 #include <os2win.h>

/*---[ Macro functions ]---------------------------------------------------------------------*/

 #define setFrameIcon(h,i) WinPostMsg(h,WMSKN_SETFRAMEICON,(MPARAM) i, 0)

/*---[ Definicoes ]--------------------------------------------------------------------------*/

 #define TIMER_TICK 25

 #pragma pack(1)
 typedef struct icqwndinfo
 {
    HICQ        icq;
    HSKIN       skn;
    ULONG       flags;
    #define     WFLG_REDRAW             0x00000001
    #define     WFLG_BLINK              0x00000002
    #define     WFLG_CONNECTING         0x00000004
    #define     WFLG_BLINKTEXT          0x00000008
    #define     WFLG_SORT               0x00000010
    #define     WFLG_DRAG               0x00000020

    /* Janelas */
    HWND        frame;

    /* Controle de temporizacao */
    USHORT      divisor;
    USHORT      ticks;
    USHORT      timer;

    /* Controle de DDE */
    HWND        ddeServer;

    /* Controle de drag&drop */
    PDRAGINFO   dragInfo;

    /* Controle do botao de sistema */
    USHORT      modeIcon;                       /* Current system icon                 */
    USHORT      eventIcon;                      /* Current system event icon           */
    USHORT      readIcon;                       /* Next system icon                    */
    USHORT      iconSize;

    /* Controle do botao de modo */
    USHORT      modeDiv;
    USHORT      modeTicks;
    USHORT      modeTimer;

    USHORT      cols[3];                /* Colunas dos menus */

    HPOINTER    icon;

    char        buffer[0x0100];

 } ICQWNDINFO;

 struct awayDialogParameters
 {
    USHORT      sz;
    UCHAR       flag;
    ULONG       uin;
    HUSER       usr;
    HICQ        icq;
    const char  *title;
    char        *txt;
 };

 struct openWindowParameters
 {
    USHORT      sz;
    BOOL        flag;
    HUSER       usr;
    const char  *id;
    int         (* _System callback)(HICQ, HWND, const char *, void *);
    void        *arg;
 };


/*---[ Constantes ]--------------------------------------------------------------------------*/

 #define ICQMENU_USER         0
 #define ICQMENU_MODES        1
 #define ICQMENU_SYSTEM       2
 #define ICQMENU_COUNT        3

 extern HMODULE                 module;
 extern HWND                    mainWindow;
 extern const SKINELEMENT       winElements[];

/*---[ Prototipos ]--------------------------------------------------------------------------*/

 MRESULT EXPENTRY       sknProcedure(HWND, ULONG, MPARAM, MPARAM);
 MRESULT EXPENTRY       icqskmgr_sendMsg(ULONG, MPARAM, MPARAM);

 void                   create(HWND, PCREATESTRUCT);
 void                   destroy(HWND);

 void                   procSysMenu(HWND,ICQWNDINFO *, ULONG);
 void                   updateUserList(HWND, ICQWNDINFO *);
 void                   systemEvent(HWND, ICQWNDINFO *, USHORT);
 void                   setModeButton(HWND, ICQWNDINFO *, ULONG);
 void                   setConnecting(HWND, ICQWNDINFO *);

 void                   doTimer(HWND);
 void                   procUsrMenu(HWND, HUSER, ICQWNDINFO *, ULONG);
 void                   wndBroadcast(ICQWNDINFO *, ULONG, ULONG, MPARAM, MPARAM);

 void                   userEvent(HWND, USHORT, ULONG);

 char *                 querySkinName(HICQ, char *);

 void                   openUserAwayDialog(HWND,struct awayDialogParameters *);

 void                   checkUserList(HWND, ICQWNDINFO *);

 void                   ddeOpenURL(HWND,ICQWNDINFO *, const char *, const char *, const char *);

 void                   ddeInitiateAck(HWND,ICQWNDINFO *,HWND,PDDEINIT);
 void                   ddeData(HWND, ICQWNDINFO *, PDDESTRUCT );

 MRESULT                dragOver(HWND, ICQWNDINFO *, PDRAGINFO);
 MRESULT                dragLeave(HWND, ICQWNDINFO *);
 MRESULT                drop(HWND, ICQWNDINFO *);

 void                   aboutUser(HICQ,ULONG,char *);



