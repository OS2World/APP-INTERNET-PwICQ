/*
 * LNXPLUG.C - Carga e descarga de plugins
 */

#ifndef linux
   #error Only for Linux version
#endif

 #include <string.h>
 #include <malloc.h>
 #include <dlfcn.h>
 #include <sys/types.h>
 #include <dirent.h>

 #include <icqkernel.h>

 #include "icqcore.h"

/*---[ Prototipos ]----------------------------------------------------------------------------------------*/

 static void *loadProcedure(HPLUGIN, const char *);
 static void *loadFunction(HMODULE, const char *, const char *);

/*---[ Implementacao ]-------------------------------------------------------------------------------------*/

 void loadPlugins(HICQ icq)
 {
    char                buffer[0x0100];
    DIR                 *dir;
    struct dirent       *arq;

    CHKPoint();

    icqLoadString(icq, "plugin-files", "",buffer, 0xFF);

        if(!*buffer)
           icqQueryProgramPath(icq, "plugins", buffer, 0xff);

    DBGMessage(buffer);

    dir = opendir(buffer);

    DBGTracex(dir);

    if(!dir)
    {
       sysLog(icq,"Unable to open the plugins directory");
       return;
    }

    arq = readdir(dir);

    while(arq)
    {
       if(*arq->d_name != '.')
          icqLoadPlugin(icq, buffer, arq->d_name);
       arq = readdir(dir);
    }

    DBGTracex(icqQueryFirstElement(icq->plugins));

    closedir(dir);
 }

 void unloadPlugins(HICQ icq)
 {
    HPLUGIN p = icqQueryFirstElement(icq->plugins);
    HPLUGIN n;

    while(p)
    {
       n = icqQueryNextElement(icq->plugins,p);
       DBGTracex(p);
       icqUnloadPlugin(icq,p);
       p = n;
    }

 }

 static int initStaticLib(HICQ icq, HMODULE module)
 {
    int (* _System cfigLib)(const struct icqftable *);

    dlerror();

    cfigLib = dlsym(module, "pwICQConfigureLibrary");

    if(dlerror())
       return 0;

    DBGMessage("*** Iniciando static-library");
    return cfigLib(&pwICQLibraryEntryPoints);
 }

 HPLUGIN EXPENTRY icqLoadPlugin(HICQ icq, const char *path, char *modName)
 {
    int         (* _System Configure)(HICQ, HMODULE);

    char        buffer[0x0100];
    char        fileName[0x0100];
    HMODULE     module;
    HPLUGIN     p;
    char        *ptr;
    char        *pos;
    int         sz      = 0;

	*fileName = 0;
	
	if(path)
	{
       strncpy(fileName,path,0xFF);
	}
	else
	{
       icqLoadString(icq, "plugin-files", "",fileName, 0xFF);	   
       if(!*fileName)
          icqQueryProgramPath(icq, "plugins", fileName, 0xff);
	}

    if(modName)
       strncat(fileName,modName,0xFF);

    DBGMessage(fileName);
    module = dlopen(fileName,RTLD_NOW);

    DBGTracex(module);

    if(!module)
    {
       strcpy(buffer,"Error \"");
       strncat(buffer,dlerror(),0xFF);
       strncat(buffer,"\" when loading ",0xFF);
       strncat(buffer,fileName,0xFF);
       sysLog(icq,buffer);
       return NULL;
    }

    for(pos=ptr=fileName;*ptr && *ptr != '.';ptr++)
    {
       if(*ptr == '/')
          pos = ptr+1;
    }
    *ptr = 0;

    strncpy(buffer,pos,0xFF);

    DBGMessage(buffer);

    if(initStaticLib(icq, module))
    {
       icqWriteSysLog(icq,buffer,"pwICQ static library version mismatch, can't load module");
       dlclose(module);
       return NULL;
    }

    Configure = loadFunction(module,"Configure",buffer);

    if( ((ULONG) Configure) != 0xFFFFFFFF)
       sz = Configure(icq, module);

    if(sz < 0)
    {
       dlclose(module);
       return NULL;
    }

    p = icqAddElement(icq->plugins, sizeof(PLUGIN)+strlen(buffer)+sz+5);
    if(!p)
    {
       dlclose(module);
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

    p->module = module;

    p->ConfigPage        = loadProcedure(p, "ConfigPage");

    p->Menu              = loadProcedure(p, "ProcessMenu");
    p->ExternCMD         = loadProcedure(p, "ExternCMD");

    p->SystemEvent       = loadProcedure(p, "SystemEvent");
    p->UserEvent         = loadProcedure(p, "UserEvent");
    p->ProcessEvent      = loadProcedure(p, "Event");
    p->Timer             = loadProcedure(p, "Timer");
    p->rpcRequest        = loadProcedure(p, "rpcRequest");

    p->PreProcessMessage = loadProcedure(p, "PreProcessMessage");
    p->Packet            = loadProcedure(p, "Packet");
    p->snd2Option        = loadProcedure(p, "addSendTo");
    p->chkCommand        = loadProcedure(p, "Interpret");

    return p;

 }

 static void *loadProcedure(HPLUGIN p, const char *name)
 {
    return loadFunction(p->module,name, (const char *) (p+1) );
 }

 int EXPENTRY icqLoadSymbol(HPLUGIN p, const char *name, void **proc)
 {
    char procName[80];

    strncpy(procName,(const char *) (p+1),79);
    strncat(procName,"_",79);
    strncat(procName,name,79);

    *proc = dlsym(p->module,procName);

    if(dlerror())
    {
       *proc = (void *) 0xFFFFFFFF;
       return -1;
    }

    return 0;
 }


 static void *loadFunction(HMODULE module, const char *name, const char *id)
 {
    char procName[80];
    void *ret;

    strncpy(procName,id,79);
    strncat(procName,"_",79);
    strncat(procName,name,79);

    dlerror();

//    DBGMessage(procName);
    ret = dlsym(module, procName);

    if(!dlerror())
       return ret;
    
    strcpy(procName,"pwICQ_");
    strncat(procName,name,79);
    DBGMessage(procName);

    ret = dlsym(module, procName);
    if(!dlerror())
       return ret;
    
    return (void *) 0xFFFFFFFF;
 }

 int EXPENTRY icqUnloadPlugin(HICQ icq, HPLUGIN p)
 {
    void    (* _System Terminate)(HICQ, void *) = loadProcedure(p, "Terminate");
    HMODULE mod                         = p->module;

    if( ((unsigned long)Terminate) != 0xFFFFFFFF)
       Terminate(icq,p->dataBlock);

    memset(p,0,sizeof(PLUGIN));

    dlclose(mod);

    return 0;
 }

 /*
 static int dunno(void)
 {
    return -1;
 }
 */

