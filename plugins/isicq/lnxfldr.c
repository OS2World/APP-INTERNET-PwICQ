/*
 * flist.c - Manage shared files list
 */

 #include <stdio.h>
 #include <malloc.h>
 #include <string.h>
 #include <dirent.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
 #include <ctype.h>

 #include "ishared.h"

/*---[ Structures ]--------------------------------------------------------------------------------------*/

 #pragma pack(1)

 struct fldBase
 {
    ULONG		    	fCount;
	DIR					*dir;
	struct dirent   	*arq;
	struct stat			st;
    ISHARED_FILEDESCR	*entry;
	int					offset;
    char		    	buffer[0x0100];
 };

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/

 static void loadFolder(ISHARED_CONFIG *, const char *, int);


/*---[ Implementation ]----------------------------------------------------------------------------------*/

 void ICQAPI ishare_loadFolder(ISHARED_CONFIG *cfg, const char *folder)
 {
//#ifdef DEBUG
//    ISHARED_FILEDESCR	*entry;
//#endif

    loadFolder(cfg,folder,0);

//#ifdef DEBUG
//    walklist(entry,cfg->sharedFiles)
//    {
//       DBGTrace(entry->fileID);
//	   DBGMessage(entry->filePath);
//       DBGMessage(entry->filePath+entry->offset);
//    }
//#endif

 }

 static void loadFolder(ISHARED_CONFIG *cfg, const char *key, int level)
 {
    struct fldBase	*s = malloc(sizeof(struct fldBase));

    if(!s)
    {
       log(cfg,"Memory allocation error when loading file list");
       return;
    }

    DBGMessage(key);
	
    s->dir = opendir(key);
    
    if(!s->dir)
    {
       strcpy(s->buffer,"Error searching ");
       strncat(s->buffer,key,0xFF);
       log(cfg,s->buffer);
       return;
    }
    
    DBGTracex(s->dir);
    
    s->arq = readdir(s->dir);
	
    while(s->arq)
	{
	   if(*s->arq->d_name != '.')
       {
		  strncpy(s->buffer,key,0xFF);
		  strncat(s->buffer,"/",0xFF);
		  s->offset = strlen(s->buffer);
		  strncat(s->buffer,s->arq->d_name,0xFF);
		
		  if(!stat(s->buffer,&s->st))
		  {
		     if(S_ISDIR(s->st.st_mode))
			 {
                loadFolder(cfg, s->buffer, level+1);
			 }
			 else if(s->st.st_size)
			 {
                s->entry = malloc(sizeof(ISHARED_FILEDESCR)+strlen(s->buffer));
                if(!s->entry)
                {
                   log(cfg,"Memory allocation error when registering a file");
                }
                else
                {
                   memset(s->entry,0,sizeof(ISHARED_FILEDESCR));
                   s->entry->fileID   = cfg->fileID++;
                   s->entry->offset   = s->offset;
                   s->entry->fileSize = s->st.st_size;
                   strcpy(s->entry->filePath,s->buffer);
                   ishare_insertItem(&cfg->sharedFiles,&s->entry->l);
                }
			 }
		  }
		
	   }
	   s->arq = readdir(s->dir);
	}
	
	closedir(s->dir);

    free(s);
 }

 void ICQAPI ishare_strlwr(char *ptr)
 {
    while(*ptr)
    {
       *ptr = tolower(*ptr);
       ptr++;
    }
 }

 void ICQAPI ishare_loadMD5(ISHARED_CONFIG *cfg)
 {
    ISHARED_FILEDESCR   *entry;
    char                md5[50];
    int                 f;
	FILE			    *arq;
	char				*ptr;
	BOOL				save			= FALSE;
	int					wrk;
	char			    buffer[0x0200];

#ifdef EXTENDED_LOG
	log(cfg,"MD5 list check started");
#endif	
	
    queryPath(cfg,"ishare.md5",buffer,0xFF);
	arq = fopen(buffer,"r");

	DBGTracex(arq);
	
	if(arq)
	{
	   *buffer = 0;
       fgets(buffer,0x01FF,arq);
	   
       while(!feof(arq))
       {
		  for(f=0;buffer[f] && buffer[f] != '\n' && f < 0x01ff;f++);
		  *(buffer+f) = 0;
		  
		  if(*buffer && *(buffer+32) == ' ')
		  {
			 *(buffer+32) = 0;
			 ptr = buffer+33;
			 for(entry= (void *) cfg->sharedFiles.first;entry && strcmp(entry->filePath,ptr);entry = (void *) entry->l.down);
			
			 if(entry)
			 {
				md5[2] = 0;
	            for(f=0; f<16 && !entry->md5[f];f++)
				{
				   md5[0] = buffer[f*2];
				   md5[1] = buffer[(f*2)+1];
				   sscanf(md5,"%02x",&wrk);
				   entry->md5[f] = wrk;
 			    }
			 }
		  }
	      *buffer = 0;
          fgets(buffer,0x01FF,arq);
	   }
	   fclose(arq);   
	}


    /* Update if necessary */
	walklist(entry,cfg->sharedFiles)
    {
       sched_yield();
	   
       if(!cfg->active)
          return;

	   for(f=0; f<16 && !entry->md5[f];f++);
		  
	   if(f==16)
	   {
		  save = TRUE;
#ifdef EXTENDED_LOG
		  log(cfg,entry->filePath);
#endif		  
          ishare_MDFile(cfg, entry->filePath, entry->md5);
	   }
	   
    }

	DBGTracex(save);
	if(!save)
	   return;
	
	/* Save MD5 list */
	
    queryPath(cfg,"ishare.md5",buffer,0xFF);
	DBGMessage(buffer);
	
	arq = fopen(buffer,"w");
	
	if(!arq)
	{
	   log(cfg,"Error saving MD5 list");
	   return;
	}
	
    walklist(entry,cfg->sharedFiles)
    {
       for(f=0;f<16;f++)
          sprintf(md5+(f*2),"%02x",entry->md5[f]);
	   fprintf(arq,"%s %s\n",md5,entry->filePath);
	}
	
    fclose(arq);	
	
#ifdef EXTENDED_LOG
	log(cfg,"MD5 list check complete");
#endif	
	
 }

 
