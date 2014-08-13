/*
 * os2file.c - OS2 File management
 */

 #pragma strings(readonly)

 #define INCL_DOSPROCESS
 #define INCL_DOSFILEMGR

 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>

 #include "peer.h"

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/


/*---[ Implement ]--------------------------------------------------------------------------------------------*/

/*

 icqSetEA(fileName,".LONGNAME",strlen(buffer+4)+4,buffer);

 int ICQAPI icqSetEA(const char *pszPathName, const char *attr, int sz, const char *vlr)
 {
    APIRET       rc;
    PFEA2LIST    fpFEA2List;
    EAOP2        InfoBuf;
    PFEA2        at;
    int          f;
    char         *ptr;
    int          ns = strlen(attr);

    if(sz < 0)
       sz = strlen(vlr);

    rc = DosAllocMem((PPVOID) &fpFEA2List,sizeof(FEA2LIST)+sz+ns+5,PAG_READ|PAG_WRITE|PAG_COMMIT);
    if(rc)
       return rc;

    memset(&InfoBuf,0,sizeof(InfoBuf));
    InfoBuf.fpFEA2List = fpFEA2List;

    fpFEA2List->cbList = sizeof(FEA2LIST) + sz + ns;
    at                 = fpFEA2List->list;

    at->oNextEntryOffset = 0;            //  Offset to next entry.
    at->fEA              = 0;            //  Extended attributes flag.
    at->cbName           = ns;           //  Length of szName, not including NULL.
    at->cbValue          = sz;           //  Value length.

    strcpy(at->szName,attr);
    ptr      = (char *)(at->szName+ns+1);

    for(f=0;f<sz;f++)
       *(ptr+f) = *(vlr+f);

    rc = DosSetPathInfo( (PSZ) pszPathName,
                         FIL_QUERYEASIZE,
                         &InfoBuf,
                         sizeof(EAOP2),
                         0);

    DosFreeMem(fpFEA2List);
    return rc;

 }

Creates a new directory.

#define INCL_DOSFILEMGR
#include <os2.h>

PSZ       pszDirName;  //  Address of the ASCIIZ directory path name, which may contain a drive specification.
PEAOP2    peaop2;      //  Address of the extended attribute buffer, which contains an EAOP2 data structure.
APIRET    ulrc;        //  Return Code.

ulrc = DosCreateDir(pszDirName, peaop2);

*/

 char * icqv8_mountFilePath(HICQ icq, PEERSESSION *cfg, char *buffer)
 {
    HUSER        usr    = icqQueryUserHandle(icq,cfg->uin);
    char         *ptr;
    APIRET       rc;
    PFEA2LIST    fpFEA2List;
    EAOP2        InfoBuf;
    PFEA2        at;

    icqLoadString(icq, "recpath", "", buffer, 0xFF);

    if(!*buffer)
       icqQueryPath(icq,"received\\",buffer,0xFF);

    if(!usr)
    {
       sprintf(buffer,"Unexpected error: No valid user %lx when processing file in session %08lx",cfg->uin,(ULONG) cfg);
       icqWriteSysLog(icq,PROJECT,buffer);
       return 0;
    }

    if(*buffer)
    {
       ptr = buffer+strlen(buffer);

       *(--ptr) = 0;
       DBGMessage(buffer);

       rc  = DosCreateDir(buffer, NULL);
       DBGTrace(rc);

       if(rc && rc != 5)
          icqWriteSysRC(icq, PROJECT, rc, buffer);

       // 5 = Access denied

       *(ptr++) = '\\';
    }
    else
    {
       ptr = buffer;
    }

    if(usr->flags & USRF_FILEDIR)
    {
       /* Monta o nome do arquivo */
       DBGMessage("**** Criar diretorio com atributo estendido ****");

       sprintf(ptr,"%ld",usr->uin);

       /* Monta o atributo estendido */
       rc = DosAllocMem((PPVOID) &fpFEA2List,sizeof(FEA2LIST)+strlen(icqQueryUserNick(usr))+0x0100,PAG_READ|PAG_WRITE|PAG_COMMIT);

       if(rc)
       {
          icqWriteSysRC(icq, PROJECT, rc, "Error creating EA information");
          rc = DosCreateDir(buffer, NULL);
       }
       else
       {
          memset(&InfoBuf,0,sizeof(InfoBuf));
          InfoBuf.fpFEA2List = fpFEA2List;

          at                 = fpFEA2List->list;

          at->oNextEntryOffset = 0;      //  Offset to next entry.
          at->fEA              = 0;      //  Extended attributes flag.
          at->cbName           = 9;      //  Length of szName, not including NULL.

          strcpy(at->szName,".LONGNAME");

          sprintf(at->szName+10,"0000Files from %s\n(ICQ#%ld)",icqQueryUserNick(usr),cfg->uin);

          *( (USHORT *) (at->szName+10)) = 0xFFFD;
          *( (USHORT *) (at->szName+12)) = strlen(at->szName+14);

          at->cbValue = strlen(at->szName+14)+4;

          fpFEA2List->cbList = sizeof(FEA2LIST) + 10 + at->cbValue;

          rc = DosCreateDir(buffer, &InfoBuf);

          DosFreeMem(fpFEA2List);
       }

       /* Cria o diretorio */

       DBGTrace(rc);

       switch(rc)
       {
       case 0:
       case 5:
          strncat(buffer,"\\",0xFF);
          break;

       default:
          icqWriteSysRC(icq, PROJECT, rc, buffer);
          *ptr = 0;

        }
    }

    DBGMessage(buffer);
    return buffer+strlen(buffer);

 }


