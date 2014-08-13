/*
 * ICQV8 Search dialogs
 */

 #include <stdlib.h>

 #include "icqv8.h"
 #include <msgdlg.h>

/*---[ Prototipes ]------------------------------------------------------------------------------------------*/

 static int _System searchByICQ(const DLGHELPER *,HWND,HICQ,ULONG,HWND,USHORT,SEARCHCALLBACK,char *);
 static int _System searchByEMail(const DLGHELPER *,HWND,HICQ,ULONG,HWND,USHORT,SEARCHCALLBACK,char *);
 static int _System searchByInfo(const DLGHELPER *,HWND,HICQ,ULONG,HWND,USHORT,SEARCHCALLBACK,char *);
 static int _System searchRandom(const DLGHELPER *,HWND,HICQ,ULONG,HWND,USHORT,SEARCHCALLBACK,char *);

 static int _System srLoad(const DLGHELPER *, HWND, HICQ, ULONG, char *);

/*---[ Constantes ]------------------------------------------------------------------------------------------*/

 static const DLGMGR byICQ   = {	sizeof(DLGMGR),
                                    sizeof(DLGHELPER),
                                    0,
                                	NULL,    	// Configure
                                	NULL,    	// Load
                                	NULL,    	// Save
                                	NULL,    	// Cancel
                                	NULL,    	// Event
                                	NULL,    	// Selected
                                	NULL,    	// Click
                                	NULL,     	// Changed
                                	searchByICQ	// sysButton
                        	};

 static const DLGMGR byEMail   = {	sizeof(DLGMGR),
                                    sizeof(DLGHELPER),
                                    0,
                                	NULL,    	// Configure
                                	NULL,    	// Load
                                	NULL,    	// Save
                                	NULL,    	// Cancel
                                	NULL,    	// Event
                                	NULL,    	// Selected
                                	NULL,  		// Click
                                	NULL,     	// Changed
                                	searchByEMail	// sysButton
                        	};

 static const DLGMGR byInfo   = {	sizeof(DLGMGR),
                                    sizeof(DLGHELPER),
                                    0,
                                	NULL,    	    // Configure
                                	NULL,    	    // Load
                                	NULL,    	    // Save
                                	NULL,      	    // Cancel
                                	NULL,    	    // Event
                                	NULL,    	    // Selected
                                	NULL,   	    // Click
                                	NULL,     	    // Changed
                                	searchByInfo	// sysButton
                        	};

 static const DLGMGR sRandom   = {	sizeof(DLGMGR),
                                    sizeof(DLGHELPER),
                                    0,
                                	NULL,    	    // Configure
                                	srLoad,       	// Load
                                	NULL,    	    // Save
                                	NULL,    	    // Cancel
                                	NULL,           // Event
                                	NULL,           // Selected
                                	NULL,           // Click
                                	NULL,           // Changed
                                	searchRandom	// sysButton
                        	};

/*---[ Implementing ]----------------------------------------------------------------------------------------*/

 void EXPENTRY icqv8_SearchPage(HICQ icq, void *lixo, USHORT type, HWND hwnd, const DLGINSERT *dlg, char *buffer)
 {
/*
    DBGTrace(type);

    if(type)
       return;

#ifdef __OS2__	
    dlg->insert(hwnd, module, 1073, &byICQ,   0);
    dlg->insert(hwnd, module, 1074, &byEMail, 0);
    dlg->insert(hwnd, module, 1075, &byInfo,  0);
    dlg->insert(hwnd, module, 1076, &sRandom, 0);
#endif	

#ifdef linux
    dlg->insertTable(hwnd, 0, icq_table,   &byICQ,   "ICQ#\nSearch by UIN");
    dlg->insertTable(hwnd, 0, email_table, &byEMail, "E-Mail\nSearch by user E-Mail");
    dlg->insertTable(hwnd, 0, info_table,  &byInfo,  "User Info\nSearch by User Information");
//    dlg->insertTable(hwnd, 0, rand_table,  &sRandom, "Random\nRandom Search");
#endif

    CHKPoint();
*/
 }


 static int _System searchByICQ(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, HWND main, USHORT id, SEARCHCALLBACK cbk,char *buffer)
 {
    ULONG key;

    if(id != ICQSHRC_SEARCH)
       return 0;
    CHKPoint();

    *buffer = 0;
    dlg->getString(hwnd,102,0xFF,buffer);
    key = atoi(buffer);

    DBGTrace(key);

    if(!key)
       return -1;

    icqv8_searchByICQ(icq, key, main, cbk);

    return 0;
 }

 static int _System searchByEMail(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, HWND main, USHORT id, SEARCHCALLBACK cbk, char *buffer)
 {
    if(id != ICQSHRC_SEARCH)
       return 0;
    CHKPoint();

    *buffer = 0;
    dlg->getString(hwnd,102,0xFF,buffer);
    DBGMessage(buffer);

    icqv8_searchByMail(icq, buffer, main, cbk);

    return 0;
 }

 static int _System searchByInfo(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, HWND main, USHORT id, SEARCHCALLBACK cbk,char *buffer)
 {
    char nick[80];
    char first[80];
    char last[80];
    char email[0x0100];

    if(id != ICQSHRC_SEARCH)
       return 0;

    CHKPoint();

    dlg->getString(hwnd,105,79,nick);
    dlg->getString(hwnd,106,79,first);
    dlg->getString(hwnd,108,79,last);
    dlg->getString(hwnd,107,79,email);

    DBGMessage(nick);
    DBGMessage(first);
    DBGMessage(last);
    DBGMessage(email);

    icqv8_searchByInformation(icq,main,first,last,nick,email,0,cbk);

    return 0;
 }

 static int _System srLoad(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
/*
   if(dlg->sz != sizeof(DLGHELPER))
      return -1;
    CHKPoint();
    dlg->setRadioButton(hwnd,101,icqLoadValue(icq,"RSKey",0));
*/
    return 0;
 }

 static int _System searchRandom(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, HWND main, USHORT id, SEARCHCALLBACK cbk,char *buffer)
 {
/*
    int sel = dlg->getRadioButton(hwnd,101,10);

    DBGTrace(sel);

    icqSaveValue(icq, "RSKey", sel);
    icqv8_searchRandom(icq, sel, main,cbk);
*/
    return 0;
 }

