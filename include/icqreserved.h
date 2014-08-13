/*
 * ICQRESERVED.H - Funcoes especiais
 */

#ifdef __cplusplus
   extern "C" {
#endif	  

 int  EXPENTRY  icqLoadConfigFile(HICQ,char *);
 int  EXPENTRY  icqSaveConfigFile(HICQ);

 ULONG EXPENTRY icqLoadReservedKey(HICQ, const char *, ULONG);

#ifdef __cplusplus
   }
#endif	  



