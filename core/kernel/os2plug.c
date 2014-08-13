/*
 * OS2PLUG.C - Carga e descarga de plugins
 */

 #define INCL_DOSMODULEMGR
 #define INCL_ERRORS
 #define INCL_DOSPROCESS

 #include <stdio.h>
 #include <time.h>
 #include <string.h>

 #include <icqkernel.h>

 #include "icqcore.h"

/*---[ Prototipos ]-----------------------------------------------------------*/

 static BOOL loadProcedure(HPLUGIN, PFN *, const char *);
 static BOOL loadFunction(HMODULE, PFN *, const char *, const char *);
 static void loadFromPath(HICQ, char *, char *);

/*---[ Implementacao ]--------------------------------------------------------*/

 void loadPlugins(HICQ icq)
 {
    char                buffer[0x0100];
    char                *pos;

//    icqQueryPath(icq, "plugins\\", buffer, 0xFF);

    strncpy(buffer,icq->programPath,0x80);
    strncat(buffer,"plugins\\",0xF0);
    pos = buffer+strlen(buffer);
    strcpy(pos,"*.dll");

    loadFromPath(icq, pos, buffer);

 }

 static void loadFromPath(HICQ icq, char *pos, char *buffer)
 {
    FILEFINDBUF3        file;
    HPLUGIN             p;
    HDIR                s;
    ULONG               count;
    USHORT              id = 0;

    DBGMessage(buffer);

    s         = HDIR_CREATE;
    count     = 1;

    if(DosFindFirst(     buffer,
                         &s,
                         0,
                         (PVOID) &file,
                         sizeof(file),
                         &count,
                         FIL_STANDARD ) == 0 )
    {
       do
       {
          *pos = 0;
          p = icqLoadPlugin(icq,buffer,file.achName);

          if(p)
          {
             p->id = id++;
          }

       } while(DosFindNext( s,(PVOID) &file,sizeof(file),&count) == 0);

       DosFindClose(s);
    }


 }

 void unloadPlugins(HICQ icq)
 {
    HPLUGIN p = icqQueryFirstElement(icq->plugins);
    HPLUGIN n;

    while(p)
    {
       n = icqQueryNextElement(icq->plugins,p);
       icqUnloadPlugin(icq,p);
       p = n;
    }

 }

 HPLUGIN EXPENTRY icqLoadPlugin(HICQ icq, const char *path, char *modName)
 {
    /* Carrega e inicializa um plugin */
    char        fileName[0x0100];
    char        buffer[0x0100];
    char	msg[0x0100];
    HPLUGIN     p;
    HMODULE     module;
    APIRET      rc;
    char        *ptr;
    char        *pos;
    int         sz      = 0;
    int         (* _System Configure)(HICQ, HMODULE);
    int         (* _System cfigLib)(const struct icqftable *);

    if(path)
    {
       strncpy(fileName,path,0xFF);
    }
    else
    {
       strncpy(fileName,icq->programPath,0x80);
       strncat(fileName,"plugins\\",0xF0);
    }

    if(modName)
       strncat(fileName,modName,0xFF);

    strlwr(fileName);

    *msg = 0;
    rc   = DosLoadModule(msg, 0x0FF, (PSZ) fileName, &module);

    DBGTracex(module);

    switch(rc)
    {
    case 0:
       break;

    case 295:
       strcpy(buffer,"Init routine failed in ");
       strncat(buffer,fileName,0xFF);
       icqWriteSysRC(icq,PROJECT,rc,buffer);
       return NULL;
       break;

    default:
       strcpy(buffer,"Error loading ");
       strncat(buffer,fileName,0xFF);
       icqWriteSysRC(icq,PROJECT,rc,buffer);
       strcpy(buffer,"Object: ");
       strncat(buffer,msg,0xFF);
       icqWriteSysLog(icq,PROJECT,buffer);
       return NULL;
    }

    for(pos=ptr=fileName;*ptr;ptr++)
    {
       if(*ptr == '\\' || *ptr == ':')
          pos = ptr+1;
    }
    strncpy(buffer,pos,0xFF);

    for(ptr = buffer;*ptr && *ptr != '.';ptr++);
    *ptr = 0;

    /* Chama a funcao de configuracao, se retornar negativo cancela */

    if(loadFunction(module, (PFN *) &cfigLib, buffer, "pwICQConfigureLibrary"))
    {
       sz = cfigLib(&pwICQLibraryEntryPoints);
       if(sz)
       {
          icqWriteSysLog(icq,buffer,"pwICQ static library version mismatch, can't load module");
          DosFreeModule(module);
          return NULL;
       }
    }

    if(loadFunction(module, (PFN *) &Configure, buffer, "Configure"))
       sz = Configure(icq, module);

    DBGTrace(sz);

    if(sz < 0)
    {
       DosFreeModule(module);
       return NULL;
    }

    p = icqAddElement(icq->plugins, sizeof(PLUGIN) + strlen(buffer) + sz + 5);
    if(!p)
    {
       DosFreeModule(module);
       return NULL;
    }
    memset(p,0,sizeof(PLUGIN));

    ptr = (char *)(p+1);
    strcpy(ptr,buffer);

    if(sz > 0)
    {
       p->dataBlock = ptr+(strlen(ptr)+1);
       memset(p->dataBlock,0,sz);
       *( (USHORT *) p->dataBlock) = sz;
    }

    DBGMessage(ptr);

    p->module    = module;

    loadProcedure(p, (PFN *) &p->ConfigPage,         "ConfigPage");

    loadProcedure(p, (PFN *) &p->Menu,               "ProcessMenu");
    loadProcedure(p, (PFN *) &p->ExternCMD,          "ExternCMD");

    loadProcedure(p, (PFN *) &p->SystemEvent,        "SystemEvent");
    loadProcedure(p, (PFN *) &p->UserEvent,          "UserEvent");
    loadProcedure(p, (PFN *) &p->ProcessEvent,       "Event");
    loadProcedure(p, (PFN *) &p->Timer,              "Timer");

    loadProcedure(p, (PFN *) &p->PreProcessMessage,  "PreProcessMessage");

    loadProcedure(p, (PFN *) &p->rpcRequest,         "rpcRequest");
    loadProcedure(p, (PFN *) &p->Packet,             "Packet");

    loadProcedure(p, (PFN *) &p->snd2Option,         "addSendTo");
    loadProcedure(p, (PFN *) &p->chkCommand,         "Interpret");

    return p;
 }

 int EXPENTRY icqLoadSymbol(HPLUGIN p, const char *name, void **proc)
 {
    char procName[80];
    int	 rc;

    strncpy(procName,(const char *) (p+1),79);
    strncat(procName,"_",79);
    strncat(procName,name,79);

    rc = DosQueryProcAddr(p->module, 0, (PSZ) procName, (PFN *) proc);

    if(rc)
       *proc = 0;

    return rc;
 }

 static BOOL loadProcedure(HPLUGIN p, PFN *proc, const char *name)
 {
    return loadFunction(p->module,proc,(const char *) (p+1), name);
 }

 static BOOL loadFunction(HMODULE mod, PFN *proc, const char *plg, const char *name)
 {
    char        procName[80];

    if(!mod)
       return FALSE;

    strncpy(procName,(char *) plg,79);
    strncat(procName,"_",79);
    strncat(procName,name,79);

//    DBGMessage(procName);
//    DBGTrace(DosQueryProcAddr(mod, 0, (PSZ) procName, proc));

    if(!DosQueryProcAddr(mod, 0, (PSZ) procName, proc))
       return TRUE;

// pwICQ_Configure

    strcpy(procName,"pwICQ_");
    strncat(procName,name,79);
    if(!DosQueryProcAddr(mod, 0, (PSZ) procName, proc))
       return TRUE;

    strcpy(procName,"icq");
    strncat(procName,name,79);
    if(!DosQueryProcAddr(mod, 0, (PSZ) procName, proc))
       return TRUE;

    strcpy(procName,"ICQP");
    strncat(procName,name,79);
    if(!DosQueryProcAddr(mod, 0, (PSZ) procName, proc))
       return TRUE;

    strcpy(procName,"ICQ");
    strncat(procName,name,79);
    if(!DosQueryProcAddr(mod, 0, (PSZ) procName, proc))
       return TRUE;

    *proc = NULL;

    return FALSE;
 }

 int EXPENTRY icqUnloadPlugin(HICQ icq, HPLUGIN p)
 {
    void    (* _System Terminate)(HICQ,void *);

    HMODULE mod                         = p->module;
    int     f;

    if(mod)
    {
       if(loadProcedure(p, (PFN *) &Terminate, "Terminate"))
          Terminate(icq,p->dataBlock);
    }

    memset(p,0,sizeof(PLUGIN));

    DosSleep(10);

    icqRemoveElement(icq->plugins, p);
    DosFreeModule(mod);

    return 0;
 }



