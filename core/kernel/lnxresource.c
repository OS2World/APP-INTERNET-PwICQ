/*
 * lnxresource.c - Linux resource file implementation
 */

 #include <malloc.h>
 #include <string.h>
 #include <dlfcn.h>
 #include <stdio.h>
 
 #include <icqtlkt.h>
 #include <pwicqrc.h>

/*---[ Structures ]-----------------------------------------------------------*/


/*---[ Constants ]------------------------------------------------------------*/


/*---[ Prototipes ]-----------------------------------------------------------*/


/*---[ Implementation ]-------------------------------------------------------*/
 
 const void * ICQAPI _icqGetResourceDescriptor(HICQ icq, HMODULE hMod, int szBlock)
 {
	const struct icqResourceDescriptor * (*getResource)(void) = NULL;
	DBGTracex(hMod);

	dlerror();
	getResource = dlsym(hMod, "icqresource_getDescriptor");

	if(dlerror())
	   return NULL;

    return getResource();
 }

 int ICQAPI icqGetString(HICQ icq, HMODULE hMod, USHORT id, char *buffer, int sz)
 {
	const struct icqResourceDescriptor *rc = (struct icqResourceDescriptor *) _icqGetResourceDescriptor(icq,hMod,sizeof(struct icqResourceDescriptor));
    const struct icqStringTable        *st;
	int                                f;

	DBGTracex(rc);

	if(!rc)
	   return -1;

    if(rc->sz != sizeof(struct icqResourceDescriptor))
	   return 1;

	st = rc->stringTable;

	for(f=0;f<rc->stringCount;f++)
	{
	   if(st[f].id == id)
	   {
		  DBGPrint("String lida: %s",st[f].str);
		  if(buffer)
			 strncpy(buffer,st[f].str,sz);
		  return strlen(st[f].str);
	   }
	}

	if(buffer)
	   *buffer = 0;

    return 0;
 }

 const void * ICQAPI _icqGetDialogDescriptor(HICQ icq, HMODULE hMod, USHORT id, int *qtd, int szBlock)
 {
	const struct icqResourceDescriptor *rc = (struct icqResourceDescriptor *) _icqGetResourceDescriptor(icq,hMod,sizeof(struct icqResourceDescriptor));
    const struct icqDialogTable        *tbl;
	int                                f;

	if(!(rc && szBlock == sizeof(struct icqDialogDescriptor)) )
	   return 0;

	tbl = rc->dialogTable;

	if(!tbl)
	   return 0;

	CHKPoint();

	for(f=0;f<rc->dialogCount && tbl[f].id != id;f++);

	if(f >= rc->dialogCount)
	   return 0;

	tbl += f;

	DBGTrace(tbl->id);
	
	if(qtd)
	   *qtd = tbl->el;

    return (void *) tbl->descriptor;

 }


 
