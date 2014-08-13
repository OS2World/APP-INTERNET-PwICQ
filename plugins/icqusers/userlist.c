/*
 * userlist.c - Management of ICQ user list
 */

#ifdef __OS2__
   #pragma strings(readonly)
   #define INCL_DOSPROCESS
   #define INCL_WINSHELLDATA
#endif

 #include <ctype.h>
 #include <string.h>
 #include <stdlib.h>
 #include <stdio.h>

#ifdef linux
  #include <unistd.h>
#else
  #include <io.h>
#endif

 #include <icqtlkt.h>

 #include "userdb.h"

/*---[ Defines ]-------------------------------------------------------------------*/

 #define MAX_LINE_SIZE  1024

 struct type
 {
    char        code;
    ULONG       flag;
 };

/*---[ Constant ]------------------------------------------------------------------*/

 const static struct type userFlags[] =
                        {
                                { 'A',    USRF_AUTHORIZE   },
                                { 'a',    USRF_AUTOOPEN    },
                                { 'E',    USRF_EXTENDED    },
                                { 'i',    USRF_IGNORE      },
                                { 'I',    USRF_INVISIBLE   },
                                { 'R',    USRF_REFRESH     },
                                { 'T',    USRF_TEMPORARY   },
                                { 'V',    USRF_VISIBLE     },
                                { 'w',    USRF_WAITING     },
                                { 'X',    USRF_REJECTED    },
                                { 'h',    USRF_HISTORY     },
                                { 'C',    USRF_CACHED      },
                                { 'd',    USRF_FILEDIR     },
                                { 'F',    USRF_AUTOACCEPT  },
                                { 'L',    USRF_ONLIST      },
                                { 'f',    0 }, // USR_AUTORESUME,
                                { 'H',    0 }, // USR_HIDE,
                                { 'l',    0 }, // USR_LOGMSG,
                                { 'm',    0 }, // USR_NOAWAY,
                                { 'n',    0 }, // USR_NOINFO,
                                { 'o',    0 }, // USR_NOSNDAWAY,
                                { 'r',    0 }, // USR_REFUSED,
                                { 'S',    0 }, // USR_REXX,
                                { 'W',    0 }, // USR_WINPOS,

                                {  0,     0 }
                        };


/*---[ Prototipes ]--------------------------------------------------------------------------------------*/

 static char  *getFilename(HICQ, char *, const char *ext);
 static char  *queryString(char **);
 static ULONG queryValue(char **);

/*---[ Implementation ]----------------------------------------------------------------------------------*/

 void icqusrdb_Load(HICQ icq)
 {
    FILE                *arq;
    char                buffer[0x0100];
    char                *ptr;
    ULONG               uin;
    char                *nickname;
    char                *firstname;
    char                *lastname;
    char                *sound;
    char                *email;
    char                *flags;
    ULONG               UFlag;
    ULONG               gMasc;
    ULONG               lastOnline;
    ULONG               lastMsg;
    ULONG               memberSince;
    ULONG               lastAction;
    HUSER               usr;
    UCHAR               age;
    UCHAR               sex;
    int                 qtd             = 0;
    BOOL                oldFormat       = TRUE;
    USHORT              c2sGroup        = 0;
    USHORT              c2sIDNumber     = 0;
    const struct type   *types;
#ifdef __OS2__
    ULONG               fileCpg         = icqLoadValue(icq,"cnvtUsersFile", 0) ? icqQueryRemoteCodePage(icq) : 0;
#endif
    sprintf(buffer,"Loading contact-list for ICQ#%lu",icqQueryUIN(icq));
    icqWriteSysLog(icq,PROJECT,buffer);

#ifdef __OS2__

    getFilename(icq,buffer,"dat");
    DBGMessage(buffer);

    if(access(buffer,0))
    {
       uin     = 0x80;
       if(PrfQueryProfileData(HINI_USERPROFILE, "pwICQ", "ProgramPath", buffer, &uin))
          *(buffer+uin) = 0;
       else
          *buffer = 0;

       sprintf(buffer+strlen(buffer),"%lu.dat",icqQueryUIN(icq));
       DBGMessage(buffer);
       fileCpg = 0;
    }

    arq = fopen(buffer,"r");

#else

        DBGMessage(getFilename(icq,buffer,"dat"));
    arq = fopen(getFilename(icq,buffer,"dat"),"r");

#endif

    if(!arq)
    {
       sprintf(buffer,"Can't open contact-list file");
       icqWriteSysLog(icq,PROJECT,buffer);
       return;
    }

    while(!feof(arq))
    {
       *buffer = 0;
       fgets(buffer,0xFF,arq);

#ifdef __OS2__
       icqTranslateCodePage(icq,fileCpg,icqQueryLocalCodePage(icq),buffer,-1);
#endif

       switch(*buffer)
       {
       case 0:
       case ';':
          break;

       case '$':
          oldFormat = FALSE;
#ifdef __OS2__
          if(!strncmp(buffer+1,"OS2",3) && atoi(buffer+5) == icqQueryCountryCode(icq))
             fileCpg = atoi(buffer+9);
#endif
          break;

       default:
          if(isdigit(*buffer))
          {
             for(ptr=buffer;*ptr && *ptr != ',';ptr++);

             if(*ptr == ',')
             {
                *(ptr++)    = 0;
                uin         = atol(buffer);
                nickname    = queryString(&ptr);
                email       = queryString(&ptr);
                flags       = queryString(&ptr);
                gMasc       = queryValue(&ptr);
                firstname   = queryString(&ptr);
                lastname    = queryString(&ptr);
                sound       = queryString(&ptr);
                lastOnline  = queryValue(&ptr);
                lastMsg     = queryValue(&ptr);
                memberSince = queryValue(&ptr);
                lastAction  = queryValue(&ptr);
                sex         = queryValue(&ptr);
                age         = queryValue(&ptr);
                c2sGroup    = queryValue(&ptr);
                c2sIDNumber = queryValue(&ptr);

                for(UFlag = 0;*flags;flags++)
                {
                   for(types = userFlags;types->code && types->code != *flags;types++);
                   if(types->code == *flags)
                      UFlag |= types->flag;
                }

                if(UFlag & USRF_TEMPORARY)
                   UFlag |= USRF_HIDEONLIST;
                else if( !(UFlag & USRF_REJECTED) )
                   UFlag |= USRF_ONLIST;

                usr = icqInsertUser(icq, uin, nickname, firstname, lastname, email, sound, UFlag);
                if(usr)
                {
                   usr->type          = USRT_ICQ;
                   usr->idle          = 10;
                   usr->groupMasc     = gMasc;
                   usr->lastOnline    = lastOnline;
                   usr->memberSince   = memberSince;
                   usr->lastMessage   = lastMsg;
                   usr->lastAction    = lastAction;
                   usr->age           = age;
                   usr->sex           = sex;
                   usr->c2sGroup      = c2sGroup;
                   usr->c2sIDNumber   = c2sIDNumber;
                   icqAjustUserIcon(icq, usr, TRUE);
                }
                qtd++;
             }
          }
       }
    }

    fclose(arq);

    if(oldFormat)
    {
       sprintf(buffer,"%d users in converted list",qtd);
       icqSystemEvent(icq, ICQEVENT_UPDATED);
    }
    else
    {
       sprintf(buffer,"%d users in list",qtd);
    }
    icqWriteSysLog(icq,PROJECT,buffer);

 }

 static char * getFilename(HICQ icq, char *buffer, const char *ext)
 {
    char key[40];
    sprintf(key,"%lu.%s",icqQueryUIN(icq),ext);
    icqQueryPath(icq,key,buffer,0xFF);
    return buffer;
 }

 static char *queryString(char **str)
 {
    char *ret = *str;
    char *ptr;

    while(*ret && *ret != '\"')
       ret++;

    if(!*ret)
    {
       *str = ret;
       return "";
    }

    ret++;

    for(ptr = ret;*ptr && *ptr != '\"';ptr++);

    if(!*ptr)
    {
       *str = ptr;
       return ret;
    }
    *(ptr++) = 0;

    *str = ptr;
    return ret;
 }

 static ULONG queryValue(char **str)
 {
    char        *ptr   = *str;
    ULONG       ret     = 0;

    while(*ptr && (isspace(*ptr) || *ptr == ','))
       ptr++;

    if(!*ptr)
    {
       *str = ptr;
       return 0;
    }

    if(*ptr == '\"')
       return atol(queryString(str));

    while(*ptr && isdigit(*ptr))
    {
       ret *= 10;
       ret += (*ptr & 0x0F);
       ptr++;
    }
    *str = ptr;

    return ret;
 }

 void icqusrdb_Save(HICQ icq, BOOL mkBackup)
 {
    char                buffer[0x0100];
    char                temp[0x0100];
    char		tmpExt[4];
    char                key[20];
    HUSER               usr;
    const struct type   *types;
    FILE                *arq;
    char                *ptr;
    int			f;
    int			qtdUsers	= 0;
    time_t              tm;
    char                *lineBuffer;
#ifdef __OS2__
    BOOL                convert         = icqLoadValue(icq,"cnvtUsersFile", 0);
#endif

    sysLog(icq,"Saving contact-list");


    if(!icqQueryFirstUser(icq))
    {
       sysLog(icq,"No users in contact-list");
       return;
    }

    time(&tm);

    lineBuffer = malloc(MAX_LINE_SIZE);

    if(!lineBuffer)
    {
       sysLog(icq,"Memory allocation error in contact-list save procedure");
       return;
    }

    sprintf(tmpExt,"%03x",rand() & 0x0FFF);
    getFilename(icq, buffer, tmpExt);

    for(f=0;f<100 && access(buffer,0);f++)
    {
       sprintf(tmpExt,"%03x",rand() & 0x0FFF);
       getFilename(icq, buffer, tmpExt);
    }

    DBGMessage(buffer);

    arq = fopen(buffer,"w");
    if(!arq)
    {
       sysLog(icq,"Unable to write user information file");
       free(lineBuffer);
       return;
    }

    fprintf(arq,   ";This is an automatically generated file.\n"
                   ";It will be read and overwritten.\n"
                   ";Do Not Edit!\n"
                   ";Created by pwICQ"
#ifdef __OS2__
                   " for OS/2"
#endif
#ifdef linux
                   " for linux"
#endif
                   " version " PWICQVERSION " Build " BUILD "\n"
                );

#ifdef __OS2__
    fprintf(arq,        "$%s %03u %04u %08lx\n",
                        "OS2",
                        icqQueryCountryCode(icq),
                        convert ? icqQueryRemoteCodePage(icq) : icqQueryLocalCodePage(icq),
                        icqLoadValue(icq, "SSUL.Update", 0) );
#endif

#ifdef linux
    fprintf(arq,        "$%s %03u %04u %08lx\n",
                        "LNX",
                        0,
                        0,
                        icqLoadValue(icq, "SSUL.Update", 0) );
#endif


    for(usr = icqQueryFirstUser(icq);usr;usr = icqQueryNextUser(icq,usr))
    {

       if(!(usr->flags&USRF_DELETED))
       {
          *(ptr=key) = 0;
          for(types = userFlags;types->code;types++)
          {
             if(usr->flags & types->flag)
                *(ptr++) = types->code;
          }
          *ptr = 0;

          qtdUsers++;

#ifdef __OS2__
          sprintf(lineBuffer,
                        "%lu,\"%s\",\"%s\",\"%s\",%lu,\"%s\",\"%s\",\"%s\",",
                        usr->uin,
                        icqQueryUserNick(usr),
                        icqQueryUserEmail(usr),
                        key,
                        usr->groupMasc,
                        icqQueryUserFirstName(usr),
                        icqQueryUserLastName(usr),
                        ""
          );

          if(convert)
             icqConvertCodePage(icq, FALSE, lineBuffer, -1);

          fprintf(arq,lineBuffer);
#else
          fprintf(arq,"%lu,",usr->uin);
          fprintf(arq,"\"%s\",",icqQueryUserNick(usr));
          fprintf(arq,"\"%s\",",icqQueryUserEmail(usr));
          fprintf(arq,"\"%s\",",key);
          fprintf(arq,"%lu,",usr->groupMasc);
          fprintf(arq,"\"%s\",",icqQueryUserFirstName(usr));
          fprintf(arq,"\"%s\",",icqQueryUserLastName(usr));
          fprintf(arq,"\"%s\",","");
#endif

          fprintf(arq,"%lu,",usr->lastOnline);
          fprintf(arq,"%lu,",usr->lastMessage);
          fprintf(arq,"%lu,",usr->memberSince);
          fprintf(arq,"%lu,",usr->lastAction);
          fprintf(arq,"%d,",usr->sex);
          fprintf(arq,"%d,",usr->age);
          fprintf(arq,"%d,",usr->c2sGroup);
          fprintf(arq,"%d",usr->c2sIDNumber);

          fprintf(arq,"\n");

       }
    }

    strcpy(buffer,ctime(&tm));

    for(ptr = buffer;*ptr && *ptr >= ' ';ptr++);
    *ptr = 0;

    fprintf(arq,   ";%s\n", buffer);

/*
#ifdef __OS2__
    fprintf(arq,   ";@#PerryWerneck: " PWICQVERSION " #@##1##%s %s:"ASDFEATUREID":%u:%u:0::" LEVEL_REQUIRED "@@pwICQ contact-list file\n",
                                buffer, getenv("HOSTNAME"), convert ? icqQueryRemoteCodePage(icq) : icqQueryLocalCodePage(icq), icqQueryCountryCode() );
#endif
*/

    fclose(arq);

    if(mkBackup)
    {
       remove(getFilename(icq, buffer, "bak"));
       rename(getFilename(icq, buffer, "dat"), getFilename(icq,temp,"bak"));
    }
    else
    {
       remove(getFilename(icq, buffer, "dat"));
    }
    rename(getFilename(icq, buffer, tmpExt), getFilename(icq,temp,"dat"));

    sprintf(lineBuffer,"%d record(s) in user database",qtdUsers);
    sysLog(icq,lineBuffer);
    free(lineBuffer);

    CHKPoint();
 }

