/*
 * Config.c - Módulo comum para acesso ao arquivo de configuracao
 */

 #include <string.h>
 #include <stdlib.h>
 #include <stdio.h>

 #include <icqkernel.h>

/*---[ Implementacao ]------------------------------------------------------------------------*/

 ULONG EXPENTRY icqLoadProfileValue(HICQ icq, const char *app, const char *key, ULONG def)
 {
    char buffer[20];

    icqLoadProfileString(icq, app, key, "", buffer, 19);

    if(*buffer)
       return atol(buffer);

    return def;
 }

 ULONG EXPENTRY icqSaveProfileValue(HICQ icq, const char *app, const char *key, ULONG vlr)
 {
    char str[40];
    sprintf(str,"%lu",vlr);
    icqSaveProfileString(icq, app, key, str);
    return vlr;
 }

 ULONG EXPENTRY icqLoadValue(HICQ icq, const char *key, ULONG def)
 {
    char app[20];

    if(icq->uin)
    {
       sprintf(app,"%lu",icq->uin);
       return icqLoadProfileValue(icq, app, key, def);
    }
    return def;
 }

 ULONG EXPENTRY icqSaveValue(HICQ icq, const char *key, ULONG vlr)
 {
    char str[40];
    sprintf(str,"%lu",vlr);
    icqSaveString(icq, key, str);
    return vlr;
 }

 char * EXPENTRY icqLoadString(HICQ icq, const char *key, const char *def, char *buffer, int sz)
 {
    char app[20];

    if(icq->uin)
    {
       sprintf(app,"%lu",icq->uin);
       return icqLoadProfileString(icq,app,key,def,buffer,sz);
    }
    strncpy(buffer,def,sz);
    return buffer;
 }

 void EXPENTRY icqSaveString(HICQ icq, const char *key, const char *string)
 {
    char app[20];

    if(!icq->uin)
       return;

    sprintf(app,"%lu",icq->uin);
    icqSaveProfileString(icq, app, key, string);
 }

 ULONG EXPENTRY icqLoadReservedKey(HICQ icq, const char *key, ULONG vlr)
 {
    char buffer[20];

    icqLoadProfileString(icq, "Reserved", key, "", buffer, 19);

    if(*buffer)
       return atol(buffer);

    return 0;
 }

 const char * EXPENTRY icqLoadUserDB(HICQ icq, HUSER usr, const char *key, const char *def, char *buffer, int sz)
 {
    if(!icq->userDB)
    {
       strncpy(buffer,def,sz);
    }
    else if(icq->userDB->readUserDB(icq, usr, key, def, buffer, sz))
    {
       strncpy(buffer,def,sz);
    }
    return buffer;
 }


 const char * EXPENTRY icqSaveUserDB(HICQ icq, HUSER usr, const char *key, char *string)
 {
    if(!icq->userDB)
       return string;

    usr->u.flags   |= USRF_EXTENDED;
    icq->cntlFlags |= ICQFC_SAVEUSER;

    if(icq->userDB->writeUserDB(icq,usr,key,string))
       return NULL;

    return string;

 }


