/*
 * adm.c - Administration port listener
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSPROCESS
#endif

 #include <stdio.h>
 #include <malloc.h>
 #include <string.h>

 #include "ishared.h"

/*---[ Structures and Macros ]---------------------------------------------------------------------------*/


/*---[ Function processors ]-----------------------------------------------------------------------------*/


/*---[ Prototipes ]--------------------------------------------------------------------------------------*/


/*---[ Implementation ]----------------------------------------------------------------------------------*/

 int ICQAPI ishare_RecBlock(int sock, int sz, void *buffer, const USHORT *flag)
 {
    static const USHORT tr = 0;

    int  toRec  = sz;
    int  bytes  = 0;
    char *ptr   = buffer;

    if(!flag)
       flag = &tr;

    while( toRec > 0 && (bytes = ishare_Recv(sock,ptr,toRec)) >= 0 && !*flag)
    {
       ptr  += bytes;
       toRec -= bytes;
    }
    return toRec ? 0 : sz;
 }

 USHORT ICQAPI ishare_RecShort(ISHARED_ADMIN *adm)
 {
    USHORT rc = 0;
    if(adm->sock > 0)
       ishare_RecBlock(adm->sock, 2, &rc, &adm->rc);
    return rc;
 }

 int ICQAPI ishare_RecString(ISHARED_ADMIN *adm, char *str, int szBlock)
 {
    char sz;
    int  rc;

    *str = 0;

    if(adm->sock < 0 || adm->rc)
       return -1;

    while( (rc = ishare_Recv(adm->sock,&sz,1)) == 0 && !adm->rc);

    if(rc < 0)
    {
       adm->rc = ISHARE_STATUS_MALFORMED;
       return -1;
    }

    if(!sz)
       return 0;

    if(sz > szBlock)
    {
       adm->rc = ISHARE_STATUS_MALFORMED;
       return -1;
    }

	*(str+sz) = 0;
    return ishare_RecBlock(adm->sock, sz, str,&adm->rc);

 }

 int ICQAPI ishare_RecFilename(ISHARED_ADMIN *adm, char *str, int szBlock)
 {
    USHORT sz = ishare_RecShort(adm);
	
	DBGTrace(sz);

    if(adm->sock < 0 || adm->rc)
       return -1;

    if(!sz)
       return 0;

    if(sz > szBlock)
    {
       adm->rc = ISHARE_STATUS_MALFORMED;
       return -1;
    }

    *(str+sz) = 0;
    return ishare_RecBlock(adm->sock, sz, str, &adm->rc);

 }

 int ICQAPI ishare_SendString(ISHARED_ADMIN *adm, char *str, int sz)
 {
	UCHAR bytes = sz;
	if(sz < 0)
	   bytes = strlen(str);


	ishare_Send(adm->sock,&bytes,1);
	
	if(bytes)
       ishare_Send(adm->sock,str,bytes);
	
	return 0;
 }

 int ICQAPI ishare_SendFilename(ISHARED_ADMIN *adm, char *str, int sz)
 {
	USHORT bytes = sz;

	if(sz < 0)
	   bytes = strlen(str);
	
	ishare_Send(adm->sock,&bytes,2);
	
	if(bytes)
       ishare_Send(adm->sock,str,bytes);
	
	return 0;
 }
