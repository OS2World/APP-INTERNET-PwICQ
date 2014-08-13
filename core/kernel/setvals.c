/*
 * SETVALS.C - Ajusta valores
 */

 #include <stdio.h>
 #include <string.h>

 #include <icqkernel.h>


/*---[ Implementacao ]-----------------------------------------------------------------------------*/

 BOOL EXPENTRY icqSetLocalIP(HICQ icq, long ip)
 {
    if(ip != icq->ipConfirmed)
    {
       DBGMessage("IP foi trocado");
       icq->ipConfirmed = ip;
       icqSaveValue(icq,"localIP",ip);
       icqSystemEvent(icq,ICQEVENT_IPCHANGED);
       return TRUE;
    }
    return FALSE;
 }

 void EXPENTRY icqAbend(HICQ icq, ULONG msg)
 {
    icqSystemEvent(icq,ICQEVENT_ABEND);
    icq->ready = FALSE;
 }

 USHORT EXPENTRY icqSetC2SPort(HICQ icq, USHORT port)
 {
    USHORT ret = icq->c2sPort;

    if(port)
       icq->c2sPort = port;

    return ret;
 }

 void EXPENTRY icqSetServerBusy(HICQ icq, BOOL mode)
 {
    if(mode)
       icq->cntlFlags &= ~ICQFC_CLEAR;
    else
       icqSystemEvent(icq,ICQEVENT_PLISTCLEAR);
 }

 int ICQAPI icqSetPassword(HICQ icq, USHORT prot, const char *oldPasswd, const char *newPasswd)
 {
   int ret = 100;

   if(prot != USRT_ICQ)
      return 99;

   if(icq->c2s && icq->c2s->setPassword)
      ret = icq->c2s->setPassword(icq,oldPasswd,newPasswd);

   if(ret)
      return ret;

   strncpy(icq->pwd,newPasswd,PASSWORD_SIZE);

   return ret;
 }

 int ICQAPI icqSetConnectTimer(HICQ icq, USHORT timer)
 {

#ifdef EXTENDED_LOG

    char buffer[0x0100];

    if(!timer)
       timer = icqLoadValue(icq, "reconnect", 0);

    sprintf(buffer,"Reconnect timer set to %d",(int) timer);
    icqWriteSysLog(icq,PROJECT,buffer);

#endif

    icq->connctTimer = timer;

    return timer;

 }

