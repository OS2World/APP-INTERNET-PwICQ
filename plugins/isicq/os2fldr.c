/*
 * flist.c - Manage shared files list
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSFILEMGR   /* File Manager values */
 #define INCL_DOSERRORS    /* DOS error values */
#endif

 #include <stdio.h>
 #include <malloc.h>
 #include <string.h>

 #include "ishared.h"

/*---[ Structures ]--------------------------------------------------------------------------------------*/

 #pragma pack(1)

 struct fldBase
 {
    HDIR		hDir;
    FILEFINDBUF3	fl;
    ULONG		fCount;
    APIRET		rc;
    ISHARED_FILEDESCR	*entry;
    char		buffer[0x0100];
 };

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/

 static void loadFolder(ISHARED_CONFIG *, const char *, int);


/*---[ Implementation ]----------------------------------------------------------------------------------*/

 void ICQAPI ishare_loadFolder(ISHARED_CONFIG *cfg, const char *folder)
 {
    /* Load the selected folder in the shared list files */
#ifdef DEBUG
    ISHARED_FILEDESCR	*entry;
#endif

//    DBGMessage(folder);
    loadFolder(cfg,folder,0);

#ifdef DEBUG
//    walklist(entry,cfg->sharedFiles)
//    {
//       DBGTrace(entry->fileID);
//       DBGMessage(entry->filePath+entry->offset);
//    }
#endif

 }

 static void loadFolder(ISHARED_CONFIG *cfg, const char *key, int level)
 {
    struct fldBase	*s = malloc(sizeof(struct fldBase));

    if(!s)
    {
       log(cfg,"Memory allocation error when loading file list");
       return;
    }

    strncpy(s->buffer,key,0xFF);
    strncat(s->buffer,"\\*.*",0xFF);

//    DBGTrace(level);
//    DBGMessage(s->buffer);

    s->fCount = 1;
    s->hDir   = HDIR_CREATE;
    s->rc     = DosFindFirst(s->buffer,&s->hDir,FILE_DIRECTORY|FILE_NORMAL,&s->fl,sizeof(FILEFINDBUF3),&s->fCount,FIL_STANDARD);

//    DBGTrace(s->rc);

    while(!s->rc)
    {
       strncpy(s->buffer,key,0xFF);
       strncat(s->buffer,"\\",0xFF);
       s->fCount = strlen(s->buffer);
       strncat(s->buffer,s->fl.achName,0xFF);
       if(s->fl.attrFile & FILE_DIRECTORY)
       {
          if(*s->fl.achName != '.')
             loadFolder(cfg, s->buffer, level+1);
       }
       else if(s->fl.cbFile)
       {
          s->entry	= malloc(sizeof(ISHARED_FILEDESCR)+strlen(s->buffer));
          if(!s->entry)
          {
             log(cfg,"Memory allocation error when registering a file");
          }
          else
          {
             memset(s->entry,0,sizeof(ISHARED_FILEDESCR));
             s->entry->fileID   = cfg->fileID++;
             s->entry->offset   = s->fCount;
             s->entry->fileSize = s->fl.cbFile;
             strcpy(s->entry->filePath,s->buffer);
             ishare_insertItem(&cfg->sharedFiles,&s->entry->l);
          }
       }

       s->fCount = 1;
       s->rc     = DosFindNext(s->hDir,&s->fl,sizeof(FILEFINDBUF3),&s->fCount);
    }

#ifdef PWICQ
    if(s->rc != ERROR_NO_MORE_FILES)
       icqWriteSysRC(cfg->icq, PROJECT, s->rc, "Error loading shared files");
#endif

    DosFindClose(s->hDir);

    free(s);
 }

 int ICQAPI ishare_SetEA(const char *pszPathName, const char *attr, const char *name)
 {
    APIRET       rc;
    PFEA2LIST    fpFEA2List;
    EAOP2        InfoBuf;
    PFEA2        at;
    USHORT       *ptr;
    int          ns = strlen(attr);
    int          sz = strlen(name);

    rc = DosAllocMem((PPVOID) &fpFEA2List,sizeof(FEA2LIST)+sz+ns+5,PAG_READ|PAG_WRITE|PAG_COMMIT);
    if(rc)
       return rc;

    memset(&InfoBuf,0,sizeof(InfoBuf));
    InfoBuf.fpFEA2List = fpFEA2List;

    fpFEA2List->cbList = sizeof(FEA2LIST) + sz + ns;
    at                 = fpFEA2List->list;

    at->oNextEntryOffset = 0;            /*  Offset to next entry. */
    at->fEA              = 0;            /*  Extended attributes flag. */
    at->cbName           = ns;           /*  Length of szName, not including NULL. */
    at->cbValue          = sz;           /*  Value length. */

    strcpy(at->szName,attr);
    ptr      = (USHORT *)(at->szName+ns+1);
    strcpy((char *) ptr,name);

    rc = DosSetPathInfo( (PSZ) pszPathName,
                         FIL_QUERYEASIZE,
                         &InfoBuf,
                         sizeof(EAOP2),
                         0);

    DosFreeMem(fpFEA2List);
    return rc;

 }

 int ICQAPI ishare_GetEA(const char *pszPathName, const char *attr, int sz, char *vlr)
 {
    EAOP2        InfoBuf;
    int		 szAttr		= strlen(attr);
    PGEA2LIST	 gea		= malloc(sizeof(GEA2LIST)+szAttr+2);
    PFEA2LIST	 ea;
    int		 rc		= -1;
    int		 f;

    *vlr = 0;

    if(!gea)
       return rc;

    ea = malloc(sizeof(FEA2)+sizeof(FEA2)+sz+10);

    if(ea)
    {
       memset(&InfoBuf,0,sizeof(EAOP2));
       memset(gea,0,sizeof(GEA2LIST)+szAttr);
       memset(ea,0,sizeof(FEA2LIST)+sz+5);

       gea->cbList         = sizeof(GEA2LIST)+szAttr+1;
       gea->list[0].cbName = szAttr;
       strcpy(gea->list[0].szName,attr);

       ea->cbList  = sizeof(FEA2LIST)+sz+5;

       InfoBuf.fpGEA2List = gea;
       InfoBuf.fpFEA2List = ea;

       rc = DosQueryPathInfo((PSZ) pszPathName, FIL_QUERYEASFROMLIST, &InfoBuf, sizeof(InfoBuf));

       if(!rc)
       {
          if(ea->list[0].szName && ea->list[0].cbValue)
          {
             strncpy(  vlr,
             	      ea->list[0].szName+ea->list[0].cbName+1,
             	      sz <= ea->list[0].cbValue ? sz : (ea->list[0].cbValue+1)
             	   );
          }
          else
          {
             rc = -2;
          }
       }

       free(ea);
    }

    free(gea);
    return rc;
 }

 void ICQAPI ishare_loadMD5(ISHARED_CONFIG *cfg)
 {
    ISHARED_FILEDESCR	*entry;
    char		md5[50];
    char		wrk[3];
    int			f;
    int			tmp;

    log(cfg,"Loading MD5 information");

    DosSetPriority(PRTYS_THREAD, PRTYC_IDLETIME, PRTYD_MINIMUM, 0);

    walklist(entry,cfg->sharedFiles)
    {
       if(!cfg->active)
          return;

       DosSleep(5);
       ishare_GetEA(entry->filePath, "MD5", 49, md5);

       if(*md5)
       {
          wrk[2] = 0;
          for(f=0;f<16;f++)
          {
             wrk[0] = md5[f*2];
             wrk[1] = md5[(f*2)+1];
             sscanf(wrk,"%02x",&tmp);
             entry->md5[f] = tmp;
          }
       }
       else if(!ishare_MDFile(cfg, entry->filePath, entry->md5))
       {
          for(f=0;f<16;f++)
             sprintf(md5+(f*2),"%02x",entry->md5[f]);

          DBGMessage(entry->filePath);
          DBGMessage(md5);
          ishare_SetEA(entry->filePath, "MD5", md5);
       }
    }

    log(cfg,"MD5 information loaded");

 }



