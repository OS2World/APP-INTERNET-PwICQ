/*
 * MAINWIN.C - Processamento da janela principal
 */

 #define INCL_WIN
 #define INCL_WINDIALOGS     /* Window Dialog Mgr Functions  */
 #define INCL_GPI
 #include <os2.h>

 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include <skinapi.h>


 #include "msgdlg.h"
 #include "icqskmgr.h"

/*---[ Prototipos ]---------------------------------------------------------------------------------*/

 static void    configure(HWND, ICQWNDINFO *, HICQ, HWND);

 static void    drawItem(HWND,USHORT,POWNERITEM);
 static MRESULT querySize(HWND, USHORT, USHORT);
 static void    loadMenu(HWND, HAB, USHORT, HPS, int, char *, USHORT *, char *, PPOINTL);
 static void    action(HWND,USHORT,ULONG);
 static void    drawUser(ICQWNDINFO *, HUSER, HPS, PRECTL, ULONG);
 static void    processControl(HWND, USHORT, USHORT);
 static void    sysButton(HWND,ICQWNDINFO *);
 static void    procAccelerator(HWND, ICQWNDINFO *,USHORT);
 static void    callMenu(HWND,MENUCALLBACK *,ULONG);
 static void    setIcon(HWND, ICQWNDINFO *,USHORT);
 static void    chk4Timer(HWND);
 static void    openChildWindow(HWND,struct openWindowParameters *);
 static void    popupMenu(HWND,ICQWNDINFO *,USHORT, ULONG);
 static void    openMessage(HICQ);

 static void    loadMenuOption(HWND, HAB, USHORT, HPS, int, USHORT, USHORT *, char *, PPOINTL);


/*---[ Implementacao ]------------------------------------------------------------------------------*/

 MRESULT EXPENTRY sknProcedure(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    switch(msg)
    {

    /*---[ Skin messages ]-----------------------------------------*/

    case WMSKN_LOADSKIN:
       sknLoad(hwnd, ((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->skn, winElements, PVOIDFROMMP(mp1));
       break;

    case WMSKN_ACTION:
       action(hwnd,SHORT1FROMMP(mp1),LONGFROMMP(mp2));
       break;

    case WMSKN_ACTIVATEHANDLER: /* MP1=Handler, MP2=ItemID */
       callMenu(hwnd,(MENUCALLBACK *) PVOIDFROMMP(mp1),LONGFROMMP(mp2));
       break;

    case WMSKN_QUERYDRAG:
       if(SHORT1FROMMP(mp2) == 1003)
       {
          ((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->flags |= WFLG_DRAG;
          return (MRESULT) TRUE;
       }
       return 0;

    case WMSKN_SETFRAMEICON: /* Change frame icon, MP1 = IconID */
       setIcon(hwnd, (ICQWNDINFO *) WinQueryWindowPtr(hwnd,0),(USHORT) mp1);
       break;

    case WMSKN_SETFRAMETEXT: /* Change frame title, MP1 = String */
       WinSetWindowText(((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->frame, (PSZ) mp1);
       break;

    case WM_SIZE:
       sknResize(hwnd, SHORT1FROMMP(mp2), SHORT2FROMMP(mp2),((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->skn,winElements);
       break;

//    case WM_USER+23:  /* Load skin */
//       DBGMessage(mp1);
//       sknLoad(hwnd, ((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->skn,  winElements, (const char *) mp1);
//       break;

    /*---[ Window messages ]---------------------------------------*/

    case WM_SAVEAPPLICATION:
       icqWriteSysLog(((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->icq,PROJECT,"System is terminating");
       icqSystemEvent(((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->icq,ICQEVENT_SAVE);
       WinPostMsg(hwnd,WM_CLOSE,0,0);
       return sknDefWindowProc(hwnd, ((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->skn, msg, mp1, mp2);

    case WM_CLOSE:
       icqStoreWindow(hwnd, ((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->icq, 0, "pos:MainWindow");
       icqSystemEvent(((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->icq, ICQEVENT_CLOSING);
       return sknDefWindowProc(hwnd, ((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->skn, msg, mp1, mp2);

    case WM_CREATE:
       create(hwnd,PVOIDFROMMP(mp2));
       return WinDefWindowProc(hwnd, msg, mp1, mp2);

    case WM_CONTROL:
       processControl(hwnd,SHORT1FROMMP(mp1),SHORT2FROMMP(mp1));
       break;

    case WM_DESTROY:
       destroy(hwnd);
       return WinDefWindowProc(hwnd, msg, mp1, mp2);

    case WM_MEASUREITEM: /* MP1=ID, MP2=Index */
       return querySize(hwnd,SHORT1FROMMP(mp1),SHORT1FROMMP(mp1));

    case WM_DRAWITEM:
       drawItem(hwnd,SHORT1FROMMP(mp1),PVOIDFROMMP(mp2));
       break;

    case WM_COMMAND:
       if(SHORT1FROMMP(mp2) == CMDSRC_ACCELERATOR)
          procAccelerator(hwnd,WinQueryWindowPtr(hwnd,0),SHORT1FROMMP(mp1));
       else if(SHORT1FROMMP(mp1) != 103)
          return sknDefWindowProc(hwnd, ((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->skn, msg, mp1, mp2);
       else
          icqOpenSystemMessage(((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->icq);
       break;

    case WM_TIMER:
       doTimer(hwnd);
       break;

    /*---[ Internas ao plugin ]------------------------------------*/

    case WM_USER+10:
       configure(hwnd,WinQueryWindowPtr(hwnd,0),PVOIDFROMMP(mp1), LONGFROMMP(mp2));
       break;

    case WM_USER+11:
       updateUserList(hwnd, WinQueryWindowPtr(hwnd,0));
#ifdef EXTENDED_LOG
       icqWriteSysLog(((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->icq,PROJECT,"Redrawing user list");
#endif
       break;

    case WM_USER+12:    /* Timer */
       chk4Timer(hwnd);
       break;

    case WMICQ_GUIEVENT:
       /*
        * GUI Event - Used by OS2WIN.DLL to serialize some operations
        *             (like creating new windows)
        *
        * The only needed action is pass it to the OS2WIN event processor
        */
       icqProcessGuiEvent(((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->icq,0,SHORT2FROMMP(mp1),SHORT1FROMMP(mp1),(ULONG) mp2);
       break;

    case WMICQ_SYSTEMEVENT:    /* System event - MP1 = Code */
       systemEvent(hwnd, WinQueryWindowPtr(hwnd,0), SHORT1FROMMP(mp1));
       icqProcessGuiEvent(((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->icq,0,'S',SHORT1FROMMP(mp1),(ULONG) mp2);
       break;

    case WMICQ_USEREVENT:    /* User Event - MP1 = Code, MP2 = UIN */
       userEvent(hwnd,SHORT1FROMMP(mp1),LONGFROMMP(mp2));
       icqProcessGuiEvent(((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->icq,(ULONG) mp2,'U',SHORT1FROMMP(mp1),0);
       break;

    case WM_USER+15:    /* Warning Window - MP1 = UIN MP2 = ID */
       DBGMessage("Abrir janela de warning");
       break;

    case WM_USER+17:    /* Mudar titulo/icone */
       icqAjustTaskList( ( (ICQWNDINFO *) WinQueryWindowPtr(hwnd,0) )->icq, hwnd, (const char *) mp1);
       break;

    case WM_USER+18:    /* Open user-away dialog box (MP1=parm) */
       openUserAwayDialog(hwnd,PVOIDFROMMP(mp1));
       break;

    case WM_USER+19:    /* Read icons */
       DBGMessage("Carregar tabela de ¡cones");
       WinPostMsg(hwnd,WM_USER+17,0,0);
       break;

    case WM_USER+20:
       break;

    case WM_USER+21:   /* Popup menu mp1 = menu mp2=UIN */
       DBGMessage("Popup Menu");
       popupMenu(hwnd,(ICQWNDINFO *) WinQueryWindowPtr(hwnd,0),SHORT1FROMMP(mp1),LONGFROMMP(mp2));
       break;

    case WM_USER+22:   /* Open Window */
       openChildWindow(hwnd,(struct openWindowParameters *) mp1);
       break;


    /*---[ DDE Processing ]----------------------------------------*/
    case WM_DDE_INITIATEACK:
       return (MRESULT) icqDDEInitiateAck(hwnd, (HWND)mp1, (PDDEINIT)mp2);

    case WM_DDE_DATA:
       icqDDEData(hwnd, (PDDESTRUCT) mp2);
       break;

    /*---[ SysBar ]------------------------------------------------*/

    case WM_BUTTON2CLICK | 0x2000:
       icqShowPopupMenu(((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->icq, 0, ICQMNU_MODES, 0, 0);
       return (MRESULT) TRUE;

    case WM_BUTTON1CLICK | 0x2000:
       openMessage(((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->icq);
       return (MRESULT) TRUE;

    case 0xCD20:
       icqDisableSysTray(((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->icq);
       return (MRESULT) TRUE;

    /*---[ Drag&Drop ]---------------------------------------------*/

    case DM_DRAGOVER:
       return dragOver(hwnd, (ICQWNDINFO *) WinQueryWindowPtr(hwnd,0), (PDRAGINFO)mp1);

    case DM_DRAGLEAVE:
       return dragLeave(hwnd, (ICQWNDINFO *) WinQueryWindowPtr(hwnd,0));

    case DM_DROP:
       return drop(hwnd, (ICQWNDINFO *) WinQueryWindowPtr(hwnd,0));

    default:
       return sknDefWindowProc(hwnd, ((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->skn, msg, mp1, mp2);

    }
    return 0;
 }

/*
 static void loadIcons(HWND hwnd, ICQWNDINFO *wnd)
 {
    WinPostMsg(hwnd,WM_USER+17,0,0);
 }
*/

 static void configure(HWND hwnd, ICQWNDINFO *ctl, HICQ icq, HWND frame)
 {
    /* Configura a janela */
//    const SHORT        sysMenu[]        = { 11, 38, 16, 17, -1 };
//    const SHORT        usrMenu[]        = { 17, 16, 29, 8, 47, 2, 0, -1 };

    HAB                 hab             = WinQueryAnchorBlock(hwnd);
    const ICQMODETABLE  *mode;
    POINTL              aptl[TXTBOX_COUNT];
    USHORT              cols;
    HPS                 hps;
    char                itens[0x0100];
#ifdef EXTENDED_LOG
    char                logBuffer[0x0100];
#endif

    ctl->icq            = icq;
    ctl->frame          = frame;

    DBGTracex(ctl->frame);

    /* Carrega o menu de opcoes */
    hps                 = WinGetPS(sknQueryMenuWindow(hwnd,101));
    ctl->cols[0]        = 0;
    for(mode = icqQueryModeTable(icq);mode && mode->descr;mode++)
    {
       GpiQueryTextBox(    hps,
                           strlen(mode->descr),
                           (char *) mode->descr,
                           TXTBOX_COUNT,
                           aptl);

       cols = (aptl[TXTBOX_TOPRIGHT].x - aptl[TXTBOX_TOPLEFT].x);
       ctl->cols[0] = max(ctl->cols[0],cols);
       sknInsertMenuItem(hwnd,101,mode,0);
    }
    WinReleasePS(hps);

    /* Carrega o menu de sistema */
    hps                 = WinGetPS(sknQueryMenuWindow(hwnd,102));

    icqQueryInternalTable(icq, 0, itens, 0x7F);
#ifdef EXTENDED_LOG
    sprintf(logBuffer,"System menu options: %s",itens);
    icqWriteSysLog(icq,PROJECT,logBuffer);
#endif
    loadMenu(hwnd, hab, 102,  hps, 1000, itens, &ctl->cols[1],ctl->buffer,aptl);

    icqQueryInternalTable(icq, 1, itens, 0x7F);
#ifdef EXTENDED_LOG
    sprintf(logBuffer,"User menu options: %s",itens);
    icqWriteSysLog(icq,PROJECT,logBuffer);
#endif
    loadMenu(hwnd, hab, 1003, hps, 1100, itens, &ctl->cols[2],ctl->buffer,aptl);

    WinReleasePS(hps);

    setModeButton(hwnd,ctl,icqQueryOfflineMode(icq));

 }


 static void loadMenuOption(HWND hwnd, HAB hab, USHORT id, HPS hps, int base, USHORT opt, USHORT *cols, char *buffer, PPOINTL aptl)
 {
    USHORT cl;

    *buffer = 0;
    if(WinLoadString(hab, module, base + opt, 0xFF, (PSZ) buffer) )
    {
       GpiQueryTextBox(    hps,
                           strlen(buffer),
                           (char *) buffer,
                           TXTBOX_COUNT,
                           aptl);

       cl = (aptl[TXTBOX_TOPRIGHT].x - aptl[TXTBOX_TOPLEFT].x);
       *cols = max(*cols,cl);
       sknInsertMenuItem(hwnd,id,opt,buffer);
    }
 }

 static void loadMenu(HWND hwnd, HAB hab, USHORT id, HPS hps, int base, char *itens, USHORT *cols, char *buffer, PPOINTL aptl)
 {
    USHORT opt;
    char   *token;

    *cols = 0;

    token = strtok(itens, ",");
    while(token)
    {
       loadMenuOption(hwnd, hab, id, hps, base, atoi(token), cols, buffer, aptl);
       token = strtok(NULL, ",");
    }

 }

/*
 static void loadMenu(HWND hwnd, HAB hab, USHORT id, HPS hps, int base, const SHORT *list, USHORT *cols, char *buffer, PPOINTL aptl)
 {
    const SHORT         *opt;

    *cols = 0;

    for(opt = list; *opt >= 0; opt++)
       loadMenuOption(hwnd, hab, id, hps, base, *opt, cols, buffer, aptl);
 }
*/

 static MRESULT querySize(HWND hwnd, USHORT id, USHORT idx)
 {
    ICQWNDINFO  *ctl = WinQueryWindowPtr(hwnd,0);
    USHORT      rows = sknQueryIconSize(ctl->skn)+4;

    switch(id)
    {
    case 1003:  /* Lista de usuarios */
       return MRFROM2SHORT(rows,1024);

    case 2000:  /* Menu de usuario */
       return MRFROM2SHORT(rows,ctl->cols[2]);

    case 2002: /* Menu de sistema */
       return MRFROM2SHORT(rows,ctl->cols[1]);

    case 2001:  /* Menu de modo */
       return MRFROM2SHORT(rows,ctl->cols[0]);
    }

    return 0;
 }

 static void drawItem(HWND hwnd, USHORT id, POWNERITEM i)
 {
    const ICQMODETABLE  *mode;
    ICQWNDINFO          *ctl = WinQueryWindowPtr(hwnd,0);

    switch(id)
    {
    case 1003:  /* Listbox */
       drawUser(ctl, (HUSER) i->hItem, i->hps, &i->rclItem, i->fsState);
       break;


    case 2000:  /* User menu */
    case 2002:  /* System menu */
       i->rclItem.xLeft += sknDrawIcon(ctl->skn, i->hps, i->idItem, i->rclItem.xLeft+4,i->rclItem.yBottom+3);
       i->rclItem.xLeft += 8;
       WinDrawText(   i->hps,
                      -1,
                      (char *) i->hItem,
                      &i->rclItem,
                      i->fsState & 1 ? SKC_SELECTEDFOREGROUND : SKC_FOREGROUND,
                      2,
                      DT_LEFT|DT_VCENTER);

       break;

    case 2001:  /* Mode menu */
       mode = (const ICQMODETABLE *) i->idItem;

       i->rclItem.xLeft += sknDrawIcon(ctl->skn, i->hps, mode->icon, i->rclItem.xLeft+4,i->rclItem.yBottom+3);
       i->rclItem.xLeft += 8;

       WinDrawText(   i->hps,
                      -1,
                      (char *) mode->descr,
                      &i->rclItem,
                      i->fsState & 1 ? SKC_SELECTEDFOREGROUND : SKC_FOREGROUND,
                      2,
                      DT_LEFT|DT_VCENTER);

       break;

    case 101:   /* Botao de selecao de modo */

       if(!(i->idItem & 8000))
       {
          mode = (const ICQMODETABLE *) i->hItem;

          if(mode)
          {
             if(ctl->flags & WFLG_CONNECTING)
             {
                i->rclItem.xLeft += sknDrawIcon(ctl->skn, i->hps, 18+(ctl->modeTicks&7) , i->rclItem.xLeft+2,i->rclItem.yBottom+3);
                i->rclItem.xLeft += 2;
             }
             else
             {
                i->rclItem.xLeft += sknDrawIcon(ctl->skn, i->hps, mode->icon, i->rclItem.xLeft+2,i->rclItem.yBottom+3);
                i->rclItem.xLeft += 2;
             }

             WinDrawText(   i->hps,
                            -1,
                            (char *) mode->descr,
                            &i->rclItem,
                            SKC_FOREGROUND,
                            2,
                            DT_LEFT|DT_VCENTER);
          }
       }
       break;

    case 103:   /* Botao de sistema */

       ctl->iconSize     =  sknDrawIcon(ctl->skn, i->hps, ctl->modeIcon, i->rclItem.xLeft+2,i->rclItem.yBottom+3)+2;

       if(ctl->eventIcon != ctl->modeIcon && (ctl->timer & 1) )
          sknDrawIcon(ctl->skn, i->hps, ctl->eventIcon, i->rclItem.xLeft+2,i->rclItem.yBottom+3)+2;

       i->rclItem.xLeft += (ctl->iconSize+4);

       WinDrawText(   i->hps,
                      -1,
                      (char *) i->hItem,
                      &i->rclItem,
                      SKC_USER4,
                      2,
                      DT_LEFT|DT_VCENTER);
       break;

    default:
       sknDefWindowProc(hwnd, ((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->skn, WM_DRAWITEM, MPFROMSHORT(id), MPFROMP(i));

    }

 }

 static void action(HWND hwnd, USHORT id, ULONG parm)
 {
    ICQWNDINFO *ctl = WinQueryWindowPtr(hwnd,0);

    switch(id)
    {
    case 2001:  /* Menu de selecao de modo */
       DBGTracex( ((ICQMODETABLE * ) parm)->mode );
       icqSetOnlineMode(ctl->icq, ((ICQMODETABLE * ) parm)->mode );
       break;

    case 2002:  /* Menu de sistema */
       procSysMenu(hwnd,ctl,parm);
       break;

    case 2000:  /* Menu de usuario */
       procUsrMenu(hwnd, sknQuerySelectedHandle(hwnd,1003), ctl, parm);
       break;
    }

 }

 static void drawUser(ICQWNDINFO *ctl, HUSER usr, HPS hps, PRECTL rcl, ULONG flags)
 {
    HSKIN       skn     = ctl->skn;
    ULONG       offline = icqQueryOfflineMode(ctl->icq);
    ULONG       color;
    USHORT      left;

    if(flags & 0x80000000)
    {
       /* It's drawing in the cache bitmap, draw only images */
       left = sknDrawIcon(skn, hps, usr->modeIcon, rcl->xLeft+4,rcl->yBottom+3);

       if(flags&2)
          sknDrawIcon(skn, hps, usr->eventIcon, rcl->xLeft+4,rcl->yBottom+3);

       rcl->xLeft += (8+left);

    }

//    usr->flags |= USRF_BLINK;

    if(flags & 0x40000000)
    {
       /* It's drawing on the screen, the images was already draw from the cache bitmap */
       if(flags & 1)
       {
          color = SKC_SELECTEDFOREGROUND;
       }
       else if(usr->flags & USRF_BLINK)
       {
          ctl->flags |= WFLG_BLINKTEXT;
          if(usr->mode == offline)
             color =  (ctl->timer&1) ? SKC_USER1 : SKC_USER3;
          else
             color =  (ctl->timer&1) ? SKC_USER3 : SKC_USER1;
       }
       else if(usr->mode == offline)
       {
          color = (usr->flags & USRF_WAITING) ? SKC_USER5 : SKC_USER1;
       }
       else
       {
          color = (usr->flags & USRF_WAITING) ? SKC_USER6 : SKC_USER3;
       }

//        CMD_PALLETE, SKC_USER1,         0,                      "OfflineColor",
//        CMD_PALLETE, SKC_USER3,         0,                      "OnlineColor",
//        CMD_PALLETE, SKC_USER4,         0,                      "SysMessageColor",
//        CMD_PALLETE, SKC_USER5,         0,                      "OfflineWaitingColor",
//        CMD_PALLETE, SKC_USER6,         0,                      "OnlineWaitingColor",

       WinDrawText(   hps,
                   -1,
                   (PSZ) icqQueryUserNick(usr),
                   rcl,
                   color,
                   2,
                   DT_LEFT|DT_VCENTER);
    }

 }

 static void processControl(HWND hwnd, USHORT id, USHORT cmd)
 {
    HUSER usr;

    if(id != 1003 || cmd != LN_ENTER)
       return;

    usr = sknQuerySelectedHandle(hwnd,1003);

    if(usr)
       icqOpenUserMessageWindow( ((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->icq, usr);

 }

 static void procAccelerator(HWND hwnd, ICQWNDINFO *ctl, USHORT id)
 {
    DBGTrace(id);

    switch(id)
    {
    case 2000:
       DBGMessage("Recarregar Skin");
       querySkinName(ctl->icq, ctl->buffer);
       sknReloadSkin(hwnd, ctl->buffer, ctl->skn, winElements);
       break;

    case 2001:
       ctl->flags |= WFLG_SORT;
       break;

    case 2002:
       DBGMessage("Redesenhar listbox");
       sknRedrawControl(hwnd,1003);
       break;

    case ICQMSGC_CLOSE:
       WinPostMsg(hwnd,WM_CLOSE,0,0);
       break;

    case ICQMSGC_SEND:
       processControl(hwnd, 1003, LN_ENTER);
       break;

    }
 }

 static void callMenu(HWND hwnd, MENUCALLBACK *cbk, ULONG item)
 {
    HUSER usr = sknQuerySelectedHandle(hwnd,1003);
    ULONG uin = 0;

    if(usr)
       uin = usr->uin;

    if(cbk)
       cbk(((ICQWNDINFO *) WinQueryWindowPtr(hwnd,0))->icq, uin, item, uin);

 }


 static void chk4Timer(HWND hwnd)
 {
    ICQWNDINFO *ctl = WinQueryWindowPtr(hwnd,0);

    if(ctl->flags & WFLG_REDRAW)
       WinPostMsg(hwnd,WM_USER+11,0,0);

    if(ctl->flags & WFLG_SORT)
    {
       DBGMessage("Reordenar a lista");
       ctl->flags &= ~WFLG_SORT;
       sknSort(hwnd,1003);
       checkUserList(hwnd, ctl);
    }

 }

 static void openChildWindow(HWND hwnd,struct openWindowParameters *parm)
 {

    ICQWNDINFO *ctl = WinQueryWindowPtr(hwnd,0);

    DBGMessage("*** Recebida requisicao de abertura de janela");
    DBGTracex(parm);

    if(!parm || (parm->sz != sizeof(struct openWindowParameters)) )
    {
       icqWriteSysLog(ctl->icq,PROJECT,"Invalid open child request");
       return;
    }

    DBGTracex(parm->callback);
    parm->callback(ctl->icq,hwnd,parm->id,parm->arg);
    parm->flag = 0;

    CHKPoint();
 }

 static void popupMenu(HWND hwnd, ICQWNDINFO *wnd, USHORT menu, ULONG uin)
 {
    DBGTrace(menu);
    DBGTracex(WinWindowFromID(hwnd,menu));
    WinSendDlgItemMsg(hwnd,menu,WMSKN_SHOWMENU,0,0);
 }

 static void setIcon(HWND hwnd, ICQWNDINFO *wnd, USHORT icon)
 {
    if(wnd->icon)
       WinDestroyPointer(wnd->icon);
    wnd->icon = icqCreatePointer(wnd->icq, icon);
    WinSendMsg(wnd->frame,WM_SETICON,MPFROMP(wnd->icon),0);
 }

 static void openMessage(HICQ icq)
 {
    HMSG        msg     = 0;
    ULONG       uin;

    if(icqQueryFirstMessage(icq,&uin,&msg))
    {
       CHKPoint();
       icqPopupUserList(icq);
       return;
    }
    CHKPoint();
    icqOpenMessage(icq, uin, msg);
 }


