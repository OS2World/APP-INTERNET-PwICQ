/*
 * REGISTER.C - Registra as classes
 */

 #define INCL_WIN
 #include "skin.h"

/*---[ Implementacao ]------------------------------------------------------------*/

APIRET EXPENTRY sknInitialize(HAB hab)
{
   WinRegisterClass(hab,(PSZ) buttonWindowName,  btnProcedure,0,sizeof(PVOID));
   WinRegisterClass(hab,(PSZ) scbarWindowName,   scbProcedure,0,sizeof(PVOID));
   WinRegisterClass(hab,(PSZ) listboxWindowName, lbsProcedure,0,sizeof(PVOID));
   WinRegisterClass(hab,(PSZ) menuWindowName,    mnuProcedure,0,sizeof(PVOID));
   return TRUE;
}


APIRET EXPENTRY sknRegisterClass(HAB hab, const char *className, PFNWP pfnWndProc, ULONG sz)
{
   WinRegisterClass(hab,(PSZ) className, pfnWndProc,CS_SIZEREDRAW,sz);
   return TRUE;
}


