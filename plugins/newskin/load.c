/*
 * LOAD.C - Load the skin definitions
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #include <io.h>
 #define R_OK 04
#endif

#ifdef linux
 #include <unistd.h>
#endif

 #include <string.h>
 #include <malloc.h>
 #include <ctype.h>

 #include <pwicqgui.h>

 #pragma pack(1)

/*---[ Structures ]---------------------------------------------------------------------------------------*/

 struct skinfile
 {
    SKINFILESECTION *first;
    SKINFILESECTION *last;
 };

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static SKINFILERECORD  *addEntry(HICQ, SKINFILESECTION *, const char *, const char *);
 static SKINFILESECTION *addSection(HICQ, const char *, struct skinfile *);
 static void            convertSkinFileFormat(HICQ, struct skinfile *);

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 const SKINFILESECTION * icqskin_getSkinFileSection(SKINFILESECTION *fl, const char *tag)
 {
    DBGMessage(tag);

    while(fl)
    {
       if(!strcmp(fl->tag,tag))
          return fl;
       fl = fl->down;
    }
    return fl;
 }

 const char *icqskin_getSkinFileEntry(SKINFILESECTION *fl, const char *tag, const char *key)
 {
    SKINFILERECORD *rec;

    while(fl)
    {
       if(!strcmp(fl->tag,tag))
       {
          for(rec = fl->first;rec;rec = rec->down)
          {
             if(!strcmp(rec->key,key))
                return rec->key + strlen(rec->key) + 1;
          }
          return NULL;
       }
       fl = fl->down;
    }
    return NULL;
 }

 static SKINFILESECTION *addSection(HICQ icq, const char *key, struct skinfile *fl)
 {
    SKINFILESECTION *ret = malloc(sizeof(SKINFILESECTION)+strlen(key));

    if(!ret)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when loading skin file section");
       return NULL;
    }

    memset(ret,0,sizeof(SKINFILESECTION));
    strcpy(ret->tag,key);

    if(fl->last)
       fl->last->down = ret;
    else
       fl->first      = ret;

    ret->up  = fl->last;
    fl->last = ret;

    return ret;
 }

 static SKINFILERECORD *addEntry(HICQ icq, SKINFILESECTION *current, const char *key, const char *ptr)
 {
    SKINFILERECORD *rec = malloc(sizeof(SKINFILERECORD)+strlen(key)+strlen(ptr)+2);

    if(!rec)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when loading skin file record");
       icqAbend(icq,0);
       return NULL;
    }

    memset(rec,0,sizeof(SKINFILERECORD));

    if(current->last)
       current->last->down = rec;
    else
       current->first      = rec;

    rec->up       = current->last;
    current->last = rec;

    strcpy(rec->key,key);
    strcpy(rec->key+strlen(rec->key)+1,ptr);

    return rec;
 }


 int icqskin_openSkinFile(SKINDATA *sk)
 {
    FILE            *arq;
    char            buffer[0x0200];
    char		    skinName[0x80];
    char            *ptr;
    char			*key;
    struct skinfile fl				= { NULL, NULL };
    SKINFILERECORD  *rec            = NULL;
    SKINFILESECTION *current        = NULL;
//    BOOL            convert		    = FALSE;
#ifdef linux
    static const char *fldr = "/";
#else
    static const char *fldr = "\\";
#endif

    icqskin_closeSkinFile(sk);

    icqLoadString(sk->icq,"skin","default.skn",buffer,0xFF);
    DBGMessage(buffer);

#ifdef DEBUG
    strcpy(buffer,".\\default.skn");
#endif

    for(key=ptr=buffer;*ptr;ptr++)
    {
       if(*ptr == ':' || *ptr == *fldr)
          key=ptr+1;
    }

    strncpy(skinName,key,0x80);
    DBGMessage(skinName);

    if(access(buffer,R_OK))
    {
       icqLoadString(sk->icq, "skinpath", "", buffer, 0xFF);
       strncat(buffer,skinName,0xFF);
       DBGMessage(buffer);
       if(access(buffer,R_OK))
       {
          icqQueryPath(sk->icq,"skins",buffer,0xFF);
          ptr = buffer+strlen(buffer);

          if(*ptr != *fldr)
             *(ptr++) = *fldr;
          strcpy(ptr,skinName);
       }
    }

    DBGMessage(buffer);

    arq = fopen(buffer,"r");

    if(!arq)
       return -1;

    for(key=ptr=buffer;*ptr;ptr++)
    {
       if(*ptr == ':' || *ptr == *fldr)
          key=ptr+1;
    }

    for(ptr=skinName;*ptr && *ptr != '.';ptr++)
       *(key++) = *ptr;
    *key = 0;
    strncat(buffer,fldr,0xFF);
    DBGMessage(buffer);
    icqSaveString(sk->icq,"SkinImages",buffer);

    while(!feof(arq))
    {
       *buffer = 0;
       fgets(buffer, 0xFF, arq);

       for(ptr=buffer;*ptr && *ptr >= ' ';ptr++);
       *ptr = 0;

       for(ptr=buffer;*ptr && isspace( ((int) *ptr) );ptr++);

       switch(*ptr)
       {
       case '[':	// New section
          for(key=ptr+1;*ptr && *ptr != ']';ptr++);
          *ptr = 0;
          DBGMessage(key);
          current = addSection(sk->icq,key,&fl);
          break;

       case '#':	// Comment
       case 0:		// No entry
          break;

       default:		// Regular entry

          for(key=ptr;*ptr && *ptr != '=';ptr++);

          if(*ptr)
          {
             *(ptr++) = 0;
             if(!current)
                current = addSection(sk->icq,"*",&fl);
             rec = addEntry(sk->icq, current,key,ptr);
          }

       }

    }

    fclose(arq);

    if(!strcmp(fl.first->tag,"*"))
       convertSkinFileFormat(sk->icq, &fl);

    sk->skin = fl.first;
    return 0;
 }

 static void clearSection(SKINFILESECTION *sk)
 {
    SKINFILERECORD *next;

    while(sk->first)
    {
       next = sk->first->down;
       free(sk->first);
       sk->first = next;
    }
    sk->last = NULL;
 }

 int icqskin_closeSkinFile(SKINDATA *skn)
 {
    SKINFILESECTION *next;
    while(skn->skin)
    {
       next = skn->skin->down;
       clearSection(skn->skin);
       free(skn->skin);
       skn->skin = next;
    }
    skn->skin = NULL;
    return 0;
 }

 static void convertSkinFileFormat(HICQ icq, struct skinfile *fl)
 {
    static const struct cnvt
    {
       const char *key;
       const char *section;
       const char *tag;
    } cnvtList[] =
    {  { "TitleLeft",					"Main",			"TitleLeft"                },
       { "TitleStretch",				"Main",			"TitleStretch"             },
       { "TitleRight",					"Main",			"TitleRight"               },
       { "MainTopLeft",					"Main",			"TopLeft"                  },
       { "MainTopRight",				"Main",			"TopRight"                 },
       { "MainMidLeft",					"Main",			"MidLeft"                  },
       { "MainMidStretch",				"Main",			"MidStretch"               },
       { "MainMidRight",				"Main",			"MidRight"                 },
       { "MainBottomLeft",				"Main",			"BottomLeft"               },
       { "MainBottomStretch",			"Main",			"BottomStretch"            },
       { "MainBottomRight",				"Main",			"BottomRight"              },
       { "MainBottomMidLeft",			"Main",			"BottomMidLeft"            },
       { "MainBottomMidRight",			"Main",			"BottomMidRight"           },
       { "MainBackgroundColor",			"Main",			"Background"               },
       { "ForegroundColor",				"Main",			"Foreground"               },
       { "SelectedBackgroundColor",		"Main",			"SelectedBackground"       },
       { "SelectedForegroundColor",		"Main",			"SelectedForeground"       },
       { "OfflineColor",				"Main",			"Offline"                  },
       { "OnlineColor",					"Main",			"Online"                   },
       { "OfflineWaitingColor",			"Main",			"OfflineWaiting"           },
       { "OnlineWaitingColor",			"Main",			"OnlineWaiting"            },
       { "ButtonForegroundColor",		"Main",			"Button.Foreground"        },

       { "MinimumSize",					NULL,			NULL                       },
       { "MaximunSize",					NULL,			NULL                       },

       { "MenuBackgroundColor",			"Menu",			"Background"               },
       { "MenuForegroundColor",			"Menu",			"Foreground"               },
       { "MenuSelectedBackgroundColor",	"Menu",			"SelectedBackground"       },
       { "MenuSelectedForegroundColor",	"Menu",			"SelectedForeground"       },
       { "MenuFont",					"Menu",			"Font"                     },

       { "Icons",						"Main",			"Icons"                    },

       { "UserList",					NULL,	   		NULL                       },
       { "UserListFont",				"Main",			"UserList.Font"            },

       { "ModeButton",					"Main",			"ModeButton"               },
       { "ModeButtonFont",				"Main",			"ModeButton.Font"          },

       { "SysButton",					"Main",			"SysButton"                },
       { "SystemButtonFont",			"Main",			"SysButton.Font"           },

       { "SysMessage",					"Main",			"SysMsgButton"             },
       { "SysMessageColor",				"Main",			"SysMsgButton.Foreground"  },
       { "SysMessageFont",				"Main",			"SysMsgButton.Font"        },

       { "ScrollBar",					NULL,			NULL                       },
       { "Up",							NULL,			NULL                       },
       { "down",						NULL,			NULL                       },

       { "CloseButton",					NULL,			NULL                       },
       { "MinimizeButton",				NULL,			NULL                       },

       { "SizeButton",					NULL,			NULL                       },
       { "logo",						NULL,			NULL                       }

    };


    int              f;
    const char       *vlr;
    SKINFILESECTION  *section;

    DBGMessage("********** Converting old format skin definition file **********");
    icqWriteSysLog(icq,PROJECT,"Converting skin file format");

    for(f=0;f < sizeof(cnvtList) / sizeof(struct cnvt); f++)
    {
       if(cnvtList[f].section && cnvtList[f].key)
       {
          vlr = icqskin_getSkinFileEntry(fl->first, "*", cnvtList[f].key);
          if(vlr)
          {
             /* Found key, create a new format entry for it */
             section = (SKINFILESECTION *) icqskin_getSkinFileSection(fl->first, cnvtList[f].section);
             if(!section)
                section = addSection(icq,cnvtList[f].section,fl);
             addEntry(icq, section, cnvtList[f].tag, vlr);
          }
       }
    }
 }


