/*
 * LOG.C - Funcao para gravacao de arquivo de log
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_DOSMISC
#endif

 #include <stdio.h>
 #include <time.h>
 #include <errno.h>
 #include <string.h>
 #include <malloc.h>
 #include <stdarg.h>

 #include <icqkernel.h>

/*---[ Constantes ]-----------------------------------------------------------------------------------------*/

 static const char *timeMasc = "%m/%d/%Y %H:%M:%S";

/*---[ Implementacao ]--------------------------------------------------------------------------------------*/

 void EXPENTRY icqWriteThreadLog(ICQTHREAD *thd, const char *id, const char *text)
 {
    icqWriteSysLog(thd->icq,id,text);
 }


#ifdef DEBUG_LOG

 void icqWriteExtendedLog(HICQ icq,const char *id, const char *text, const char *file, int line)
 {
    char         wrk[70];
    time_t       ltime;

    time(&ltime);
    strftime(wrk, 69, timeMasc, localtime(&ltime));

    printf("%s %s(%d)\t%s\n",wrk,file,line,text);

#ifdef DEBUG
    fprintf(stderr,"%s %s(%d)\t%s\n",wrk,file,line,text);
    fflush(stderr);
#endif

    fflush(stdout);

 }

#endif

 void EXPENTRY icqWriteUserLog(HICQ icq,const char *id, HUSER usr, const char *txt)
 {
    char         wrk[70];
    time_t       ltime;

    time(&ltime);
    strftime(wrk, 69, timeMasc, localtime(&ltime));

    if(usr)
       printf("%s %-10s [%s ICQ#%ld]: %s\n",wrk,id,icqQueryUserNick(usr),usr->u.uin,txt);
    else
       printf("%s %-10s %s\n",wrk,id,txt);

    fflush(stdout);

 }

 void EXPENTRY icqWriteSysRC(HICQ icq, const char *id, int rc, const char *text)
 {
    char        wrk[70];
    time_t      ltime;
    char        szOutMsg[0x0100];

#ifdef __OS2__

    UCHAR       *IvTable[2]             = {0};
    ULONG       ulMsgLen                = 0;
    int         err;

#endif

    DBGTrace(rc);

    if(!rc)
       return;

    time(&ltime);
    strftime(wrk, 69, timeMasc, localtime(&ltime));

    if(rc == -1)
    {
       printf("%s %-10s %s: %s (rc=%d)\n",wrk,id,text,strerror(errno),errno);
       fflush(stdout);
       return;
    }
    else if(rc < 0)
    {
       sprintf(szOutMsg,"%s (rc=%d)",text,rc);
       icqWriteSysLog(icq, id, szOutMsg);
       return;
    }

#ifdef __OS2__

    *szOutMsg = 0;

    err = DosGetMessage( IvTable,          /* Message insert pointer array */
                         0,                /* Number of inserts */
                         szOutMsg,         /* Output message */
                         0xFF,             /* Length of output message area */
                         rc,               /* Number of message requested */
                         "OSO001.MSG",     /* Message file (created by MKMSGF) */
                         &ulMsgLen);       /* Length of resulting output message */

    if(err)
       sprintf(szOutMsg,"Error %d loading OSO001.MSG for message %d",err,rc);

    printf("%s %-10s %s (rc=%d:%s)\n",wrk,id,text,rc,szOutMsg);

#endif

#ifdef linux
    printf("%s %-10s %s: %s (rc=%d)\n",wrk,id,text,strerror(rc),rc);
#endif

    fflush(stdout);

 }

 void EXPENTRY icqPrintLog(HICQ icq, const char *id, const char *fmt, ...)
 {
    char    string[0x0100];
    va_list arg_ptr;

    DBGMessage(fmt);

    va_start(arg_ptr, fmt);
    vsprintf(string, fmt, arg_ptr);
    va_end(arg_ptr);

    icqWriteSysLog(icq,id,string);

 }

 void EXPENTRY icqWriteSysLog(HICQ icq, const char *id, const char *text)
 {
    char         wrk[70];
    time_t       ltime;

    time(&ltime);
    strftime(wrk, 69, timeMasc, localtime(&ltime));

    printf("%s %-10s %s\n",wrk,id,text);
    fflush(stdout);

#ifdef DEBUG
    fprintf(stderr,"%s %-10s %s\n",wrk,id,text);
    fflush(stderr);
#endif

 }

 void EXPENTRY icqDumpPacket(HICQ icq, HUSER usr, const char *msg, int size, unsigned char *dbg)
 {
    int  f;
    char *linha         = malloc(strlen(msg)+80);
    char wrk[35];
    char *pos;

    if(!linha)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when dumping packet");
       return;
    }

    strcpy(linha,msg);
    sprintf(linha+strlen(linha)," (%d bytes)",size);
    icqWriteSysLog(icq,PROJECT,linha);

    if(usr)
       printf("User: %s (ICQ# %lu)\n",(char *)(usr+1),usr->u.uin);

    memset(linha,' ',79);
    *(linha+79) = 0;

    f = 0;
    while(size--)
    {
       if(*dbg < 32)
          *(linha+(f+50)) = '.';
       else
          *(linha+(f+50)) = *dbg;

#ifdef linux
       if(*dbg > 0x80)
          *(linha+(f+50)) = '.';
#endif

       pos = linha+(f*3);

       sprintf(wrk,"%02x",*dbg);

       *pos     = *wrk;
       *(pos+1) = *(wrk+1);

       if(++f >= 16)
       {
          printf("%s\n",linha);
          memset(linha,' ',79);
          f = 0;
       }

       dbg++;

    }
    printf("%s\n",linha);
    fflush(stdout);

    free(linha);

 }


 void EXPENTRY icqWarning(HICQ icq, HUSER usr, const char *id, USHORT type, const char *msg)
 {
    icqWriteSysLog(icq, id, msg);

    if(icq->skin && icq->skin->warning)
       icq->skin->warning(icq,usr,type,icq->skinData,msg);
 }


