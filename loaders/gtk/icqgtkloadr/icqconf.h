
#ifdef __OS2__
 #define INCL_WIN
#endif

 #include <pwMacros.h>
 #include <icqtlkt.h>

 extern HMODULE         module;

 extern const DLGMGR    antiSpam;
 extern const DLGMGR    request;
 extern const DLGMGR    usrBasic;
 extern const DLGMGR    away;
 extern const DLGMGR    groupNames;
 extern const DLGMGR    userGroups;
 extern const DLGMGR    miscOptions;

#ifdef __OS2__

 #pragma pack(1)
 struct cfgdlgparm
 {
    USHORT              sz;
    ULONG               uin;
    HICQ                icq;
 };

 MRESULT EXPENTRY cfgcpg(HWND, ULONG, MPARAM, MPARAM);

#endif

#ifdef linux

   extern const TABLEDEF antiSpam_table[];
   extern const TABLEDEF grpConfig_table[];
   extern const TABLEDEF mode_table[];
   extern const TABLEDEF request_table[];
   
#endif
