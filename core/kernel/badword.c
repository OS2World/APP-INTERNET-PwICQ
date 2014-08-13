/*
 * badword.c
 */


#ifdef __IBMC__
 #pragma strings(readonly)
#endif

 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>
 #include <time.h>
 #include <ctype.h>

 #include <icqkernel.h>
 #include "icqcore.h"


/*---[ Definicoes ]------------------------------------------------------------------------------------------*/


/*---[ Prototipos ]------------------------------------------------------------------------------------------*/



/*---[ Constantes ]------------------------------------------------------------------------------------------*/


/*---[ Implementacao ]---------------------------------------------------------------------------------------*/

 BOOL EXPENTRY icqCheckBadWord(HICQ icq, const char *text)
 {
    const char  *src;
    char        *dst;
    char        *dup;
    BOOL        ret;

    if(!icq->badWords)
       return FALSE;

    if(icqLoadValue(icq,"badWord.Strip",1))
    {
       dst = dup = strdup(text);

       if(!dst)
       {
          icqWriteSysLog(icq,PROJECT,"Memory allocation error when striping spaces");
          return FALSE;
       }

       for(src=text;*src;src++)
       {
          if(!isspace(*src))
             *(dst++) = *src;
       }
       *dst = 0;

	   DBGMessage(dup);
       ret = searchTextFile( dup, icq->badWords);

       free(dup);

    }
    else
    {
       ret = searchTextFile( text, icq->badWords);
    }
    return ret;
 }

