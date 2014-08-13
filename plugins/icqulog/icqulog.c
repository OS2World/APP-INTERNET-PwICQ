/*
 * ICQULOG.C - Very simple user logger plugin for pwICQ V2
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdio.h>
 #include <ctype.h>
 #include <string.h>
 #include <malloc.h>

#ifndef linux
 #include <io.h>
#endif

 #include <icqtlkt.h>

#ifndef NULLHANDLE
   #define NULLHANDLE 0
#endif

/*---[ Prototipes ]-------------------------------------------------------------------------------------*/

 static void writeLogEntry(HICQ, ULONG, BOOL, BOOL, HMSG);
 static char * makeFileName(HICQ, ULONG, BOOL, char *);

/*---[ Publics ]----------------------------------------------------------------------------------------*/

 HMODULE module = NULLHANDLE;

/*---[ Implement ]--------------------------------------------------------------------------------------*/

 int EXPENTRY icqulog_Configure(HICQ icq, HMODULE mod)
 {
    /*
     *
     * PLUGIN configuration routine, the first called by pwICQ, must return 0
     * or the a size for the plugin data-area to be allocated by the core
     *
     * * If returns a negative value the plugin is unloaded
     *
     */

    CHKPoint();

    module = mod;

    if(icqQueryUserDataBlockLength() != sizeof(ICQUSER))
    {
       icqWriteSysLog(icq,PROJECT,"Can't start user logger due to invalid pwICQ core version");
       return -1;
    }

    icqWriteSysLog(icq, PROJECT,"UserLog plugin starting (Build " BUILD ")");
    icqSetCaps(icq,ICQMF_HISTORY); // Notify Core - You have history now

    return 0; // No data

 }


 void EXPENTRY icqulog_UserEvent(HICQ icq, void *nothing, HUSER usr, short eventCode)
 {
    /*
     * User Event procedure - Called by core for every user event
     */
    char        buffer[0x0100];

    if(eventCode != ICQEVENT_DELETED)
       return;

    makeFileName(icq,usr->uin,FALSE,buffer);

    if(!remove(buffer))
    {
       sprintf(buffer,"%s's log file removed",icqQueryUserNick(usr));
       icqWriteSysLog(icq,PROJECT,buffer);
    }

 }

 void EXPENTRY icqulog_Event(HICQ icq, void *nothing, ULONG uin, char id, USHORT eventCode, HMSG msg)
 {
    /*
     * Process core events, the only events really usefull for this plugin is the message ones
     */
    HUSER       usr;

    if( (toupper(id) != 'M') || !msg)
       return;

    if(msg->sz != sizeof(ICQMSG))
       return;

    usr = icqQueryUserHandle(icq,uin);

    DBGTracex(usr);

    if(usr)
    {
       DBGTrace(usr->flags & USRF_HISTORY);
       if(usr->flags & USRF_HISTORY)
          writeLogEntry(icq,uin,id == 'm',FALSE,msg);
    }
    else
    {
       writeLogEntry(icq,uin,id == 'm',TRUE,msg);
    }

    CHKPoint();

 }

 static char * makeFileName(HICQ icq, ULONG uin, BOOL sys, char *fileName)
 {
    char        key[40];

#ifdef linux	
    if(sys)
       strcpy(key,"logs/system.log");
    else
       sprintf(key,"logs/%lu.log",uin);
#else
    if(sys)
       strcpy(key,"logs\\system.log");
    else
       sprintf(key,"logs\\%lu.log",uin);
#endif	

    icqQueryPath(icq,key,fileName,0xFF);

    return fileName;
 }

 static void writeLogEntry(HICQ icq, ULONG uin, BOOL dir, BOOL sys, HMSG msg)
 {
    char        fileName[0x0100];
    char        *buffer;
    FILE        *out;
    time_t      ltime;

    if(msg->mgr && msg->mgr->sz != sizeof(MSGMGR))
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected message manager");
       return;
    }

    makeFileName(icq,uin,sys,fileName);

    DBGMessage(fileName);

    buffer = malloc(MAX_MSG_SIZE+10);

    if(!buffer)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error");
       return;
    }

#ifdef __OS2__
    if(access(fileName,0))
    {
       out = fopen(fileName,"a");
       if(out)
       {
          fclose(out);
          icqWriteSysLog(icq,PROJECT,"New log file created");

          sprintf(buffer,"0000%s\n(ICQ#%ld)",icqQueryUserNick(icqQueryUserHandle(icq,uin)),uin);

          *( (USHORT *) buffer)     = 0xFFFD;
          *( (USHORT *) (buffer+2)) = strlen(buffer+4);

          icqSetEA(fileName,".LONGNAME",strlen(buffer+4)+4,buffer);
       }
    }
#endif

    out = fopen(fileName,"a");


    if(!out)
    {
       strncpy(buffer,"Can't open logfile: ",MAX_MSG_SIZE);
       strncat(buffer,fileName,MAX_MSG_SIZE);;
       icqWriteSysLog(icq,PROJECT,buffer);
       free(buffer);
       return;
    }

    time(&ltime);
    strftime(buffer, 69, "%d/%m/%Y %H:%M:%S", localtime(&ltime));

    fprintf(out,"--- %s ",buffer);

    icqQueryMessageTitle(icq, uin, dir, msg, buffer, MAX_MSG_SIZE);
    fprintf(out,"%s ---\n\n",buffer);

    if(msg->mgr && msg->mgr->formatLog)
       msg->mgr->formatLog(icq,dir,msg,buffer,MAX_MSG_SIZE); // Ask message manager for the text format
    else
       strncpy(buffer,(char *)(msg+1),MAX_MSG_SIZE);

    fprintf(out,buffer);
    fprintf(out,"\n\n");

    fclose(out);

    free(buffer);

 }

