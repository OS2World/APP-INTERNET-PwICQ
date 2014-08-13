#ifndef __ICQAPINOTIFICATIONS_H_
#define __ICQAPINOTIFICATIONS_H_


#define ICQAPINOTIFY_ONLINELIST_CHANGE				0
#define ICQAPINOTIFY_ONLINE_FULLUSERDATA_CHANGE		1
#define ICQAPINOTIFY_APPBAR_STATE_CHANGE			2
#define ICQAPINOTIFY_ONLINE_PLACEMENT_CHANGE		3
#define ICQAPINOTIFY_OWNER_CHANGE					4
#define ICQAPINOTIFY_OWNER_FULLUSERDATA_CHANGE		5
#define ICQAPINOTIFY_ONLINELIST_HANDLE_CHANGE		6

#define ICQAPINOTIFY_FILE_RECEIVED					8

#define ICQAPINOTIFY_LAST							80


#define ICQAPINOTIFY_ONLINELISTCHANGE_ONOFF			1
#define ICQAPINOTIFY_ONLINELISTCHANGE_FLOAT			2
#define ICQAPINOTIFY_ONLINELISTCHANGE_POS			3



typedef void (WINAPI *BICQAPINotify_OnlineListChange)(int iType);

typedef void (WINAPI *BICQAPINotify_OnlineFullUserDataChange)(int iUIN);

typedef void (WINAPI *BICQAPINotify_AppBarStateChange)(int iDockingState);

typedef void (WINAPI *BICQAPINotify_OnlinePlacementChange)();

typedef void (WINAPI *BICQAPINotify_OwnerChange)(int iUIN);

typedef void (WINAPI *BICQAPINotify_OwnerFullDataChange)();

typedef void (WINAPI *BICQAPINotify_OnlineListHandleChange)(HWND hWindow);

typedef void (WINAPI *BICQAPINotify_FileReceived)(char *pszFileNames);


void WINAPI ICQAPIUtil_SetUserNotificationFunc(UINT uNotificationCode, void *pUserFunc);


#endif	// __ICQAPINOTIFICATIONS_H_
