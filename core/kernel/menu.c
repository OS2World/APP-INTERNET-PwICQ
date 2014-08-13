/*
 * tools.c - Ferramentas diversas
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>

 #include <icqkernel.h>

/*---[ Prototipos ]--------------------------------------------------------------------*/

 static int _System shutdown(HICQ, ULONG, USHORT,ULONG);
 static int _System disconnect(HICQ, ULONG, USHORT,ULONG);
 static int _System updateall(HICQ, ULONG, USHORT,ULONG);
 static int _System message(HICQ, ULONG, USHORT,ULONG);
 static int _System update(HICQ, ULONG, USHORT,ULONG);
 static int _System aboutusr(HICQ, ULONG, USHORT,ULONG);
 static int _System config(HICQ, ULONG, USHORT,ULONG);

/*---[ Tabelas ]-----------------------------------------------------------------------*/

 static const char *aboutUserURL   = "http://web.icq.com/whitepages/about_me/1,,,00.html?Uin=%ld";

 #pragma pack(1)

 struct menu_element
 {
    USHORT       id;
    MENUCALLBACK *cbk;
    ULONG        parm;
    const  char  *text;
 };

 static const struct menu_element user_menu[] =
 { {  0,                 (MENUCALLBACK *) message,    MSG_NORMAL,   "Send message"           },
   {  2,                 (MENUCALLBACK *) message,    MSG_URL,      "Send URL"               },
   {  8,                 (MENUCALLBACK *) message,    MSG_REQUEST,  "Ask for authorization"  },
   { 29,                 (MENUCALLBACK *) message,    MSG_ACCEPTED, "Send authorization"     },
   { ICQICON_UPDATE,     (MENUCALLBACK *) update,     0,            "Update basic info"      },
   { ICQICON_REMOVE,     (MENUCALLBACK *) NULL,       0,            "Remove from list"       },
   { ICQICON_FILE,       (MENUCALLBACK *) message,    MSG_FILE,     "Send file"              },
   { ICQICON_USERCONFIG, (MENUCALLBACK *) config,     0,            "User info"              },
   { ICQICON_ABOUT,      (MENUCALLBACK *) aboutusr,   0,            "About user"             }
 };


 static const struct menu_element system_menu[] =
 { { ICQICON_DISCONNECT, (MENUCALLBACK *) disconnect, 0,            "Disconnect"             },
   { ICQICON_SEARCH,     (MENUCALLBACK *) NULL,       0,            "Search/Add"             },
   { 26,                 (MENUCALLBACK *) NULL,       0,            "About pwICQ"            },
   { ICQICON_UPDATE,     (MENUCALLBACK *) updateall,  0,            "Update all users"       },
   { ICQICON_SHUTDOWN,   (MENUCALLBACK *) shutdown,   0,            "Shutdown"               },
   { ICQICON_CONFIGURE,  (MENUCALLBACK *) config,     1,            "Configure"              },
   { ICQICON_ABOUT,      (MENUCALLBACK *) NULL,       0,            "About Me"               }
 };

/*---[ Implementacao ]-----------------------------------------------------------------*/

 int EXPENTRY icqInsertMenuOption(HICQ icq, USHORT menu, const char *text, USHORT icon, MENUCALLBACK *cbk, ULONG parm)
 {
    if(icq->skin && icq->skin->insertMenu)
       return icq->skin->insertMenu(icq,menu,text,icon,cbk,parm);
    return -1;
 }

 int EXPENTRY icqShowPopupMenu(HICQ icq, ULONG uin, USHORT id, USHORT x, USHORT y)
 {
    if(icq->skin && icq->skin->popupMenu)
       return icq->skin->popupMenu(icq,uin,id,x,y);
    return -1;
 }

 static void loadMenu(HICQ icq, USHORT id, const struct menu_element *el, int sz)
 {
    static const char mnuID[] = { ICQMNU_SYSTEM, ICQMNU_USER };

    char   itens[0x0100];
    char   *token;
    int    f;
    USHORT icon;

    icqQueryInternalTable(icq, id, itens, 0x7F);

    token = strtok(itens, ",");
    while(token)
    {
       icon  = atoi(token);

       for(f=0;f<sz && (el+f)->id != icon;f++);

       if(f < sz)
          icqInsertMenuOption(icq, mnuID[id], (el+f)->text,  icon, (el+f)->cbk, (el+f)->parm);
       else
          icqInsertMenuOption(icq, mnuID[id], "Unknown", icon, NULL, icon);

       token = strtok(NULL, ",");
    }

 }

 void icqLoadMenus(HICQ icq)
 {

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"Loading menus");
#endif

    loadMenu(icq, 0, system_menu, sizeof(system_menu)/sizeof(struct menu_element));
    loadMenu(icq, 1, user_menu,   sizeof(user_menu)/sizeof(struct menu_element));

 }

 static int _System shutdown(HICQ icq, ULONG uin, USHORT id, ULONG parm)
 {
    icqWriteSysLog(icq,PROJECT,"Stopping by user request");
    icqEvent(icq, uin, 'S', ICQEVENT_TERMINATE, 0);
    return 0;
 }

 static int _System disconnect(HICQ icq, ULONG uin, USHORT id, ULONG parm)
 {
    icqWriteSysLog(icq,PROJECT,"Disconnecting by user request");
    icqDisconnect(icq);
    return 0;
 }

 static int _System updateall(HICQ icq, ULONG uin, USHORT id, ULONG parm)
 {
    HUSER usr;
    icqWriteSysLog(icq,PROJECT,"Updating contact-list");
    for(usr = icqQueryFirstUser(icq); usr; usr = icqQueryNextUser(icq,usr) )
       usr->u.flags |= USRF_REFRESH;
    return 0;
 }

 static int _System message(HICQ icq, ULONG uin, USHORT id, ULONG parm)
 {
    DBGTracex(parm);
    icqNewUserMessage(icq, uin, (USHORT) parm, 0, 0);
    return 0;
 }

 static int _System update(HICQ icq, ULONG uin, USHORT id, ULONG parm)
 {
    icqRequestUserUpdate(icq, uin);
    return 0;
 }

 static int _System aboutusr(HICQ icq, ULONG uin, USHORT id, ULONG parm)
 {
    char url[0x0180];
    char key[0x0100];
    icqLoadProfileString(icq, "MAIN", "aboutUserURL", aboutUserURL, key, 0xFF);
    sprintf(url,key,uin);
    DBGMessage(key);
    DBGMessage(url);
    icqOpenURL(icq, url);
    return 0;
 }

 static int _System config(HICQ icq, ULONG uin, USHORT id, ULONG parm)
 {
    if(id == 1)
       uin = 0;

    DBGTrace(id);
    icqOpenConfigDialog(icq, uin, (USHORT) parm);
    return 0;
 }

