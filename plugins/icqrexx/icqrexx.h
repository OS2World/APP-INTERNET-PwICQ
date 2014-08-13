/*
 * icqrexx.h -
 */

 #include <pwMacros.h>
 #include <icqtlkt.h>

 extern const DLGMGR evtConfig;

 int EXPENTRY icqrexx_startHelper(HICQ, ULONG, UCHAR *, UCHAR *, UCHAR *);
 int EXPENTRY icqrexx_expandSystemMacros(HICQ, USHORT, ULONG, char *);
 int EXPENTRY icqrexx_expandUserMacros(HICQ, HUSER, ULONG, char *);





