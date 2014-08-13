/*
 * DRAG.C - Processa uma requisicao DRAG&DROP
 */

 #pragma strings(readonly)

 #define INCL_WIN
 #include <os2.h>

 #include <stdio.h>
 #include <malloc.h>
 #include <string.h>

 #include "icqskmgr.h"

/*---[ Prototipos ]---------------------------------------------------------------------------------*/


/*---[ Statics ]------------------------------------------------------------------------------------*/


/*---[ Implementacao ]------------------------------------------------------------------------------*/

 MRESULT dragOver(HWND hwnd, ICQWNDINFO *wnd, PDRAGINFO dragInfo)
 {
    int         i;
    USHORT      cItems;
    PDRAGITEM   dragItem;
    USHORT      rc              = DOR_NODROPOP;
    char buffer[0x0100];

    if(!(wnd->flags & WFLG_DRAG))
       return (MRFROM2SHORT(rc, DO_COPY));

    wnd->dragInfo = dragInfo;

    DrgAccessDraginfo(dragInfo);

    cItems = DrgQueryDragitemCount(dragInfo);

    for (i = 0; i < cItems && rc == DOR_NODROPOP; i++)
    {
       dragItem = DrgQueryDragitemPtr(dragInfo, i);

       DrgQueryStrName(dragItem->hstrType, 0xFF, buffer);

       if(!strcmp(buffer,"UniformResourceLocator"))
       {
#ifdef DEBUG
          rc = DOR_DROP;
#else
          if(DrgQueryStrName(dragItem->hstrSourceName, 0xFF, buffer))
             rc = DOR_DROP;
#endif
       }
    }

    DrgFreeDraginfo(dragInfo);
    return (MRFROM2SHORT(rc, DO_COPY));
 }

 MRESULT dragLeave(HWND hwnd, ICQWNDINFO *wnd)
 {
    wnd->dragInfo = NULL;
    wnd->flags &= ~WFLG_DRAG;
    return (MRESULT) FALSE;
 }

 MRESULT drop(HWND hwnd, ICQWNDINFO *wnd)
 {
    char        buffer[0x0100];
    int         i;
    USHORT      cItems;
    PDRAGITEM   dragItem;
    int         sz;
    HUSER       usr;

    DBGTracex(wnd->dragInfo);

    if(!wnd->dragInfo)
       return (MRFROM2SHORT (DOR_NODROPOP, 0));

    DrgAccessDraginfo(wnd->dragInfo);

    cItems = DrgQueryDragitemCount(wnd->dragInfo);
    DBGTrace(cItems);
    for(i = 0; i < cItems; i++)
    {
       dragItem = DrgQueryDragitemPtr(wnd->dragInfo, i);
       usr      = sknQuerySelectedHandle(hwnd,1003);

       DrgQueryStrName(dragItem->hstrType, 0xFF, buffer);
       if(usr && !strcmp(buffer,"UniformResourceLocator"))
       {
#ifdef DEBUG
          sz = DrgQueryStrName(dragItem->hstrContainerName, 0xFF, buffer);
          DBGTrace(sz);
          *(buffer+sz) = 0;
          DBGMessage(buffer);

          sz = DrgQueryStrName(dragItem->hstrTargetName, 0xFF, buffer);
          DBGTrace(sz);
          *(buffer+sz) = 0;
          DBGMessage(buffer);
#endif

          sz = DrgQueryStrName(dragItem->hstrSourceName, 0xFF, buffer);
          DBGTrace(sz);

          if(sz > 0 && usr)
          {
             *(buffer+sz) = 0;
             DBGMessage(buffer);

             icqNewUserMessage(wnd->icq, usr->uin, MSG_URL, 0, buffer);

          }
       }
    }

    CHKPoint();

    DrgFreeDraginfo(wnd->dragInfo);

    dragLeave(hwnd,wnd);

    CHKPoint();
    return (MRFROM2SHORT(DOR_DROP, DO_COPY));

 }

