/*
 * OS2WIN.C - pwICQ for OS/2 Window management functions module (ICQWIN.DLL)
 */

#ifndef OS2WIN_INCLUDED

 #define OS2WIN_INCLUDED

 #include <os2.h>

 #include <icqtlkt.h>

/*---[ Estruturas ]--------------------------------------------------------------------------------*/

/*---[ Defines ]-----------------------------------------------------------------------------------*/

 #define PAGELOADER void

/*---[ Prototipes ]--------------------------------------------------------------------------------*/

 APIRET         EXPENTRY icqStartWindowTools(HICQ, HAB);
 APIRET         EXPENTRY icqRegisterConfigurationWindow(HICQ,HAB);

 APIRET         EXPENTRY icqStopWindowTools(HICQ);

 APIRET         EXPENTRY icqOpenMsgWindow(HICQ, ULONG, USHORT, HMSG, const MSGMGR *, const char *, const char *);
 HWND           EXPENTRY icqOpenDefaultConfigurationWindow(HICQ, ULONG, USHORT);

 void           EXPENTRY icqDefaultMsgPreProcessor(HICQ, HUSER, ULONG, struct icqmsg *, BOOL);
 const MSGMGR * EXPENTRY icqQueryDefaultMsgMgr(HICQ);

 void           EXPENTRY icqAjustDefaultMsgDialog(HWND, HICQ, ULONG, USHORT, BOOL);
 void           EXPENTRY icqAjustTaskList(HICQ, HWND, const char *);

 APIRET         EXPENTRY icqExecuteWindowLoop(HICQ,HWND);

 int            EXPENTRY icqGetDefaultMsgMgrSize(void);

 int            EXPENTRY icqValidateDefaultEditHelper(USHORT);

 int	   	    EXPENTRY icqOpenSearchWindow(HICQ,USHORT);

 HPOINTER       EXPENTRY icqCreatePointer(HICQ, int);

 int            EXPENTRY icqMakeBitmapTransparent(HPS, HBITMAP, HBITMAP *, HBITMAP *);
 int            EXPENTRY icqProcessGuiEvent(HICQ, ULONG, UCHAR, USHORT, ULONG);

 HWND           EXPENTRY icqDDEQueryServer(HICQ, const char *, const char *);
 void           EXPENTRY icqDDEData(HWND, PDDESTRUCT);
 BOOL           EXPENTRY icqDDEInitiateAck(HWND, HWND, PDDEINIT);
 int            EXPENTRY icqUpdateSysTray(HICQ);
 int            EXPENTRY icqDisableSysTray(HICQ);
 int            EXPENTRY icqSetBitmap(HICQ, hWindow, const char *);

#endif

