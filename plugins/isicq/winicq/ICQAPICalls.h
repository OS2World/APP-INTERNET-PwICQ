#ifndef __ICQAPICALLS_H_
#define __ICQAPICALLS_H_


#include "ICQAPIData.h"

#define BICQAPI_USER_STATE_ONLINE		0
#define BICQAPI_USER_STATE_CHAT			1
#define BICQAPI_USER_STATE_AWAY			2
#define BICQAPI_USER_STATE_OUT			3
#define BICQAPI_USER_STATE_OCCUPIED		4
#define BICQAPI_USER_STATE_DND			5
#define BICQAPI_USER_STATE_INVISIBLE	6
#define BICQAPI_USER_STATE_OFFLINE		7


BOOL WINAPI ICQAPICall_SetLicenseKey(char *pszName, char *pszPassword, char *pszLicense);

BOOL WINAPI ICQAPICall_GetVersion(int *iVersion);

BOOL WINAPI ICQAPICall_GetDockingState(int *iDockingState);

BOOL WINAPI ICQAPICall_GetFirewallSettings(BSICQAPI_FireWallData *oFireWallData);

BOOL WINAPI ICQAPICall_GetFullOwnerData(BSICQAPI_User **ppUser, int iVersion);

BOOL WINAPI ICQAPICall_GetFullUserData(BSICQAPI_User *pUser, int iVersion);

BOOL WINAPI ICQAPICall_GetOnlineListDetails(int *iCount, BPSICQAPI_User **ppUsers);

BOOL WINAPI ICQAPICall_GetOnlineListHandle(HWND *hWindow);

BOOL WINAPI ICQAPICall_GetOnlineListPlacement(int *iIsShowOnlineList);

BOOL WINAPI ICQAPICall_GetWindowHandle(HWND *hWindow);

BOOL WINAPI ICQAPICall_RegisterNotify(int iVersion, int iCount, BYTE *piEvents);

BOOL WINAPI ICQAPICall_UnRegisterNotify();

BOOL WINAPI ICQAPICall_SendFile(int iPIN, char *pszFileNames);


// Version 1.0001

BOOL WINAPI ICQAPICall_GetOnlineListType(int *iListType);

BOOL WINAPI ICQAPICall_GetGroupOnlineListDetails(int *iGroupCount, BPSICQAPI_Group **ppGroups);

BOOL WINAPI ICQAPICall_SetOwnerState(int iState);

BOOL WINAPI ICQAPICall_SetOwnerPhoneState(int iPhoneState);

BOOL WINAPI ICQAPICall_SendMessage(int iUIN, char *pszMessage);

BOOL WINAPI ICQAPICall_SendURL(int iUIN, char *pszMessage);

BOOL WINAPI ICQAPICall_SendExternal(int iUIN, char *pszExternal, char *pszMessage, BYTE bAutoSend);


BOOL WINAPI ICQAPICall_Generic(int iCode, void *pInBuffer, int iInSize, void **ppOutBuffer, int *iOutSize);


#endif	// __ICQAPICALLS_H_
