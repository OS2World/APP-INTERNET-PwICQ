/*
 * config.c - Configuration dialog box
 */

 #pragma strings(readonly)
 #define INCL_DOSFILEMGR   /* File Manager values */
 #define INCL_DOSERRORS    /* DOS error values */

 #include <stdio.h>
 #include <string.h>
 #include <io.h>

 #include "icqskmgr.h"

/*---[ Prototipes ]------------------------------------------------------------------------------------------*/

 static int   _System optLoad(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int   _System optSave(const DLGHELPER *,HWND,HICQ,ULONG,char *);

 static int   _System optClick(const DLGHELPER *,HWND,HICQ,ULONG,  USHORT, char * );

 static int   _System skLoad(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int   _System skSave(const DLGHELPER *,HWND,HICQ,ULONG,char *);

 static void  _System skSelected(const DLGHELPER *, HWND, HICQ, ULONG, USHORT, ULONG, char *);

 const DLGMGR options   = {     sizeof(DLGMGR),
								sizeof(DLGHELPER),
								0,
                                NULL,                   // Configure
                                optLoad,                // Load
                                optSave,                // Save
                                NULL,                   // Cancel
                                NULL,                   // Event
                                NULL,                   // selected
                                optClick,               // click
                                NULL,                   // changed
                                NULL	             	// sysButton
                        };

 const DLGMGR skins     = {     sizeof(DLGMGR),
								sizeof(DLGHELPER),
								0,
                                NULL,                   // Configure
                                skLoad,                 // Load
                                skSave,                 // Save
                                NULL,                   // Cancel
                                NULL,                   // Event
                                skSelected,             // selected
                                NULL,                   // click
                                NULL,                   // changed
                                NULL		            // sysButton
                        };

/*---[ Implementing ]----------------------------------------------------------------------------------------*/

 void EXPENTRY icqskmgr_ConfigPage(HICQ icq, void *lixo, ULONG uin, USHORT type, HWND hwnd, const DLGINSERT *mgr, char *buffer)
 {
    if(type != 1)
       return;

    mgr->loadPage(hwnd, module, 22, &options, CFGWIN_OPTIONS);
    mgr->loadPage(hwnd, module, 23, &skins,   CFGWIN_OPTIONS);

 }

 static int optLoad(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    dlg->setTextLimit(hwnd,112,0xFF);

    dlg->readRadioButton( hwnd, 101, "TaskICON", 3,2);   // Change ICON
    dlg->readRadioButton( hwnd, 105, "TaskTITLE",3,2);   // Change Title
    dlg->readRadioButton( hwnd, 121, "Quote",3,1);       // Quote

    dlg->readCheckBox(    hwnd, 124, "Chk4URL",    TRUE);   // Detect URL
    dlg->readCheckBox(    hwnd, 125, "ESC2Close",  TRUE);   // Close on ESC
    dlg->readCheckBox(    hwnd, 109, "NextOnSend", TRUE);   // Next after send

    dlg->readString(      hwnd, 112, "browser","mozilla.exe");

    dlg->readCheckBox(    hwnd, 114, "urlOpen",    TRUE);   // Enable "Open" Button
    dlg->readCheckBox(    hwnd, 115, "urlSend",    FALSE);  // Enable "Send to Browser" Button
    dlg->readCheckBox(    hwnd, 116, "urlInfo",    FALSE);  // Enable "Open Information" Menu

    return 0;
 }

 static int optSave(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {

    dlg->writeRadioButton(hwnd,101,"TaskICON",3);
    dlg->writeRadioButton(hwnd,105,"TaskTITLE",3);
    dlg->writeRadioButton(hwnd,121,"Quote",3);

    dlg->writeCheckBox(hwnd,124,"Chk4URL");
    dlg->writeCheckBox(hwnd,125,"ESC2Close");
    dlg->writeCheckBox(hwnd,109,"NextOnSend");

    dlg->writeString(hwnd,112,"browser");

    dlg->writeCheckBox(hwnd,114,"urlOpen");
    dlg->writeCheckBox(hwnd,115,"urlSend");
    dlg->writeCheckBox(hwnd,116,"urlInfo");

    return 0;
 }

 static int _System optClick(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, USHORT id, char *buffer)
 {
    if(id != 113)
       return 0;

    dlg->selectFile(hwnd, 104, FALSE, "browse", "*.exe", "Select Browser", NULL, NULL);

    return id;
 }

 static int skLoad(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    char                *ptr;
    HDIR                hDir;
    FILEFINDBUF3        fl;
    ULONG               fCount;
    int                 rc;
    char		*f;
    int			qtd        = 1;
    char		atual[80];
    BOOL		selected   = FALSE;

    if(dlg->sz != sizeof(DLGHELPER))
       return -1;

    icqLoadString(icq, "skin", "default.skn", atual, 0xFF);

    icqLoadString(icq, "skinpath", "", buffer, 0xFF);

    DBGMessage(buffer);

    if(!*buffer)
       icqQueryPath(icq,"skins\\",buffer,0xF0);

    ptr = buffer+strlen(buffer);

    strncat(buffer,"*.skn",0xFF);
    DBGMessage(buffer);

    if(!*buffer)
       return -1;

    dlg->readCheckBox(hwnd, 105, "SkinPreview",  TRUE);

    fCount = 1;
    hDir   = HDIR_CREATE;
    rc     = DosFindFirst(buffer,&hDir,FILE_NORMAL,&fl,sizeof(FILEFINDBUF3),&fCount,FIL_STANDARD);

    while(!rc)
    {
       DBGTrace(stricmp(atual,fl.achName));
       selected = !stricmp(atual,fl.achName);

       for(f=fl.achName;*f && *f != '.';f++);
       *f = 0;

       DBGMessage(fl.achName);
       dlg->listBoxInsert(hwnd,102,qtd++,fl.achName,selected);

       fCount = 1;
       rc     = DosFindNext(hDir,&fl,sizeof(FILEFINDBUF3),&fCount);
    }

    DosFindClose(hDir);

    dlg->readCheckBox(hwnd, 101, "SkinLoad", FALSE);

    return 0;
 }

 static int skSave(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {

    dlg->writeCheckBox(hwnd,105,"SkinPreview");
    dlg->writeCheckBox(hwnd,101,"SkinLoad");

    *buffer = 0;
    dlg->getString(hwnd,102,0xFF,buffer);

    if(*buffer)
    {
       strncat(buffer,".skn",0xFF);
       DBGMessage(buffer);
       icqSaveString(icq, "skin", buffer);
    }

    DBGMessage(buffer);

    return 0;
 }


 static void _System skSelected(const DLGHELPER *dl, HWND hwnd, HICQ icq, ULONG uin, USHORT id, ULONG hdl, char *buffer)
 {
    int  	sz;
    int  	rc;
    HWND 	hBtm;
    HPS         hps;
    HBITMAP 	bitmap;

    DBGTrace(id);

    if(id != 102)
       return;

    DBGTrace(hdl);

    if(dl->getCheckBox(hwnd,101))
    {
       icqLoadString(icq, "skinpath", "", buffer, 0xFF);
       if(!*buffer)
          icqQueryPath(icq,"skins\\",buffer,0xFF);

       sz = strlen(buffer);

       dl->getString(hwnd,102,0xFF-sz,buffer+sz);

       DBGMessage(buffer);

       if(!*(buffer+sz))
          return;

       CHKPoint();

       strncat(buffer,".skn",0xFF);
       *(buffer+0xFF) = 0;

       DBGMessage(buffer);

       rc = sknLoadNewSkin(mainWindow,winElements,buffer);
       if(rc)
          icqWriteSysRC(icq, PROJECT, rc, "Failure changing skin");

    }

    if(dl->getCheckBox(hwnd,105))
    {
       icqLoadString(icq, "skinpath", "", buffer, 0xFF);
       if(!*buffer)
          icqQueryPath(icq,"skins\\",buffer,0xFF);

       sz = strlen(buffer);

       dl->getString(hwnd,102,0xFF-sz,buffer+sz);

       DBGMessage(buffer);

       if(!*(buffer+sz))
          return;

       CHKPoint();

       strncat(buffer,"\\preview.jpg",0xFF);
       *(buffer+0xFF) = 0;

       WinSetWindowPos(WinWindowFromID(hwnd,1000),0,0,0,141,304,SWP_SIZE);
       dl->setBitmap(icq, hwnd, module, 1000, buffer);

    }

    // Troca as imagems de about
    icqLoadString(icq, "skinpath", "", buffer, 0xFF);
    sz = strlen(buffer);

    dl->getString(hwnd,102,0xFF-sz,buffer+sz);

    strncat(buffer,"\\about.jpg",0xFF);
    *(buffer+0xFF) = 0;

    WinSetWindowPos(WinWindowFromID(hwnd,1001),0,0,0,257,80,SWP_SIZE);
    dl->setBitmap(icq, hwnd, module, 1001, buffer);

 }

