/*
 * cfile.c - Control-file management
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSFILEMGR
#endif

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>

#ifdef WIN32
 #include <windows.h>
#endif 

#ifdef __GNUC__
 #include <unistd.h>
#endif

#ifndef linux 
 #include <io.h>
#endif

#include <fcntl.h>
 #include <sys/stat.h>
 #include <stdlib.h>

 #include "ishared.h"

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/

 #ifndef O_BINARY
    #define O_BINARY 0
 #endif

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/


/*---[ Implementation ]----------------------------------------------------------------------------------*/

 char * ICQAPI ishare_makeControlFilePath(ISHARED_CONFIG *cfg, char *buffer)
 {
    char *ptr;
	
    loadString(cfg, "workfiles", "", buffer, 0x7F);

    if(!*buffer)
       queryPath(cfg,"ishare.tmp",buffer,0x7F);

    ptr = buffer+strlen(buffer);
#ifdef linux
    if(*(ptr-1) != '/')
       *(ptr++) = '/';
#else
    if(*(ptr-1) != '\\')
       *(ptr++) = '\\';
#endif

	return ptr;
 }

 int ICQAPI ishare_createControlFile(ISHARED_TRANSFER *trf)
 {
    char              buffer[0x0100];
    char              *ptr;
    ULONG             key;
    int               f;
    int               hFile;
#ifdef __OS2__
    HFILE             os2File;
#endif
    ISHARE_CTL_HEADER h;

	ptr = ishare_makeControlFilePath(trf->adm.cfg,buffer);

    sprintf(ptr,"%08lx.ctl",key = rand());
    hFile = open(buffer, O_CREAT|O_EXCL|O_WRONLY|O_BINARY,S_IREAD | S_IWRITE);

    for(f=0;f<100 && hFile == -1;f++)
    {
       key <<= 1;
       key ^=  rand();
       sprintf(ptr,"%08lx.ctl",key);
       hFile = open(buffer, O_CREAT|O_EXCL|O_WRONLY|O_BINARY,S_IREAD | S_IWRITE);
    }

    DBGTrace(hFile);
    if(hFile == -1)
    {
#ifdef DEBUG
	   perror(buffer);
#endif	
       return -1;
	}

    DBGMessage(buffer);
    trf->fileKey = key;

    DBGTracex(trf->fileKey);

    memset(&h,0,sizeof(h));

    h.sz         = sizeof(ISHARE_CTL_HEADER);
    h.fileSize   = trf->fileSize;
    h.fileKey    = trf->fileKey;
    h.szFilename = strlen(trf->filename)+1;

    for(f=0;f<16;f++)
       h.md5[f] = trf->md5[f];

    if(write(hFile,&h,sizeof(h)) == -1)
    {
       close(hFile);
       remove(buffer);
       return -1;
    }

    if(write(hFile,trf->filename,h.szFilename) == -1)
    {
       close(hFile);
       remove(buffer);
       return -1;
    }

    close(hFile);

    /* Create data file */
    sprintf(ptr,"%08lx.dat",trf->fileKey);

#ifdef __OS2__
   if(!DosOpen( buffer,&os2File,&key,trf->fileSize,FILE_ARCHIVED | FILE_NORMAL,
                OPEN_ACTION_CREATE_IF_NEW |OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_FLAGS_NOINHERIT|OPEN_SHARE_DENYNONE|OPEN_ACCESS_READWRITE,0L) )
    {
       DosClose(os2File);
    }
#ifdef DEBUG
    else
    {
       DBGMessage("********* FAILURE CREATING DATA FILE");
    }
#endif

#else
    hFile = open(buffer, O_CREAT|O_EXCL|O_WRONLY|O_BINARY,S_IREAD | S_IWRITE);
    close(hFile);
#endif

    return 0;
 }

 int ICQAPI ishare_updateControlFile(ISHARED_CONFIG *cfg, char *md5, ULONG fileKey, ULONG begin, ULONG offset)
 {
    static const char *env[] = { "ishare.incoming", "ishare", "incoming", "tmp", "TMP", NULL };

    char	          keyPath[0x0100];
    char	          *keyName;
    char              buffer[0x0200];
    char              *ptr      = NULL;
    FILE              *hFile;
    BOOL              complete	= FALSE;
    int               f;
    ISHARE_CTL_HEADER h;
    ISHARE_CTL_RECORD r;

    DBGMessage("Updating control-file");

    keyName = ishare_makeControlFilePath(cfg, keyPath);
    sprintf(keyName,"%08lx.ctl",fileKey);

    hFile = fopen(keyPath, "r+b");

    if(!hFile)
    {
       log(cfg,"Error opening control file");
       return 1;
    }

    fseek(hFile, 0L, SEEK_SET);

    if(fread(&h,sizeof(h),1,hFile) != 1)
    {
       log(cfg,"Error reading control file");
       perror(buffer);
       return 2;
    }

    if(h.sz != sizeof(ISHARE_CTL_HEADER))
    {
       DBGMessage("Invalid control file");
       perror(buffer);
       return 3;
    }

    loadString(cfg,"incoming","",buffer,0xFF);

    for(f=0;!*buffer && env[f];f++)
    {
       if(getenv(env[f]))
          strncpy(buffer,getenv(env[f]),0xFF);
    }

    DBGMessage(buffer);
    if(*buffer)
    {
       ptr = buffer+strlen(buffer);
#ifdef linux
       if(*(ptr-1) != '/')
          *(ptr++) = '/';
#else
       if(*(ptr-1) != '\\')
          *(ptr++) = '\\';
#endif

    }

    DBGMessage(buffer);

    DBGTrace(h.szFilename);

    if(h.szFilename > (0x01ff - strlen(buffer)))
    {
       log(cfg,"File name is too large");
       return 4;
    }

    if(fread(ptr,h.szFilename,1,hFile) != 1)
    {
       log(cfg,"Error reading control file");
       return 5;
    }

    DBGMessage(buffer);

    if(!begin && offset == h.fileSize)
    {
       log(cfg,"File was received in one transfer");

       for(f=0;f<16 && h.md5[f] == md5[f];f++);

       if(f != 16)
       {
          log(cfg,"MD5 mismatch");
          h.status = 1;
       }
       else
       {
          h.status = 2;
          complete = TRUE;
#ifdef __OS2__
          sprintf(keyName,"%08lx.dat",fileKey);
          ishare_SetEA(keyPath, "MD5", md5);
#endif
       }
    }
    else
    {
       memset(&r,0,sizeof(ISHARE_CTL_RECORD));
       r.sz = sizeof(r);

       for(f=0;f<16;f++)
          r.md5[f] = md5[f];

       r.time  = time(NULL);
       r.begin = begin;
       r.end   = offset;

       fseek(hFile, 0L, SEEK_END);

       if(fwrite(&r,sizeof(r),1,hFile) != 1)
       {
          log(cfg,"Error writing control-file record");
       }

    }

    fseek(hFile, 0L, SEEK_SET);

    if(fwrite(&h,sizeof(h),1,hFile) != 1)
    {
       log(cfg,"Error writing control-file header");
       fclose(hFile);
       return 7;
    }

    fclose(hFile);

    if(complete)
    {
       sprintf(keyName,"%08lx.dat",fileKey);

       DBGMessage(buffer);
       DBGMessage(keyPath);

#ifdef __OS2__
       if(DosMove(keyPath,buffer))
       {
          CHKPoint();
          if(DosCopy(keyPath,buffer,DCPY_EXISTING ))
          {
             if(DosDelete(keyPath))
                log(cfg,"Failure removing temporary file");
          }
          else
          {
             log(cfg,"Failure copying temporary file");
             return 6;
          }
       }

#else
       if(rename(keyPath,buffer))
       {
          log(cfg,"Failure renaming temporary file");
          return 6;
       }

#endif

       DBGMessage(keyPath);
       sprintf(keyName,"%08lx.ctl",fileKey);
       remove(keyPath);
    }


    return 0;
 }


