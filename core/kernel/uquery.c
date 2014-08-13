/*
 * UQUERY.C - Get/Set user informations
 */

 #include <stdio.h>
 #include <string.h>

 #include <icqkernel.h>

/*---[ Prototipos ]---------------------------------------------------------------------------------------*/

 static const char *GetSubString(HUSER, int);

/*---[ Implementacao ]------------------------------------------------------------------------------------*/

 int EXPENTRY icqRequestUserInfo(HICQ icq, ULONG uin)
 {
    char  buffer[0x0100];
    HUSER usr = icqQueryUserHandle(icq, uin);

    sprintf(buffer,"Requesting information about ICQ#%ld",uin);
    icqSysLog(icq,buffer);

    if(usr)
    {
       icq->cntlFlags |= ICQFC_SAVEUSER;
       usr->u.flags   |= (USRF_REFRESH|USRF_SEARCHING);
    }

    if(usr && usr->c2s && usr->c2s->requestUserInfo)
       return usr->c2s->requestUserInfo(icq,uin,icq->searchs);
    else if(icq->c2s && icq->c2s->requestUserInfo)
       return icq->c2s->requestUserInfo(icq,uin,icq->searchs);

    icqWriteSysLog(icq,PROJECT,"Unable to request user info (No available protocol manager");

    return -1;
 }


 int EXPENTRY icqRequestUserUpdate(HICQ icq, ULONG uin)
 {
    char  buffer[0x0100];
	HSEARCH sr  = NULL;
    HUSER   usr = icqQueryUserHandle(icq, uin);

    DBGTrace(uin);

    sprintf(buffer,"Updating basic information about ICQ#%ld",uin);
    icqSysLog(icq,buffer);

    if(usr)
    {
       icq->cntlFlags |= ICQFC_SAVEUSER;
       usr->u.flags   |= (USRF_REFRESH|USRF_SEARCHING);
    }

    DBGTracex(icq->c2s->requestUserUpdate);

    icq->cntlFlags &= ~ICQFC_CLEAR;

    if(usr && usr->c2s && usr->c2s->requestUserUpdate)
       sr = usr->c2s->requestUserUpdate(icq,uin,icq->searchs);
    else if(icq->c2s && icq->c2s->requestUserUpdate)
       sr = icq->c2s->requestUserUpdate(icq,uin,icq->searchs);
    else
       icqWriteSysLog(icq,PROJECT,"Unable to request user update (No available protocol manager)");

    return sr ? 0 : -1;

 }

 const char * EXPENTRY icqQueryUserNick(HUSER usr)
 {
    if(!usr || usr->u.sz != sizeof(ICQUSER))
       return "";

    return (const char *) (usr+1);
 }

 const char * EXPENTRY icqQueryUserEmail(HUSER usr)
 {
    return GetSubString(usr,3);
 }

 const char * EXPENTRY icqQueryUserFirstName(HUSER usr)
 {
    return GetSubString(usr,1);
 }

 const char * EXPENTRY icqQueryUserLastName(HUSER usr)
 {
    return GetSubString(usr,2);
 }

 const char * EXPENTRY icqQueryUserSound(HUSER usr)
 {
    return GetSubString(usr,4);
 }

 static const char *GetSubString(HUSER usr, int qtd)
 {
    char        *ptr;

    if(!usr || usr->u.sz != sizeof(ICQUSER))
       return "";

    ptr = (char *) (usr+1);

    while(qtd > 0)
    {
       while(*(ptr++));
       qtd--;
    }
    return ptr;
 }

 int EXPENTRY icqQueryUserDataBlockLength(void)
 {
    return sizeof(ICQUSER);
 }

 long EXPENTRY icqQueryUserIP(HICQ icq, HUSER usr)
 {
    if(!usr)
       return 0;

/*
       usr->ip[0] = gateway;
       usr->ip[1] = ip;
*/

    // Verifica o gateway
    if(usr->ip[0] == icq->ipConfirmed)
       return usr->ip[1];

    if(usr->ip[0] && usr->ip[0] != icq->ipConfirmed)
      return usr->ip[0];

    return usr->ip[1];
 }

 void EXPENTRY icqSetUserIP(HICQ icq, HUSER usr, long gateway, long ip)
 {
    icqWriteSysLog(icq,PROJECT,"Deprecated call to icqSetUserIP");
    icqSetUserIPandPort(icq, usr->u.uin, gateway, ip, usr->port);
 }

 HUSER EXPENTRY icqSetUserIPandPort(HICQ icq, ULONG uin, long gateway, long ip, USHORT port)
 {
#ifdef EXTENDED_LOG
    char	extLog[0x0100];
#endif
    HUSER usr = icqQueryUserHandle(icq,uin);

    if(!usr)
       return usr;

    usr->u.idle  = 0;

    if(!gateway)
       gateway = usr->ip[0] ? usr->ip[0] : ip;

    if(ip == usr->ip[0] || ip == usr->ip[1])
    {
       /* Reenviou um IP que eu ja tenho, remarco para copiar somente a porta (se necessario) */
       gateway = usr->ip[0];
       ip      = usr->ip[1];
    }

    if(usr->ip[0] == gateway && usr->ip[1] == ip && usr->port == port)
       return usr;

    if(port)
       usr->port = port;

    if(usr->ip[0] != gateway || usr->ip[1] != ip)
    {
       usr->ip[0] = gateway;
       usr->ip[1] = ip;
       icqUserEvent(icq, usr, ICQEVENT_IPCHANGED);
    }


#ifdef EXTENDED_LOG

    sprintf(extLog,"%s (ICQ#%ld) gateway is ",icqQueryUserNick(usr),usr->u.uin);
    icqIP2String(usr->ip[0],extLog+strlen(extLog));
    strcat(extLog,",IP is ");
    icqIP2String(usr->ip[1],extLog+strlen(extLog));
    sprintf(extLog+strlen(extLog)," and port %d",(int) usr->port);

    if(gateway == icq->ipConfirmed)
       strncat(extLog," (Same network)",0xFF);

    icqWriteSysLog(icq,PROJECT,extLog);

#endif

    return usr;
 }

 long EXPENTRY icqQueryUserGateway(HICQ icq, HUSER usr)
 {
    return usr->ip[0];
 }

