/*
 * OS2QUERY.C - Query core information (OS2 info)
 */


#ifndef __OS2__
   #error Only for OS2 version
#endif

 #include <string.h>
 #include <icqkernel.h>

/*---[ Implementacao ]------------------------------------------------------------*/

 HAB EXPENTRY icqQueryAnchorBlock(HICQ icq)
 {
    return icq->hab;
 }

 HINI EXPENTRY icqQueryINIFile(HICQ icq)
 {
    return icq->ini;
 }

 USHORT EXPENTRY icqQueryRemoteCodePage(HICQ icq)
 {
    return icq->cpgOut;
 }

 USHORT EXPENTRY icqSetRemoteCodePage(HICQ icq, USHORT cpg)
 {
    USHORT ret = icq->cpgOut;

    icq->cpgOut = cpg;

    icqSaveValue(icq, "cpgRemote", cpg);

    return ret;
 }

 const char * EXPENTRY icqQueryProgramPath(HICQ icq, const char *ext, char *buffer, int sz)
 {
    CHKPoint();

    strncpy(buffer,icq->programPath,sz);

    if(ext)
    {
       strncat(buffer,ext,sz);
    }

    return buffer;
 }

 int EXPENTRY icqQueryCountryCode(HICQ icq)
 {
    return icq->countryCode;
 }

 void EXPENTRY icqTranslateCodePage(HICQ icq, ULONG cpIn, ULONG cpOut, UCHAR *ptr, int sz)
 {
    if(cpIn != cpOut && icq->convertCpg)
       icq->convertCpg(icq,cpIn,cpOut,ptr,sz);
 }

 USHORT EXPENTRY icqQueryLocalCodePage(HICQ icq)
 {
    return icq->cpgIn;
 }


