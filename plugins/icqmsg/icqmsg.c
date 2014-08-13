/*
 * icqmsg.c - Default message pre-processor
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #define INCL_WIN
 #include <string.h>

 #include "icqmsg.h"

/*----------------------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------------------*/

 static MSGMGR  *msgDescr[]    = {  &icqmsg_normal,
                                    &icqmsg_url,
                                    &icqmsg_added,
                                    &icqmsg_request,
                                    &icqmsg_authorized,
                                    &icqmsg_refuse,
                                    &icqmsg_file,
                                    NULL
                                 };


/*----------------------------------------------------------------------------------------------------*/

 HMODULE module = NULLHANDLE;


/*----------------------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------------------*/

 int EXPENTRY icqmsg_Configure(HICQ icq, HMODULE mod)
 {
    CHKPoint();

#ifdef DEBUG
    icqWriteSysLog(icq,PROJECT,"Loading default message manager " __DATE__ " " __TIME__ " (Debug Version)");
#else
    icqWriteSysLog(icq,PROJECT,"Loading default message manager Build " BUILD);
#endif

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"**** Extensive logging enabled ****");
#endif

    module = mod;

    CHKPoint();
	
    if(icqQueryUserDataBlockLength() != sizeof(ICQUSER))
    {
       icqWriteSysLog(icq,PROJECT,"Can't start message manager due to invalid pwICQ core version");
       return -1;
    }

    CHKPoint();
	
    if(icqValidateMsgEditHelper(icq, sizeof(MSGEDITHELPER)))
    {
       DBGMessage("*** Nao foi possivel validar a estrutura de apoio");
       icqWriteSysLog(icq,PROJECT,"Unable to use the installed Message Editor");
       return -1;
    }

    CHKPoint();
	
    if(icqRegisterDefaultMessageProcessor(icq, sizeof(MSGMGR), (const MSGMGR **) &msgDescr))
    {
       DBGMessage("*** Erro ao registrar o gerenciador de mensagems");
       icqWriteSysLog(icq,PROJECT,"Can't register message manager (Incompatible versions?)");
       return -1;
    }

    DBGMessage("Processador de mensagems registrado");

    return 0;
 }

 int EXPENTRY icqmsg_Start(HICQ icq, HPLUGIN p, void *reserved)
 {

    DBGMessage("** Start");

#ifdef DEBUG
//    icqInsertMessage(icq,  27241234, 0, MSG_NORMAL, 0, 0, -1, "Teste de inclusao de mensagem");
//    icqInsertMessage(icq,  27241234, 0, MSG_URL, 0, 0, -1, "Teste de inclusao de URLþhttp://www.os2brasil.com.br");
//    icqInsertMessage(icq,  28986464, 0, MSG_REQUEST, 0, 0, -1, "SalþRobertoþSalomonþsalomon@scr.com.brþ0þResolve, Pô!!!");
//    icqInsertMessage(icq,  27241234, 0, MSG_FILE, 0, 0, -1, "FileþTeste de envio de arquivosþconfig.sysþ120þ64");
//    icqInsertMessage(icq,  28986464, 0, MSG_ADD, 0, 0, -1, "SalþRobertoþSalomonþsalomon@scr.com.brþ0");
//    icqInsertMessage(icq,  27241234, 0, MSG_AUTHORIZED, 0, 0, -1, "Autorizado!!");
//    icqInsertMessage(icq,  27241234, 0, MSG_REFUSED, 0, 0, -1, "Voce nao e bom o bastante para entrar na minha lista");
#endif

    return 0;

 }

 int EXPENTRY icqmsg_Terminate(HICQ icq, void *reserved)
 {
#ifdef __OS2__
    MSGMGR **descr;

    for(descr = msgDescr;*descr;descr++)
       WinDestroyPointer( (*descr)->icon[0]);
#endif

    return 0;
 }


 static int _System openURL(HICQ icq, ULONG uin, const MSGMGR *mgr, const char *text, const char *url)
 {
    if(url)
    {
       /* Open the supplied URL */
       DBGMessage(url);
       icqOpenURL(icq,url);
    }

    return 0;

 }

 void EXPENTRY icqmsg_addSendTo(HICQ icq, MSGEDITWINDOW hwnd, USHORT type, BOOL out, const MSGMGR *mgr, int ( * _System addOption)(HWND, USHORT, const char *, SENDTOCALLBACK *))
 {
    /*
     * Add option in the Send to button
     */

    DBGTracex(type);
    DBGTrace(out)

    if(out)
       return;

    CHKPoint();

    if(type == MSG_URL || type == MSG_NORMAL)
       addOption(hwnd, 0, "Browser", openURL);

 }


