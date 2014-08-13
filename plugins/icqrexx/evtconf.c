/*
 * rxconf.c - Event helpers dialog box
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdio.h>
 #include <ctype.h>

 #include "icqrexx.h"

/*---[ Configuration data ]----------------------------------------------------------------------------------*/

 #pragma pack(1)

 struct config
 {
    ULONG       last;
    ULONG       groups;
 };

/*---[ Prototipes ]------------------------------------------------------------------------------------------*/

 static int   _System configure(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int   _System load(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static int   _System save(const DLGHELPER *,HWND,HICQ,ULONG,char *);
 static void  _System selected( const DLGHELPER *,HWND,HICQ,ULONG,  USHORT, ULONG, char *);
 static int   _System click(const DLGHELPER *,HWND,HICQ,ULONG,  USHORT, char * );

/*---[ Constants ]-------------------------------------------------------------------------------------------*/


 const DLGMGR evtConfig = {   sizeof(DLGMGR),
							  sizeof(DLGHELPER),
							  sizeof(struct config),
                              configure,		// Configure
                              load,			    // Load
                              save,			    // Save
                              NULL,			    // Cancel
                              NULL,			    // Event
                              selected,			// Selected
                              click,			// Click
                              NULL,			    // Changed
                              NULL			// sysButton

                         };

/*---[ Implementation ]--------------------------------------------------------------------------------------*/

 static int _System configure(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
    dlg->setTextLimit(hwnd,111,80);     // Title
    dlg->setTextLimit(hwnd,112,0xFF);   // FileName
    dlg->setTextLimit(hwnd,114,80);     // Parameters
    return sizeof(struct config);
 }

 static int _System load(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
/*
#ifdef __OS2__
    struct config       *cfg    = (struct config *) dlg->getConfig(hwnd);
    int                 f;
    char                key[20];
    ULONG               masc    = 1;

    DBGTracex(cfg);
#endif	

    if(dlg->sz != sizeof(DLGHELPER))
       return -1;

    dlg->populateEventList(hwnd,103);

#ifdef __OS2__
    for(f=0;f<9;f++)
    {
       sprintf(key,"Group%02d",f);
       icqLoadString(icq,key,"",buffer,40);
       if(*buffer)
       {
          cfg->groups |= masc;
          dlg->setString(hwnd,300+f,buffer);
       }
       masc <<= 1;
    }
#endif	
*/
    return 0;
 }

 static int _System save(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, char *buffer)
 {
/*
    char        key[20];
    struct      config *cfg  = (struct config *) dlg->getConfig(hwnd);

#ifdef __OS2__
    ULONG       flag         = 0;
    char        *ptr;
    ULONG       masc;
    int         f;
#endif	

    if(!cfg || !cfg->last)
       return 0;

#ifdef __OS2__
    strcpy(key,"helper:NCNNN");

    *( (ULONG *) (key+8) ) = cfg->last;
    *(key+12) = 0;

    dlg->getString(hwnd,112,0xFF,buffer);

    for(ptr = buffer;*ptr && isspace(*ptr);ptr++);

    icqSaveString(icq,key,ptr);

    *(key+7) = 'T';
    dlg->saveString(hwnd,0,111,key,0xFF);

    *(key+7) = 'P';
    dlg->saveString(hwnd,0,114,key,0xFF);

    *(key+7) = 'F';

    DBGMessage("**** Salvando flags ****");

    if(*(key+8) == 'U')
    {
       masc = 1;
       for(f=0;f<9;f++)
       {
          if( (cfg->groups & masc) && dlg->getCheckBox(hwnd,300+f))
             flag |= masc;
          masc <<= 1;
       }
    }

    DBGTracex(flag);

    flag <<= 2;
    flag |= dlg->getRadioButton(hwnd,124,3);
    flag <<= 2;
    flag |= dlg->getRadioButton(hwnd,121,3);
    flag <<= 2;
    flag |= dlg->getRadioButton(hwnd,117,3);

    if(*ptr && !isspace(*ptr))
       flag |= 0x80000000;

    DBGTracex(flag);
    icqSaveValue(icq, key, flag);
	
#else
	
    strcpy(key,"helper:CNNN");
    *( (ULONG *) (key+7) ) = cfg->last;
	DBGMessage(key);
    dlg->saveString(hwnd,0,112,key,0xFF);

#endif
*/	
    return 0;
 }


 void _System selected(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, USHORT id, ULONG hdl, char *buffer)
 {
/*
    char        key[20];
    struct      config *cfg = (struct config *) dlg->getConfig(hwnd);
#ifdef __OS2__	
    ULONG       flag;
    int         f;
    ULONG       masc;
#endif	

    if(id != 103 || !cfg)
       return;

    save(dlg,hwnd,icq,uin,buffer);
	
    cfg->last = hdl;

#ifdef __OS2__	
	
    strcpy(key,"helper:NCNNN");

    *( (ULONG *) (key+8) ) = hdl;
    *(key+12) = 0;

    dlg->loadString(hwnd,0,112,key,0xFF,"");
    dlg->setEnabled(hwnd,112,TRUE);

    *(key+7) = 'T';
    dlg->loadString(hwnd,0,111,key,0xFF,"");
    dlg->setEnabled(hwnd,111,TRUE);

    *(key+7) = 'P';
    dlg->loadString(hwnd,0,114,key,0xFF,"");
    dlg->setEnabled(hwnd,114,TRUE);

    *(key+7) = 'F';
    flag = icqLoadValue(icq,key,0);

    DBGMessage("***** CARREGANDO FLAGS");
    DBGTracex( flag );
    DBGTracex( (flag&0x7Fc0) >> 6 );
    DBGTracex( (flag&0x003F) );

    dlg->setRadioButton(hwnd,117,flag & 0x03);

    flag >>= 2;
    dlg->setRadioButton(hwnd,121,flag & 0x03);
    flag >>= 2;
    dlg->setRadioButton(hwnd,124,flag & 0x03);
    flag >>= 2;

    dlg->setEnabled(hwnd,115,TRUE);

    for(f=0;f<3;f++)
    {
       dlg->setEnabled(hwnd,117+f,TRUE);
       dlg->setEnabled(hwnd,121+f,TRUE);
       dlg->setEnabled(hwnd,124+f,TRUE);
    }

    if(*(key+8) == 'U')
    {
       // User event - Enable group filter

       DBGTracex(flag);

       masc = 1;
       for(f=0;f<9;f++)
       {
          if(cfg->groups & masc)
          {
             dlg->setEnabled(hwnd,300+f,TRUE);
             dlg->setCheckBox(hwnd,300+f,flag&masc);
          }
          else
          {
             dlg->setEnabled(hwnd,300+f,FALSE);
          }
          masc <<= 1;
       }
    }
    else
    {
       for(f=0;f<9;f++)
       {
          dlg->setCheckBox(hwnd,300+f,FALSE);
          dlg->setEnabled(hwnd,300+f,FALSE);
       }
    }
#else
	
    strcpy(key,"helper:CNNN");
    *( (ULONG *) (key+7) ) = cfg->last;
	
	DBGTracex(cfg->last);
	
	DBGMessage(key);
	
    dlg->loadString(hwnd,0,112,key,0xFF,"");

#endif
*/
	
 }

 static int _System click(const DLGHELPER *dlg, HWND hwnd, HICQ icq, ULONG uin, USHORT id, char *buffer )
 {
/*
    if(id == 115)
       dlg->browseFiles(hwnd,112,0,"eventHelpers","*.cmd", "Select Helper", NULL, NULL);
    else
       return id;
*/
    return 0;
 }


