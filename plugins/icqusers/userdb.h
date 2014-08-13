/*
 * USERDB.H
 */

 #include <icqtlkt.h>

 #define sysLog(i,s)     icqWriteSysLog(i,PROJECT,s)

 extern const USERDBMGR icqusrdb_entryPoints;

 void icqusrdb_Load(HICQ);
 void icqusrdb_Save(HICQ, BOOL);





