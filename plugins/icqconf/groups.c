/*
 * Group dialogs
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdlib.h>
 #include <string.h>
 #include <stdio.h>

 #include "icqconf.h"

/*---[ Configuration ]---------------------------------------------------------------------------------------*/

 #define GROUPS 18

/*---[ Prototipes ]------------------------------------------------------------------------------------------*/

 static int   _System name_load(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int   _System name_save(const DLGHELPER *,HWND,HICQ,ULONG,  char *);
 static int   _System user_load(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int   _System user_save(const DLGHELPER *,HWND,HICQ,ULONG,  char *);

/*---[ Constants ]-------------------------------------------------------------------------------------------*/

 DECLARE_SIMPLE_DIALOG_MANAGER(groupNames,name_load,name_save);
 DECLARE_SIMPLE_DIALOG_MANAGER(userGroups,user_load,user_save);

/*---[ Implementing ]----------------------------------------------------------------------------------------*/

 static int name_load(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    char        key[20];
    int         f;

    icqLoadString(icq, "Group00", "All", buffer, 40);
    dlg->setString(hwnd,200,buffer);

    for(f=1;f<GROUPS;f++)
    {
       sprintf(key,"Group%02d",f);
       icqLoadString(icq, key, "", buffer, 40);
       dlg->setString(hwnd,200+f,buffer);
    }

    return 0;
 }

 static int _System name_save(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin,  char *buffer)
 {
    char        key[20];
    int         f;
    for(f=0;f<GROUPS;f++)
    {
       dlg->getString(hwnd,200+f,40,buffer);
       sprintf(key,"Group%02d",f);
       icqSaveString(icq, key, buffer);
    }
    return 0;
 }

 static int user_load(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    char        key[20];
    int         f;
    HUSER       usr             = icqQueryUserHandle(icq,uin);
    ULONG       groupMasc       = 0;
    ULONG       masc            = 1;

    if(usr)
       groupMasc = usr->groupMasc;

    for(f=0;f<GROUPS;f++)
    {
       sprintf(key,"Group%02d",f);
       icqLoadString(icq,key,"",buffer,40);
       if(*buffer)
       {
          dlg->setString(hwnd,200+f,buffer);
          dlg->setCheckBox(hwnd,200+f,groupMasc & masc);
       }
       dlg->setEnabled(hwnd,200+f,*buffer && usr);
       masc <<= 1;
    }
    return 0;
 }

 static int _System user_save(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin,  char *buffer)
 {
    HUSER       usr             = icqQueryUserHandle(icq,uin);
    ULONG       groupMasc;
    ULONG       masc            = 1;
    int         f;

    if(!usr)
       return -1;

    groupMasc = usr->groupMasc & 0xfffc0000;

    for(f=0;f<GROUPS;f++)
    {
       if(dlg->getCheckBox(hwnd,200+f))
          groupMasc |= masc;
       masc <<= 1;
    }

    DBGTracex(groupMasc);

    if(groupMasc != usr->groupMasc)
    {
       usr->groupMasc = groupMasc;
       icqUserEvent(icq, usr, ICQEVENT_MODIFIED);
       DBGTracex(usr->groupMasc);
    }
    return 0;
 }

