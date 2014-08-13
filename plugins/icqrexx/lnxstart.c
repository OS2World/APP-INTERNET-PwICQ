/*
 * LNXStart.c - Start session
 */

 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>

 #include "icqrexx.h"


/*---[ Prototipes ]--------------------------------------------------------------------------*/


/*---[ Public ]------------------------------------------------------------------------------*/


/*---[ Implement ]---------------------------------------------------------------------------*/

 void EXPENTRY icqrexx_Event(HICQ icq, void *dataBlock, ULONG uin, char id, USHORT event, ULONG parm)
 {
    char key[20];
    char program[0x0100];

    sprintf(key,"helper:%c%03d",id,(int) event);
    icqLoadString(icq,key,"",program,0xFF);
	
	if(!*program)
	   return;

    if(uin)
    {
       if(icqrexx_expandUserMacros(icq, icqQueryUserHandle(icq, uin), 0, program))
	   {
		  icqWriteSysLog(icq,PROJECT,"Error expanding user macro");
          return;
	   }
    }

	if(icqrexx_expandSystemMacros(icq, event, uin, program))
	{
	   icqWriteSysLog(icq,PROJECT,"Error expanding system macro");
	   return;
	}

	DBGMessage(program);

#ifdef EXTENDED_LOG	
	icqWriteSysLog(icq,PROJECT,program);
#endif
	
	system(program);

 }



