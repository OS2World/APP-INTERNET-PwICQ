/*
 * scheme.c - Load/Save pwICQ's sound schemes
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>
 #include <stdio.h>

#ifdef __OS2__
 #include <direct.h>
#endif

 #include <icqtlkt.h>

#include "icqsound.h"

/*---[ Structures ]--------------------------------------------------------------------------*/

 #pragma pack(1)

 struct header
 {
    USHORT	sz;
    ULONG	id;
    ULONG	flag;
    ULONG	from;
    USHORT	nickSize;
 };

 struct record
 {
    USHORT	sz;
    ULONG	fileSize;
    ULONG	eventCode;
    USHORT	nameSize;
 };

/*---[ Defines ]-----------------------------------------------------------------------------*/

 #define FILE_WRITE	"wb"
 #define FILE_READ	"rb"

/*---[ Prototipes ]--------------------------------------------------------------------------*/

 static int copyFile(int, FILE *, FILE *, USHORT *);

/*---[ Public ]------------------------------------------------------------------------------*/

 static const char *fileID = "pWsN";

/*---[ Implement ]---------------------------------------------------------------------------*/

 int EXPENTRY icqsound_saveScheme(HICQ icq, const char *fileName)
 {
    /* Save the current configuration as scheme */

    struct header	h;
    struct record	r;


    char		buffer[0x0100];
    char		key[20];
    FILE		*outFile;
    FILE		*inFile;
    FILE		*events;
    char		*ptr;
    char		*base;
    int			rc 		= SCHEME_NOERROR;
    USHORT		crc;

    DBGMessage(fileName);

#ifdef linux
    icqQueryProgramPath(icq,NULL,buffer,0xFF);
	strncat(buffer,"events.dat",0xFF);
#else	
    icqQueryProgramPath(icq,"events.dat",buffer,0xFF);
#endif	
    DBGMessage(buffer);

    events = fopen(buffer,"r");
    DBGTracex(events);

    if(!events)
       return SCHEME_NOEVENTS;

    DBGMessage(fileName);
    outFile = fopen(fileName,FILE_WRITE);

    DBGTracex(outFile);

    if(!outFile)
    {
       fclose(events);
       return SCHEME_NOOUTFILE;
    }

    memset(&h,0,sizeof(h));
    h.sz   = sizeof(h);
    h.id   = * ( (ULONG *) fileID );
    h.from = icqQueryUIN(icq);

    icqLoadString(icq, "NickName", "", buffer, 0xFF);
    DBGMessage(buffer);

    h.nickSize = strlen(buffer);
    h.nickSize = h.nickSize < 0x80 ? h.nickSize : 0x80;

    fwrite(&h,sizeof(h),1,outFile);
    fwrite(buffer,h.nickSize,1,outFile);

    while(!feof(events) && !rc)
    {
       *buffer = 0;
       fgets(buffer,0xFF,events);

       if(strlen(buffer) > 4 && *(buffer+4) == ' ')
       {
          *(buffer+4) = 0;
          memset(&r,0,sizeof(r));
          r.sz        = sizeof(struct record);
          r.eventCode = * ( (ULONG *) buffer );

          strcpy(key,"sound:");
          strncat(key,buffer,19);

          icqLoadString(icq, key, "", buffer, 0xFF);

          for(ptr=base=buffer;*ptr;ptr++)
          {
             if(*ptr == ':' || *ptr == '\\' || *ptr == '/')
                base = ptr+1;
          }

          r.nameSize = strlen(base);

          if(!r.nameSize)
          {
             fwrite(&r,sizeof(r),1,outFile);
          }
          else if(*buffer == '%')
          {
             r.fileSize = 0;
             fwrite(&r,sizeof(r),1,outFile);
             fwrite(base,r.nameSize,1,outFile);
          }
          else
          {
             DBGMessage(buffer);
             inFile = fopen(buffer,"r");

             if(inFile)
             {
	        fseek(inFile,0L,SEEK_END);
                r.fileSize = ftell(inFile);
			
		DBGTrace(r.fileSize);
				
		if(r.fileSize)
		   rc = SCHEME_CANTSEEK;

                fwrite(&r,sizeof(r),1,outFile);
                fwrite(base,r.nameSize,1,outFile);

                fseek(inFile,0,SEEK_SET);

                rc = copyFile(r.fileSize, inFile, outFile, &crc);

                fclose(inFile);

                fwrite(&crc,sizeof(crc),1,outFile);

             }
             else
             {
		r.fileSize = 0;
                r.nameSize = 0;
                fwrite(&r,sizeof(r),1,outFile);
             }

          }

       }
    }
    fclose(events);
    fclose(outFile);

    CHKPoint();

    return rc;
 }


 int EXPENTRY icqsound_loadScheme(HICQ icq, const char *path)
 {
    struct header	h;
    struct record	r;
    int			rc	= 0;
    char   		buffer[0x0100];
    char		basePath[0x0100];
    char		key[20];
    FILE		*in	= fopen(path,FILE_READ);
    FILE		*out;
    char		*ptr;
    char		*file;
    USHORT		crc;
    USHORT		crcFile;

    strcpy(buffer,"Reading scheme ");
    strncat(buffer,path,0xFF);
    icqWriteSysLog(icq,PROJECT,buffer);

    if(!in)
    {
       icqWriteSysLog(icq,PROJECT,"Can't open scheme file");
       return SCHEME_CANTOPEN;
    }

    if(fread( &h, sizeof(h), 1, in) != 1)
    {
       icqWriteSysLog(icq,PROJECT,"Can't read scheme file header");
       return SCHEME_READERROR;
    }

    if(h.sz != sizeof(h) || h.id != * ( (ULONG *) fileID ))
    {
       icqWriteSysLog(icq,PROJECT,"Invalid scheme file");
       return SCHEME_INVALIDFILE;
    }

    /* Split base Path */
    strncpy(buffer,path,0xFF);
    for(file=ptr=buffer;*ptr;ptr++)
    {
       if(*ptr == ':' || *ptr == '/' || *ptr == '\\')
          file = (ptr+1);
    }
    for(ptr = file;*ptr && *ptr != '.';ptr++);
    *ptr = 0;

    DBGMessage(file);

#ifdef __OS2__
    icqQueryProgramPath(icq,NULL,basePath,0xFF);
    strncat(basePath,"sounds\\",0xFF);
    strncat(basePath,file,0xFF);
    mkdir(basePath);
    strncat(basePath,"\\",0xFF);
#else	
    icqQueryProgramPath(icq,NULL,basePath,0xFF);
    strncat(basePath,"sounds/",0xFF);
    strncat(basePath,file,0xFF);
    strcpy(buffer,"mkdir ");
    strncat(buffer,basePath,0xFF);
    DBGMessage(buffer);
    system(buffer);
    strncat(basePath,"/",0xFF);
#endif	

    file = basePath + strlen(basePath);

    DBGMessage(basePath);

    memset(buffer,0,0xFF);
    sprintf(buffer,"Scheme created by ICQ#%ld: ",h.from);
    fread(buffer+strlen(buffer),h.nickSize,1,in);
    icqWriteSysLog(icq,PROJECT,buffer);

    while(!rc && fread(&r,sizeof(r),1,in) == 1)
    {
       if(r.sz != sizeof(r))
       {
          DBGMessage("Invalid record");
          icqWriteSysLog(icq,PROJECT,"Invalid record in scheme file");
          rc = SCHEME_INVALIDRECORD;
       }
       else
       {
          strcpy(key,"sound:");
          *(key+10) = 0;
          *( (ULONG *) (key+6) ) = r.eventCode;

          if(!r.nameSize)
          {
             icqSaveString(icq, key, NULL);
          }
          else if(fread(buffer,r.nameSize,1,in) != 1)
          {
             rc = SCHEME_READERROR;
          }
          else
          {
             *(buffer+r.nameSize) = 0;

             if(!r.fileSize)
             {
                DBGMessage(buffer);
                icqSaveString(icq, key, buffer);
             }
             else
             {
                *file = 0;
                strncat(basePath,buffer,0xFF);

                DBGTrace(r.fileSize);
                DBGMessage(basePath);
                out = fopen(basePath,FILE_WRITE);

                DBGTracex(out);

                if(!out)
                {
#ifdef DEBUG
                   perror("Failure opening output file");
#endif
                   rc = -7;
                }
                else
                {

                   if(!copyFile(r.fileSize, in, out, &crcFile))
                   {
                      DBGMessage("File copied");
                      icqSaveString(icq, key, basePath);
                   }
                   fclose(out);

                   if(fread(&crc,sizeof(crc),1,in) != 1)
                   {
                      rc = SCHEME_READERROR;
                   }
                   else if(crc != crcFile)
                   {
                      rc = SCHEME_CRCERROR;
                   }
                }
             }
          }
       }
    }

    fclose(in);

    return rc;
 }

 static int copyFile(int sz, FILE *from, FILE *to, USHORT *crcFile)
 {
    /* Copy file, make a CRC16 */

    char   buffer[0x0100];
    int    bytes;
    short  count;
    short  c;
    short  crc		= 0;
    int    f;

    DBGTrace(sz);

    while(sz > 0)
    {
       bytes = fread(buffer,1,sz > 0xFF ? 0xFF : sz,from);

       if(!bytes)
          return SCHEME_CANTCOPY;

       for(f=0;f<bytes;f++)
       {
          c = *(buffer+f);
          for (count = 8; --count >= 0; )
          {
             if (crc & 0x8000)
             {
                crc <<= 1;
                crc += (short) (((c <<= 1) & 0400) != 0);
                crc ^= 0x1021;
             }
             else
             {
                crc <<= 1;
                crc += (short) (((c <<= 1) & 0400) != 0);
             }
          }
       }

       fwrite(buffer,1,bytes,to);
       sz -= bytes;

    }

    *crcFile = crc;

    DBGTrace(sz);

    return SCHEME_NOERROR;
 }


