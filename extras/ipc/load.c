/*
 * OS2LOAD.C - Loader OS/2 para teste do modulo de envio de mensagens para fila
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_ERRORS
 #define INCL_DOSQUEUES
 #define INCL_DOSPROCESS
 #include <os2.h>
#endif

 #include <string.h>

 #include <pwMacros.h>
 #include <icqtlkt.h>
 #include <icqqueue.h>

/*---[ Prototipos ]----------------------------------------------------------------------------------------*/


/*---[ Implementacao ]-------------------------------------------------------------------------------------*/

 int main(int numpar, char *param[])
 {
//    ULONG 	uin;
//    char	buffer[0x0100];

/*
    ICQSHAREMEM *block = icqIPCGetSharedBlock();

    DBGTracex(block);
    icqIPCReleaseSharedBlock(block);

    DBGTrace(icqIPCQueryInstance(&uin));
    DBGTrace(uin);

    DBGTrace(icqIPCSetModeByName(0,"Away"));

    DBGTrace(icqIPCQueryString(0, "EMail", 0xFF, buffer));
    DBGMessage(buffer);
*/
/*
    DBGTracex(icqIPCQueryOnlineMode(0,0xFF,buffer));
    DBGMessage(buffer);

    strcpy(buffer,"αινσϊ");
    DBGTrace(icqIPCConvertCodePage(0, 1, buffer, -1));
    DBGMessage(buffer);

    strcpy(buffer,"αινσϊ");
    DBGTrace(icqIPCConvertCodePage(0, 0, buffer, -1));
    DBGMessage(buffer);

    DBGTrace(icqIPCSetPeerInfo(27241234, 6838492, 0x0100007f, 2301, 8));
*/

    DBGTrace(icqIPCQueryAvailable(27241234));
    DBGTrace(icqIPCQueryAvailable(6838492));
    DBGTrace(icqIPCQueryAvailable(0));
	DBGTrace(icqIPCAddUser(27241234,67317627));

//    DBGTrace(icqIPCSendFile(6838492,27241234,"w:\\temp\\icqipc.exe", "Teste"));

    return 0;
 }

