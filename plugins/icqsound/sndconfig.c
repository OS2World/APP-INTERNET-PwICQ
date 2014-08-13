/*
 * icqSoundConfig - Sound plugin configuration
 */

 #include <string.h>
 #include <stdio.h>

 #include "icqsound.h"

/*---[ Configuration data ]----------------------------------------------------------------------------------*/

 #pragma pack(1)

 struct config
 {
    ULONG       last;
    BOOL        canPlay;
 };

/*---[ Statics ]---------------------------------------------------------------------------------------------*/

/*---[ Prototipes ]------------------------------------------------------------------------------------------*/

 static int   _System configure(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int   _System load(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int   _System save(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static void  _System selected( const DLGHELPER *,HWND,HICQ,ULONG,  USHORT, ULONG, char *);
 static int   _System click(const DLGHELPER *,HWND,HICQ,ULONG,  USHORT, char * );
 static int   _System changed(const DLGHELPER *,HWND,HICQ,ULONG, USHORT, USHORT, char * );
 static void  _System event(const DLGHELPER *,HWND,HICQ,ULONG,char,USHORT,char *);

 static int   _System saveScheme(const DLGHELPER *,HWND,HICQ,char *);
 static int   _System loadScheme(const DLGHELPER *,HWND,HICQ,char *);

 static void saveLast(const DLGHELPER *, HWND, struct config *);

/*---[ Constants ]-------------------------------------------------------------------------------------------*/

 const DLGMGR sndConfig = {   sizeof(DLGMGR),
							  sizeof(DLGHELPER),
	                          sizeof(struct config),
                              configure,	     // Configure
                              load,			     // Load
                              save,			     // Save
                              NULL,			     // Cancel
                              event,			 // Event
                              selected,	 	 	 // Selected
                              click,			 // Click
                              changed,			 // Changed
                              NULL			     // sysButton
                         };

/*---[ Implementation ]--------------------------------------------------------------------------------------*/

 static int _System configure(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    dlg->setTextLimit(hwnd,104,0xFF);
    dlg->setTextLimit(hwnd,112,0xFF);
    return sizeof(struct config);
 }

 static int _System load(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    struct config *cfg = (struct config *) dlg->getUserData(hwnd);

    icqQueryProgramPath(icq,"events.dat",buffer,0xFF);
	DBGMessage(buffer);

    DBGTracex(cfg);

    cfg->canPlay = TRUE;

    dlg->setEnabled(hwnd,104,FALSE);
    dlg->setEnabled(hwnd,105,FALSE);
    dlg->setEnabled(hwnd,106,FALSE);
    dlg->readString(hwnd, 112, "sound:Scheme","");

    dlg->populateEventList(icq,hwnd,102);

#ifdef __OS2__
    dlg->readCheckBox(hwnd,108,"sound:Share",  TRUE);
#endif

    return 0;
 }

 static int _System save(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    struct config *cfg = (struct config *) dlg->getUserData(hwnd);

#ifdef __OS2__
    dlg->writeCheckBox(hwnd,108,"sound:Share");
#endif

    dlg->writeString(hwnd, 112, "sound:Scheme");

    saveLast(dlg,hwnd,cfg);

    return 0;
 }

 void _System selected(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, USHORT id, ULONG hdl, char *buffer)
 {
    char key[20];
    struct config *cfg = (struct config *) dlg->getUserData(hwnd);

    if(id != 102 || !cfg)
       return;

    saveLast(dlg,hwnd,cfg);

    strcpy(key,"sound:CNNN");

    cfg->last = hdl;
    *( (ULONG *) (key+6) ) = hdl;
    *(key+10) = 0;

    dlg->readString(hwnd,104,key,"");
    dlg->setEnabled(hwnd,104,TRUE);
    dlg->setEnabled(hwnd,105,TRUE);

    *buffer = 0;
    dlg->getString(hwnd,104,0xFF,buffer);
    dlg->setEnabled(hwnd,106,cfg->canPlay && *buffer);
	
 }

 static void saveLast(const DLGHELPER *dlg, HWND hwnd, struct config *cfg)
 {
    char key[20];

    DBGTrace(cfg->last);

    if(!cfg->last)
       return;

    strcpy(key,"sound:CNNN");

    *( (ULONG *) (key+6) ) = cfg->last;
    *(key+10) = 0;

    DBGMessage(key);

    dlg->writeString(hwnd,104,key);
 }

 static int _System click(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, USHORT id, char *buffer )
 {
    switch(id)
    {
    case 105:   // Browse
       dlg->selectFile(hwnd, 104, FALSE, "sounds", "*.wav", "Select Sound", NULL, NULL);
       break;

    case 106:   // Play
       dlg->getString(hwnd,104,0xFF,buffer);
       DBGMessage(buffer);
       icqsound_Play(icq, buffer);
       break;

    case 101:   // Save scheme
//       dlg->browseFiles(hwnd,112,0,"sounds","*.scm","Save scheme as", NULL, saveScheme);
       break;

    case 113:   // Load scheme
//       dlg->browseFiles(hwnd,112,0,"sounds","*.scm","Load scheme", NULL, loadScheme);
       break;

    default:
       return id;
    }

    return 0;
 }

 static int _System changed(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, USHORT id, USHORT bytes, char *buffer )
 {
    struct config *cfg = (struct config *) dlg->getUserData(hwnd);

	DBGTrace(id);
	
	if(id == 104)
       dlg->setEnabled(hwnd,106,(bytes>0) && cfg->canPlay);

    return 0;
 }

 static void  _System event(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char type, USHORT event, char *buffer)
 {
/*
    struct config *cfg = (struct config *) dlg->getUserData(hwnd);

    switch(event)
    {
    case ICQEVENT_BEGINPLAY:
       cfg->canPlay = FALSE;
       dlg->setEnabled(hwnd,106,FALSE);
       break;

    case ICQEVENT_ENDPLAY:
       cfg->canPlay = TRUE;
       dlg->setEnabled(hwnd,106,dlg->getLength(hwnd,104) > 0);
       break;
    }
*/
 }

 static int _System saveScheme(const DLGHELPER *dlg, HWND hwnd, HICQ icq, char *fileName)
 {
    icqsound_saveScheme(icq,fileName);
	return 0;
 }

 static int _System loadScheme(const DLGHELPER *dlg, HWND hwnd, HICQ icq, char *fileName)
 {
    icqsound_loadScheme(icq,fileName);
    return 0;
 }




