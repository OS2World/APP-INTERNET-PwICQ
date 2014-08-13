/*
 * LIST.C - Administrador de listas encadeadas
 */

 #define INCL_DOSPROCESS
 #include <malloc.h>
 #include <string.h>

#ifndef __OS2__
   #include <unistd.h>
#endif

 #include <icqkernel.h>
 #include <pwMacros.h>
 #include "icqcore.h"

/*---[ Prototipos ]---------------------------------------------------------------*/

 static void ItemDelete(HLIST, ICQITEM *);

/*---[ Implementacao ]------------------------------------------------------------*/

 HLIST EXPENTRY icqCreateList(void)
 {
    HLIST ret = malloc(sizeof(ICQLIST));

    DBGTracex(ret);

    if(ret)
       memset(ret,0,sizeof(ICQLIST));

    return ret;
 }

 HLIST EXPENTRY icqDestroyList(HLIST lst)
 {
    /* Limpa uma lista */

    if(!lst)
       return NULL;

    while( lst->last )
       ItemDelete(lst, lst->last);

    free(lst);

    return NULL;
 }

 void EXPENTRY icqClearList(HLIST lst)
 {
    /* Limpa uma lista */
    while( lst->last )
       ItemDelete(lst, lst->last);
 }

 void * EXPENTRY icqAddElement(HLIST lst, int size)
 {
    ICQITEM *ret = malloc(size + sizeof(ICQITEM));

    if(ret)
    {
       memset(ret,0,size+sizeof(ICQITEM));
       icqLockList(lst);
       if(lst->first == NULL)
       {
          /* A lista est  vazia */
          lst->first = lst->last = ret;
          ret->up    = ret->down = NULL;
       }
       else
       {
          /* Tem elemento na lista, anexa no final */
          lst->last->down = ret;
          ret->up         = lst->last;
          ret->down       = NULL;
          lst->last       = ret;
       }
       icqUnlockList(lst);
       return (ret+1);
    }

    return NULL;
 }

 void * EXPENTRY icqResizeElement(HLIST lst, int size, void *ptr)
 {
    ICQITEM *element 	= (ICQITEM *) ptr;
    ICQITEM *ret;

    if(!lst || !element)
       return NULL;

    ret = malloc(size + sizeof(ICQITEM));

    if(!ret)
       return ret;

    memset(ret,0,sizeof(ICQITEM));

    element--;

    icqLockList(lst);

    if(element->up)
    {
       ret->up       = element->up;
       ret->up->down = ret;
    }
    else
    {
       lst->first = ret;
    }

    if(element->down)
    {
       ret->down   = element->down;
       ret->down->up = ret;
    }
    else
    {
       lst->last = ret;
    }

    icqUnlockList(lst);

    free(element);

    return (ret+1);
 }

 int EXPENTRY icqRemoveElement(HLIST lst, void *ptr)
 {
    /* Remove um elemento de uma lista */
    ICQITEM *element = (ICQITEM *) ptr;

    if(!lst || !element)
       return -1;

    element--;

    ItemDelete(lst,element);

    return 0;
 }

 static void ItemDelete(HLIST ctl, ICQITEM *reg)
 {
    icqLockList(ctl);

    if(reg->up)
       (reg->up)->down = reg->down;
    else
       ctl->first = reg->down;

    if(reg->down)
       (reg->down)->up = reg->up;
    else
       ctl->last = reg->up;

    icqUnlockList(ctl);

    free(reg);

 }

 void * EXPENTRY icqQueryFirstElement(HLIST ctl)
 {
//    ICQITEM *element;

    if(ctl && ctl->first)
       return (ctl->first+1);

    return NULL;
 }

 void * EXPENTRY icqQueryLastElement(HLIST ctl)
 {
    if(ctl && ctl->last)
       return( (void *) (ctl->last+1) );
    return NULL;
 }

 void * EXPENTRY icqQueryNextElement(HLIST ctl, void *ptr)
 {
    /* Retorna o proximo elemento de uma lista */
    ICQITEM *element = (ICQITEM *) ptr;

    if(element)
    {
       element--;
       if(element->down)
          return( (void *) (element->down +1) );
    }

    return NULL;
 }

 void * EXPENTRY icqQueryPreviousElement(HLIST ctl, void *ptr)
 {
    /* Retorna o proximo elemento de uma lista */
    ICQITEM *element = (ICQITEM *) ptr;

    if(element)
    {
       element--;
       if(element->up)
          return( (void *) (element->up +1) );
    }

    return NULL;
 }

 void EXPENTRY icqLockList(HLIST lst)
 {
    while(lst->lock)
    {
#ifdef __OS2__
       DosSleep(5);
#else
       usleep(100);
#endif
    }
    lst->lock = TRUE;
 }

 void EXPENTRY icqUnlockList(HLIST lst)
 {
    lst->lock = FALSE;
 }


