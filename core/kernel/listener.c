/*
 * guievent.c - Administra os event-listeners de interface gráfica
 */

 #include <icqkernel.h>
 #include "icqcore.h"

 /*---[ Definicoes ]----------------------------------------------------------------------------------*/

 #define GUICALLBACK void (* _System callback)(HICQ,ULONG,UCHAR,USHORT,ULONG,HWND)

 #pragma pack(1)

 typedef struct _guilistener
 {
    HWND        hwnd;                   // Identificacao da janela a ser passada para a funcao de callback
    GUICALLBACK;                        // A funcao de callback em si
 } GUILISTENER;


 /*---[ Implementacao ]-------------------------------------------------------------------------------*/

 int EXPENTRY icqAddEventListener(HICQ icq, void (* _System callBack)(HICQ, void *, ULONG, char, USHORT, ULONG), void *parm)
 {
    EVENTLISTENER *listener = icqAddElement(icq->listeners, sizeof(EVENTLISTENER));

    if(!listener)
    {
       icqWriteSysLog(icq,PROJECT,"Unable to allocate memory for user event listener");
       return -1;
    }

    DBGMessage("New Event listener added");

    listener->timer    = 0;
    listener->parm     = parm;
    listener->listener = (void (* _System)(void *, void *, ULONG, char, USHORT, ULONG)) callBack;

    return 0;
 }

 int EXPENTRY icqRemoveEventListener(HICQ icq, void (* _System callBack)(HICQ, void *, ULONG, char, USHORT, ULONG), void *parm)
 {
    EVENTLISTENER       *listener;

    for(listener = icqQueryFirstElement(icq->listeners);listener;listener = icqQueryNextElement(icq->listeners,listener))
    {
       if( (void *) listener->listener == (void *) callBack && listener->parm == parm)
       {
          DBGMessage("Event listener removed");
          listener->timer    = 0;
          listener->listener = NULL;
          listener->parm     = NULL;
          return 0;
       }
    }

    return -1;
 }

 int EXPENTRY icqAddGuiEventListener(HICQ icq, void *callback, HWND hwnd)
 {
    GUILISTENER *listener = icqAddElement(icq->guiListeners, sizeof(GUILISTENER));

    if(!listener)
    {
       icqWriteSysLog(icq,PROJECT,"Unable to allocate memory for gui event listener");
       return -1;
    }

    DBGMessage("New GUI Event listener added");

    listener->hwnd      = hwnd;
    listener->callback  = (void (* _System)(HICQ,ULONG,UCHAR,USHORT,ULONG,HWND)) callback;

    return 0;
 }

 int EXPENTRY icqRemoveGuiEventListener(HICQ icq, void *callback, HWND hwnd)
 {
    GUILISTENER       *listener;

    for(listener = icqQueryFirstElement(icq->guiListeners);listener;listener = icqQueryNextElement(icq->guiListeners,listener))
    {
       if( ((void *) listener->callback) == callback && listener->hwnd == hwnd)
       {
          DBGMessage("GUI Event listener removed");
          listener->callback = 0;
          listener->hwnd     = 0;
          return 0;
       }
    }
    return -1;
 }

 void EXPENTRY icqExecuteGuiListeners(HICQ icq, ULONG uin, char id, USHORT event, ULONG parm)
 {
    GUILISTENER       *listener;
    GUILISTENER       *next;

    listener = icqQueryFirstElement(icq->guiListeners);
    while(listener)
    {
       next = icqQueryNextElement(icq->guiListeners,listener);
       if(listener->callback)
          listener->callback(icq,uin,id,event,parm,listener->hwnd);
       else
          icqRemoveElement(icq->guiListeners,listener);
       listener = next;
    }
 }

