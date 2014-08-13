/*
 * icqkget.c - KGet interface plugin
 *
 */

 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 
 #include <icqtlkt.h>

/*----------------------------------------------------------------------------*/

 int EXPENTRY icqkget_Configure(HICQ icq, HMODULE mod)
 {
	CHKPoint();
    icqWriteSysLog(icq, PROJECT,"KGET interface plugin starting (Build " BUILD ")");
    return 0;
 }

 static int _System beginDownload(HICQ icq, ULONG uin, const MSGMGR *mgr, const char *text, const char *url)
 {
	char buffer[0x0100];
	char result[20];
	
	strcpy(buffer,"kget \"");
	strncat(buffer,url,0xFF);
	strncat(buffer,"\"",0xFF);
	*(buffer+0xFF) = 0;
	
	sprintf(result," (rc=%d)",system(buffer));
	
	strncat(buffer,result,0xFF);
	*(buffer+0xFF) = 0;
  	
	icqWriteSysLog(icq,PROJECT,buffer);
	
    return 0;
 }

 void EXPENTRY icqkget_addSendTo(HICQ icq, hWindow hwnd, USHORT type, BOOL out, const MSGMGR *mgr, int ( * _System addOption)(HWND, USHORT, const char *, SENDTOCALLBACK *))
 {
    /*
     * Add option in the Send to button
     */
    addOption(hwnd, 47, "KGet", beginDownload);

 }

