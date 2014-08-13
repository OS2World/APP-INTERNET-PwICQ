/*
 * cfgEvent.c - Configuration Window event processor
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>
 #include <time.h>
 #include <ctype.h>

 #include <pwicqgui.h>

/*---[ Defines ]------------------------------------------------------------------------------------------*/

#ifdef __OS2__
   #define CHILD_WINDOW(h,i) WinWindowFromID(h,DLG_ELEMENT_ID+i)
#else
   #define CHILD_WINDOW(h,i) icqskin_getDlgItemHandle(h,i)
#endif

 #define icqDeclareCfgSystemEvent(e)           void icqCfgEditSysEvent_##e(HICQ,HWND,ICQCONFIGDIALOG *)
 #define icqCfgSystemEvent(e)                  { 'S', e, (void(*)(void *,HWND,ICQCONFIGDIALOG *)) icqCfgEditSysEvent_##e }
 #define icqBeginCfgSystemEvent(m,i,h,p)       void icqCfgEditSysEvent_##m(HICQ i,HWND h,ICQCONFIGDIALOG *p)

 #define icqDeclareCfgUserEvent(e)             void icqCfgEditUserEvent_##e(HICQ,HWND,ICQCONFIGDIALOG *)
 #define icqCfgUserEvent(e)                    { 'U', e, (void(*)(void *,HWND,ICQCONFIGDIALOG *)) icqCfgEditUserEvent_##e }
 #define icqBeginCfgUserEvent(m,i,h,p)         void icqCfgEditUserEvent_##m(HICQ i,HWND h, ICQCONFIGDIALOG *p)

/*---[ Event Processors ]---------------------------------------------------------------------------------*/

 icqDeclareCfgSystemEvent(ICQEVENT_ONLINE);
 icqDeclareCfgSystemEvent(ICQEVENT_OFFLINE);
 icqDeclareCfgUserEvent(ICQEVENT_UPDATED);
 icqDeclareCfgUserEvent(ICQEVENT_ONLINE);
 icqDeclareCfgUserEvent(ICQEVENT_OFFLINE);
 icqDeclareCfgUserEvent(ICQEVENT_CHANGED);

/*---[ Event Processor table ]----------------------------------------------------------------------------*/

 #pragma pack(1)

 static const struct icqConfigurationEventProcessor
 {
    UCHAR  type;
    USHORT event;
    void   (*exec)(HICQ,HWND, ICQCONFIGDIALOG *);
 } procTable[] =
 {
   icqCfgSystemEvent(ICQEVENT_ONLINE),
   icqCfgSystemEvent(ICQEVENT_OFFLINE),
   icqCfgUserEvent(ICQEVENT_UPDATED),
   icqCfgUserEvent(ICQEVENT_ONLINE),
   icqCfgUserEvent(ICQEVENT_OFFLINE),
   icqCfgUserEvent(ICQEVENT_CHANGED)
 };

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/


 void _System icqskin_cfgEventProcessor(HICQ icq, ULONG uin, UCHAR id, USHORT event, ULONG parm, hWindow hwnd)
 {
    int                                         f;
    const struct icqConfigurationEventProcessor *ptr = procTable;
	ICQCONFIGDIALOG                             *cfg = icqskin_getWindowDataBlock(hwnd);

    if(id == 'U' && cfg->uin != uin)
       return;

#ifdef __OS2__
    WinPostMsg(hwnd,WMICQ_DLGEVENT,MPFROM2SHORT(((USHORT)id),event),(MPARAM) parm);
#endif

    for(f=0;f < (sizeof(procTable) / sizeof(struct icqConfigurationEventProcessor)); f++)
    {
       if(ptr->type == id && ptr->event == event)
       {
          ptr->exec(icq,hwnd,cfg);
          return;
       }
       ptr++;
    }

 }

 icqBeginCfgSystemEvent(ICQEVENT_ONLINE,icq,hwnd,cfg)
 {
    icqskin_setEnabled(CHILD_WINDOW(hwnd,MSGID_REFRESH),     TRUE);
    icqskin_setEnabled(CHILD_WINDOW(hwnd,MSGID_ABOUTBUTTON), TRUE);
    icqskin_setEnabled(CHILD_WINDOW(hwnd,MSGID_MESSAGE),     TRUE);
 }

 icqBeginCfgSystemEvent(ICQEVENT_OFFLINE,icq,hwnd,cfg)
 {
    icqskin_setEnabled(CHILD_WINDOW(hwnd,MSGID_ABOUTBUTTON), FALSE);
    icqskin_setEnabled(CHILD_WINDOW(hwnd,MSGID_REFRESH),     FALSE);
    icqskin_setEnabled(CHILD_WINDOW(hwnd,MSGID_MESSAGE),     FALSE);
 }

 static void setUserIcon(hWindow hwnd, HICQ icq, ULONG uin)
 {
    HUSER usr = icqQueryUserHandle(icq,uin);

    if(usr)
       icqskin_setDlgItemIcon(hwnd,MSGID_USERMODE,usr->modeIcon);
    else
       icqskin_setDlgItemIcon(hwnd,MSGID_USERMODE,ICQICON_OFFLINE);
 }

 icqBeginCfgUserEvent(ICQEVENT_ONLINE,icq,hwnd,cfg)
 {
    CHKPoint();
    setUserIcon(hwnd,cfg->fr.icq,cfg->uin);
 }

 icqBeginCfgUserEvent(ICQEVENT_OFFLINE,icq,hwnd,cfg)
 {
    CHKPoint();
    setUserIcon(hwnd,cfg->fr.icq,ICQICON_OFFLINE);
 }

 icqBeginCfgUserEvent(ICQEVENT_CHANGED,icq,hwnd,cfg)
 {
    CHKPoint();
    setUserIcon(hwnd,cfg->fr.icq,cfg->uin);
 }


 icqBeginCfgUserEvent(ICQEVENT_UPDATED,icq,hwnd,cfg)
 {
    HUSER usr;
    char  buffer[0x0100];

    if(!cfg->uin)
       return;

    CHKPoint();

    usr = icqQueryUserHandle(icq,cfg->uin);

    if(usr)
    {
       icqskin_setMsgWindowText(hwnd,MSGID_NICKENTRY,icqQueryUserNick(usr));
       strncpy(buffer,icqQueryUserFirstName(usr),0xFF);
       strncat(buffer," ",0xFF);
       strncat(buffer,icqQueryUserLastName(usr),0xFF);
       *(buffer+0xFF) = 0;
       icqskin_setMsgWindowText(hwnd,MSGID_NAMEENTRY,buffer);
    }
    else
    {
       icqskin_setMsgWindowText(hwnd,MSGID_NICKENTRY,"N/A");
       icqskin_setMsgWindowText(hwnd,MSGID_NAMEENTRY,"N/A");
    }

 }
