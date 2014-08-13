/*
 * pwICQLIB.C - Biblioteca esta¡tica para acesso as funcoes do nucleo
 */

#include <icqkernel.h>

#ifndef USEICQLIB
   #error Use this module only with the USEICQLIB macro defined
#endif   

#ifdef __cplusplus
   extern "C" {
#endif	  

/*---[ Tabela ]----------------------------------------------------------------------------*/

 const struct icqftable *pwICQStaticLibraryEntryPoints = NULL;

/*---[ Implementacao ]---------------------------------------------------------------------*/

 int EXPENTRY pwICQConfigureLibrary(const struct icqftable *tbl)
 {
    if( tbl->sz == sizeof(struct icqftable)
        && tbl->Version      == ICQFTABLE_VERSION
        && tbl->szKrnl       == sizeof(ICQ)
        && tbl->szUser       == sizeof(USERINFO)
        && tbl->pwICQVersion == PWICQVERSIONID
      )
    {
       DBGMessage(tbl->ident);
       pwICQStaticLibraryEntryPoints = tbl;
       return 0;
    }
	
	DBGMessage("Invalid library");
    return -1;
 }
 /*
 void EXPENTRY icqAbend(HICQ icq, ULONG msg)
 {
	pwICQStaticLibraryEntryPoints->icqAbend(icq,msg);
 }
 
 void EXPENTRY icqWriteSysLog(HICQ icq, const char *proj, const char *txt)
 {
	pwICQStaticLibraryEntryPoints->icqWriteSysLog(icq,proj,txt);
 }
 */ 
 
#ifdef __cplusplus
   }
#endif	  

   
