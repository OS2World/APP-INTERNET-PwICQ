/*
 * Group dialogs
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdlib.h>
 #include <string.h>
 #include <stdio.h>

 #include "icqconf.h"

/*---[ Configuration ]---------------------------------------------------------------------------------------*/

 #pragma pack(1)

 struct relations
 {
    USHORT      id;
    ULONG       flag;
 };

 #pragma pack()

/*---[ Prototipes ]------------------------------------------------------------------------------------------*/

 static int   _System load(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int   _System save(const DLGHELPER *,HWND,HICQ,ULONG,  char *);

/*---[ Constants ]-------------------------------------------------------------------------------------------*/


 static const struct relations tbl[] = { { 300, USRF_HISTORY    },      // Enable History
                                         { 103, 0               },      // Preserve Window Positions
                                         { 303, USRF_IGNORE     },      // Ignore user
                                         { 304, 0               },      // Don't send away
                                         { 305, USRF_AUTOOPEN   },      // Auto-open
                                         { 306, 0               },      // Don't show away
                                         { 308, USRF_INVISIBLE  },      // Always Invisible
                                         { 309, USRF_VISIBLE    },      // Always Visible
                                         { 104, USRF_FILEDIR    },      // Received files in user folder
                                         { 105, USRF_AUTOACCEPT },      // Auto accept files from this user
                                         {   0, 0               }
                                       };



 DECLARE_SIMPLE_DIALOG_MANAGER(miscOptions,load,save);


/*---[ Implementing ]----------------------------------------------------------------------------------------*/

 static int load(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    const struct relations  *ptr;
    HUSER                   usr     = icqQueryUserHandle(icq,uin);
    BOOL                    hasFile = ((icqGetCaps(icq)&ICQMF_FILE)!= 0);

    DBGTracex(icqGetCaps(icq)&ICQMF_FILE);
    DBGTracex(hasFile);

    if(!usr)
    {
       for(ptr=tbl;ptr->id;ptr++)
          dlg->setEnabled(hwnd,ptr->id,FALSE);
    }
    else
    {
       if(!(usr->flags & (USRF_INVISIBLE|USRF_VISIBLE)))
          dlg->setCheckBox(hwnd,307,TRUE);

       DBGTracex(usr->flags);

       for(ptr=tbl;ptr->id;ptr++)
       {
          dlg->setEnabled(hwnd,ptr->id,ptr->flag);

          DBGTracex(usr->flags & ptr->flag);
          dlg->setCheckBox(hwnd,ptr->id,usr->flags & ptr->flag);
       }

       dlg->setEnabled(hwnd,300,(icqGetCaps(icq)&ICQMF_HISTORY)!= 0);
    }

    dlg->setEnabled(hwnd,104,hasFile);
    dlg->setEnabled(hwnd,105,hasFile);

    return 0;
 }

 static int _System save(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin,  char *buffer)
 {
    const struct relations      *ptr;
    ULONG                       masc;
    ULONG                       flag;
    HUSER                       usr             = icqQueryUserHandle(icq,uin);

    if(!usr)
       return 0;

    masc = flag = 0;

    for(ptr=tbl;ptr->id;ptr++)
    {
       masc |= ptr->flag;
       if(dlg->getCheckBox(hwnd,ptr->id))
          flag |= ptr->flag;
    }

    flag |= usr->flags & ~masc;

    DBGTracex(flag);


    DBGTracex(flag);

    if(usr->flags != flag)
    {
       CHKPoint();
       usr->flags = flag;
       icqUserEvent(icq, usr, ICQEVENT_MODIFIED);
    }

    return 0;
 }


