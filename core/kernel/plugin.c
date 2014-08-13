/*
 * PLUGIN.C - Chamada a funcoes de plugin
 */

#ifdef __OS2__
 #define INCL_DOSMODULEMGR
#endif

 #include <string.h>
 #include <icqkernel.h>

/*---[ Implementacao ]------------------------------------------------------*/

 BOOL EXPENTRY icqPacket(HICQ icq, USHORT vrs, void *pkt, int size, long ip, int port)
 {
    /* Chama a funcao Packet de todos os plugins ate alguma retornar TRUE */
    HPLUGIN     p;

    for(p=icqQueryFirstElement(icq->plugins);p;p=icqQueryNextElement(icq->plugins,p))
    {
       if(isProcValid(p,p->Packet))
       {
          if(p->Packet(icq,p->dataBlock,vrs,pkt,size,ip,port))
             return TRUE;
       }
    }

    return FALSE;
 }

 HPLUGIN EXPENTRY icqQueryPlugin(HICQ icq, const char *name)
 {
    HPLUGIN p;
    for(p=icqQueryFirstElement(icq->plugins);p;p=icqQueryNextElement(icq->plugins,p))
    {
       if(!stricmp((char *)(p+1),name) )
          return p;
    }
    return NULL;
 }

 HPLUGIN EXPENTRY icqQueryNextPlugin(HICQ icq, HPLUGIN plugin)
 {
    /* Obtem o proximo plugin da lista */

    if(!icq->plugins)
       return NULL;

    if(plugin)
       plugin = icqQueryNextElement(icq->plugins,plugin);
    else
       plugin = icqQueryFirstElement(icq->plugins);

    return plugin;

 }

 void EXPENTRY icqInsertConfigPage(HICQ icq, ULONG uin, USHORT type, HWND hwnd, const DLGINSERT *insert, char *buffer)
 {
    HPLUGIN     p;
#ifdef __OS2__
    void        (EXPENTRY *icqnetw_ConfigPage)(HICQ, void *, ULONG, USHORT, HWND, const DLGINSERT *, char *);
#endif

    for(p=icqQueryFirstElement(icq->plugins);p;p=icqQueryNextElement(icq->plugins,p))
    {
       if(isProcValid(p,p->ConfigPage))
          p->ConfigPage(icq,p->dataBlock,uin,type,hwnd,insert,buffer);
    }

#ifdef __OS2__
    if(icq->icqNetW)
    {
       if(!DosQueryProcAddr(icq->icqNetW, 0, "icqnetw_ConfigPage", (PFN *) &icqnetw_ConfigPage))
          icqnetw_ConfigPage(icq, NULL, uin, type, hwnd, insert, buffer);
    }
#endif

 }

 void * EXPENTRY icqGetPluginDataBlock(HICQ icq, HMODULE mod)
 {
    HPLUGIN     p;
    for(p=icqQueryFirstElement(icq->plugins);p;p=icqQueryNextElement(icq->plugins,p))
    {
       if(p->module == mod)
          return p->dataBlock;
    }
    return NULL;
 }

 void * EXPENTRY icqQueryPluginDataBlock(HICQ icq, HPLUGIN p)
 {
    return p->dataBlock;
 }

 void EXPENTRY icqInsertSendToOptions(HICQ icq, hWindow hwnd, USHORT type, BOOL out, const MSGMGR *mgr, int ( * _System insert)(HWND, USHORT, const char *, SENDTOCALLBACK *))
 {
    HPLUGIN     p;

    CHKPoint();

    for(p=icqQueryFirstElement(icq->plugins);p;p=icqQueryNextElement(icq->plugins,p))
    {
       if(isProcValid(p,p->snd2Option))
          p->snd2Option(icq, hwnd, type, out, mgr, insert);
    }

    CHKPoint();
 }


 int EXPENTRY icqStopPlugin(HICQ icq, HPLUGIN p)
 {
    int (* _System  stop)(HICQ, HPLUGIN, void *);

    if(icqLoadSymbol(p, "Stop", (void *) &stop))
       return 0;

    return stop(icq,p,p->dataBlock);
 }

 int EXPENTRY icqStartPlugin(HICQ icq, HPLUGIN p)
 {
    int (* _System  start)(HICQ, HPLUGIN, void *);

    if(icqLoadSymbol(p, "Start", (void *) &start))
       return 0;

    return start(icq,p,p->dataBlock);
 }

 void startPlugins(HICQ icq)
 {
    HPLUGIN     n;
    HPLUGIN     p;

    p=icqQueryFirstElement(icq->plugins);
    while(p)
    {
       n = icqQueryNextElement(icq->plugins,p);

       if(icqStartPlugin(icq,p))
          icqUnloadPlugin(icq, p);

       /* Inicializacao completa, pego o proximo */
       p = n;
    }
 }




