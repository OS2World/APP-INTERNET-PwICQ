/*
 * config.c - Manage the configuration file
 */

  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <ctype.h>
  #include <errno.h>
#ifdef linux
  #include <unistd.h>
#else
  #include <io.h>
#endif

   #include "ishared.h"

/*---[ Secoes ]---------------------------------------------------------------------------------------*/

  #pragma pack(1)
  
  struct config_entry
  {
     ISHARED_LISTHEADER l;
     char		        str[1];
  };

/*---[ Static ]---------------------------------------------------------------------------------------*/

 static char * makeCfgFileName(ISHARED_CONFIG *, char *);


/*---[ Implementacao ]--------------------------------------------------------------------------------*/

 void ICQAPI ishare_loadConfig(ISHARED_CONFIG *cfg)
 {
    char                buffer[0x0100];
    struct config_entry *entry;
    FILE                *arq;
    char		*ptr;

    makeCfgFileName(cfg,buffer);
    DBGMessage(buffer);

    arq = fopen(buffer,"r");

    if(!arq)
       return;

    *buffer = 0;
    fgets(buffer,0xFF,arq);
    while(!feof(arq))
    {
       if(*buffer)
       {
		  for(ptr = buffer;*ptr && *ptr != '\n';ptr++);
		  *ptr = 0;
		  
          for(ptr = buffer;*ptr && *ptr != '=';ptr++);
          if(*ptr)
          {
             *(ptr++) = 0;
			 
			 if(*ptr == '$')
			 {
				ptr++;
			    if(getenv(ptr))
				   ptr = getenv(ptr);
			 }
			 
             entry = malloc(sizeof(struct config_entry)+strlen(buffer)+strlen(ptr)+3);
             if(entry)
             {
                memset(entry,0,sizeof(struct config_entry));
                strcpy(entry->str,buffer);
                strcpy(entry->str+strlen(entry->str)+1,ptr);
                ishare_insertItem(&cfg->config, (ISHARED_LISTHEADER *) entry);
				DBGMessage(entry->str);
             }
          }
       }
       *buffer = 0;
       fgets(buffer,0xFF,arq);
    }
    fclose(arq);

 }

 static char * makeCfgFileName(ISHARED_CONFIG *cfg, char *buffer)
 {
    static const char *confName = "ishare.conf";
    static const char *env[]    = { "ISHARE", "ETC", NULL };
    char              *ptr;
    int	              f;

    if(!access(confName, 4))
    {
       strncpy(buffer,confName,0xFF);
       return buffer;
    }

    *buffer = 0;
    for(f=0;env[f];f++)
    {
       if(getenv(env[f]))
       {
          strncpy(buffer,getenv(env[f]),0xFF);
          ptr = buffer+strlen(buffer);
#ifdef linux
          if(*(ptr-1) != '/')
             *(ptr++) = '/';
#else
          if(*(ptr-1) != '\\')
             *(ptr++) = '\\';
#endif
          strncat(buffer,confName,0xFF);
          if(!access(buffer,4))
             return buffer;
       }
    }

    strncpy(buffer,confName,0xFF);
    return buffer;

 }

 const char * ICQAPI ishare_loadString(ISHARED_CONFIG *cfg, const char *key, const char *def, char *buf, int sz)
 {
    struct config_entry *entry;
	   
	CHKPoint();
	
    walklist(entry,cfg->config)
	{
	   DBGTracex(entry);
	   DBGMessage(entry->str);
	   DBGMessage(key);
	   
       if(!strcmp(entry->str,key))
       {
          strncpy(buf,entry->str+strlen(entry->str)+1,sz);
          return buf;
       }
    }
	
	CHKPoint();
	
    strncpy(buf,def,sz);
	
	CHKPoint();
	
    return buf;
 }

 int ICQAPI ishare_loadValue(ISHARED_CONFIG *cfg, const char *key, int def)
 {
    char buffer[20];
	
    ishare_loadString(cfg,key,"",buffer,19);

	DBGMessage(buffer);
	
    if(*buffer)
       return(atoi(buffer));
	
	CHKPoint();
	
    return def;
 }


