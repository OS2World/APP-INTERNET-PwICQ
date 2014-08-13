 /*
  * CodePage Translation dialog box
  */

#ifndef __OS2__
 #error OS/2 only
#endif

 #pragma strings(readonly)

 #define INCL_WIN
 #define INCL_DOSNLS

 #include <malloc.h>
 #include <string.h>
 #include <stdlib.h>

 #include "icqconf.h"

/*---[ Definitions ]-----------------------------------------------------------------------------------------*/

/*---[ Prototipes ]------------------------------------------------------------------------------------------*/

 static int   _System load(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int   _System save(const DLGHELPER *,HWND,HICQ,ULONG,  char *);

 const DLGMGR codepage = {    sizeof(DLGMGR),
                              sizeof(DLGHELPER),
                              0,
                              NULL,	        // Configure
                              load,		    // Load
                              save,		    // Save
                              NULL,         // Cancel
                              NULL,         // Event
                              NULL,	   	    // Selected
                              NULL,         // Click
                              NULL,         // changed
                              NULL          // sysbtn
                         };


/*---[ Implement ]-------------------------------------------------------------------------------------------*/

 static int load(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    HAB    hab     = WinQueryAnchorBlock(hwnd);
    int    current = icqQueryRemoteCodePage(icq);
    int    id      = 6200;
    HWND   h       = WinWindowFromID(hwnd,102);
    USHORT pos;
    char   *ptr;
    ULONG  cpg;

    while(WinLoadString(hab, module, id, 0xFF, buffer) > 0)
    {
       for(ptr = buffer;*ptr && *ptr != '.';ptr++);

       if(*ptr == '.')
       {
          *(ptr++) = 0;
           pos = SHORT1FROMMR(WinSendMsg(h,LM_INSERTITEM,MPFROMSHORT(LIT_END),MPFROMP(ptr)));

           cpg = atoi(buffer);
           DBGTrace(cpg);

           if(pos != LIT_ERROR)
           {
              WinSendMsg(h,LM_SETITEMHANDLE,MPFROMSHORT(pos),MPFROMLONG(cpg));
              if(cpg == current)
                 WinSendMsg(h,LM_SELECTITEM,MPFROMSHORT(pos),MPFROMSHORT(TRUE));
           }
       }
       id++;
    }

    WinCheckButton(hwnd,103,icqLoadValue(icq,"cnvtUsersFile", 0));

    return 0;
 }

 static int _System save(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    HWND                h       = WinWindowFromID(hwnd,102);
    USHORT              pos;
    ULONG               cpg;

    pos = SHORT1FROMMR(WinSendMsg(h,LM_QUERYSELECTION,MPFROMSHORT(LIT_FIRST),0));

    if(pos == LIT_NONE)
       return 0;

    cpg = LONGFROMMR(WinSendMsg(h,LM_QUERYITEMHANDLE,MPFROMSHORT(pos),0));

    DBGTrace(cpg);

    icqSetRemoteCodePage(icq,cpg);

    icqSaveValue(icq,"cnvtUsersFile",WinSendDlgItemMsg(hwnd,103,BM_QUERYCHECK,0,0) ? 1 : 0);

    return 0;
 }


