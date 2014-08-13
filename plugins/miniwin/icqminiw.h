/*
 * icqminiw.h
 */

#ifdef __OS2__
   #define INCL_WIN
   #include <os2.h>
#endif

 #include <icqtlkt.h>

 extern HMODULE module;

 void icqminiw_initializeGui(HICQ);
 void icqminiw_userEvent(HWND, HICQ, USHORT, ULONG);
 void icqminiw_systemEvent(HWND, HICQ, USHORT);
 void icqminiw_modeButton(HWND,HICQ);
 void icqminiw_sysButton(HWND,HICQ);
 void icqminiw_openButton(HWND ,HICQ);

#ifdef __OS2__
 #define icqminiw_setIcon(h,m) WinPostMsg(h,WM_USER+1002,MPFROMLONG(m),0)
 #define icqminiw_setBlink(h,m) WinPostMsg(h,WM_USER+1003,MPFROMLONG(m),0)
#endif

