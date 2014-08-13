/*
 * TFILE.C - Carrega arquivos texto para a memoria
 */

#ifdef __IBMC__
 #pragma strings(readonly)
#endif

#ifdef linux
 #include <unistd.h>
 #include <sys/types.h>
 #define O_TEXT 0
#else
 #include <io.h>
#endif

 #include <fcntl.h>
 #include <sys/stat.h>

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <ctype.h>

 #include <icqkernel.h>

 #include "icqcore.h"

/*---[ Implementacao ]-----------------------------------------------------------------------------------*/

 char *loadTextFile(HICQ icq, const char *name, time_t *lastWrite)
 {
    char        buffer[0x0100];
    int         hdl;
    long        sz;

    char        *ret,
                *src,
                *dst;

    struct stat fileInfo;

    icqQueryPath(icq,name,buffer,0xFF);

    DBGMessage(buffer);

    hdl = open(buffer, O_RDONLY|O_TEXT, 0);

    if(hdl < 0)
       return NULL;

    if(lastWrite)
    {
       memset(&fileInfo,0,sizeof(fileInfo));
       fstat(hdl, &fileInfo);
       *lastWrite = fileInfo.st_mtime;
    }

    sz = lseek(hdl,0,SEEK_END);
    DBGTrace(sz);

    ret = malloc(sz+5);

    if(!ret)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when trying to load file");
       return NULL;
    }

    lseek(hdl,0,SEEK_SET);

    sz = read(hdl, ret, sz+4);
    close(hdl);

    DBGTrace(sz);
    if(sz < 0)
    {
       free(ret);
       strncat(buffer," -> CANT LOAD",0xFF);
       icqWriteSysLog(icq,PROJECT,buffer);
       return NULL;
    }

    if(sz == 0)
    {
       free(ret);
       return NULL;
    }
    *(ret+sz) = 0;


    strncat(buffer," loaded",0xFF);
    icqWriteSysLog(icq,PROJECT,buffer);

    src=dst=ret;

    sz = 0;
    while(*src)
    {
       if(*src >= ' ')
       {
          *(dst++) = tolower(*src);
		  src++;
          sz++;
       }
       else
       {
          while(*src && *src <= ' ')
             src++;
          *(dst++) = 0;
          sz++;
       }
    }

    *dst++ = 0;

    ret = realloc(ret,sz+3);

    return ret;
 }
 
 static const char *searchString(const char *msg, const char *list)
 {
	int f;
	
	while(*msg)
	{
	   if(tolower(*msg) == *list)
	   {
	      for(f=0;list[f] && tolower(msg[f]) == tolower(list[f]); f++);
		  if(!list[f])
			 return msg;
	   }
	   msg++;
	}
	return NULL;
 }

 BOOL searchTextFile(const char *msg, const char *list)
 {
    if(!list)
       return FALSE;

    while(*list)
    {
       if(searchString(msg,list))
          return TRUE;
       while(*list)
          list++;
       list++;
    }
    return FALSE;
 }



