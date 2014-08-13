/*
 * edit.c - Window management callbacks
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_WIN
 #define INCL_DOSMODULEMGR
#endif

 #define MSGEDITWINDOW hWindow

 #include <string.h>
 #include <pwicqgui.h>

/*---[ Macros ]-------------------------------------------------------------------------------------------*/

 #pragma pack(1)

 typedef struct _icqedit
 {
    ICQFRAME_STANDARD_PREFIX
 } ICQEDIT;

 #pragma pack()

/*---[ Macros ]-------------------------------------------------------------------------------------------*/

#ifdef __OS2__

   HWND os2getChildWindow(HWND, USHORT);

   #define CHILD_WINDOW(h,i) os2getChildWindow(h,i)
   #define DIALOG_ID(id)     if(id >= DLG_ELEMENT_ID) id -= DLG_ELEMENT_ID;
   #define VALIDATE_ID(id)   if(id >= DLG_ELEMENT_ID) id -= DLG_ELEMENT_ID

#else

   #define CHILD_WINDOW(h,i) icqskin_getDlgItemHandle(h,i)
   #define DIALOG_ID(id)     /* */
   #define VALIDATE_ID(id)   /* */

#endif

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

 static int     _System setTitle(hWindow,const char *);
 static int     _System setMessage(hWindow, USHORT, BOOL, HMSG);

 static int     _System setVisible(hWindow, USHORT, BOOL);
 static int     _System getVisible(hWindow, USHORT);

 static int     _System setEnabled(hWindow, USHORT, BOOL);

 static int     _System setString(hWindow,USHORT,const char *);
 static int     _System getString(hWindow,USHORT,int,char *);

 static int     _System setCheckBox(hWindow,USHORT,BOOL);
 static int     _System getCheckBox(hWindow,USHORT);

 static int     _System loadUserFields(hWindow);
 static void    _System loadString(hWindow, USHORT, USHORT);

 static void    _System setReply(hWindow, USHORT);
 static void    _System setTime(hWindow, USHORT, time_t);

 static void    _System setEditable(hWindow, USHORT, BOOL);
 static void    _System setIcon(hWindow,USHORT,USHORT);

 static int     _System queryClipboard(HICQ, int, char *);
 static int     _System populateEventList(HICQ, hWindow, USHORT);

 static hWindow _System dialog(hWindow, HMODULE, USHORT, const DLGMGR *, USHORT);

 static hWindow _System loadDialog(HICQ,hWindow,HMODULE,USHORT,const struct dlgmgr *);

 static void    _System setTextLimit(hWindow,USHORT,USHORT);
 static void    _System listBoxInsert(hWindow,USHORT,ULONG,const char *,BOOL);
 static void *  _System getUserData(hWindow);

 static void    _System readCheckBox(hWindow,USHORT,const char *,BOOL);
 static void    _System readRadioButton(hWindow,USHORT,const char *, USHORT, USHORT);
 static void    _System readString(hWindow,USHORT,const char *,const char *);
	
 static void    _System writeCheckBox(hWindow,USHORT,const char *);
 static void    _System writeRadioButton(hWindow,USHORT,const char *,USHORT);
 static void    _System writeString(hWindow,USHORT,const char *);

 static int     _System setSpinButton(hWindow,USHORT, int, int, int);
 static int     _System getSpinButton(hWindow, USHORT);

 static int     _System selectFile(HWND, USHORT, BOOL, const char *, const char *, const char *, char *, int (* _System)(const struct _dialogcalls *,HWND,HICQ,char *));

 static int lbInsert(hWindow, ULONG, const char *, BOOL);


/*---[ Constants ]----------------------------------------------------------------------------------------*/

 const DIALOGCALLS icqskin_dialogCalls =
 {
    sizeof(DIALOGCALLS),
    XBUILD,

    /* Frame/Dialog box */
    setTitle,
    setVisible,
    getVisible,
    setEnabled,
    setString,
    getString,
    setCheckBox,
    getCheckBox,
    setSpinButton,
    getSpinButton,
    loadString,
    setTime,
	setEditable,
	setIcon,
	setTextLimit,
	listBoxInsert,
	
    /* Frame Window/Dialogs + Configuration files */
    readCheckBox,
    readRadioButton,
    readString,

    writeCheckBox,
    writeRadioButton,
    writeString,

    /* Dialog box only */
    getUserData,
	
    /* Miscellaneous */	
	queryClipboard,
	populateEventList,
	selectFile,
	icqskin_setItemBitmap,

    /* Message dialog only */
    setMessage,
    loadUserFields,
    setReply,

    /* Configuration boxes only */
    icqskin_insertConfigPage,
    dialog,

    /* Resource management */
    loadDialog

 };

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

#ifdef __OS2__
 HWND os2getChildWindow(HWND hwnd, USHORT id)
 {
    HWND h = WinWindowFromID(hwnd,id);
    if(!h && id >= DLG_ELEMENT_ID)
       return WinWindowFromID(hwnd,id-DLG_ELEMENT_ID);
    return h;
 }
#endif

 static int _System setTitle(MSGEDITWINDOW hwnd,const char *title)
 {
    return 0;
 }

 static int _System setMessage(MSGEDITWINDOW hwnd, USHORT type, BOOL out, HMSG msg)
 {
    return 0;
 }

 static int _System setVisible(MSGEDITWINDOW hwnd, USHORT id, BOOL visible)
 {
    hwnd = CHILD_WINDOW(hwnd,id);

    if(!hwnd)
       return -1;

    if(visible)
       icqskin_show(hwnd);
    else
       icqskin_hide(hwnd);

    return 0;
 }

 static int _System setEnabled(MSGEDITWINDOW hwnd, USHORT id, BOOL enabled)
 {
    hwnd = CHILD_WINDOW(hwnd,id);

    if(!hwnd)
       return -1;

    icqskin_setEnabled(hwnd,enabled);

    return 0;
 }

 static int _System setString(MSGEDITWINDOW hwnd, USHORT id, const char *text)
 {
    hwnd = CHILD_WINDOW(hwnd,id);

    if(!hwnd)
       return -1;

    icqskin_setWindowText(hwnd,text);

    return 0;
 }

 static int _System getString(MSGEDITWINDOW hwnd, USHORT id, int sz, char *text)
 {
    hwnd = CHILD_WINDOW(hwnd,id);

    if(!hwnd)
       return -1;

    icqskin_getWindowText(hwnd,sz,text);

    return 0;
 }

 static int _System setCheckBox(MSGEDITWINDOW hwnd, USHORT id, BOOL check)
 {
    hwnd = CHILD_WINDOW(hwnd,id);

    if(check)
       icqskin_setChecked(hwnd);
    else
       icqskin_setUnchecked(hwnd);

    return 0;
 }

 static int _System getCheckBox(MSGEDITWINDOW hwnd, USHORT id)
 {
    hwnd = CHILD_WINDOW(hwnd,id);

    DBGTrace(id);
    DBGTracex(hwnd);
    DBGTracex(icqskin_queryCheckstate(hwnd));

    return icqskin_queryCheckstate(hwnd);
 }

 static int _System loadUserFields(MSGEDITWINDOW hwnd)
 {
    ICQMSGDIALOG *cfg             = icqskin_getWindowDataBlock(hwnd);
    HUSER        usr              = NULL;
    char         buffer[0x0100];

    if(cfg->uin > SYSTEM_UIN)
       usr = icqQueryUserHandle(cfg->fr.icq,cfg->uin);

    icqskin_setMsgWindowText(hwnd,MSGID_NICK,"Nick:");
    icqskin_setMsgWindowText(hwnd,MSGID_NAME,"Name:");

    if(usr)
    {
       icqskin_setMsgWindowText(hwnd,MSGID_NICKENTRY,icqQueryUserNick(usr));

       strncpy(buffer,icqQueryUserFirstName(usr),0xFF);
       strncat(buffer," ",0xFF);
       strncat(buffer,icqQueryUserLastName(usr),0xFF);
       *(buffer+0xFF) = 0;
       icqskin_setMsgWindowText(hwnd,MSGID_NAMEENTRY,buffer);

       if(usr->modeIcon == usr->offlineIcon)
          icqskin_setDlgItemIcon(hwnd,MSGID_USERMODE,ICQICON_OFFLINE);
       else
          icqskin_setDlgItemIcon(hwnd,MSGID_USERMODE,usr->modeIcon);
    }
    else
    {
       icqskin_setMsgWindowText(hwnd,MSGID_NICKENTRY,"N/A");
       icqskin_setMsgWindowText(hwnd,MSGID_NAMEENTRY,"N/A");
       icqskin_setDlgItemIcon(hwnd,MSGID_USERMODE,ICQICON_OFFLINE);
    }

    return 0;
 }

 static void _System loadString(MSGEDITWINDOW hwnd, USHORT id, USHORT resource)
 {
    char buffer[40];
    *buffer = 0;
    icqskin_loadString(icqframe_getHandle(hwnd), module, resource, buffer, 39);
    setString(hwnd,id,buffer);
 }

 static void _System setReply(MSGEDITWINDOW hwnd, USHORT type)
 {

 }

 static void _System setTime(MSGEDITWINDOW hwnd, USHORT id, time_t tm)
 {
    char buffer[80];
    strftime(buffer,79,"%m/%d/%Y %H:%M:%S ",localtime(&tm));
    setString(hwnd,id,buffer);
 }

 static void  _System setEditable(hWindow hwnd, USHORT id, BOOL flag)
 {
    hwnd = CHILD_WINDOW(hwnd,id);

    if(flag)	
       icqskin_setEditable(hwnd);
    else
	   icqskin_setReadOnly(hwnd);

 }

 static int _System getVisible(hWindow hwnd, USHORT id)
 {
    hwnd = CHILD_WINDOW(hwnd,id);
	
	DBGTracex(hwnd);
    DBGTrace(icqskin_getVisibility(hwnd));
	CHKPoint();
	
    return (int) icqskin_getVisibility(hwnd);
 }

 static void _System setIcon(hWindow hwnd, USHORT id, USHORT icon)
 {
    hwnd = CHILD_WINDOW(hwnd,id);
    icqskin_setButtonIcon(hwnd,icon);
 }

 static int _System queryClipboard(HICQ icq, int sz, char *buffer)
 {
#ifdef __OS2__
    return icqskin_queryClipboard(icq, sz, buffer);
#else
    /* For some reason GTK2 hangs when using icqskin_queryClipboard in this context */
    *buffer = 0;
    return 0;	
#endif	
 }

 static int _System populateEventList(HICQ icq, hWindow hwnd, USHORT id)
 {
    FILE *arq;
    char buffer[0x0100];
    char *ptr;

    hwnd = CHILD_WINDOW(hwnd,id);

    icqQueryProgramPath(icq,"events.dat",buffer,0xFF);

    DBGMessage(buffer);

    arq = fopen(buffer,"r");

    if(!arq)
    {
       icqWriteSysRC(icq, PROJECT, -1, buffer);
       return -1;
    }

    while(!feof(arq))
    {
       *buffer = 0;
       fgets(buffer,0xFF,arq);

       DBGMessage(buffer);

       *(buffer+0xFF) = 0;
       for(ptr = buffer;*ptr && *ptr >= ' ';ptr++);
       *ptr = 0;

       DBGMessage(buffer);

       if(strlen(buffer) > 6 && *buffer != ';')
          lbInsert(hwnd, *((ULONG *) buffer), buffer+5, FALSE);
    }

    fclose(arq);
    return 0;
 }

 static hWindow _System loadDialog(HICQ icq, hWindow hwnd, HMODULE mod, USHORT id, const DLGMGR *mgr)
 {
    CHKPoint();

    if( mgr && (mgr->sz != sizeof(DLGMGR) || mgr->szCalls != sizeof(icqskin_dialogCalls)))
    {
       DBGMessage("Unexpected dialog manager");
       icqWriteSysLog(icq,PROJECT,"Invalid manager when loading dialog box");
       return 0;
    }

	CHKPoint();
    hwnd = icqskin_loadDialog(icq, hwnd, mod, id, mgr);
	DBGTracex(hwnd);

    if(!hwnd)
       return hwnd;

    return hwnd;
 }

 static hWindow _System dialog(hWindow owner, HMODULE hMod, USHORT id, const DLGMGR *mgr, USHORT section)
 {
    HICQ    icq              = icqframe_getHandle(owner);
    char    buffer[0x0100];
	hWindow dialog;

    if( mgr && (mgr->sz != sizeof(DLGMGR) || mgr->szCalls != sizeof(icqskin_dialogCalls)))
    {
       DBGTrace(id);

       DBGTrace(mgr->sz);
       DBGTrace(sizeof(DLGMGR));

       DBGTrace(mgr->szCalls);
       DBGTrace(sizeof(icqskin_dialogCalls));

#ifdef __OS2__
       DosQueryModuleName(hMod, 0x80, buffer);
       sprintf(buffer+strlen(buffer),": Unexpected manager for dialog %d", (int) id);
#else
       sprintf(buffer,"Unexpected dialog manager when loading dialog %d",(int) id);
#endif

       DBGMessage(buffer);
       icqWriteSysLog(icq,PROJECT,buffer);

       return 0;
    }

    if(icqskin_loadString(icq, hMod, id, buffer, 0xFF))
	{
	   DBGMessage(buffer);
	
	   dialog = loadDialog(icq, owner, hMod, id, mgr);
	
	   DBGTracex(dialog);
	
	   if(dialog)
          icqskin_insertConfigPage(owner, section, dialog, id, buffer);
    }	

    return 0;
 }

 static void  _System setTextLimit(hWindow hwnd, USHORT id, USHORT sz)
 {
#ifdef __OS2__
    WinSendDlgItemMsg(hwnd,id,EM_SETTEXTLIMIT,MPFROMSHORT(sz),0);
#endif

 }

 static int lbInsert(hWindow hwnd, ULONG hdl, const char *txt, BOOL selected)
 {
#ifdef __OS2__

    short pos;

    DBGMessage(txt);
    DBGTracex(hwnd);

    pos = SHORT1FROMMR(WinSendMsg(hwnd,LM_INSERTITEM,MPFROMSHORT(LIT_END),MPFROMP(txt)));

    if(pos == LIT_ERROR || pos == LIT_MEMERROR)
       return -1;

    WinSendMsg(hwnd,LM_SETITEMHANDLE,MPFROMSHORT(pos),MPFROMLONG(hdl));

    if(selected)
       WinSendMsg(hwnd,LM_SELECTITEM,MPFROMSHORT(pos),MPFROMSHORT(TRUE));

#endif

    return 0;
 }

 static void  _System listBoxInsert(hWindow hwnd, USHORT id, ULONG hdl, const char *txt, BOOL selected)
 {
    lbInsert(CHILD_WINDOW(hwnd,id), hdl, txt, selected);
 }

 static void *  _System getUserData(hWindow hwnd)
 {
    char *ptr = icqskin_getWindowDataBlock(hwnd);
    if( !(((ICQEDIT *) ptr)->ft & 1) )
       return NULL;
    ptr += ( ((ICQEDIT *)ptr)->sz );
    return (void *)  ptr;
 }

 static void _System readCheckBox(hWindow hwnd, USHORT id, const char *key, BOOL def)
 {
    ICQEDIT *cfg = icqskin_getWindowDataBlock(hwnd);

    hwnd = CHILD_WINDOW(hwnd,id);

    if(icqLoadValue(cfg->icq,key,def ? 1 : 0))
       icqskin_setChecked(hwnd);
    else
       icqskin_setUnchecked(hwnd);
 }

 static void _System readRadioButton(hWindow hwnd, USHORT id, const char *key, USHORT sz, USHORT def)
 {
    ICQEDIT *cfg = icqskin_getWindowDataBlock(hwnd);
    int     pos  = icqLoadValue(cfg->icq,key,def);

    if(pos > sz)
       pos = def;

#ifdef __OS2__
    WinSendDlgItemMsg(hwnd,id+pos,BM_SETCHECK,MPFROMSHORT(1),0);
#endif

 }

 static void _System readString(hWindow hwnd, USHORT id, const char *key, const char *def)
 {
    ICQEDIT *cfg = icqskin_getWindowDataBlock(hwnd);
    char    buffer[0x0100];

    DBGTrace(id);

    VALIDATE_ID(id);

    hwnd = CHILD_WINDOW(hwnd,id);

    DBGMessage(key);
    DBGMessage(def);

    icqLoadString(cfg->icq, key, def, buffer, 0xFF);

    DBGMessage(buffer);
    icqskin_setWindowText(hwnd,buffer);

 }
	
 static void _System writeCheckBox(hWindow hwnd, USHORT id, const char *key)
 {
    ICQEDIT *cfg = icqskin_getWindowDataBlock(hwnd);

    VALIDATE_ID(id);

    hwnd = CHILD_WINDOW(hwnd,id);

    if(icqskin_queryCheckstate(hwnd))
       icqSaveValue(cfg->icq,key,1);
    else
       icqSaveValue(cfg->icq,key,0);

 }

 static void _System writeRadioButton(hWindow hwnd, USHORT id, const char *key, USHORT qtd)
 {
#ifdef __OS2__

    ICQEDIT *cfg = icqskin_getWindowDataBlock(hwnd);
    int     f;

    for(f=0;f<qtd;f++)
    {
       if(WinSendDlgItemMsg(hwnd,id+f,BM_QUERYCHECK,0,0))
       {
          DBGMessage(key);
          DBGTrace(f);
          icqSaveValue(cfg->icq,key,f);
          return;
       }
    }

#endif

 }

 static void _System writeString(hWindow hwnd, USHORT id, const char *key)
 {
    ICQEDIT *cfg = icqskin_getWindowDataBlock(hwnd);
    char    buffer[0x0100];

    *buffer = 0;
    icqskin_getWindowText(CHILD_WINDOW(hwnd,id),0xFF,buffer);

    DBGMessage(key);
    DBGMessage(buffer);

    icqSaveString(cfg->icq, key, buffer);

 }

 static int _System selectFile(HWND hwnd, USHORT id, BOOL save, const char *key, const char *masc, const char *title, char *vlr, int (* _System callBack)(const struct _dialogcalls *,HWND,HICQ,char *))
 {
    ICQEDIT *cfg = icqskin_getWindowDataBlock(hwnd);
    return icqskin_selectFile(hwnd, cfg->icq, id, save, key, masc, title, vlr, callBack);
 }

 static int _System setSpinButton(hWindow hwnd,USHORT id, int vlr, int min, int max)
 {

#ifdef __OS2__
    WinSendDlgItemMsg(hwnd,id,SPBM_SETLIMITS,MPFROMLONG(max),MPFROMLONG(min));
    WinSendDlgItemMsg(hwnd,id,SPBM_SETCURRENTVALUE,MPFROMLONG(vlr),0);
#endif

    return 0;
 }

 static int _System getSpinButton(hWindow hwnd, USHORT id)
 {
#ifdef __OS2__
    ULONG vlr = 0;
    WinSendDlgItemMsg(hwnd,id,SPBM_QUERYVALUE,MPFROMP(&vlr),MPFROM2SHORT(0,SPBQ_ALWAYSUPDATE));
    return (int) vlr;
#else
    return 0;	
#endif
 }

