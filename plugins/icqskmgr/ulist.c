/*
 * ULIST. C - User Listbox management
 */

 #define INCL_WIN
 #include <string.h>
 #include <stdio.h>

 #include "icqskmgr.h"

/*---[ Prototipos ]---------------------------------------------------------------------------------*/

 static int _System lbSort(HUSER, HUSER);

/*---[ Implementacao ]------------------------------------------------------------------------------*/

 void updateUserList(HWND hwnd, ICQWNDINFO *ctl)
 {
    HWND        h       = WinWindowFromID(hwnd,1003);
    HUSER       usr;

    ctl->flags &= ~(WFLG_REDRAW|WFLG_BLINK);

    DBGMessage("Atualizando lista de usuarios");

    WinEnableWindowUpdate(h, FALSE);

    sknClearListBox(hwnd,1003);

/*
    for(f=0;f<icqQueryUserPosLimit(ctl->icq);f++)
    {
       for(usr = icqQueryFirstUser(ctl->icq); usr; usr = icqQueryNextUser(ctl->icq,usr))
       {
          if(usr->index == f && !(usr->flags&USRF_HIDEONLIST) )
             sknInsertItemHandle(hwnd,1003,usr);
       }
    }
*/

    for(usr = icqQueryFirstUser(ctl->icq); usr; usr = icqQueryNextUser(ctl->icq,usr))
    {
       if(!(usr->flags&USRF_HIDEONLIST) )
          sknInsertItemHandle(hwnd,1003,usr);
    }

    sknSetSortCallBack(hwnd,1003,lbSort);

    checkUserList(hwnd,ctl);

    WinEnableWindowUpdate(h, TRUE);
    WinShowWindow(h, TRUE);

#ifdef DEBUG

    usr = icqQueryFirstUser(ctl->icq);
    DBGTracex(usr);
    DBGTracex(sknQueryHandler(hwnd,1003,usr));

    usr = icqQueryNextUser(ctl->icq,usr);
    DBGTracex(usr);
    DBGTracex(sknQueryHandler(hwnd,1003,usr));

#endif


 }

 void checkUserList(HWND hwnd, ICQWNDINFO *ctl)
 {
    HUSER       usr;

    for(        usr = icqQueryFirstUser(ctl->icq);
                usr && (usr->eventIcon == usr->modeIcon);
                usr = icqQueryNextUser(ctl->icq,usr) );
    if(usr)
       ctl->flags |= WFLG_BLINK;
    else
       sknBlinkControl(hwnd,1003,0);
 }

 static int _System lbSort(HUSER h1, HUSER h2)
 {
    if(h1->index != h2->index)
       return h1->index - h2->index;
    return stricmp(icqQueryUserNick(h1),icqQueryUserNick(h2));
 }

