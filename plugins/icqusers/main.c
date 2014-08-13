/*
 * ICQCONF.C - Modulo principal do plugin de configuracao
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdio.h>
 #include <string.h>

 #include "userdb.h"

/*---[ Prototipes ]------------------------------------------------------------------------------------------*/


/*---[ Publics ]---------------------------------------------------------------------------------------------*/

 HMODULE module = 0;

/*---[ Implementing ]----------------------------------------------------------------------------------------*/

 int EXPENTRY icqusrdb_Configure(HICQ icq, HMODULE hmod)
 {
    module = hmod;

    CHKPoint();

    icqWriteSysLog(icq,PROJECT,"User Database manager starting (BUILD " BUILD ")" );
    icqRegisterUserDBManager(icq,&icqusrdb_entryPoints);
    return 0;
 }

 int EXPENTRY icqusrdb_Terminate(HICQ icq, void *lixo)
 {
    icqWriteSysLog(icq,PROJECT,"User Database manager stopping" );
    icqDeRegisterUserDBManager(icq, &icqusrdb_entryPoints);
    icqusrdb_Save(icq, TRUE);
    return 0;
 }

 void EXPENTRY icqusrdb_Event(HICQ icq, void *dunno, ULONG uin, char id, USHORT eventCode, ULONG parm)
 {

    if(id == 'S')
    {
       switch(eventCode)
       {
       case ICQEVENT_SAVE:
          DBGMessage("Save user list");
          icqusrdb_Save(icq, FALSE);
          break;

       case ICQEVENT_LOADUSERS:
          DBGMessage("Load user list");
          icqusrdb_Load(icq);
          break;
       }
    }

 }


