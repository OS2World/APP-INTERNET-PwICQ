/*
 * icqSoundConfig
 */

 #include <stdio.h>
 #include <string.h>

 #include "icqsound.h"

/*---[ Configuration data ]----------------------------------------------------------------------------------*/

 #pragma pack(1)

 struct config
 {
    ULONG       selected;
    ULONG       masc;
    BOOL        canPlay;
 };

/*---[ Prototipes ]------------------------------------------------------------------------------------------*/

 static int   _System configure(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int   _System load(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int   _System save(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static void  _System selected( const DLGHELPER *,HWND,HICQ,ULONG,  USHORT, ULONG, char *);
 static int   _System changed(const DLGHELPER *,HWND,HICQ,ULONG, USHORT, USHORT, char * );
 static void  _System event(const DLGHELPER *,HWND,HICQ,ULONG,char,USHORT,char *);
 static int   _System click(const DLGHELPER *,HWND,HICQ,ULONG,  USHORT, char * );

/*---[ Constants ]-------------------------------------------------------------------------------------------*/

 const DLGMGR grpConfig = {   sizeof(DLGMGR),
	                          sizeof(DLGHELPER),
							  sizeof(struct config),
                              configure,
                              load,
                              save,
                              NULL,
                              event,
                              selected,
                              click,
                              changed
                         };
/*---[ Implementation ]--------------------------------------------------------------------------------------*/


 static int _System configure(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    int f;
    char        key[20];

    for(f=0;f<PWICQGROUPCOUNTER;f++)
    {
       sprintf(key,"Group%02d",f);
       icqLoadString(icq,key,"",buffer,40);
       if(*buffer)
          dlg->listBoxInsert(hwnd,102,(ULONG) f,buffer,FALSE);
    }
    dlg->setTextLimit(hwnd,104,0xFF);
	
    return sizeof(struct config);
 }

 static int _System load(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    struct config *cfg = (struct config *) dlg->getUserData(hwnd);
    cfg->selected = 0xFFFFFFFF;
    cfg->canPlay  = TRUE;
    cfg->masc     = icqLoadValue(icq,"GSound",0);
    return 0;
 }

 static int _System save(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    struct config *cfg = (struct config *) dlg->getUserData(hwnd);
    char   key[20];
    ULONG  masc = 1;

    if(cfg->selected == 0xFFFFFFFF)
       return 0;

    masc <<= cfg->selected;

    dlg->getString(hwnd,104,0xFF,buffer);

    sprintf(key,"GSound%02d",(int) cfg->selected);

    if(*buffer)
    {
       cfg->masc |= masc;
       icqSaveString(icq,key,buffer);
    }
    else
    {
       cfg->masc &= ~masc;
       icqSaveString(icq,key,NULL);
    }

    icqSaveValue(icq,"GSound",cfg->masc);

    DBGTracex(cfg->masc);

    return 0;
 }

 void _System selected(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, USHORT id, ULONG hdl, char *buffer)
 {
    struct config *cfg = (struct config *) dlg->getUserData(hwnd);
    char   key[20];
    ULONG  masc = 1;

    DBGTrace(hdl);

    save(dlg,hwnd,icq,uin,buffer);

    masc <<= hdl;

    sprintf(key,"GSound%02d",(int) hdl);

    icqLoadString(icq,key,"",buffer,0xFF);

    if(*buffer)
       cfg->masc |= masc;
    else
       cfg->masc &= ~masc;

    dlg->setString(hwnd,104,buffer);
    cfg->selected = hdl;

    dlg->setEnabled(hwnd,105,TRUE);
 }

 static int _System changed(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, USHORT id, USHORT bytes, char *buffer )
 {
    struct config *cfg = (struct config *) dlg->getUserData(hwnd);

    if(id != 104)
       return -1;

    dlg->setEnabled(hwnd,106,(bytes>0) && cfg->canPlay);
    return 0;
 }

 static void  _System event(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char type, USHORT event, char *buffer)
 {
    struct config *cfg = (struct config *) dlg->getUserData(hwnd);

    switch(event)
    {
    case ICQEVENT_BEGINPLAY:
       cfg->canPlay = FALSE;
       dlg->setEnabled(hwnd,106,FALSE);
       break;

    case ICQEVENT_ENDPLAY:
       cfg->canPlay = TRUE;
//       dlg->setEnabled(hwnd,106,dlg->getLength(hwnd,104) > 0);
       break;

    }
 }

 static int _System click(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, USHORT id, char *buffer )
 {
/*
    switch(id)
    {
    case 105:   // Browse
       dlg->browseFiles(hwnd,104,0,"sounds","*.wav", "Select group sound", NULL, NULL);
       break;

    case 106:   // Play
       dlg->getString(hwnd,104,0xFF,buffer);
       DBGMessage(buffer);
       icqsound_Play(icq, buffer);
       break;

    default:
       return id;
    }
*/
    return 0;
 }


