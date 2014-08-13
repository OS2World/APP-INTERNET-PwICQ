/*
 * BLACKLIST.C - Carrega a blacklist
 */

#ifdef __IBMC__
 #pragma strings(readonly)
#endif

 #include <stdio.h>
 #include <stdlib.h>
 #include <ctype.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <string.h>

 #include <pwMacros.h>
 #include <malloc.h>

 #include <icqkernel.h>
 #include "icqcore.h"

/*---[ Prototipos ]-----------------------------------------------------------------------------*/

 static BOOL ajust(char *,char *);

/*---[ Implementacao ]--------------------------------------------------------------------------*/

 void loadBlackList(HICQ icq)
 {
    char         buffer[0x0100];
    FILE         *arq;
    ULONG        qtd          = 0;
    ULONG        *ptr;
    char         *f;
    struct stat fileInfo;

    icqQueryPath(icq,"blacklist.dat",buffer,0xFF);
	DBGMessage(buffer);
	
	memset(&fileInfo,0,sizeof(fileInfo));
	stat(buffer, &fileInfo);

    arq = fopen(buffer,"r");

    if(arq == NULL)
       return;

    while(!feof(arq))
    {
       *buffer = 0;
       fgets(buffer,0xFF,arq);
       if(*buffer && *buffer != ';')
          qtd++;
    }

    DBGTrace(qtd);

    if(icq->blacklist)
    {
       free(icq->blacklist);
       icqWriteSysLog(icq,PROJECT,"Reloading black list file");
    }

    icq->blacklist = malloc((qtd+1) * sizeof(ULONG));

    if(!icq->blacklist)
    {
       icqWriteSysLog(icq,PROJECT,"Unable to allocate memory for the blacklist information");
       return;
    }

    rewind(arq);

    *icq->blacklist = qtd;
    ptr             = icq->blacklist+1;

    qtd = 0;
    while(!feof(arq) && qtd < *icq->blacklist)
    {
       *buffer = 0;
       fgets(buffer,0xFF,arq);

       for(f=buffer;*f && *f != ';' && *f >= ' ';f++);
       *f   = 0;

       if(ajust(buffer,(char *) ptr))
       {
          qtd++;
          ptr++;
       }
    }

    DBGTrace(qtd);

    if(qtd != *icq->blacklist)
    {
       ptr = realloc(icq->blacklist, (qtd+1) * sizeof(ULONG));
       if(ptr)
          icq->blacklist = ptr;
       *icq->blacklist = qtd;
    }

    DBGTrace(*icq->blacklist);

    fclose(arq);

//    DBGTrace(icqCheckBlacklist(icq, 0x0100007f));
//    DBGTrace(icqCheckBlacklist(icq, 0x0200007f));
//    DBGTrace(icqCheckBlacklist(icq, 0x1a8ba1c8));

    sprintf(buffer,"%d IPs/Networks in the blacklist file",(int) *icq->blacklist);
    icqWriteSysLog(icq,PROJECT,buffer);
    icq->blkListTime = fileInfo.st_mtime;

 }

 void loadIgnoreList(HICQ icq)
 {
    char   buffer[0x0100];
    FILE   *arq;
    ULONG  qtd          = 0;
    ULONG  *ptr;
    char   *f;

    icqQueryPath(icq,"ignore.dat",buffer,0xFF);
	DBGMessage(buffer);
	
    arq = fopen(buffer,"r");

    if(arq == NULL)
       return;

    icqWriteSysLog(icq,PROJECT,"Reading ignore list");

    while(!feof(arq))
    {
       *buffer = 0;
       fgets(buffer,0xFF,arq);
       if(*buffer && *buffer != ';')
          qtd++;
    }

    DBGTrace(qtd);

    icq->ignore = malloc((qtd+1) * sizeof(ULONG));

    if(!icq->ignore)
    {
       icqWriteSysLog(icq,PROJECT,"Unable to allocate memory for the ignore list information");
       return;
    }

    rewind(arq);

    *icq->ignore = qtd;
    ptr          = icq->ignore+1;

    qtd = 0;
    while(!feof(arq) && qtd < *icq->ignore)
    {
       *buffer = 0;
       fgets(buffer,0xFF,arq);

       for(f=buffer;*f && *f != ';' && *f >= ' ';f++);
       *f   = 0;
       *ptr = atol(buffer);
       if(*ptr)
       {
          qtd++;
          ptr++;
       }
    }

    DBGTrace(qtd);

    if(qtd != *icq->ignore)
    {
       ptr = realloc(icq->ignore, (qtd+1) * sizeof(ULONG));
       if(ptr)
          icq->ignore = ptr;
       *icq->ignore = qtd;
    }

    DBGTrace(*icq->ignore);

    fclose(arq);

    sprintf(buffer,"%d UINs in the ignore list file",(int) *icq->ignore);
    icqWriteSysLog(icq,PROJECT,buffer);

 }


 ULONG EXPENTRY icqString2IP(char *buffer)
 {
    ULONG ret = 0;
    ajust(buffer,(char *) &ret);
    return ret;
 }

 static BOOL ajust(char *buffer, char *vlr)
 {
    int f=0;

    *vlr = 0;

    while(*buffer && f < 4 && (isdigit(*buffer) || *buffer == '.' || *buffer == '*') )
    {
       if(*buffer == '.')
       {
          f++;
          vlr++;
          *vlr = 0;
       }
       else if(*buffer == '*')
       {
          *vlr = 0xFF;
       }
       else
       {
          *vlr *= 10;
          *vlr += *buffer - '0';
       }

       buffer++;
    }

    return f==3;
 }

 BOOL EXPENTRY icqCheckBlacklist(HICQ icq, ULONG ip)
 {
    ULONG qtd;
    ULONG *ptr  = icq->blacklist;

    if(!icq->blacklist)
       return FALSE;

    qtd = *(ptr++);
    while(qtd--)
    {
//       DBGMessage(icqFormatIP(*ptr));
       if( (ip & *ptr) == ip)
          return TRUE;
       ptr++;
    }
    return FALSE;

 }

 BOOL EXPENTRY icqCheckIgnoreList(HICQ icq, ULONG uin)
 {
    ULONG qtd;
    ULONG *ptr  = icq->ignore;

    if(!icq->ignore)
       return FALSE;

    qtd = *(ptr++);
    while(qtd--)
    {
//       DBGTrace(*ptr);
       if(*ptr == uin)
          return TRUE;
       ptr++;
    }
    return FALSE;

 }

 int EXPENTRY icqBlacklistUser(HICQ icq, ULONG uin)
 {
    /* Anexa um usuario  na blacklist */
    ULONG  qtd;
    ULONG  *ptr;
    char   buffer[0x0100];

    CHKPoint();

    if(icq->ignore)
    {
       DBGTrace(*(icq->ignore));

       qtd = *(icq->ignore)+1;
       ptr = realloc(icq->ignore, (qtd+1) * sizeof(ULONG));
       if(!ptr)
       {
          icqWriteSysLog(icq,PROJECT,"Memory allocation error expanding ignore list");
          icqAbend(icq,0);
          return -1.;
       }
    }
    else
    {
       qtd = 1;
       ptr = malloc( (qtd+1) * sizeof(ULONG) );

       if(!ptr)
       {
          icqWriteSysLog(icq,PROJECT,"Memory allocation error creating ignore list");
          icqAbend(icq,0);
          return -1.;
       }
    }

    DBGTrace(qtd);

    icq->ignore  = ptr;
    *icq->ignore = qtd;

    *(ptr+qtd) = uin;

    sprintf(buffer,"ICQ#%ld ignored",uin);
    icqWriteSysLog(icq,PROJECT,buffer);

    return 0;

 }



