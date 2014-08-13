/*
 * QUERY.C - Obtem informacoes sobre o nucleo
 */

 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>

 #include <icqkernel.h>

/*---[ Implementacao ]------------------------------------------------------------------------------------------*/

 ULONG EXPENTRY icqQueryUIN(HICQ icq)
 {
    return icq->uin;
 }

 ULONG * EXPENTRY icqQueryOnlineFlags(HICQ icq)
 {
    return &icq->onlineFlags;
 }

 void EXPENTRY icqQueryPassword(HICQ icq, char *buffer)
 {
    strcpy(buffer,icq->pwd);
 }

 int EXPENTRY icqQueryMaxMessageLength(HICQ icq, HUSER usr)
 {
    if(usr && usr->peer)
       return ((PEERPROTMGR *) usr->peer)->maxMsgSize;
    if(icq->c2s)
       return icq->c2s->maxSize;
    return 0x0100;
 }

 BOOL EXPENTRY icqClearToSend(HICQ icq)
 {
    return icq->cntlFlags & ICQFC_CLEAR;
 }

 long EXPENTRY icqQueryLocalIP(HICQ icq)
 {
    return icq->ipConfirmed;
 }

 ULONG EXPENTRY icqQueryModeFlags(HICQ icq)
 {
    char  key[20];

    if(!icq->offline || icq->currentMode == icq->offline->mode)
       return 0;

    sprintf(key,"Away:%04x.flg",(USHORT) icq->currentMode & 0xFFFF);

    return icqLoadValue(icq, key, ICQMF_AUTOOPEN|ICQMF_SOUND) | ICQMF_ONLINE;
//    return icqLoadValue(icq, key, 0) | ICQMF_ONLINE;
 }

 ULONG EXPENTRY icqGetCaps(HICQ icq)
 {
   /* ATENCAO: FUNCAO REPLICADA EM PWICQLIB.C */
    return icq->modeCaps;
 }

 ULONG EXPENTRY icqSetCaps(HICQ icq, ULONG caps)
 {
   /* ATENCAO: FUNCAO REPLICADA EM PWICQLIB.C */
    icq->modeCaps |= caps;
    return icq->modeCaps;
 }

 void * EXPENTRY icqGetSkinDataBlock(HICQ icq)
 {
    return icq->skinData;
 }

 void EXPENTRY icqSetSkinDataBlock(HICQ icq, void *dataBlock)
 {
    icq->skinData = dataBlock;
 }

 int EXPENTRY icqValidateMsgEditHelper(HICQ icq, USHORT sz)
 {
	CHKPoint();
	
    if(sz != sizeof(DIALOGCALLS))
       return 1;
	
	CHKPoint();
	
    if(icq->skin && icq->skin->validateEditHelper)
       return icq->skin->validateEditHelper(sz);
	
	CHKPoint();
	
    return 0;
 }

 CPGCONV * EXPENTRY icqSetCodePageConverter(HICQ icq, CPGCONV *convertCpg)
 {
    CPGCONV *ret = icq->convertCpg;
    icq->convertCpg = convertCpg;
    return ret;
 }


 int EXPENTRY icqIsOnline(HICQ icq)
 {
    if(icq->c2s && icq->c2s->isOnline)
       return icq->c2s->isOnline(icq);

    return icq->onlineFlags &= ICQF_ONLINE;
 }

 const C2SPROTMGR * EXPENTRY icqQueryProtocolManager(HICQ icq, USHORT id)
 {
    if(id)
       return NULL;
    return icq->c2s;
 }

 const SKINMGR * EXPENTRY icqQuerySkinManager(HICQ icq)
 {
    return icq->skin;
 }

 BOOL EXPENTRY icqQueryTextModeFlag(HICQ icq)
 {
    return icq->cntlFlags & ICQFC_NOGUI;
 }

 USHORT EXPENTRY icqQueryShortSequence(HICQ icq)
 {
    return icq->shortSeq++;
 }

 const void * ICQAPI icqQueryInternalTable(HICQ icq, USHORT id, char *buffer, int sz)
 {
    static const struct tbl
    {
       const char *key;
       const char *base;
    } values[]	= {	{ "sysMenuOptions", "50,27,11,38,16,17"  },
    			    { "usrMenuOptions", "50,31,17,16,47,2,0" }
    	          };

   if( id > (sizeof(values) / sizeof(struct tbl)) )
      *buffer = 0;
   else
      icqLoadString(icq, values[id].key, values[id].base, buffer, sz);

   return buffer;          	

 }

 ULONG ICQAPI icqQueryBuild(void)
 {
    return XBUILD;
 }

 const char * ICQAPI icqQueryURL(HICQ icq, const char *txt)
 {
    static const char *keywords[]  = { "http://", "https://", "ftp://" };
    const char        *ret         = NULL;
    int               f;

    for(f=0;!ret && f<(sizeof(keywords)/sizeof(const char *)); f++)
       ret = strstr(txt, keywords[f]);

    return ret;
 }


