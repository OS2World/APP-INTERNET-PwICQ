#ifdef __IBMC__
 #pragma strings(readonly)
#endif

 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>
 #include <time.h>
 #include <ctype.h>

 #include <icqkernel.h>
 #include "icqcore.h"

/*---[ Definicoes ]------------------------------------------------------------------------------------------*/

 #pragma pack(1)

 struct cmd
 {
 	const char *key;
 	char * ( * _System exec)(HICQ,char *,char **);
 };

 struct xlat
 {
    int        id;
    const char *description;
    const char *emoticon;
    const char *image;
 };


/*---[ Prototipos ]------------------------------------------------------------------------------------------*/

 static const char *stripRTF(HICQ,int,const char *, char **);
 static void       stripCodePage(char *);

 static const struct xlat emoticons[] =
 {
     { 0x00, "Smile",			":-)",		"<##icqimage0000>" },
     { 0x08, "Wink",			";-)",		"<##icqimage0008>" },
     { 0x04, "Sad",				":-(",		"<##icqimage0004>" },
     { 0x0c, "Pratty",			":-P",		"<##icqimage000c>" },
     { 0x01, "Surprised",		"=-O",		"<##icqimage0001>" },
     { 0x05, "Kiss",			":-*",		"<##icqimage0005>" },
     { 0xFF, "Angry",			">:o",		NULL               },
     { 0x0F, "Big Grin",		":-D",		"<##icqimage000f>" },
     { 0xFF, "Money",			":-$",		NULL               },
     { 0xFF,  "Smokin'",		":-!",		NULL               }, 	
     { 0xFF, "Blush",			":-[",		NULL               },
     { 0x0E, "Angel",			"O:-)",		"<##icqimage000e>" },
     { 0xFF, "Skeptical", 		":-\\",		NULL               },
     { 0x07, "Cry",				":'(",		"<##icqimage0007>" },
     { 0x0B, "Lips are Sealed",	":-X",		"<##icqimage000b>" },
     { 0x0D, "Cool dude",		"8-)",		"<##icqimage000d>" }
 };

/*---[ Constantes ]------------------------------------------------------------------------------------------*/

/*

 happy face:       <##icqimage0000> :-)
 surprised:        <##icqimage0001> :-O
 serious:          <##icqimage0002> :-|
 sem jeito:        <##icqimage0003> :-/
 sad face:         <##icqimage0004> :-(
 kiss:             <##icqimage0005> :-{}
 olhando de lado:  <##icqimage0006> :*)
 chorando:         <##icqimage0007> :'-(
 winking face:     <##icqimage0008> ;-)
 complaining:      <##icqimage0009> :-@
 envergonhado:     <##icqimage000a> :-<
 calado(band-aid): <##icqimage000b> :-X
 bleh!:            <##icqimage000c> :-P
 sunglasses:       <##icqimage000d> 8-)
 anjinho:          <##icqimage000e> O-)
 rindo (muito):    <##icqimage000f> :-D

 Pelo ICQ do Palm

 Smile				:-)
 Wink				;-)
 Sad				:-(
 Pratty				:-P
 Surprised			=-O
 Kiss				:-*
 Angry				>:o
 Big Grin			:-D
 Money				:-$
 Smokin'			:-! 	
 Blush				:-[
 Angel				O:-)
 Skeptical  		:-\
 Cry				:'(
 Lips are Sealed	:-X
 Cool dude			8-)

*/

/*

dbgrtf \f0\fswiss MS Sans Serif; teste
dbgrtf {\rtf1\ansi\def0{\fonttbl{\f0\fswiss MS Sans Serif;}}{\colortbl ;\red0\green0\blue0;}\viewkind4\pard\cf1\f0 http://lan.serverrummet.dk/~kaos/palm/books/LOTR/}
dbgrtf {\rtf1\ansi\ansicpg1252\deff0\deflang1046{\fonttbl{\f0\fnil\fcharset0 Times New Roman;}}{\colortbl ;\red0\green0\blue0;}\viewkind4\uc1\pard\cf1\f0\fs20 Ta ai ?\par}
dbgrtf {\rtf1\ansi\def0{\fonttbl{\f0\fswiss MS Sans Serif;}{\f1\fswiss Sans;}}{\colortbl ;\red45\green45\blue45;}\viewkind4\pard\cf1\f0 \f1\highlight0\pard Achso - hier mal Umlaute\u246?\u228?\u223? und ein Euro \u8364?\par}

*/

 static char * _System stripDigits(HICQ,char *, char **);
 static char * _System stripText(HICQ,char *, char **);
 static char * _System stripChars(HICQ,char *, char **);
 static char * _System stripFont(HICQ,char *, char **);
 static char * _System hexChar(HICQ,char *, char **);
 static char * _System newLine(HICQ,char *, char **);
 static char * _System stripKey(HICQ,char *, char **);
 static char * _System addSpace(HICQ,char *, char **);
 static char * _System unicodeChar(HICQ,char *, char **);

 static const struct cmd cmdList[] =
 {
	{ "\\fnil",	     stripText	    },	
	{ "\\fname",	 stripFont	    },	
	{ "\\fcharset",  stripText	    },
	{ "\\fscript",   stripDigits	},
	{ "\\fonttbl",	 stripText	    },
	{ "\\colortbl",  stripText	    },
	{ "\\pan",       stripKey 	    },
	{ "\\red",	     stripText	    },
	{ "\\green",	 stripText	    },
	{ "\\blue",	     stripText	    },
	{ "\\rtf",	     stripDigits	},
	{ "\\ansicpg",	 stripDigits	},
	{ "\\deflangfe", stripDigits	},
	{ "\\deflang",	 stripDigits	},
	{ "\\lang",	     stripDigits	},
	{ "\\deff",	     stripDigits	},
	{ "\\def",	     stripDigits	},
	{ "\\sb",	     stripDigits	},
	{ "\\line",	     newLine       	},
	{ "\\sa",	     stripDigits	},
	{ "\\pard",      stripKey	    },
	{ "\\par",	     addSpace    	},
	{ "\\viewkind",	 stripDigits	},
	{ "\\uc",	     stripDigits	},
	{ "\\cf",        stripDigits	},
	{ "\\ansi",      stripText	    },
	{ "\\fswiss",    stripText	    },
	{ "\\fs",        stripDigits	},
	{ "\\b",	     stripDigits	},
	{ "\\f",         stripChars	    },
	{ "\\*",         stripChars	    },
	{ "\\ul",        stripChars     },
	{ "\\u",         unicodeChar    },
	{ "\\\'",        hexChar        },
    { "\\highlight", stripDigits    },

 	{ NULL,		     NULL           }
 };

/*---[ Implementacao ]---------------------------------------------------------------------------------------*/

 BOOL EXPENTRY icqInsertRTFMessage(HICQ icq, ULONG uin, ULONG sequenceNumber, USHORT type, time_t tm, ULONG flags, const char *txt)
 {
//    char                *buffer;
    int                 sz;
//    char		*out;
    char		*filtered	= NULL;
    char		*text;

    if(txt)
       sz = strlen(txt);
    else
       sz = 0;

    if(!sz || (icq->modeCaps & ICQMF_RTF))
       return icqInsertMessage(icq, uin, sequenceNumber, type, tm, flags, sz, txt);

    /* Retira códigos de formatacao de caracteres */
    text = strdup(txt);

    if(!text)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when duplicating RTF message");
       return icqInsertMessage(icq, uin, sequenceNumber, type, tm, flags, sz, txt);
    }

//    stripCodePage(text);

    /* Retirar a formatacao RTF da mensagem */

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,text);
#endif

    filtered = malloc(strlen(text)+10);
    if(!filtered)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when processing RTF message");
       return icqInsertMessage(icq, uin, sequenceNumber, type, tm, flags, sz, text);
    }

    *filtered = 0;

//    DBGMessage("***************************************************************************");
    stripRTF(icq,0,text,&filtered);
//    DBGMessage("***************************************************************************");

    DBGMessage(filtered);
    icqInsertMessage(icq, uin, sequenceNumber, type, tm, flags, strlen(filtered), filtered);

    free(filtered);
    free(text);

    return -1;

 }

 static char hexDigit(char *ptr)
 {
    if(*ptr >= '0' && *ptr <= '9')
       return *ptr - '0';

    if(*ptr >= 'a' && *ptr <= 'f')
       return (*ptr - 'a') + 10;

    if(*ptr >= 'A' && *ptr <= 'F')
       return (*ptr - 'A')+10;

    return 0;
 }

 static void stripCodePage(char *src)
 {
    char *dst = src;

    while(*src)
    {
       if(*src == '\\' && *(src+1) == '\'')
       {
          *(dst++) = (hexDigit(src+2) << 4)|hexDigit(src+3);
          src += 4;
       }
       else if(*src == '\\' && *(src+1) == '\\')
       {
          *(dst++) = '\\';
          src += 2;
       }
       else
       {
          *(dst++) = *(src++);
       }

    }

    *dst = 0;
    DBGMessage(src);
 }

 static const char *stripRTF(HICQ icq, int level, const char *txt, char **outTxt)
 {
    const char 		*in	= txt;
    char		*out;
    char		*ptr;
    char 		*buffer = malloc(strlen(txt)+5);
    const struct cmd	*cmd;
    int			f;
//    int                 szKey;

    if(!buffer)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error processing RTF message");
       return "";
    }

    out = buffer;
    while(*in && *in != '}')
    {
       if(*in == '{')
          in = stripRTF(icq,level+1,in+1,outTxt);
       else
          *(out++) = *(in++);
    }
    *out = 0;

//    DBGMessage(buffer);

    /* Interpreta comandos no bloco descompactado */

    ptr = buffer;
    while(*ptr)
    {
       if(*ptr == '\\')
       {
          for(cmd = cmdList;cmd->key && memcmp(cmd->key,ptr,strlen(cmd->key));cmd++);
          if(cmd->key)
          {
             ptr   = cmd->exec(icq,ptr+strlen(cmd->key),outTxt);
          }
          else
          {
             icqWriteSysLog(icq,PROJECT,"Unexpected RTF command");
             icqWriteSysLog(icq,PROJECT,ptr);
             ptr++;
             while(*ptr && *ptr != '\\')
                ptr++;
          }
       }
       else
       {
//          DBGMessage(ptr);
          for(f=0;*(ptr+f) && *(ptr+f) != '\\';f++);
//          DBGTrace(f);

          strncat(*outTxt,ptr,f);
//          DBGMessage(*outTxt);
          ptr += f;

       }
    }


    free(buffer);

    if(*in)
       return in+1;

    return in;
 }

 static char * _System stripDigits(HICQ icq, char *in, char **out)
 {
    while(*in)
    {
       if(!isdigit(*in))
          return in;
       in++;
    }
    return in;
 }

 static char * _System stripText(HICQ icq, char *in, char **out)
 {
    while(*in)
    {
       if(*in == ';')
          return in+1;
       else if(*in == '\\')
          return in;
       else if(*in < ' ')
          return in;
       in++;
    }
    return in;
 }

 static char * _System stripChars(HICQ icq, char *in, char **out)
 {
    while(*in)
    {
       if(*in == ';')
          return in+1;
       else if(*in == '\\')
          return in;
       else if(*in < ' ')
          return in;
       else if(*in == ' ')
          return in+1;
       in++;
    }
    return in;
 }

 static char * _System stripFont(HICQ icq, char *in, char **out)
 {
    while(*in)
    {
       if(*in == ';')
          return in+1;
       in++;
    }
    return in;
 }

 static char * _System newLine(HICQ icq, char *in, char **out)
 {
    strcat(*out,"\n");
    return in;
 }

 static char * _System hexChar(HICQ icq, char *in, char **out)
 {
    char key[3] = { *in, *(in+1), 0 };
    char str[2] = { '?', 0 };
    int  v      = 0;

    sscanf(key,"%02x",&v);

    if(v)
       str[0] = v;

    strcat(*out,str);
    return in+2;
 }


 static char * _System stripKey(HICQ icq, char *in, char **out)
 {
    return in;
 }

 static char * _System addSpace(HICQ icq, char *in, char **out)
 {
    strcat(*out," ");
    return in;
 }

 static char * _System unicodeChar(HICQ icq, char *in, char **out)
 {
    int  v      = 0;
    char str[2] = { '*', 0 };

    while(*in && isdigit(*in))
    {
       v *= 10;
       v += (*in - '0');
       in++;
    }

    if(*in == '?')
       in++;

    if(v)
       str[0] = v;

    strcat(*out,str);

    return in;
 }


