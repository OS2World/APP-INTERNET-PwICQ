/*
 *
 */

#ifdef __OS2__
   #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdio.h>
 #include <ctype.h>

 #include "icqpipe.h"

 #define GROUP_MASC  0x00010000

/*---[ Constants ]-------------------------------------------------------------------------------------------*/

 static const USHORT dlgItems[] = { 101, 103, 105, 106 };


/*---[ Prototipes ]------------------------------------------------------------------------------------------*/

 static int   _System load(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int   _System save(const DLGHELPER *,HWND,HICQ,ULONG,char *);

/*---[ Constants ]-------------------------------------------------------------------------------------------*/

 DECLARE_SIMPLE_DIALOG_MANAGER(pipeConfig,load,save);

/*---[ Constants ]-------------------------------------------------------------------------------------------*/

 void EXPENTRY icqpipe_ConfigPage(HICQ icq, struct icqpipe_config *cfg, ULONG uin, USHORT type, HWND hwnd, const DLGINSERT *dlg, char *buffer)
 {
    if(type == 1)
       dlg->loadPage(hwnd, module, 625, &pipeConfig, CFGWIN_ADVANCED);
 }

 static int _System load(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    int                 f;
    char                key[20];
    ULONG               masc    = GROUP_MASC;
    ULONG               flags   = icqLoadValue(icq, "icqPipe:Flags", 0x03);

    dlg->readString(hwnd,111,"icqPipe:Name","\\PIPE\\pwICQ");

    for(f=0;f<9;f++)
    {
       sprintf(key,"Group%02d",f);
       icqLoadString(icq,key,"",buffer,40);
       if(*buffer)
       {
          dlg->setString(hwnd,300+f,buffer);
          dlg->setCheckBox(hwnd,300+f,flags & masc);
       }
       dlg->setEnabled(hwnd,300+f,*buffer);
       masc <<= 1;
    }

    DBGTracex(flags);
    DBGTracex(sizeof(dlgItems));

    masc = 1;
    for(f = 0;f<sizeof(dlgItems);f++)
    {
       dlg->setCheckBox(hwnd,dlgItems[f],flags & masc);
       masc <<= 1;
    }

    return 0;
 }

 static int _System save(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    ULONG       flag = 0;
    int         masc = GROUP_MASC;
    int         f;

    dlg->writeString(hwnd,111,"icqPipe:Name");

    for(f=0;f<9;f++)
    {
       if(dlg->getCheckBox(hwnd,300+f))
          flag |= masc;
       masc <<= 1;
    }

    DBGTracex(flag);

    masc = 1;

    for(f = 0;f<sizeof(dlgItems);f++)
    {
       if(dlg->getCheckBox(hwnd,dlgItems[f]))
          flag |= masc;
       masc <<= 1;
    }

    DBGTracex(flag);

    icqSaveValue(icq,"icqPipe:Flags",flag);

    return 0;
 }


