/*
 * CREATE.C - Cria janela principal e filhas
 */


 #define INCL_WIN
 #include "skin.h"

 #include <string.h>

/*---[ Prototipos ]---------------------------------------------------------------*/


/*---[ Constantes ]---------------------------------------------------------------*/

extern HMODULE module = NULLHANDLE;

/*---[ Implementacao ]------------------------------------------------------------*/

HWND EXPENTRY sknCreateWindow(const char *className, const char *skin, HWND *frm, const char *title, HMODULE module, ULONG flags, USHORT id)
{
//   ULONG  ulFrameFlags  = FCF_TASKLIST|FCF_NOMOVEWITHOWNER|FCF_NOBYTEALIGN;; /* |FCF_SIZEBORDER; */
   HWND   frame;
   HWND   ret           = NULLHANDLE;

   frame = WinCreateStdWindow(  HWND_DESKTOP,
                                0,
                                &flags,
                                (PSZ) className,
                                (PSZ) title,
                                WS_VISIBLE,
                                module,
                                id,
                                &ret
                            );
   DBGTracex(frame);
   DBGTracex(ret);

   if(ret)
      WinSendMsg(ret,WMSKN_LOADSKIN,PVOIDFROMMP(skin),0);

   if(frm)
      *frm = frame;

   return ret;

}

HWND CreatePopupMenu(HWND hwnd, USHORT id, ULONG style)
{
   ULONG  ulFrameFlags  = FCF_NOMOVEWITHOWNER; /* |FCF_DLGBORDER; */
   HWND   ret           = NULLHANDLE;
   HWND   frame;

   frame = WinCreateStdWindow(  HWND_DESKTOP,
                                0,
                                &ulFrameFlags,
                                (PSZ) menuWindowName,
                                "sknMenu",
                                WS_TOPMOST|style,
                                (HMODULE) 0,
                                FID_CLIENT,
                                &ret
                            );
   DBGTracex(frame);
   DBGTracex(ret);

   if(ret)
      WinSendMsg(ret,WM_USER+10,MPFROMLONG(hwnd),MPFROMSHORT(id));

   return ret;

/*
   HWND ret = WinCreateWindow(  HWND_DESKTOP,
                                (PSZ) menuWindowName,
                                "",
                                WS_TOPMOST|style,
                                0, 0, 100, 100,
                                hwnd,
                                HWND_BOTTOM,
                                1000,
                                0,
                                0 );


   if(ret)
      WinSendMsg(ret,WM_USER+10,MPFROMLONG(hwnd),MPFROMSHORT(id));

   return ret;
*/
}


