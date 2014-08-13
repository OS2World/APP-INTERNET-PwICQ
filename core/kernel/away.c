/*
 * AWAY.C - Administra mensagems de away
 */

#ifdef __IBMC__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdio.h>
 #include <malloc.h>

 #include <icqkernel.h>

/*---[ Implementacao ]---------------------------------------------------------------------------------------*/

 BOOL EXPENTRY icqInsertAwayMessage(HICQ icq, ULONG uin, const char *title, char *txt)
 {
    DBGTracex(icq);

    if(!icqIsActive(icq))
       return FALSE;

    DBGMessage(txt);

    if(icq->convertCpg && txt)
       icqConvertCodePage(icq, TRUE, txt, strlen(txt));

    CHKPoint();

    DBGTracex(icq->skin);

#ifdef DEBUG
    if(icq->skin)
       DBGTracex(icq->skin->awayMessage);
#endif

    if(icq->skin && icq->skin->awayMessage)
       return icq->skin->awayMessage(icq,uin,title,txt);

    return FALSE;
 }


 const char * EXPENTRY icqQueryAwayMessage(HICQ icq)
 {
    if(icq->awayMsg)
       return icq->awayMsg;

    return "";
 }

 int EXPENTRY icqSetAwayMessage(HICQ icq, const char *txt)
 {
    if(icq->awayMsg)
       free(icq->awayMsg);

    if(txt && strlen(txt))
       icq->awayMsg = strdup(txt);
    else
       icq->awayMsg = NULL;

    return icq->awayMsg != NULL;
 }


