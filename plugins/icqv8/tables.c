/*
 * TABLES.C
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif


 #include "icqv8.h"

/*---[ Constantes publicas ]----------------------------------------------------------------------------*/

 const ICQMODETABLE icqv8_modes[] =
 {
       { ICQ_ONLINE,             ICQICON_ONLINE,         "Online"        },
       { ICQ_AWAY,               ICQICON_AWAY,           "Away"          },
       { ICQ_NA,                 ICQICON_NA,             "N/A"           },
       { ICQ_OCCUPIED,           ICQICON_BUSY,           "Busy"          },
       { ICQ_DND,                ICQICON_DND,            "DND"           },
       { ICQ_FREEFORCHAT,        ICQICON_FREEFORCHAT,    "Free for chat" },
       { ICQ_OFFLINE,            ICQICON_OFFLINE,        "Offline"       },

       { 0,                      0,                      NULL            }
 };


//                                        ICQ_INVISIBLE,          14,      "Invisible",
/*
      ULONG             (* _System setMode)(HICQ,ULONG,ULONG);
      int               (* _System sendCmd)(HICQ, unsigned short, void *, int);
      HSEARCH           (* _System requestUserUpdate)(HICQ, ULONG, int);
      int               (* _System requestUserInfo)(HICQ, ULONG, int);
      USHORT            (* _System sendMessage)(HICQ, ULONG, USHORT, UCHAR *, ULONG *);
      int               (* _System sendConfirmation)(HICQ, ULONG, USHORT, BOOL, char *);
      int               (* _System addUser)(HICQ, HUSER);
      int               (* _System delUser)(HICQ, HUSER);
      USHORT            (* _System queryModeIcon)(HICQ, ULONG);
      int               (* _System setPassword)(HICQ,const char *, const char *);
      int               (* _System isOnline)(HICQ);
      int               (* _System searchByICQ)(HICQ, ULONG, HWND, SEARCHCALLBACK);
      int               (* _System searchByMail)(HICQ, const char *, HWND, SEARCHCALLBACK);
      int               (* _System searchRandom)(HICQ, USHORT, HWND, SEARCHCALLBACK);
      int               (* _System searchByInformation)(HICQ,HWND, const char *, const char *,const char *, const char *,BOOL, SEARCHCALLBACK );
*/

 const C2SPROTMGR icqv8_ftable = {       sizeof(C2SPROTMGR),
                                         0x08,
                                         XBUILD,
#ifdef DEBUG
    "ICQ Version 8 protocol manager Build " BUILD " (Debug Version)",
#else
    "ICQ Version 8 protocol manager Build " BUILD,
#endif
                                         512,
                                         icqv8_modes,
                                         icqv8_setMode,
                                         NULL,
                                         icqv8_searchByUIN,
                                         NULL,
                                         icqv8_sendMessage,
                                         NULL,
                                         icqv8_addUser,
                                         icqv8_delUser,
                                         icqv8_queryModeIcon,
                                         icqv8_setPassword,
                                         icqv8_isOnline,
                                         icqv8_searchByICQ,
                                         icqv8_searchByMail,
                                         icqv8_searchRandom,
                                         icqv8_searchByInformation
                                 };

 const struct xmlRequest icqv8_xmltable[] = {
                                        { 0x01, "ChannelsURL"                     },
                                        { 0x02, "BannersURL"                      },
                                        { 0x03, "DataFilesURL"                    },
                                        { 0x04, "PartnersURL"                     },
                                        { 0x05, "ReadersURL"                      },
                                        { 0x06, "CLBannersURL"                    },
                                        { 0x07, "DomainsURL"                      },
                                        { 0x08, "LicenseURL"                      },
                                        { 0x09, "ShowMOTDOnFirstTime"             },
                                        { 0x0a, "SMSShortSigTail"                 },
                                        { 0x0b, "SMSMedSigTail"                   },
                                        { 0x0c, "SMSLongSigTail"                  },

                                        { 0x00, NULL                                         }
                                };


#ifdef EXTENDED_LOG

 const char *icqv8_statusLog[] = {      "Offline"                          ,
                                        "Finding server"                   ,
                                        "Connecting to login server"       ,
                                        "Connecting to service server"     ,
                                        "Connected"                        ,

                                        NULL
                                 };

#endif




