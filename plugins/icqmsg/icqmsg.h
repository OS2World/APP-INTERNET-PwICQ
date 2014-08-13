/*
 * icqmsg.h
 */


 #include <pwMacros.h>
 #include <icqtlkt.h>
 #include <msgdlg.h>

 extern HMODULE module;

 extern MSGMGR  icqmsg_normal;
 extern MSGMGR  icqmsg_url;
 extern MSGMGR  icqmsg_authorized;
 extern MSGMGR  icqmsg_request;
 extern MSGMGR  icqmsg_refuse;
 extern MSGMGR  icqmsg_added;
 extern MSGMGR  icqmsg_file;

 void _System icqmsg_filterURL(HICQ, UCHAR *, ICQMSG *);

#ifdef OLD_FORMAT
 #define FILTER_REQUEST icqmsg_filterRequest
 void _System icqmsg_filterRequest(HICQ, UCHAR *, ICQMSG *);
#else
 #define FILTER_REQUEST NULL
#endif

 #ifndef MSGEDITWINDOW
    #define MSGEDITWINDOW hWindow
 #endif

 void _System icqmsg_requestWindowChanged(const MSGEDITHELPER *, MSGEDITWINDOW, HICQ, ULONG, USHORT, USHORT);
 int  _System icqmsg_getField(HICQ, ICQMSG *, int, int, char *);
 void         icqmsg_loadUserInfoFromRequest(const MSGEDITHELPER *, MSGEDITWINDOW, HICQ, ULONG, HMSG);







