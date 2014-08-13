#ifndef __ICQAPIDATA_H_
#define __ICQAPIDATA_H_


typedef struct {
	BYTE m_bEnabled;
	BYTE m_bSocksEnabled;
	short m_sSocksVersion;
	char m_szSocksHost[512];
	int m_iSocksPort;
	BYTE m_bSocksAuthenticationMethod;
} BSICQAPI_FireWallData;


typedef struct
{
	int m_iUIN;
	HWND m_hFloatWindow;
	int m_iIP;
	char m_szNickname[20];
	char m_szFirstName[20];
	char m_szLastName[20];
	char m_szEmail[100];
	char m_szCity[100];
	char m_szState[100];
	int m_iCountry;
	char m_szCountryName[100];
	char m_szHomePage[100];
	int m_iAge;
	char m_szPhone[20];
	BYTE m_bGender;
	int m_iHomeZip;

	// Version 1.0001

	int m_iStateFlags;
} BSICQAPI_User;


typedef BSICQAPI_User *BPSICQAPI_User;


typedef struct
{
	char m_szName[50];
	int m_iUserCount;
	BPSICQAPI_User *m_ppUsers;

} BSICQAPI_Group;


typedef BSICQAPI_Group *BPSICQAPI_Group;


void WINAPI ICQAPIUtil_FreeUser(BSICQAPI_User *pUser);
void WINAPI ICQAPIUtil_FreeUsers(int iCount, BSICQAPI_User **pUsers);
void WINAPI ICQAPIUtil_FreeGroup(BSICQAPI_Group *pGroup);

#endif	// __ICQAPIDATA_H_
