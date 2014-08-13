/*
 * lnxconfig.c - Configuracao ICQGTK - Linux
 */

 #include <stdio.h>
 #include <icqgtk.h>

/*---[ Constants ]---------------------------------------------------------------------------*/
 

/*---[ Prototipos ]--------------------------------------------------------------------------*/


/*---[ Implementacao ]-----------------------------------------------------------------------*/

 void EXPENTRY icqgtk_ConfigPage(HICQ icq, void *dBlock, ULONG uin, USHORT type, HWND hwnd,  const DLGINSERT *dlg, char *buffer)
 {
    if(type)
       return;
 
    DBGMessage("Anexar paginas de configuracao da interface grafica");
 
 }

