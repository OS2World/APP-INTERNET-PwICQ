/*
 * UKNFORMAT.C - Formata mensagems desconhecidas
 */

 #include <stdio.h>

 #include <msgdlg.h>

#ifdef linux
 #include <icqgtk.h>
#else
 #include "wintlkt.h"
#endif

/*---[ Implement ]------------------------------------------------------------------------------------------*/

#ifdef linux
 int icqgtk_unknownMsgFormatter(const MSGEDITHELPER *dlg, MSGEDITWINDOW hwnd, HICQ icq, ULONG uin, USHORT type, BOOL out, HMSG msg)
#else
 int _System unknownMsgFormatter(const MSGEDITHELPER *dlg, MSGEDITWINDOW hwnd, HICQ icq, ULONG uin, USHORT type, BOOL out, HMSG msg)
#endif
 {
    static const USHORT hide[] = {      ICQMSGC_ENTRY,
                                        ICQMSGC_BROWSE,
                                        ICQMSGC_ACCEPT,
                                        ICQMSGC_REJECT,
                                        ICQMSGC_REFUSE,
                                        ICQMSGC_STATICSUBJ,
                                        ICQMSGC_SUBJECT,
                                        ICQMSGC_SEND,
                                        ICQMSGC_OPEN,
                                        0 };

    const USHORT           *wdg;
    BOOL                   in           = !out;

    for(wdg = hide; *wdg; wdg++)
       dlg->setVisible(hwnd,*wdg,FALSE);

    dlg->setVisible(hwnd,ICQMSGC_REPLY, in);
    dlg->setVisible(hwnd,ICQMSGC_NEXT,  in);

    dlg->loadUserFields(hwnd);

    if(msg)
       dlg->setString(hwnd,ICQMSGC_TEXT,(const char *)(msg+1));
    else
       dlg->setString(hwnd,ICQMSGC_TEXT,"");

    return 0;
 }



