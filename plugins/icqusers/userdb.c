/*
 * USERDB.C - Manipulacao da base de usuarios
 */

#ifdef __OS2__
 #define INCL_DOSPROFILE
#endif

 #include <stdio.h>
 #include <stdlib.h>

 #include <icqtlkt.h>

 #include "userdb.h"

/*---[ Prototipos ]---------------------------------------------------------------------------*/

 static int _System writeUserDB(HICQ, HUSER, const char *, char *);
 static int _System readUserDB(HICQ, HUSER, const char *, const char *, char *, int);
 static int _System deleteUserDB(HICQ, HUSER);

/*---[ Constantes ]---------------------------------------------------------------------------*/

 const USERDBMGR icqusrdb_entryPoints = { sizeof(USERDBMGR), writeUserDB, readUserDB, deleteUserDB };

/*---[ Implementacao ]------------------------------------------------------------------------*/

 static int _System writeUserDB(HICQ icq, HUSER usr, const char *key, char *string)
 {
    char app[20];
    if(!usr)
       return -1;
    sprintf(app,"db.%ld",usr->uin);
    icqSaveProfileString(icq, app, key, string);
    return 0;
 }

 static int _System readUserDB(HICQ icq, HUSER usr, const char *key, const char *def, char *buffer, int sz)
 {
    char app[20];
    if(!usr)
       return -1;
    sprintf(app,"db.%ld",usr->uin);
    icqLoadProfileString(icq,app,key,def,buffer,sz);
    return 0;
 }

 static int _System deleteUserDB(HICQ icq, HUSER usr)
 {
    char app[20];
    if(!usr)
       return -1;
     sprintf(app,"db.%ld",usr->uin);
     return 0;
     icqDeleteProfile(icq,app);
 }

