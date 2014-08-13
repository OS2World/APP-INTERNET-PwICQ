/*
 * tools.c - pwICQ's Skin manager tools
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>
 #include <pwicqgui.h>


/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 void icqskin_loadUsersList(hWindow hwnd, HICQ icq)
 {
    char     key[20];
    int      f                 = 0;
    char     *ptr;
    char     buffer[0x0100];
    HUSER    usr;

    /* Add group names */
	
	CHKPoint();
	if(icqLoadValue(icq,"ShowAllUserGroups",0))
	{
	   *buffer = 1;
       for(f=1;*buffer;f++)
       {
          sprintf(key,"UserGroup%02d",f);
          DBGMessage(key);
          icqLoadString(icq, key, "", buffer, 0xFF);
          DBGMessage(buffer);

          if(*buffer)
          {
             ptr = strstr(buffer, ",");
             if(ptr)
             {
                *(ptr++) = 0;
                icqskin_insertUserGroup(hwnd,atoi(buffer),ptr);
             }
          }
       }
       icqskin_insertUserGroup(hwnd,0x0F00,"Not in list");
	}

    /* Add users */

	CHKPoint();
    icqWalkUsers(icq,usr)
    {
       if(!(usr->flags&USRF_HIDEONLIST))
          icqskin_addUser(hwnd,usr);
    }

	CHKPoint();
    icqskin_sortUserList(hwnd);
	CHKPoint();

 }


