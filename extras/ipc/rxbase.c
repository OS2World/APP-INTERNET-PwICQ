/*
 * rxbase.c - Funcoes de apoio Rexx
 */

 #define INCL_REXXSAA

#ifdef __OS2__
 #pragma strings(readonly)
 #include <os2.h>
 #include <rexxsaa.h>
#else
 #include <rexx.h>
#endif

 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include <malloc.h>

 #include "rxapi.h"

/*---[ Implementacao ]----------------------------------------------------------*/

 static void doMemCpy(char *d, const char *o, int sz)
 {
    while(sz--)
       *(d++) = *(o++);
 }

 void CopyInteger( int val, PRXSTRING dest)
 {
    char buffer[34];
#ifdef __OS2__    
    CopyString( itoa(val,buffer,10), dest );
#else    
    sprintf(buffer,"%d",val);
    CopyString( buffer, dest );
#endif    
 }

 void CopyString( char *src, PRXSTRING dest )
 {
    void *mem = NULL;
    int  len  = strlen(src);

    if( ( !src || len == 0 ) && dest->strptr != NULL )
    {
       SetNullRXString( dest );
    }
    else if( dest->strptr != NULL && len <= dest->strlength )
    {
        memset( dest->strptr, 0, dest->strlength );
        doMemCpy( dest->strptr, src, len );

        dest->strlength = len;
    }
    else
    {
        /* OK, at this point we've determined the buffer is too small
           for use to use, so we'll allocate a new one */

        SetNullRXString( dest );

#ifdef __OS2__
        if( DosAllocMem( &mem, len + 1, PAG_COMMIT | PAG_WRITE | PAG_READ ) )
            return;
#else
        mem = malloc(len+1);
        if(!mem)
           return;
#endif             
        dest->strptr    = mem;
        dest->strlength = len;

         memset( dest->strptr, 0, len + 1 );
         doMemCpy( dest->strptr, src, len );

    }

    return;

 }

 void SetNullRXString( PRXSTRING str )
 {
    if( str )
    {
       if( str->strptr )
          str->strptr[0] = NULLCHAR;
       str->strlength = 0;
    }
 }

