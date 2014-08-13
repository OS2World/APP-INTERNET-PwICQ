/*
 * SEARCH.C - Administra bloco de pesquisas
 */


#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdio.h>
 #include <string.h>
 #include <icqkernel.h>

/*---[ Estruturas ]-------------------------------------------------------------------------------------------*/


/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 HSEARCH icqBeginSearch(HICQ icq, ULONG uin, ULONG sequence, HWND hwnd, BOOL event, SEARCHCALLBACK callBack)
 {
#ifdef EXTENDED_LOG
    char    buffer[0x0100];
#endif
    HSEARCH 		sr 		= icqAddElement(icq->pendingSearchs, sizeof(struct search));
    ICQSEARCHRESPONSE   response;

    if(!sr)
    {
       icqWriteSysLog(icq,PROJECT,"Unable to allocate search block");
       return sr;
    }

    DBGMessage("Pesquisa iniciada");

    sr->sz       = sizeof(ICQSEARCH);
    sr->uin      = uin;
    sr->hwnd     = hwnd;
    sr->idle     = 0;
    sr->sequence = sequence;
    sr->callBack = callBack;

    icq->searchs++;

#ifdef EXTENDED_LOG
    sprintf(buffer,"Search #%ld of %d started",sr->sequence,icq->searchs);
    icqWriteSysLog(icq,PROJECT,buffer);
#endif

    if(event)
    {
       sr->flags |= ICQSEARCH_EVENTS;
       icqEvent(icq, uin, 'S', ICQEVENT_BEGINSEARCH, (ULONG) sr);
    }

    if(sr->callBack)
    {
       memset(&response,0,sizeof(response));
       response.id    = sizeof(ICQSEARCHRESPONSE);
       response.flags = ICQSR_BEGIN;
       sr->callBack(icq,sr->uin,sr->sequence,sr->hwnd,&response);
    }

    return sr;

 }

 int EXPENTRY icqEndSearch(HICQ icq, ULONG sequence)
 {
#ifdef EXTENDED_LOG
    char    buffer[0x0100];
#endif
    HSEARCH 		sr;
    ICQSEARCHRESPONSE   response;

    if(icq->searchs)
       icq->searchs--;

    for(sr = icqQueryFirstElement(icq->pendingSearchs);sr;sr = icqQueryNextElement(icq->pendingSearchs, sr))
    {
       if(sr->sequence == sequence)
       {
          if(sr->callBack)
          {
             memset(&response,0,sizeof(response));
             response.id    = sizeof(ICQSEARCHRESPONSE);
             response.flags = ICQSR_END;
             sr->callBack(icq,sr->uin,sr->sequence,sr->hwnd,&response);
          }

          if(sr->flags & ICQSEARCH_EVENTS)
             icqEvent(icq, sr->uin, 'S', ICQEVENT_ENDSEARCH, (ULONG) sr);

          icqRemoveElement(icq->pendingSearchs, sr);
#ifdef EXTENDED_LOG
          sprintf(buffer,"Search #%ld of %d terminated",sequence,icq->searchs+1);
          icqWriteSysLog(icq,PROJECT,buffer);
#endif
          return 0;
       }
    }

    return -1;
 }

 ULONG EXPENTRY icqQuerySearchUIN(HICQ icq, ULONG sequence)
 {
    HSEARCH sr;

    for(sr = icqQueryFirstElement(icq->pendingSearchs);sr;sr = icqQueryNextElement(icq->pendingSearchs, sr))
    {
       if(sr->sequence == sequence)
       {
          sr->idle = 0;
          return sr->uin;
       }
    }
    return 0;
 }

 HSEARCH EXPENTRY icqQuerySearchHandle(HICQ icq, ULONG sequence)
 {
    HSEARCH sr;

    for(sr = icqQueryFirstElement(icq->pendingSearchs);sr;sr = icqQueryNextElement(icq->pendingSearchs, sr))
    {
       if(sr->sequence == sequence)
       {
          sr->idle = 0;
          return sr;
       }
    }
    return 0;
 }

 int EXPENTRY icqSetSearchResponse(HICQ icq, ULONG sequence, const ICQSEARCHRESPONSE *response)
 {
    HSEARCH sr;

    for(sr = icqQueryFirstElement(icq->pendingSearchs);sr;sr = icqQueryNextElement(icq->pendingSearchs, sr))
    {
       if(sr->sequence == sequence)
       {
          sr->idle = 0;
          if(sr->callBack)
             return sr->callBack(icq,sr->uin,sr->sequence,sr->hwnd,response);
          return 0;
       }
    }
    return -1;
 }

 int EXPENTRY icqAbortSearchByWindow(HICQ icq, HWND hwnd)
 {
#ifdef EXTENDED_LOG
    char    buffer[0x0100];
#endif
    HSEARCH sr;
    HSEARCH next;

    sr = icqQueryFirstElement(icq->pendingSearchs);

    while(sr)
    {
       next = icqQueryNextElement(icq->pendingSearchs, sr);

       if(sr->hwnd == hwnd)
       {
#ifdef EXTENDED_LOG
          sprintf(buffer,"Search #%ld of %d aborted",sr->sequence,icq->searchs);
          icqWriteSysLog(icq,PROJECT,buffer);
#endif
          if(icq->searchs)
             icq->searchs--;

          if(sr->flags & ICQSEARCH_EVENTS)
             icqEvent(icq, sr->uin, 'S', ICQEVENT_ABORTSEARCH,   (ULONG) sr);

          icqRemoveElement(icq->pendingSearchs, sr);
       }
       sr = next;
    }
    return 0;
 }

 HUSER EXPENTRY icqCacheSearchResult(HICQ icq, USHORT seq, ULONG uin, BOOL clean)
 {
    HSEARCH 		sr    = icqQuerySearchHandle(icq,seq);
    ULONG		flags;
    HUSER   		usr;
    HWND   		hwnd;
    SEARCHCALLBACK 	callBack;
    ICQSEARCHRESPONSE   response;

    if(!sr)
       return NULL;

    hwnd 	= sr->hwnd;
    callBack 	= sr->callBack;
    flags       = sr->flags;

    if(clean)
    {
       if(icq->searchs)
          icq->searchs--;
       icqRemoveElement(icq->pendingSearchs, sr);
    }

    usr = icqUpdateUserInfo(icq, uin, FALSE, "", "", "", "", TRUE);
    if(!usr)
       return usr;

    usr->u.flags &= ~USRF_TEMPORARY;
    usr->u.flags |= USRF_CACHED;

    if(usr->c2s && usr->c2s->requestUserUpdate)
       sr = usr->c2s->requestUserUpdate(icq,uin,icq->searchs);
    else if(icq->c2s && icq->c2s->requestUserUpdate)
       sr = icq->c2s->requestUserUpdate(icq,uin,icq->searchs);
    else
       sr = NULL;

    if(sr)
    {
       sr->hwnd     = hwnd;
       sr->callBack = callBack;
       sr->flags    = flags;
       if(sr->callBack)
       {
          memset(&response,0,sizeof(response));
          response.id    = sizeof(ICQSEARCHRESPONSE);
          response.flags = ICQSR_BEGIN;
          sr->callBack(icq,sr->uin,sr->sequence,sr->hwnd,&response);
       }
    }

    return usr;
 }



