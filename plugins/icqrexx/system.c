/*
 * System.c - expands system macros
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

 int EXPENTRY icqrexx_expandSystemMacros(HICQ icq, USHORT event, ULONG uin, char *cmdLine)
 {
    char *buffer = malloc(512);
    char *src	 = cmdLine;
    char *dst	 = buffer;

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
	  case 'M':	// %M  Your Current mode
	     sprintf(dst,"%lu",icqQueryOnlineMode(icq));
	     break;
	
	  case 'I':	// %I  IP address confirmed by ICQ server
             icqIP2String(icqQueryLocalIP(icq),dst);
	     break;
	
	  case 'E':	// %E  Event code (same as '%e')
	  case 'e':
	     sprintf(dst,"%d",event);
	     break;
	
	  case 'U':	// %U  Your UIN
	     sprintf(dst,"%lu",icqQueryUIN(icq));
	     break;
	
	  case 'N':	// %N  Your nickname
	     icqLoadString(icq, "NickName", "John Doe", dst, 79);
	     break;
	
	  case 'F':	// %F  Your first name
	     icqLoadString(icq, "FirstName", "John", dst, 79);
	     break;
	
	  case 'L':	// %L  Your last name
	     icqLoadString(icq, "LastName", "Doe", dst, 79);
	     break;
	
          case 'A':	// %A  Your e-mail
	     icqLoadString(icq, "E-Mail", "", dst, 79);
	     break;

          case 'u':	// %u  User's UIN
	     sprintf(dst,"%lu",uin);
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


