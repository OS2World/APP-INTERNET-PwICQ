/*
 * passwd.c - Password management
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include "icqv8.h"

/* 	
CLI_METASETPASS
	
Channel: 2 SNAC(21,2) 2000/1070

Set a new password. Note: Never start your password with "ó".
Parameters
Data type 	Content 	Name 	Description
LNTS 	xx xx ... 	PASS 	The new password.
Examples

No examples.
*/

/*---[ Statics ]----------------------------------------------------------------------------------------------*/




/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 int _System icqv8_setPassword(HICQ icq,const char *oldPwd, const char *newPwd)
 {
    ICQV8 *cfg = icqGetPluginDataBlock(icq, module);

    if(!(cfg->status&ICQV8_STATUS_ONLINE))
       return 1;

    icqWriteSysLog(icq,PROJECT,"Password change isn't implemented");

    return -1;
 }

