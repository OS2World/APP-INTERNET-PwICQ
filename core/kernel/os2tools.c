/*
 * OS2Tools.c - OS/2 Help functions
 */

#ifndef __OS2__
 #error OS2 Only
#endif

 #pragma strings(readonly)

 #include <stdio.h>
 #include <malloc.h>
 #include <string.h>

 #include <icqtlkt.h>

/*---[ Implementation ]----------------------------------------------------------------------------------*/

 int ICQAPI icqSetEA(const char *pszPathName, const char *attr, int sz, const char *vlr)
 {
    APIRET       rc;
    PFEA2LIST    fpFEA2List;
    EAOP2        InfoBuf;
    PFEA2        at;
    int		 f;
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

    at->oNextEntryOffset = 0;            /*  Offset to next entry. */
    at->fEA              = 0;            /*  Extended attributes flag. */
    at->cbName           = ns;           /*  Length of szName, not including NULL. */
    at->cbValue          = sz;           /*  Value length. */

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

 int ICQAPI icqGetEA(const char *pszPathName, const char *attr, int sz, char *vlr)
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



