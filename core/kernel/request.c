/*
 * request.c - Gerencia requisicoes diversas
 */

 #include <stdio.h>
 #include <malloc.h>
 #include <string.h>

 #include <icqkernel.h>


/*---[ Implement ]--------------------------------------------------------------------------------------------*/


 void * EXPENTRY icqCreateRequest(HICQ icq, ULONG uin, BOOL out, ULONG session, ULONG id, const REQUESTMGR *mgr, void *data, int sz)
 {
    REQUEST *req;
#ifdef EXTENDED_LOG
    char logBuffer[0x0100];
#endif

    DBGTracex(id);

    if(mgr->sz != sizeof(REQUESTMGR))
    {
       icqWriteSysLog(icq,PROJECT,"Invalid request manager when creating request entry");
       return NULL;
    }

    req = icqAddElement(icq->requests, sizeof(REQUEST)+sz);

    if(!req)
       return NULL;

    memset(req,0,sizeof(REQUEST)+sz);

    req->sz      = sizeof(REQUEST);
    req->session = session;
    req->id      = id;
    req->uin     = uin;
    req->out     = out;
    req->mgr     = mgr;
    req->data    = data;

    icqEvent(icq, uin, 'U', ICQEVENT_REQCREATED, id);

#ifdef EXTENDED_LOG
    sprintf(logBuffer,"Request %08lx created for message %08lx",(ULONG) req, id);
    icqWriteSysLog(icq,PROJECT,logBuffer);
#endif

    return (req+1);
 }

 int EXPENTRY icqDestroyRequest(HICQ icq, ULONG id, BOOL out)
 {
    REQUEST *req;
#ifdef EXTENDED_LOG
    char logBuffer[0x0100];
#endif

    for(req = icqQueryFirstElement(icq->requests);req;req = icqQueryNextElement(icq->requests,req))
    {
       if(req->id == id && req->out == out)
       {
#ifdef EXTENDED_LOG
          sprintf(logBuffer,"Request %08lx for message %08lx destroyed",(ULONG) req,id);
          icqWriteSysLog(icq,PROJECT,logBuffer);
#endif
          icqEvent(icq, req->uin, 'U', ICQEVENT_REQDELETED, id);
          icqRemoveElement(icq->requests,req);
          return 0;
       }
    }
    return -1;
 }


 int EXPENTRY icqAcceptRequest(HICQ icq, ULONG id, const char *reason)
 {
    REQUEST *req;
#ifdef EXTENDED_LOG
    char logBuffer[0x0100];
#endif

    for(req = icqQueryFirstElement(icq->requests);req;req = icqQueryNextElement(icq->requests,req))
    {

       DBGTracex(req->id);

       if(req->id == id && !req->out)
       {
#ifdef EXTENDED_LOG
          sprintf(logBuffer,"Accepting request %08lx for message %08lx",(ULONG) req,id);
          icqWriteSysLog(icq,PROJECT,logBuffer);
#endif

          if(req->mgr && req->mgr->accept)
             req->mgr->accept(icq,req->id,req->data,req->session,(req+1),reason);

          icqEvent(icq, req->uin, 'U', ICQEVENT_REQACCEPTED, id);
          return 0;
       }
    }
    return -1;
 }

 int EXPENTRY icqRefuseRequest(HICQ icq, ULONG id, const char *reason)
 {
    REQUEST *req;
#ifdef EXTENDED_LOG
    char logBuffer[0x0100];
#endif

    for(req = icqQueryFirstElement(icq->requests);req;req = icqQueryNextElement(icq->requests,req))
    {
       DBGTracex(req->id);

       if(req->id == id && !req->out)
       {
#ifdef EXTENDED_LOG
          sprintf(logBuffer,"Refusing request %08lx for message %08lx",(ULONG) req,id);
          icqWriteSysLog(icq,PROJECT,logBuffer);
#endif

          if(req->mgr && req->mgr->refuse)
             req->mgr->refuse(icq,req->id,req->data,req->session,(req+1),reason);

          icqEvent(icq, 0, 'U', ICQEVENT_REQREFUSED, id);
          icqRemoveElement(icq->requests,req);
          return 0;
       }
    }
    return -1;
 }

 int EXPENTRY icqDestroySessionRequests(HICQ icq, ULONG session)
 {
#ifdef EXTENDED_LOG
    char logBuffer[0x0100];
#endif

    REQUEST *req;
    REQUEST *next;

    next = icqQueryFirstElement(icq->requests);

    while(next)
    {
       req  = next;
       next = icqQueryNextElement(icq->requests,req);

       if(req->session == session)
       {
#ifdef EXTENDED_LOG
          sprintf(logBuffer,"Request %08lx for message %08lx destroyed",(ULONG) req, req->id);
          icqWriteSysLog(icq,PROJECT,logBuffer);
#endif

          /*--- Nesse ponto deveria chamar a funcao de cancelamento do request ---*/
          icqEvent(icq, req->uin, 'U', ICQEVENT_REQDELETED, req->id);
          icqRemoveElement(icq->requests,req);
       }
    }
    return 0;
 }

 int EXPENTRY icqRequestAccepted(HICQ icq, ULONG id, int port, int sz, const char *reason)
 {
    REQUEST *req;
    char    *txt;
#ifdef EXTENDED_LOG
    char logBuffer[0x0100];
#endif

    if(!sz)
       reason = NULL;

    if(reason && sz < 0 )
       sz = strlen(reason)+1;

    DBGTracex(id);

    for(req = icqQueryFirstElement(icq->requests);req;req = icqQueryNextElement(icq->requests,req))
    {
       if(req->id == id && req->out)
       {
#ifdef EXTENDED_LOG
          sprintf(logBuffer,"Request %08lx for message %08lx accepted",(ULONG) req,id);
          icqWriteSysLog(icq,PROJECT,logBuffer);
#endif
          if(reason)
          {
             txt = malloc(sz+5);
             if(txt)
             {
                strncpy(txt,reason,sz);
#ifdef EXTENDED_LOG
                icqWriteSysLog(icq,PROJECT,txt);
#endif
                DBGMessage(txt);
                /********************* REGISTRAR A MENSAGEM ************************/
                free(txt);
             }
          }

          if(req->mgr && req->mgr->accepted)
             req->mgr->accepted(icq,req->id,req->data,req->session, port,(req+1));

          icqEvent(icq, req->uin, 'U', ICQEVENT_REQACCEPTED, id);
          icqRemoveElement(icq->requests,req);
          return 0;
       }
    }
    return -1;
 }

 int EXPENTRY icqRequestRefused(HICQ icq, ULONG id, int port, int sz, const char *reason)
 {
    REQUEST *req;
    char    *txt;
#ifdef EXTENDED_LOG
    char logBuffer[0x0100];
#endif

    if(!sz)
       reason = NULL;

    if(reason && sz < 0 )
       sz = strlen(reason)+1;

    DBGTracex(id);

    for(req = icqQueryFirstElement(icq->requests);req;req = icqQueryNextElement(icq->requests,req))
    {
       if(req->id == id && req->out)
       {
#ifdef EXTENDED_LOG
          sprintf(logBuffer,"Request %08lx for message %08lx refused",(ULONG) req,id);
          icqWriteSysLog(icq,PROJECT,logBuffer);
#endif

          if(reason)
          {
             txt = malloc(sz+5);
             if(txt)
             {
                strncpy(txt,reason,sz);
#ifdef EXTENDED_LOG
                icqWriteSysLog(icq,PROJECT,txt);
#endif
                DBGMessage(txt);
                /********************* REGISTRAR A MENSAGEM ************************/
                free(txt);
             }
          }

          if(req->mgr && req->mgr->refused)
             req->mgr->refused(icq,req->id,req->data,req->session,port,(req+1));

          icqEvent(icq, req->uin, 'U', ICQEVENT_REQREFUSED, id);
          icqRemoveElement(icq->requests,req);
          return 0;
       }
    }
    return -1;
 }

 void * EXPENTRY icqQueryNextRequest(HICQ icq, ULONG session, ULONG *id)
 {
    REQUEST *req;

    if(*id)
    {
       for(req = icqQueryFirstElement(icq->requests);req && req->id != *id;req = icqQueryNextElement(icq->requests,req));

       if(req)
          for(req = icqQueryNextElement(icq->requests,req);req && (req->session != session && session);req = icqQueryNextElement(icq->requests,req));
    }
    else if(session)
    {
       for(req = icqQueryFirstElement(icq->requests);req && req->session != session;req = icqQueryNextElement(icq->requests,req));
    }
    else
    {
       req = icqQueryFirstElement(icq->requests);
    }

    if(req)
    {
       *id = req->id;
       return (req+1);
    }

    return 0;
 }

 int EXPENTRY icqSetRequestSession(HICQ icq, ULONG id, BOOL out, ULONG session)
 {
     REQUEST *req;
#ifdef EXTENDED_LOG
    char logBuffer[0x0100];
#endif

    for(req = icqQueryFirstElement(icq->requests);req;req = icqQueryNextElement(icq->requests,req))
    {
       if(req->id == id && req->out == out)
       {
#ifdef EXTENDED_LOG
          sprintf(logBuffer,"Request %08lx for message %08lx moved to session %08lx",(ULONG) req,id, (ULONG) session);
          icqWriteSysLog(icq,PROJECT,logBuffer);
#endif
          req->session = session;
          return 0;
       }
    }
    return -1;

 }

