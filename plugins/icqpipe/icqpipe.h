/*
 * ICQPIPE.H
 */


 #include <pwMacros.h>
 #include <stdio.h>
 #include <icqtlkt.h>

 #pragma pack(1)

 struct icqpipe_config
 {
    USHORT      sz;
    ULONG       flags;
	
#ifdef __OS2__	
    char        pipeName[0x0100];
#endif	
 };


 extern HMODULE module;

 /*---[ Prototipes ]-----------------------------------------------------------------------------------*/

 void icqpipe_Write(HICQ, struct icqpipe_config *, const char *);
 void icqpipe_loadConfig(HICQ, struct icqpipe_config *);



