/*
 * EXEC.C - Executa comando
 */

#ifdef __OS2__

   #pragma strings(readonly)

   #define INCL_DOSMODULEMGR
   #define INCL_DOSPROCESS
   #define sleep(x) DosSleep(x*1000)

#endif

#ifdef linux
   #include <unistd.h>
   #include <signal.h>
   #include <sys/time.h>
   #include <sys/types.h>
#endif

   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include <ctype.h>

   #include <icqkernel.h>

/*---[ Definicoes ]--------------------------------------------------------------*/

 #pragma pack(1)
 struct cmd
 {
    const char  *keyword;
    int         (* _System exec)(HICQ, char *);
 };

/*---[ Constantes ]--------------------------------------------------------------*/

#ifdef DEBUG

// static char *dbgRTF = "{\\rtf1\\ansi\\ansicpg1251\\deff0\\deflang1059{\\fonttbl{\\f0\\fmodern\\fprq1\\fcharset204{\\*\\fname Courier New;}Courier New CYR;}}\n\\colortbl ;\\red0\\green0\\blue0;}\n\\viewkind4\\uc1\\pard\\cf1\\f0\\fs20хотя аська глюкает страшно \\lang1049иногда\\par\npar\nессаги глотает как крокодил\\par\npar\nак что если я чего не получил -- это нормально =)\\lang1059\\par\n";
// static char *dbgRTF =  "{\\rtf1\\ansi\\ansicpg1251\\deff0\\deflang1049{\\fonttbl{\\f0\\fswiss\\fcharset204{\\*\\fname Arial;}Arial CYR;}{\\f1\\fswiss\\fprq2\\fcharset204{\\*\\fname MS Sans Serif;}MS Sans Serif;}}{\\colortbl ;\\red0\\green0\\blue0;}\\viewkind4\\uc1\\pard\\cf1\\f0\\fs20Teste\\f1\\par";
// static char *dbgRTF =  "{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1046{\\fonttbl{\\f0\\fnil\\fcharset0 Verdana;}{\\f1\\froman\\fcharset0{\\colortbl ;\\red255\\green255\\blue255;\\red0\\green0\\blue0;}\\viewkind4\\uc1\\pard\\sb100\\sa100\\cf1\\b\\f0\\fs20\\line\\line Pбgina nгo encontrada.\\b0\\line\\cf0\\f1\\fs18 O link para a pбgina nгo funciono\\pard\\cf2\\f2\\par}";

// static char *dbgRTF =  "{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1033{\\fonttbl{\\f0\\fnil\\fcharset0 Times New Roman;}}{\\colortbl ;\\red0\\green0\\blue0;}\\viewkind4\\uc1\\pard\\cf1\\f0\\fs20 ministro da Seguran\\'e7a Alimentar e Combate\\par\\pard\\'e0 Fome\\parinteressante esse t\\'edtulo, quer dizer que manter a seguran\\'e7a alimentar de alguem nao significa combater a fome?\\par\\par}";
// static char *dbgRTF =  "{\\rtf1\\ansi\\ansicpg950\\deff0\\deflang1033\\deflangfe1028{\\fonttbl{\\f0\\fswiss\\fcharset0 Arial;}{\\f1\\fscript\\fprq2\\fcharset136 Comic Sans MS;}}{\\colortbl ;\\red255\\green128\\blue192;}\\viewkind4\\uc1\\pard\\cf1\\lang1028\\f0\\fs24 hi\\f1\\par}";

// static char *dbgRTF =  "{\\rtf1\\ansi\\def0{\\fonttbl{\\f0\\fswiss MS Sans Serif;}{\\f1\\fswiss Sans;}}{\\colortbl ;\\red45\\green45\\blue45;}\\viewkind4\\pard\\cf1\\f0 \\f1\\highlight0\\pard Achso - hier mal Umlaute\\u246?\\u228?\\u223? und ein Euro \\u8364?\\par}";

//   static char *dbgRTF =  "{\\rtf1\\ansi\\ansicpg1200\\deff0\\deflang1046{\\fonttbl {\\f0\\fnil\\fcharset0 Times New Roman;}}{\\colortbl ;\\red30\\green60\\blue110;\\red0\\green0\\blue255;}\\uc1\\pard\\cf1\\f0 aquele perfuem da minha m\\'e3e\\par\\cf2\\ul http://www.epinions.com/Fragrances-BEAUTIFUL_ESTEE_LAUDER_WOMEN_2_5_oz_EDP_SPRAY_FRAGRANCE_BEAUTY_WOMEN_S_PERFUMES_BEAUTIFUL_For_Women_By_ESTEE_LAUDER\\cf1\\ul0\\par}";

   static char *dbgRTF =  "{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0\\fswiss MS Sans Serif;}{\\f1\\fswiss helvetica;}}{\\colortbl ;\\red0\\green0\\blue0;}\\viewkind4\\pard\\cf1\\f0 \\f1\\highlight0\\pard mach ich hier z.b \\u220?\\paroder hier \\u228?\\par}";

#endif



/*---[ Prototipos ]--------------------------------------------------------------*/

// static void execCMD(HICQ,char *);

 static int  _System    bye(HICQ, char *);
 static int  _System    message(HICQ, char *);
 static int  _System    mode(HICQ, char *);
 static int  _System    unload(HICQ, char *);
 static int  _System    load(HICQ, char *);
 static int  _System    basicInfo(HICQ,char *);
 static int  _System    fullInfo(HICQ,char *);
 static int  _System    url(HICQ,char *);
 static int  _System    ask(HICQ,char *);
 static int  _System    accept(HICQ,char *);
 static int  _System    reject(HICQ,char *);
 static int  _System    add(HICQ,char *);
 static int  _System    del(HICQ,char *);
 static int  _System    sendto(HICQ,char *);
 static int  _System    openURL(HICQ,char *);

#ifdef DEBUG
 static int  _System    RtfDebug(HICQ,char *);
 static int  _System    doDebug(HICQ,char *);
#endif

/*---[ Constantes ]--------------------------------------------------------------*/

 static const struct cmd cmdList[] = {  { "bye",          bye            },
                                        { "msg",          message        },
                                        { "ask",          ask            },
                                        { "accept",       accept         },
                                        { "reject",       reject         },
                                        { "remove",       del            },
                                        { "del",          del            },
                                        { "add",          add            },
                                        { "url",          url            },
                                        { "mode",         mode           },
                                        { "update",       basicInfo      },
                                        { "info",         fullInfo       },
                                        { "sendto",       sendto         },
                                        { "openurl",      openURL        },
#ifdef DEBUG
                                        { "rtfdebug",     RtfDebug       },
                                        { "debug",        doDebug        },
#endif
                                        { "unload",       unload         },
                                        { "load",         load           },

                                        { NULL,           NULL           }
                   };


/*---[ Implementacao ]-----------------------------------------------------------*/

 int EXPENTRY icqExecuteCmd(HICQ icq, char *lin)
 {
    const struct cmd    *c;
    char                *ptr;
    HPLUGIN             p;
    int                 rc      = -1;

    for(ptr=lin;*ptr && *ptr >= ' ';ptr++);
    *ptr = 0;

    for(ptr=lin;*ptr && !isspace(*ptr);ptr++);
    *(ptr++) = 0;

    for(c=cmdList;c->keyword && strcmp(lin,c->keyword);c++);

    if(c->keyword)
       return c->exec(icq,ptr);

    for(p=icqQueryFirstElement(icq->plugins);p && rc == -1;p=icqQueryNextElement(icq->plugins,p))
    {
       if(isProcValid(p,p->chkCommand))
          rc = p->chkCommand(icq,lin,ptr);
    }

    DBGTrace(rc);
    return rc;

 }

 static int bye(HICQ icq, char *parm)
 {
    icqAbend(icq,0);
    return 0;
 }

 static int _System message(HICQ icq, char *parm)
 {
    char        *ptr;
    ULONG       uin;
//    int         rc;

    for(ptr = parm;*ptr && !isspace(*ptr);ptr++);
    *(ptr++) = 0;

    while(*ptr && isspace(*ptr))
       ptr++;

    uin = atol(parm);

    if(!(uin && ptr))
       return -1;

    return icqSendMessage(icq, uin, MSG_NORMAL, ptr);
 }

 static int _System ask(HICQ icq, char *parm)
 {
    char        *ptr;
    ULONG       uin;
//    int         rc;

    for(ptr = parm;*ptr && !isspace(*ptr);ptr++);
    *(ptr++) = 0;

    while(*ptr && isspace(*ptr))
       ptr++;

    uin = atol(parm);

    if(!(uin && ptr))
       return -1;

    return icqSendMessage(icq, uin, MSG_REQUEST, ptr);
 }

 static int _System mode(HICQ icq, char *mode)
 {
    DBGMessage(icq->pwd);
    icqSetModeByName(icq,mode);
    return 0;
 }

 static int _System unload(HICQ icq, char *plugin)
 {
    HPLUGIN p   = icqQueryPlugin(icq,plugin);

    if(p)
    {
       icqUnloadPlugin(icq, p);
       return 0;
    }
    return -1;
 }

 static int _System load(HICQ icq, char *filename)
 {
    HPLUGIN     p;

    p = icqLoadPlugin(icq, "", filename);

    if(p)
    {
       icqStartPlugin(icq,p);
       return 0;
    }
    return -1;
 }

 static int _System basicInfo(HICQ icq, char *base)
 {
    ULONG uin = atol(base);

    if(!uin)
       return -1;

    icqRequestUserUpdate(icq, uin);
    return 0;
 }

 static int _System fullInfo(HICQ icq, char *base)
 {
    ULONG uin = atol(base);

    DBGTrace(uin);

    if(!uin)
       return -1;

    icqRequestUserInfo(icq, uin);

    return 0;
 }

 static int _System url(HICQ icq, char *txt)
 {
    ULONG uin;
    char        *url;
    char        *msg;
    char        *pkt;

    CHKPoint();

    uin = atol(txt);

    DBGTrace(uin);

    if(!uin)
       return -1;

    for(url = txt;*url && !isspace(*url);url++);

    if(!*url)
       return -2;

    url++;

    for(msg = url;*msg && !isspace(*msg);msg++);

    if(!*msg)
       return -3;

    *(msg++) = 0;

    DBGTrace(uin);
    DBGMessage(url);
    DBGMessage(msg);

    pkt = malloc(strlen(url)+strlen(msg)+5);

    if(!pkt)
       return -4;

    sprintf(pkt,"%s\x01%s",msg,url);

    DBGMessage(pkt);

    icqSendMessage(icq, uin, MSG_URL, pkt);

    free(pkt);

    return 0;
 }

 static int _System add(HICQ icq, char *parm)
 {
    char        *ptr;
    ULONG       uin;
//    int         rc;

    for(ptr = parm;*ptr && !isspace(*ptr);ptr++);
    *(ptr++) = 0;

    while(*ptr && isspace(*ptr))
       ptr++;

    uin = atol(parm);

    if(!(uin && ptr))
       return -1;

    return icqAddUserInContactList(icq, uin);

 }

 static int _System del(HICQ icq, char *parm)
 {
    char        *ptr;
    ULONG       uin;
//    int         rc;
    HUSER       usr;

    for(ptr = parm;*ptr && !isspace(*ptr);ptr++);
    *(ptr++) = 0;

    while(*ptr && isspace(*ptr))
       ptr++;

    uin = atol(parm);

    if(!(uin && ptr))
       return -1;

    usr = icqQueryUserHandle(icq,uin);

    if(!usr)
       return -2;

    return icqRemoveUser(icq, usr);

 }

 static int _System accept(HICQ icq, char *parm)
 {
    char        *ptr;
    ULONG       uin;

    for(ptr = parm;*ptr && !isspace(*ptr);ptr++);
    *(ptr++) = 0;

    while(*ptr && isspace(*ptr))
       ptr++;

    uin = atol(parm);

    if(!(uin && ptr))
       return -1;

    return icqSendMessage(icq, uin, MSG_AUTHORIZED, ptr);
 }

 static int _System reject(HICQ icq, char *parm)
 {
    char        *ptr;
    ULONG       uin;

    for(ptr = parm;*ptr && !isspace(*ptr);ptr++);
    *(ptr++) = 0;

    while(*ptr && isspace(*ptr))
       ptr++;

    uin = atol(parm);

    if(!(uin && ptr))
       return -1;

    return icqSendMessage(icq, uin, MSG_REFUSED, ptr);
 }

#ifdef DEBUG
 static int _System RtfDebug(HICQ icq, char *parm)
 {
    icqInsertRTFMessage(icq, 0, 0, MSG_NORMAL, 0, 0, parm);
    return 0;
 }

 static int _System doDebug(HICQ icq, char *parm)
 {
    icqInsertRTFMessage(icq, 0, 0, MSG_NORMAL, 0, 0, dbgRTF);
    return 0;
 }

#endif

 static int  _System sendto(HICQ icq,char *parm)
 {
    ULONG uin;
    char  *filename;
    char  *text;

    for(filename = parm;*filename && !isspace(*filename);filename++);

    if(*filename)
    {
       *(filename++) = 0;
       uin           = atol(parm);

       for(text = filename;*text && !isspace(*text);text++);

       if(*text)
       {
          *(text++) = 0;

          DBGTrace(uin);
          DBGMessage(filename);
          DBGMessage(text);
          return icqSendFile(icq, uin, filename, text);
       }
    }

    return -1;
 }

 static int  _System openURL(HICQ icq, char *url)
 {
	DBGMessage(url);
	return icqOpenURL(icq,url);
 }

