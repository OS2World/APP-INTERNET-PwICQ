/*
 * SVLIST.C - Salva/Carrega a lista de mensagems pendentes
 */

#ifdef __IBMC__
 #pragma strings(readonly)
#endif

 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include <time.h>

 #include <icqkernel.h>

 #include "icqcore.h"

/*---[ Estruturas ]-----------------------------------------------------------------------------*/

 #pragma pack(1)

 struct fileHeader
 {
    USHORT      sz;
    ULONG       prefix;
    ULONG       codePage;
    UCHAR       os;
    ULONG       uin;
    time_t      saved;
 };

 struct msgHeader
 {
    USHORT      msgType;
    time_t      msgTime;
    USHORT      msgSize;
 };

 enum PREFIXES
 {
    PREFIX_USER,
    PREFIX_USERMESSAGE,
    PREFIX_SYSTEMMESSAGE,
    PREFIX_SENDQUEUE,

    PREFIX_END
 };


/*---[ Prototipos ]-----------------------------------------------------------------------------*/

 static void saveMessage(FILE *, HMSG,  struct msgHeader *);
 static void loadMessage(HICQ, FILE *, ULONG, HUSER, struct msgHeader *);
 static void loadMQueue(HICQ, FILE *, ULONG);

/*---[ Implementacao ]--------------------------------------------------------------------------*/

 void saveMessageList(HICQ icq)
 {
    HUSER       usr;
    HMSG        msg;
    SYSMSG      *sysMsg;
    MQUEUE      *mQueue;
    BOOL        svUser;
    FILE        *arq;
//    USHORT      prefix;
    char        buffer[0x0100];

    if(!(icq->cntlFlags & ICQFC_SAVEQUEUES))
       return;

    icqWriteSysLog(icq,PROJECT,"Saving message queues");

    DBGMessage("Gravando lista de mensagems pendentes");

    icqQueryPath(icq,"",buffer,0x80);
    sprintf(buffer+strlen(buffer),"%ld.sav",icq->uin);

    DBGMessage(buffer);

    arq = fopen(buffer,"w+");

    if(arq == NULL)
    {
       sysLog(icq,"Unable to create the message file");
       return;
    }

    memset(buffer,0,sizeof(struct fileHeader));

    ( (struct fileHeader *) buffer)->sz         = sizeof(struct fileHeader);
    ( (struct fileHeader *) buffer)->prefix     = PWICQVERSIONID;
    ( (struct fileHeader *) buffer)->uin        = icq->uin;
    ( (struct fileHeader *) buffer)->saved      = time(NULL);

#ifdef __OS2__
    ( (struct fileHeader *) buffer)->os         = 0x0001;
    ( (struct fileHeader *) buffer)->codePage   = icqLoadValue(icq,"cnvtUsersFile", 0) && icq->convertCpg ? icq->cpgOut : icq->cpgIn;
#endif

#ifdef linux
    ( (struct fileHeader *) buffer)->os         = 0x0002;
#endif

    fwrite(buffer,sizeof(struct fileHeader),1,arq);

    /* Salva lista de mensagems recebidas */

    for(sysMsg = icqQueryFirstElement(icq->msg);sysMsg;sysMsg = icqQueryNextElement(icq->msg,sysMsg))
    {
       *( (USHORT *) buffer ) = PREFIX_SYSTEMMESSAGE;
       fwrite(buffer,sizeof(USHORT),1,arq);
       fwrite(&sysMsg->uin,sizeof(ULONG),1,arq);
       saveMessage(arq, &sysMsg->m, (struct msgHeader *) buffer);
    }

    for(usr = icqQueryFirstUser(icq);usr;usr = icqQueryNextUser(icq,usr))
    {
       svUser = TRUE;

       for(msg = icqQueryFirstElement(usr->msg);msg;msg = icqQueryNextElement(usr->msg,msg))
       {
          if(svUser)
          {
             *( (USHORT *) buffer ) = PREFIX_USER;
             fwrite(buffer,sizeof(USHORT),1,arq);
             fwrite(&usr->u.uin,sizeof(ULONG),1,arq);
             svUser = FALSE;
          }
          *( (USHORT *) buffer ) = PREFIX_USERMESSAGE;
          fwrite(buffer,sizeof(USHORT),1,arq);
          saveMessage(arq, msg, (struct msgHeader *) buffer);
       }
    }

    /* Salva fila de mensagems nao enviadas */

    for(mQueue = icqQueryFirstElement(icq->msgQueue);mQueue;mQueue = icqQueryNextElement(icq->msgQueue,mQueue))
    {
       *( (USHORT *) buffer ) = PREFIX_SENDQUEUE;
       fwrite(buffer,sizeof(USHORT),1,arq);

       fwrite(&mQueue->uin,  sizeof(ULONG),  1,arq);
       fwrite(&mQueue->type, sizeof(USHORT), 1,arq);

       *( (USHORT *) buffer ) = strlen( (char *) (mQueue+1) )+1;
       fwrite(buffer,sizeof(USHORT),1,arq);

       fwrite((mQueue+1),*( (USHORT *) buffer ),1,arq);

    }

    /* Finaliza o arquivo */

    *( (USHORT *) buffer ) = 0xFFFF;
    fwrite(buffer,sizeof(USHORT),1,arq);

#ifdef __OS2__
    fprintf(arq,   "@#PerryWerneck: " PWICQVERSION " #@##1##" __DATE__ " " __TIME__ "     " HOSTNAME ":"ASDFEATUREID":%u:%u:0::" LEVEL_REQUIRED "@@pwICQ save file\n",
                                icq->cpgIn,icq->countryCode );
#endif

    fclose(arq);

    icq->cntlFlags &= ~ICQFC_SAVEQUEUES;

 }

 static void saveMessage(FILE *arq, HMSG msg, struct msgHeader *hdr)
 {
    hdr->msgType = msg->type;
    hdr->msgTime = msg->msgTime;
    hdr->msgSize = msg->msgSize;
    fwrite(hdr,sizeof(struct msgHeader),1,arq);
    fwrite( (char *) (msg+1), msg->msgSize,1,arq);
 }

 void loadMessageList(HICQ icq)
 {
    FILE        *arq;
    USHORT      prefix          = 0;
    ULONG       uin             = 0;
    HUSER       usr             = NULL;
    char        buffer[0x0100];

    icqQueryPath(icq,"",buffer,0x80);
    sprintf(buffer+strlen(buffer),"%ld.sav",icq->uin);

    DBGMessage(buffer);

    arq = fopen(buffer,"r");

    DBGTracex(arq);

    if(!arq)
       return;

    fread(buffer,sizeof(struct fileHeader),1,arq);

//    DBGTracex( ((struct fileHeader *) buffer)->prefix );
//    DBGTrace(  ((struct fileHeader *) buffer)->uin    );
//    DBGTracex( ((struct fileHeader *) buffer)->saved  );

    if( ((struct fileHeader *) buffer)->sz != sizeof(struct fileHeader) )
    {
       CHKPoint();
       icqWriteSysLog(icq,PROJECT,"Invalid .SAV file");
       fclose(arq);
       return;
    }

    if( ((struct fileHeader *) buffer)->uin != icq->uin )
    {
       CHKPoint();
       icqWriteSysLog(icq,PROJECT,"Unexpected UIN indicator in the .SAV file");
       fclose(arq);
       return;
    }

    fread(&prefix,sizeof(prefix),1,arq);
    while(!feof(arq) && prefix != 0xFFFF)
    {
       DBGTrace(prefix);

       switch(prefix)
       {
       case PREFIX_USER:
          fread(&uin,sizeof(ULONG),1,arq);
          usr = icqQueryUserHandle(icq,uin);
          break;

       case PREFIX_USERMESSAGE:
          fread(buffer,sizeof(struct msgHeader),1,arq);
          loadMessage(icq,arq,uin,usr,(struct msgHeader *) buffer);
          break;

       case PREFIX_SYSTEMMESSAGE:
          fread(&uin,sizeof(ULONG),1,arq);
          fread(buffer,sizeof(struct msgHeader),1,arq);
          loadMessage(icq,arq,uin,icqQueryUserHandle(icq,uin),(struct msgHeader *) buffer);
          break;

       case PREFIX_SENDQUEUE:
          fread(&uin,sizeof(ULONG),1,arq);
          loadMQueue(icq,arq,uin);
          break;
       }

       fread(&prefix,sizeof(prefix),1,arq);
    }


    fclose(arq);

 }

 static void loadMessage(HICQ icq, FILE *arq, ULONG uin, HUSER usr, struct msgHeader *hdr)
 {
    char *text;
    HMSG m;

    DBGTracex(hdr->msgType);
//    DBGTracex(hdr->msgTime);
//    DBGTrace(hdr->msgSize);

    text = malloc(hdr->msgSize+5);

    if(!text)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when loading message list");
       return;
    }

    fread(text,hdr->msgSize,1,arq);
    *(text+hdr->msgSize) = 0;

    DBGMessage(text);

    m = addInputMessage(icq,uin,&usr,hdr->msgType,hdr->msgTime,hdr->msgSize,text,TRUE);

    DBGTracex(m);

    free(text);

    CHKPoint();

    if(!m)
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when storing message");

    m->mgr = icqQueryMessageManager(icq, m->type);

    if(usr)
    {
       DBGTracex(usr->u.flags & USRF_HIDEONLIST);

       if( (m->msgFlags & MSGF_UNHIDE) ||(usr->u.flags & USRF_HIDEONLIST))
          icqUserEvent(icq, usr, ICQEVENT_SHOW);

       icqAjustUserIcon(icq, usr, FALSE);

       DBGTracex(usr->u.index);

    }

 }

 static void loadMQueue(HICQ icq, FILE *arq, ULONG uin)
 {
    USHORT type;
    USHORT size;
    char   *text;

    fread(&type,sizeof(USHORT),1,arq);
    fread(&size,sizeof(USHORT),1,arq);

    DBGTrace(size);

    text = malloc(size+5);

    if(!text)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when loading message list");
       return;
    }

    fread(text,size,1,arq);
    *(text+size) = 0;

    DBGMessage(text);

    free(text);
 }

