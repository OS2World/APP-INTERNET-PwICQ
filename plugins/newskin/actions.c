/*
 * actions.c - Button actions
 */

 #include <string.h>
 #include <pwicqgui.h>

/*---[ Constants ]------------------------------------------------------------*/

 static const char *aboutUserURL   = "http://web.icq.com/whitepages/about_me/1,,,00.html?Uin=%ld";

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 void ICQCALLBACK icqskin_searchButton(hWindow hwnd, SKINDATA *cfg)
 {
	DBGMessage("Search button");
 }

 void ICQCALLBACK icqskin_configButton(hWindow hwnd, SKINDATA *cfg)
 {
	DBGMessage("SysConfig button");
	icqShowPopupMenu(cfg->icq,0,ICQMNU_SYSTEM,0,0);
 }

 void ICQCALLBACK icqskin_modeButton(hWindow hwnd, SKINDATA *cfg)
 {
	DBGMessage("Mode button");
	icqShowPopupMenu(cfg->icq,0,ICQMNU_MODES,0,0);
 }

 void ICQCALLBACK icqskin_sysmsgButton(hWindow hwnd, SKINDATA *cfg)
 {
	DBGMessage("System message button");
    icqOpenSystemMessage(cfg->icq);
 }

 void ICQCALLBACK icqskin_userlistClick(HICQ icq, HUSER usr)
 {
    if(!usr)
       return;

   DBGPrint("User Click: %s",icqQueryUserNick(usr));
   icqOpenUserMessageWindow( icq, usr);
 }

 void ICQCALLBACK icqskin_userContextMenu(HICQ icq, HUSER usr)
 {
    if(!usr)
       return;

	DBGPrint("User Context Menu: %s",icqQueryUserNick(usr));
	icqShowPopupMenu(icq,usr->uin,ICQMNU_USER,0,0);
 }

/*
 int ICQCALLBACK icqskin_openUserInfo(HICQ icq, ULONG uin, USHORT id, ULONG parm)
 {
    DBGTrace(uin);
    return 0;
 }
*/

 int ICQCALLBACK icqskin_openAboutUser(HICQ icq, ULONG uin, USHORT id, ULONG parm)
 {
    char         url[0x0100];
    char		 key[0x0100];

    icqLoadProfileString(icq, "MAIN", "aboutUserURL", aboutUserURL, key, 0xFF);

    sprintf(url,key,uin);

    icqOpenURL(icq, url);

    return 0;

 }


