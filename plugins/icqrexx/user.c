/*
 * user.c - expands user macros
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif


 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>

 #include "icqrexx.h"

/*---[ Implement ]---------------------------------------------------------------------------*/

 int EXPENTRY icqrexx_expandUserMacros(HICQ icq, HUSER usr, ULONG flags, char *cmdLine)
 {
    unsigned char *buffer = malloc(512);
    unsigned char *src    = cmdLine;
    unsigned char *dst    = buffer;
    unsigned char *ptr;

    if(!usr)
#ifdef EXTENDED_LOG
    {
       icqWriteSysLog(icq,PROJECT,"No user defined");
       return -1;
    }
#else
       return -1;
#endif

    if(flags && !(usr->groupMasc & flags))
    {
#ifdef EXTENDED_LOG
       icqWriteSysLog(icq,PROJECT,"Rejecting by group filter");
#endif
       DBGMessage("Nao pertence ao grupo");
       return 1;
    }

    while(*src)
    {
       if(*src != '%')
       {
          *(dst++) = *(src++);
       }
       else
       {
          src++;
          *dst = 0;

          DBGMessage(src);

          switch(*src)
          {
		  case 'u':		// %u  User's uin
			 sprintf(dst,"%ld",usr->uin);
			 break;
		
          case 'm':     // %m  Current user mode (numeric)
             sprintf(dst,"%ld",usr->mode);
             break;

          case 'd':     // %d  1 if user can accept direct connections, 0 if not, 2 if undeterminated
             strcpy(dst,"2");
             break;

          case 'i':     // %i  IP address to connect with the user
             icqIP2String(icqQueryUserIP(icq,usr),dst);
             break;

          case '$':     // %$ User's nick (no spaces)
             strcpy(dst,icqQueryUserNick(usr));
             for(ptr=dst;*ptr;ptr++)
             {
                if(*ptr <= ' ' || *ptr >= 0x80)
                   *ptr = '_';
             }
             break;

          case 'n':     // %n  User's nick
             strcpy(dst,icqQueryUserNick(usr));
             break;

          case 'f':     // %f  User's first name
             strcpy(dst,icqQueryUserFirstName(usr));
             break;

          case 'l':     // %l  User's last name
             strcpy(dst,icqQueryUserLastName(usr));
             break;

          case 'a':     // %a  User's E-Mail addr (don't ask me what the meaning of the 'a')
             strcpy(dst,icqQueryUserLastName(usr));
             break;

          case 'g':     // %g  User's group masc
             sprintf(dst,"%8lx",usr->groupMasc);
             break;

          default:
             *(dst++) = '%';
             *(dst++) = *src;
             *dst = 0;
          }
          while(*dst)
             dst++;
          src++;
       }

    }
    *dst = 0;

    strncpy(cmdLine,buffer,0xFF);
    free(buffer);
    return 0;
 }


