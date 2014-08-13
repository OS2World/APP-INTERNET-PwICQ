/*
 * SYSMENU.C - Processa selecoes pelo menu de usuario
 */

 #include <stdio.h>



 #include "icqskmgr.h"

/*---[ Constants ]----------------------------------------------------------------------------------*/

 static const char *aboutUserURL   = "http://web.icq.com/whitepages/about_me/1,,,00.html?Uin=%ld";

/*---[ Prototipos ]---------------------------------------------------------------------------------*/

 static void removeUser(HICQ,HWND,ULONG,char *);

/*---[ Implementacao ]------------------------------------------------------------------------------*/

 void procUsrMenu(HWND hwnd, HUSER usr, ICQWNDINFO *ctl, ULONG id)
 {
    if(!usr)
       return;

    switch(id)
    {
    case  ICQICON_UNREADMESSAGE: // Send message
       icqNewUserMessage(ctl->icq, usr->uin, MSG_NORMAL, 0, 0);
       break;

    case  ICQICON_UNREADURL: // Send URL
       icqNewUserMessage(ctl->icq, usr->uin, MSG_URL, 0, 0);
       break;

    case  8: // Ask for authorization
       icqNewUserMessage(ctl->icq, usr->uin, MSG_REQUEST, 0, 0);
       break;

    case  29: // Send authorization
       icqNewUserMessage(ctl->icq, usr->uin, MSG_AUTHORIZED, 0, 0);
       break;

    case ICQICON_UPDATE: // Update basic info
       icqRequestUserUpdate(ctl->icq, usr->uin);
       break;

    case ICQICON_REMOVE: // Remove from list
       removeUser(ctl->icq,hwnd,usr->uin,ctl->buffer);
       break;

    case ICQICON_FILE: // Send File
       icqNewUserMessage(ctl->icq, usr->uin, MSG_FILE, 0, 0);
       break;

    case ICQICON_ABOUT: // About user
       aboutUser(ctl->icq,usr->uin,ctl->buffer);
       break;

    case ICQICON_USERCONFIG:
       icqOpenConfigDialog(ctl->icq, usr->uin, 0);
       break;

#ifdef DEBUG
    default:
       DBGTrace(usr->uin);
       DBGTrace(id);
#endif
    }

 }

 static void removeUser(HICQ icq,HWND hwnd, ULONG uin, char *buffer)
 {
    HUSER       usr             = icqQueryUserHandle(icq,uin);
    HAB         hab             = WinQueryAnchorBlock(hwnd);
    char        title[40];
    char        text[0x0100];

    if(!usr)
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected user handler when deleting");
       return;
    }

    WinLoadString(hab, module, 1203, 0x30, (PSZ) buffer);
    sprintf(title,buffer,uin);

    WinLoadString(hab, module, 1204, 0x80, (PSZ) buffer);
    sprintf(text,buffer,icqQueryUserNick(usr), uin);

    DBGMessage(title);
    DBGMessage(text);

    if( WinMessageBox(  HWND_DESKTOP,
                        hwnd,
                        text,
                        title,
                        10,
                        MB_OKCANCEL|MB_QUERY|MB_APPLMODAL|MB_MOVEABLE|MB_DEFBUTTON2 ) == MBID_OK )
    {
       sprintf(buffer,"Removing %s (ICQ#%d) by user request",icqQueryUserNick(usr),uin);
       DBGMessage(buffer);
       icqWriteSysLog(icq,PROJECT,buffer);
       icqRemoveUser(icq,usr);
    }


 }

 void aboutUser(HICQ icq, ULONG uin, char *key)
 {
    char url[0x0180];

    icqLoadProfileString(icq, "MAIN", "aboutUserURL", aboutUserURL, key, 0xFF);

    sprintf(url,key,uin);

    DBGMessage(key);
    DBGMessage(url);

    icqOpenURL(icq, url);

 }

