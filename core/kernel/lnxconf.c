/*
 * LNXCONF.C - Acesso ao arquivo de configuracao, versao Linux
 */

#ifndef linux
   #error Linux only
#endif

  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <ctype.h>
  #include <errno.h>
  #include <unistd.h>

  #include <icqkernel.h>

  #include "icqcore.h"

/*---[ Secoes ]---------------------------------------------------------------------------------------*/

 typedef struct _section
 {
    HLIST keys;
 } SECTION;

/*---[ Static ]---------------------------------------------------------------------------------------*/

 static char config_fileName[0x0100] = "";
 
/*---[ Implementacao ]--------------------------------------------------------------------------------*/

 void icqChkConfigFile(HICQ icq, char *buffer)
 {
    char temp[0x0100];
    
    if(icq->config)
       return;
/*
    strncpy(buffer,icq->programPath,0xFF);
    strncat(buffer,"pwicq.conf",0xFF);
    DBGMessage(buffer);

*/

    strncpy(buffer,getenv("HOME"),0xFF);
    strncat(buffer,"/.pwicq",0xFF);
    
    if(access(buffer,0))
    {
       icqWriteSysLog(icq,PROJECT,"Creating ~/.pwicq folder");
       strcpy(temp,"mkdir ");
       strncat(temp,buffer,0xFF);
       system(temp);
    }
    
    DBGTrace(chdir(buffer));
    
    strncat(buffer,"/pwicq.conf",0xFF);
        
    icqLoadConfigFile(icq,buffer);
 }

 int EXPENTRY icqLoadConfigFile(HICQ icq, char *name)
 {
    time_t      ltime;
    SECTION     *s		= NULL;
    char        buffer[0x0100];
    char        *id;
    char        *ptr;
    char	    *vlr;
    FILE        *arq;

    if(icq->config)
    {
       icqWriteSysLog(icq,PROJECT,"Configuration file already loaded");
       return -1;
    }
    
    icq->config = icqCreateList();

    if(name)
       strncpy(buffer,name,0xFF);
    else
       icqQueryPath(icq, "pwicq.conf", buffer, 0xFF);

    setenv("pwicq-inifile",buffer,TRUE);
    strcpy(config_fileName,buffer);
	
    arq = fopen(buffer,"r");

    DBGMessage(buffer);
    DBGTracex(arq);

    if(arq)
    {
       while(!feof(arq))
       {
          *buffer = 0;
          fgets(buffer,0xFF,arq);
          *(buffer+0xFF) = 0;

          for(id = buffer;*id && isspace(*id);id++);

          for(ptr=buffer;*ptr;ptr++)
          {
             if(*ptr < ' ')
                *ptr = 0;
          }

          if(*id == '[')
          {
             id++;
             for(ptr = id;*ptr && *ptr != ']';ptr++);
             if(*ptr == ']')
             {
                *ptr = 0;
                s =  icqAddElement(icq->config, sizeof(SECTION)+strlen(id)+1);
                if(s)
                {
                   s->keys = icqCreateList();
                   strcpy((char *) (s+1),id);
                }
             }
          }
          else if(!s && *id)
          {
             sysLog(icq,"Warning: The configuration file appears to be damaged");
          }
          else if(*id)
          {
             for(vlr = id;*vlr && *vlr != '=';vlr++);
             if(*vlr == '=')
             {
                for(ptr=(vlr-1);isspace(*ptr);ptr--);
                *(ptr+1) = 0;

                vlr++;
                while(*vlr && isspace(*vlr))
                   vlr++;
                if(*vlr)
                {
                   ptr =  icqAddElement(s->keys,strlen(id)+strlen(vlr)+3);
                   if(ptr)
                   {
                      strcpy(ptr,id);
                      strcpy((ptr+strlen(ptr))+1,vlr);
                   }
                }
             }
          }
       }
       fclose(arq);
    }
    
    if(!icq->uin)
       icq->uin = icqLoadProfileValue(icq,"MAIN","uin",0);

    DBGTrace(icq->uin);
    
	if(icq->uin)
	{
	   icqLoadString(icq,"log","",buffer,0xFF);    
	   if(*buffer)
	      freopen(buffer,"a",stdout);

	   icqLoadString(icq,"debuglog","",buffer,0xFF);    
	   if(*buffer)
	   {
	      freopen(buffer,"a",stderr);

          time(&ltime);
          strftime(buffer, 69, "%m/%d/%Y %H:%M:%S", localtime(&ltime));

          fprintf(stderr,"\n-----[ %s ]---------------------------------------------\n\n",buffer);
		  fflush(stderr);
		  
	   }
    }

    return 0;
 }

 char * EXPENTRY icqLoadProfileString(HICQ icq, const char *tag, const char *key, const char *def, char *buf, int sz)
 {
    SECTION     *s;
    char        *ptr;
    char		*dst;

    for(s = icqQueryFirstElement(icq->config); s; s = icqQueryNextElement(icq->config,s) )
    {
       if(!strcmp((char *) (s+1),tag))
       {
          for(ptr = icqQueryFirstElement(s->keys);ptr;ptr = icqQueryNextElement(s->keys,ptr))
          {
             if(!strcmp(ptr,key))
             {
                /* Achei a entrada, copio removendo os caracteres de controle */
                dst =  buf;
                ptr += strlen(ptr)+1;

                while(sz-- > 0 && *ptr)
                {
                   if(*ptr == '\\')
                   {
                      ptr++;

                      if(*ptr == '\\')
                         *(dst++) = '\\';
                      else
                         *(dst++) = (toupper(*ptr)) - 68;
                   }
                   else
                   {
                      *(dst++) = *ptr;
                   }
                   ptr++;
                }
                *dst = 0;
                return buf;
             }
          }
       }
    }
    strncpy(buf,def,sz);
    return buf;
 }

 static void updateKey(HICQ icq, SECTION *s, const char *id, const char *vlr)
 {
    char *ptr;

    for(ptr = icqQueryFirstElement(s->keys);ptr;ptr = icqQueryNextElement(s->keys,ptr))
    {
       if(!strcmp(ptr,id))
       {
          /* Encontrei a chave */
          
          if(!vlr)
          {
             DBGMessage("Remover chave");
             icqRemoveElement(s->keys,ptr);
			 icq->cntlFlags |= ICQFC_SAVECONFIG;
             return;
          }
          
	      if(strlen(vlr) != strlen(ptr+(strlen(ptr)+1)))
	      {
	         DBGMessage("Atualizar chave, redefinindo o tamanho");
	         DBGTrace(strlen(id));
	         DBGTrace(strlen(vlr));
             ptr = icqResizeElement(s->keys, strlen(id)+strlen(vlr)+3, ptr);
	         DBGTracex(ptr);
             strcpy(ptr,id);
          }
          strcpy((ptr+strlen(ptr))+1,vlr);
 		  icq->cntlFlags |= ICQFC_SAVECONFIG;
          return;
       }
    }
    
    if(!vlr)
       return;

    DBGMessage("Anexar uma nova chave");
    DBGTracex(s->keys);
    DBGTrace(strlen(id));
    DBGTrace(strlen(vlr));
    DBGTracex(s->keys);

    ptr =  icqAddElement(s->keys,strlen(id)+strlen(vlr)+3);

    DBGTracex(ptr);

    if(ptr)
    {
       strcpy(ptr,id);
       
       CHKPoint();
       
       strcpy((ptr+strlen(id))+1,vlr);
    }

    CHKPoint();
    
    icq->cntlFlags |= ICQFC_SAVECONFIG;
 }

 static int saveProfileString(HICQ icq, const char *tag, const char *key, const char *vlr)
 {
    SECTION     *s;

	icq->cntlFlags |= ICQFC_SAVECONFIG;
	
    for(s = icqQueryFirstElement(icq->config); s; s = icqQueryNextElement(icq->config,s) )
    {
       if(!strcmp((char *) (s+1),tag))
       {
          updateKey(icq,s,key,vlr);
	      return 1;
       }
    }

    DBGMessage("Anexar uma nova raiz");

    s =  icqAddElement(icq->config, sizeof(SECTION)+strlen(tag));

    if(s)
    {
       s->keys = icqCreateList();
       strcpy((char *) (s+1),tag);
       updateKey(icq,s,key,vlr);
    }
    else
    {
       icqWriteSysLog(icq,PROJECT,"Unable to add new elemente in the configuration file");
       return -1;
    }

    return 0;
 }

 void EXPENTRY icqSaveProfileString(HICQ icq, const char *tag, const char *key, const char *vlr)
 {
     const unsigned char *ptr = (unsigned char *) vlr;
     int		         sz;
     char	            *buffer;
     char	            *dst;

     if(!vlr)
     {
        saveProfileString(icq, tag, key, vlr);
        return;
     }

     while(*ptr && *ptr >= ' ' && *ptr != '\\')
        ptr++;
        
     if(!*ptr)
     {
        saveProfileString(icq, tag, key, vlr);
        return;
     }

     DBGMessage("Precisa converter a string");
     
     sz = strlen(vlr)+5;
     
     for(ptr=vlr;*ptr;ptr++)
     {
        if(*ptr < ' ' || *ptr == '\\')
           sz++;
     }
     
     DBGTrace(sz);
     
     buffer = malloc(sz+10);
     if(!buffer)
     {
        icqWriteSysLog(icq,PROJECT,"Memory allocation error when updating configuration file");
        return;
     }

     dst = buffer;
     ptr = vlr;
     
     while(*ptr)
     {
        if(*ptr == '\\')
        {
           *(dst++) = '\\';
           *(dst++) = '\\';
        }
        else if(*ptr < ' ')
        {
           *(dst++) = '\\';
           *(dst++) = *ptr + 100;
        }
        else
        {
           *(dst++) = *ptr;
        }
        ptr++;
     }

     *(dst) = 0;

     DBGTrace(strlen(buffer));
     DBGTrace(sz);
     
#ifdef DEBUG
     if(strlen(buffer) > sz)
        DBGMessage("************ TAMANHO INESPERADO");
#endif
          
     DBGMessage(buffer);
     saveProfileString(icq, tag, key, buffer);

     free(buffer);          
 }
 
 int EXPENTRY icqSaveConfigFile(HICQ icq)
 {
    FILE 	   *arq;
    SECTION    *s		= NULL;
    char	   *key;
	const char *fileName = config_fileName;
	
//    const char  *fileName	= getenv("pwicq-inifile");

#ifdef EXTENDED_LOG
	icqWriteSysLog(icq,PROJECT,"Saving config file");
#endif	
	
    if(!fileName)
    {
       sysLog(icq,"Failure getting the name of the configuration file");
       return -1;
    }

    DBGMessage(fileName);

    arq = fopen(fileName,"w");

    if(!arq)
    {
	   icqWriteSysRC(icq,PROJECT,errno,"Can't write config file");
       return errno;
    }

    sysLog(icq,"Saving configuration");

    icq->cntlFlags &= ~ICQFC_SAVECONFIG;

    for(s = icqQueryFirstElement(icq->config); s; s = icqQueryNextElement(icq->config,s) )
    {
       fprintf(arq,"\n[%s]\n",(char *) (s+1));
       for(key = icqQueryFirstElement(s->keys);key;key = icqQueryNextElement(s->keys,key))
          fprintf(arq,"   %s=%s\n",key,key+(strlen(key)+1));
    }

    fclose(arq);

    return 0;
 }

