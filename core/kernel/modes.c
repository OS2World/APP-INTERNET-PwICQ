/*
 * MODES.C - Ajusta modos de conexao ICQ *
 */

 #include <string.h>
 #include <malloc.h>

 #include <icqkernel.h>


/*---[ Tabela de modos ]-------------------------------------------------------------------------*/
/*
 const ICQMODETABLE tbl[] = {   ICQ_ONLINE,     5,      "Online",
                                ICQ_AWAY,       6,      "Away",
                                ICQ_NA,        40,      "N/A",
                                ICQ_OCCUPIED,  12,      "Busy",
                                ICQ_DND,       42,      "DND",
                                ICQ_FREECHAT,  13,      "Free for chat",
                                ICQ_PRIVACY,   14,      "Invisible",
                                ICQ_OFFLINE,    7,      "Offline",

                                0,              0,      NULL
                             };

*/
/*---[ Implementacao ]---------------------------------------------------------------------------*/

 const ICQMODETABLE *icqQueryModeTable(HICQ icq)
 {
#ifndef NDEBUG
    DBGTracex(icq);
    if(icq->sz != sizeof(ICQ))
       DBGMessage("Parametro invalido passado a icqQueryModeTable");
#endif

    DBGTracex(icq->c2s);

    if(!icq->c2s)
       return NULL;

    DBGTracex(icq->c2s->modeButton);

    return icq->c2s->modeButton;
 }

 ULONG EXPENTRY  icqSetModeByName(HICQ icq, const char *mode)
 {
    const ICQMODETABLE *t;

    for(t = icqQueryModeTable(icq); t && t->descr; t++)
    {
       if(!stricmp(t->descr,mode))
          return icqSetOnlineMode(icq,t->mode);
    }

    return icq->currentMode;
 }

 ULONG EXPENTRY icqSetOnlineMode(HICQ icq, ULONG mode)
 {
    DBGTracex(icq->currentMode);

    if(icq->currentMode != mode && icq->c2s && icq->c2s->setMode)
    {
       icq->currentMode = icq->c2s->setMode(icq,mode,icq->currentMode);

       if(icq->currentMode == mode)
       {
          icqSetAwayMessage(icq, NULL);

          if(icq->onlineFlags & ICQF_ONLINE)
             icqSystemEvent(icq,ICQEVENT_CHANGED);

          if(mode != icqQueryOfflineMode(icq))
             icqSaveValue(icq,"lastMode",mode);
       }
    }

    return icq->currentMode;
 }

 ULONG EXPENTRY icqQueryOnlineMode(HICQ icq)
 {
    return icq->currentMode;
 }

 ULONG EXPENTRY icqDisconnect(HICQ icq)
 {
    ULONG mode    = icq->currentMode;
    ULONG offline = icqQueryOfflineMode(icq);

    icqSetOnlineMode(icq,offline);
    icqSaveValue(icq,"lastMode",offline);

    return mode;
 }

 ULONG EXPENTRY icqReconnect(HICQ icq)
 {
    ULONG lastMode = icqLoadValue(icq,"lastMode",icqQueryOfflineMode(icq));

    DBGTracex(lastMode);
    if(icq->loginRetry++ > icqLoadValue(icq,"maxReconnects",10))
    {
       icqWriteSysLog(icq,PROJECT,"Too much reconnects");
       return icqQueryOfflineMode(icq);
    }

    if(lastMode != icqQueryOfflineMode(icq))
    {
       icqWriteSysLog(icq,PROJECT,"Activating auto-reconnect");
       icqSetOnlineMode(icq,lastMode);
    }

    return lastMode;
 }

 USHORT  EXPENTRY icqQueryModeIcon(HICQ icq, ULONG mode)
 {
    if(icq->c2s && icq->c2s->queryModeIcon)
       return icq->c2s->queryModeIcon(icq,mode);

    return mode == 0xFFFFFFFF ? 7 : 5;
 }

 ULONG EXPENTRY icqSetOffline(HICQ icq)
 {
    return icqSetOnlineMode(icq,icqQueryOfflineMode(icq));
 }

 ULONG EXPENTRY icqQueryOfflineMode(HICQ icq)
 {
   /* ATENCAO: Esssa funcao e replicada em pwICQLIB.C */
    if(icq->offline)
       return icq->offline->mode;
    return 0xFFFFFFFF;
 }

