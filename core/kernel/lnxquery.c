/*
 * lnxquery.c - Linux queries
 */

#ifndef linux
   #error Only for Linux version 
#endif 

 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>
 #include <stdlib.h>
 #include <unistd.h>

 #include <icqkernel.h>

/*---[ Prototipos ]---------------------------------------------------------------*/ 
 
 static int chkFile(HICQ, const char *, const char *, char *, int);
  
/*---[ Statics ]------------------------------------------------------------------*/ 
 
 
 

/*---[ Implementacao ]------------------------------------------------------------------------------------------*/

 const char * EXPENTRY icqQueryProgramPath(HICQ icq, const char *ext, char *buffer, int sz)
 {
	char *path;
	char *ptr;
	char *token;
	
    strncpy(buffer,"/usr/share/pwicq/",sz);
	
	if(!ext)
	   return buffer;

    strncat(buffer,ext,sz);
    if(!access(buffer,R_OK))
	{
	   strncat(buffer,"/",sz);
	   DBGMessage(buffer);
	   return buffer;
	}
	
	strncpy(buffer,ext,sz);
    if(!access(buffer,R_OK))
	{
	   strncat(buffer,"/",sz);
	   DBGMessage(buffer);
	   return buffer;
	}
	
    path = getenv("PATH");
	if(path)
	{
	   *buffer = 0;
	   path = strdup(path);
	   
	   for(ptr = strtok_r(path,":",&token);ptr && !*buffer;ptr = strtok_r(NULL,":",&token))
	   {
		  strncpy(buffer,ptr,sz);
		  strncat(buffer,"/pwicq/",sz);
		  strncat(buffer,ext,sz);
		  DBGMessage(buffer);
		  
		  if(access(buffer,R_OK))
			 *buffer = 0;
	   }
	   free(path);
	   
	   if(*buffer)
		  return buffer;
    }	   
	
    path = getenv("PATH");
	if(path)
	{
	   *buffer = 0;
	   path = strdup(path);
	   
	   for(ptr = strtok_r(path,":",&token);ptr && !*buffer;ptr = strtok_r(NULL,":",&token))
	   {
		  strncpy(buffer,ptr,sz);
		  strncat(buffer,"/",sz);
		  strncat(buffer,ext,sz);
		  DBGMessage(buffer);
		  
		  if(access(buffer,R_OK))
			 *buffer = 0;
	   }
	   free(path);
	   
	   if(*buffer)
		  return buffer;
    }	   
	
    strncpy(buffer,"Can't find ",sz);
	strncat(buffer,ext,sz);
	strncat(buffer,"/*",sz);
	icqWriteSysLog(icq,PROJECT,buffer);
    DBGMessage("Falhou ao tentar localizar diretorio");

	strncpy(buffer,ext,sz);
	strncat(buffer,"/",sz);
	DBGMessage(buffer);
	
    return buffer;
 }

 char * EXPENTRY icqQueryPath(HICQ icq, const char *ext, char *buffer, int size) 
 {
    char work[0x0100];
   
    if(!(ext && *ext))
    {
       strncpy(buffer,getenv("HOME"),size);
       strncat(buffer,"/.pwicq/",size);
 	  return buffer;
    }
   
    strcpy(work,"path.");
    strncat(work,ext,0xFF);
   
    if(chkFile(icq,work,ext,buffer,size))
       return buffer;
   
    if(chkFile(icq,"text-files",ext,buffer,size))
       return buffer;
   
    if(chkFile(icq,"config-files",ext,buffer,size))
       return buffer;
   
    strncpy(buffer,"/usr/share/pwicq/",size);
    strncat(buffer,ext,size);

    if(!access(buffer,R_OK))
       return buffer;
   
    icqQueryPath(icq,NULL,buffer,size); // Recursive call
    strncat(buffer,ext,size);
   
    DBGMessage(buffer);
    return buffer;
 }

 static int chkFile(HICQ icq, const char *key, const char *ext, char *buffer, int size)
 {
    if(!*key)
  	   return 0;
   
    icqLoadString(icq,key,"",buffer,size);
   
    if(!*buffer)
	   return 0;
   
    strncat(buffer,ext,size);
    if(!access(buffer,R_OK))
       return 1;
   
    return 0;
 }
 

