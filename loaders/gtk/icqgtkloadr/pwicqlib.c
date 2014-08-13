/*
 * pwICQLIB.C - Biblioteca estática para acesso às funções do núcleo
 */

 #include <icqkernel.h>

/*---[ Tabela ]----------------------------------------------------------------------------*/

 const struct icqftable *pwicq = NULL;

/*---[ Implementacao ]---------------------------------------------------------------------*/

 int EXPENTRY pwICQConfigureLibrary(const struct icqftable *tbl)
 {
    if( tbl->sz == sizeof(struct icqftable)
        && tbl->Version      == ICQFTABLE_VERSION
        && tbl->szKrnl       == sizeof(ICQ)
                && tbl->szUser       == sizeof(USERINFO)
                && tbl->pwICQVersion == PWICQVERSIONID )
    {
       DBGMessage(tbl->ident);
       pwicq = tbl;
       return 0;
    }
    return -1;
 }

 const char * EXPENTRY icqQueryUserNick(HUSER usr)
 {
    return pwicq->icqQueryUserNick(usr);
 }

 HUSER EXPENTRY icqQueryNextUser(HICQ icq, HUSER usr)
 {
    return pwicq->icqQueryNextUser(icq,usr);
 }

 void * EXPENTRY icqGetSkinDataBlock(HICQ icq)
 {
    return pwicq->icqGetSkinDataBlock(icq);
 }

 void EXPENTRY icqSetSkinDataBlock(HICQ icq, void *ptr)
 {
    pwicq->icqSetSkinDataBlock(icq,ptr);
 }

 ULONG EXPENTRY icqQueryUIN(HICQ icq)
 {
    return icq->uin;
 }

 void EXPENTRY icqWriteSysLog(HICQ icq, const char *proj, const char *txt)
 {
    pwicq->icqWriteSysLog(icq,proj,txt);
 }

 BOOL EXPENTRY icqRegisterSkinManager(HICQ icq, const SKINMGR *mgr)
 {
    return pwicq->icqRegisterSkinManager(icq,mgr);
 }

 void EXPENTRY icqSetServerBusy(HICQ icq, BOOL modo)
 {
    pwicq->icqSetServerBusy(icq,modo);
 }

 HUSER EXPENTRY icqSetUserOnlineMode(HICQ icq, ULONG p1, ULONG p2)
 {
    return pwicq->icqSetUserOnlineMode(icq,p1,p2);
 }

 void EXPENTRY icqDumpPacket(HICQ icq, HUSER usr, const char *msg, int size, unsigned char *dbg)
 {
 }

 void EXPENTRY icqSystemEvent(HICQ icq, USHORT event)
 {
    pwicq->icqSystemEvent(icq,event);
 }

 void EXPENTRY icqUserEvent(HICQ icq, HUSER user, USHORT event)
 {
    pwicq->icqUserEvent(icq,user,event);
 }

 void EXPENTRY icqAbend(HICQ icq, ULONG msg)
 {
    pwicq->icqAbend(icq,msg);
 }

 int EXPENTRY icqQueryUserDataBlockLength(void)
 {
    return pwicq->icqQueryUserDataBlockLength();
 }

 HUSER EXPENTRY icqUserOnline(HICQ icq, ULONG uin, ULONG status, long ip, long gateway, short port, long tcpVersion, ULONG version)
 {
    return pwicq->icqUserOnline(icq, uin, status, ip, gateway, port, tcpVersion, version);
 }

 HLIST EXPENTRY icqCreateList(void)
 {
    return pwicq->icqCreateList();
 }

 HLIST EXPENTRY icqDestroyList(HLIST lst)
 {
    return pwicq->icqDestroyList(lst);
 }

 void EXPENTRY icqClearList(HLIST lst)
 {
    return pwicq->icqClearList(lst);
 }

 void * EXPENTRY icqAddElement(HLIST lst, int sz)
 {
    return pwicq->icqAddElement(lst,sz);
 }

 int EXPENTRY icqRemoveElement(HLIST lst, void *ptr)
 {
    return pwicq->icqRemoveElement(lst,ptr);
 }

 void * EXPENTRY icqQueryFirstElement(HLIST lst)
 {
    return pwicq->icqQueryFirstElement(lst);
 }

 void * EXPENTRY icqQueryLastElement(HLIST lst)
 {
    return pwicq->icqQueryLastElement(lst);
 }

 void * EXPENTRY icqQueryNextElement(HLIST lst, void *ptr)
 {
    return pwicq->icqQueryNextElement(lst,ptr);
 }

 void * EXPENTRY icqQueryPreviousElement(HLIST lst, void *ptr)
 {
    return pwicq->icqQueryPreviousElement(lst,ptr);
 }

 void * EXPENTRY icqResizeElement(HLIST lst, int sz, void *ptr)
 {
    return pwicq->icqResizeElement(lst,sz,ptr);
 }

 ULONG EXPENTRY _System icqLoadValue(HICQ icq, const char *key, ULONG def)
 {
    return pwicq->icqLoadValue(icq,key,def);
 }

 char * EXPENTRY icqLoadString(HICQ icq, const char *p1, const char *p2, char *p3, int p4)
 {
    return pwicq->icqLoadString(icq,p1,p2,p3,p4);
 }

 ULONG EXPENTRY icqSaveValue(HICQ icq, const char *key, ULONG def)
 {
    return pwicq->icqSaveValue(icq,key,def);
 }

 void EXPENTRY icqSaveString(HICQ icq, const char *key, const char *vlr)
 {
    pwicq->icqSaveString(icq,key,vlr);
 }

 void EXPENTRY icqSetServiceStatus(HICQ icq,int id, BOOL status)
 {
    pwicq->icqSetServiceStatus(icq,id,status);
 }

 BOOL EXPENTRY icqInsertMessage(HICQ icq, ULONG uin, USHORT type, time_t tm, ULONG flags, int sz, const char *txt)
 {
    return pwicq->icqInsertMessage(icq, uin, type, tm, flags, sz, txt);
 }

 BOOL EXPENTRY icqRegisterServerProtocolManager(HICQ icq, const C2SPROTMGR *mgr)
 {
    return pwicq->icqRegisterServerProtocolManager(icq, mgr);
 }

 void EXPENTRY icqRegisterUserAction(HUSER usr)
 {
    return pwicq->icqRegisterUserAction(usr);
 }

 BOOL EXPENTRY icqInsertAwayMessage(HICQ icq, ULONG uin, const char *title, char *txt)
 {
    return pwicq->icqInsertAwayMessage(icq, uin, title, txt);
 }

 long EXPENTRY icqQueryPeerPort(HICQ icq)
 {
     return pwicq->icqQueryPeerPort(icq);
 }

 UCHAR * EXPENTRY icqConvertCodePage(HICQ icq, BOOL in, UCHAR *txt, int sz)
 {
    return pwicq->icqConvertCodePage(icq, in, txt, sz);
 }

 BOOL EXPENTRY icqDeRegisterServerProtocolManager(HICQ icq, const C2SPROTMGR *mgr)
 {
    return pwicq->icqDeRegisterServerProtocolManager(icq, mgr);
 }

 BOOL EXPENTRY icqIsActive(HICQ icq)
 {
    return pwicq->icqIsActive(icq);
 }

 int EXPENTRY icqCreateThread(HICQ icq, void (* _System start_address)(ICQTHREAD *), unsigned int stack_size, int sz, void *arglist, const char *id)
 {
    return pwicq->icqCreateThread(icq, start_address, stack_size, sz, arglist, id);
 }

 void EXPENTRY icqProcessServerConfirmation(HICQ icq, ULONG uin, ULONG id)
 {
    pwicq->icqProcessServerConfirmation(icq, uin, id);
 }

 ULONG EXPENTRY icqReconnect(HICQ icq)
 {
    return pwicq->icqReconnect(icq);
 }

 BOOL EXPENTRY icqSetLocalIP(HICQ icq, long ip)
 {
    return pwicq->icqSetLocalIP(icq, ip);
 }

 void EXPENTRY icqQueryPassword(HICQ icq, char *buffer)
 {
    pwicq->icqQueryPassword(icq, buffer);
 }

 ULONG EXPENTRY icqSetOnlineMode(HICQ icq, ULONG mode)
 {
    return pwicq->icqSetOnlineMode(icq, mode);
 }

 BOOL EXPENTRY icqClearToSend(HICQ icq)
 {
    return pwicq->icqClearToSend(icq);
 }

 HUSER EXPENTRY icqQueryUserHandle(HICQ icq, ULONG uin)
 {
    return pwicq->icqQueryUserHandle(icq,uin);
 }

 HUSER EXPENTRY icqUpdateUserInfo(HICQ icq, ULONG uin, BOOL add, const char *nickName, const char *firstName, const char *lastName, const char *email, BOOL authorize)
 {
    return pwicq->icqUpdateUserInfo(icq, uin, add, nickName, firstName, lastName, email, authorize);
 }

 ULONG EXPENTRY icqQueryOnlineMode(HICQ icq)
 {
    return pwicq->icqQueryOnlineMode(icq);
 }

 int EXPENTRY icqOpenURL(HICQ icq, const char *url)
 {
    return pwicq->icqOpenURL(icq,url);
 }


