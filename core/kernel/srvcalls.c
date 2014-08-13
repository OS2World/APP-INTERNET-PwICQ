/*
 * SRVCALLS.C - Pontos de entrada para as chamadas ao servidor
 */


#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdio.h>
 #include <icqkernel.h>

/*---[ Estruturas ]-------------------------------------------------------------------------------------------*/


/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 int EXPENTRY icqSearchByICQ(HICQ icq, ULONG key, HWND hwnd, SEARCHCALLBACK callBack)
 {
    if(icq->c2s && icq->c2s->searchByICQ)
       return icq->c2s->searchByICQ(icq, key, hwnd, callBack);
    return -1;
 }

 int EXPENTRY icqSearchByMail(HICQ icq, const char *key, HWND hwnd, SEARCHCALLBACK callBack)
 {
    if(icq->c2s && icq->c2s->searchByMail)
       return icq->c2s->searchByMail(icq, key, hwnd, callBack);
    return -1;
 }

 int EXPENTRY icqSearchRandom(HICQ icq, USHORT key, HWND hwnd, SEARCHCALLBACK callBack)
 {
    if(icq->c2s && icq->c2s->searchRandom)
       return icq->c2s->searchRandom(icq, key, hwnd, callBack);
    return -1;
 }

 int EXPENTRY icqSearchByInformation(HICQ icq, HWND hwnd,	const char 	*firstName,
 								const char 	*lastName,
 								const char 	*nickName,
 								const char 	*email,
 								BOOL	   	online,
 								SEARCHCALLBACK 	callBack )
 {
    if(icq->c2s && icq->c2s->searchByInformation)
       return icq->c2s->searchByInformation(icq,hwnd,firstName,lastName,nickName,email,online,callBack);
    return -1;
 } 								

