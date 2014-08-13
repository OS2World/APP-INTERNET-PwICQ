/*
 * lnxurl.c - Abre uma URL (Versao Linux)
 */

 #include <string.h>
 #include <stdlib.h>
 #include <stdio.h>

 #include <icqtlkt.h>

/*---[ Implementacao ]---------------------------------------------------------------------------------*/

 int EXPENTRY icqOpenURL(HICQ icq, const char *url)
 {
    int	ret;
	char key[0x0100];
    char buffer[0x0200];
	
	icqLoadString(icq,"browser","mozilla -remote \"openurl(\"%s,new-tab\")\"",key,0xFF);

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,key);
#endif

	sprintf(buffer,key,url);
	
#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,buffer);
#endif	
	
	ret = system(buffer);
	DBGTrace(ret);
	

/*
    icqLoadString(icq,"browser", "mozilla", buffer, 0xFF);
    strncat(buffer," \"",0x02F0);
    strncat(buffer,url,0x02F0);
    strncat(buffer,"\" &",0x02FF);

    DBGMessage(buffer);

    ret = system(buffer);

    DBGTrace(ret);
*/	

    return ret;
 }


