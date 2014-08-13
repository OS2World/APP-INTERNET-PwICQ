/*
 * list.c - Administracao de listas encadeadas
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

 #include <stdio.h>
 #include <malloc.h>

 #include "ishared.h"

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/


/*---[ Implementation ]----------------------------------------------------------------------------------*/

 void ICQAPI ishare_insertItem(ISHARED_LIST *lst, ISHARED_LISTHEADER *itn)
 {
#ifdef __OS2__
    DosEnterCritSec();
#endif
    if(lst->first == NULL)
    {
       /* Empty list */
       lst->first = lst->last = itn;
       itn->up    = itn->down = NULL;
    }
    else
    {
       /* Insert in the end of the list */
       lst->last->down = itn;
       itn->up         = lst->last;
       itn->down       = NULL;
       lst->last       = itn;
    }
#ifdef __OS2__
    DosExitCritSec();
#endif
    yield();
 }

 void ICQAPI ishare_removeItem(ISHARED_LIST *ctl, ISHARED_LISTHEADER *reg)
 {
#ifdef __OS2__
    DosEnterCritSec();
#endif

   if(reg->up)
      (reg->up)->down = reg->down;
   else
      ctl->first = reg->down;

   if(reg->down)
      (reg->down)->up = reg->up;
   else
      ctl->last = reg->up;

#ifdef __OS2__
    DosExitCritSec();
#endif

    yield();

 }

 void ICQAPI ishare_deleteList(ISHARED_LIST *l)
 {
    ISHARED_LISTHEADER *ptr;

    while(l->first)
    {
       ptr      = l->first;
       l->first = ptr->down;
       yield();
       yield();
       free(ptr);
    }
    l->last = NULL;

 }


