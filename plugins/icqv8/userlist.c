/*
 * userlist.c - Processa o pacote de atualizacao do cadastro de usuarios
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>

 #include "icqv8.h"

/*---[ Defines ]----------------------------------------------------------------------------------------------*/

 #pragma pack(1)

 struct cfg
 {
    const UCHAR *pkt;
    int         sz;
    USHORT      tag;
    USHORT      id;
//    USHORT    hdr;
 };


// 01 31 00 07 54 72 65 76 69 7a 65                  .1..Trevize

 struct userinfo
 {
    USHORT      x1;
    USHORT      sz;
 };

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static void procUser(HICQ, unsigned long, USHORT, USHORT, const struct userinfo *);
 static void procGroup(HICQ, USHORT, USHORT, const char *);


/*---[ Constantes ]-------------------------------------------------------------------------------------------*/

#ifdef DEBUG_DL

   static const UCHAR testPacket[] =
   {
          0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0c, 0x00, 0xc8, 0x00,
          0x08, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x0c, 0x20, 0x20, 0x20, 0x20, 0x20,
          0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x11, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x04, 0x00, 0x05, 0x00, 0xca, 0x00, 0x01, 0x04, 0x00, 0x06,
          0x49, 0x43, 0x51, 0x54, 0x49, 0x43, 0x00, 0x00, 0x00, 0x03, 0x00, 0x09, 0x00, 0x0e, 0x00, 0xcd,
          0x00, 0x0a, 0x33, 0x30, 0x38, 0x30, 0x2c, 0x30, 0x2c, 0x30, 0x2c, 0x30, 0x00, 0x07, 0x47, 0x65,
          0x6e, 0x65, 0x72, 0x61, 0x6c, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0c, 0x00, 0xc8, 0x00,
          0x08, 0x00, 0x02, 0x00, 0x03, 0x00, 0x05, 0x00, 0x06, 0x00, 0x08, 0x32, 0x37, 0x32, 0x34, 0x31,
          0x32, 0x33, 0x34, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x0b, 0x01, 0x31, 0x00, 0x07, 0x54,
          0x72, 0x65, 0x76, 0x69, 0x7a, 0x65, 0x00, 0x08, 0x37, 0x30, 0x34, 0x32, 0x31, 0x33, 0x37, 0x38,
          0x00, 0x01, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0d, 0x01, 0x31, 0x00, 0x09, 0x44, 0x61, 0x6e, 0x69,
          0x65, 0x6c, 0x6c, 0x65, 0x5f, 0x00, 0x07, 0x35, 0x34, 0x39, 0x36, 0x37, 0x31, 0x32, 0x00, 0x01,
          0x00, 0x05, 0x00, 0x00, 0x00, 0x0d, 0x01, 0x31, 0x00, 0x09, 0x53, 0x4c, 0x61, 0x55, 0x47, 0x68,
          0x54, 0x65, 0x52, 0x00, 0x09, 0x31, 0x33, 0x36, 0x39, 0x31, 0x36, 0x35, 0x31, 0x31, 0x00, 0x01,
          0x00, 0x06, 0x00, 0x00, 0x00, 0x09, 0x01, 0x31, 0x00, 0x05, 0x43, 0x61, 0x72, 0x6f, 0x6c, 0x00,
          0x06, 0x46, 0x61, 0x6d, 0x69, 0x6c, 0x79, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
          0x07, 0x46, 0x72, 0x69, 0x65, 0x6e, 0x64, 0x73, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
          0x00, 0x0a, 0x43, 0x6f, 0x2d, 0x57, 0x6f, 0x72, 0x6b, 0x65, 0x72, 0x73, 0x00, 0x04, 0x00, 0x00,
          0x00, 0x01, 0x00, 0x00, 0x3c, 0x21, 0x9d, 0xb5
   };

#endif


/*---[ Implementacao ]----------------------------------------------------------------------------------------*/

#ifdef DEBUG_DL

 void debug_dlUserList(HICQ icq)
 {
    CHKPoint();
    icqv8_downloadUserList(icq,NULL,296,testPacket);
 }

#endif

 int icqv8_downloadUserList(HICQ icq, ICQV8 *cfg, int sz, const UCHAR *pkt)
 {
#ifdef EXTENDED_LOG
    char                buffer[0x0100];
#endif

    unsigned short      groups;
    unsigned char       prefix[80];
    unsigned short      szString;
    unsigned short      tag;
    unsigned short      id;
    unsigned short      type;
    unsigned short      recSize;
    unsigned short      gCount          = 0;
    HUSER               usr;

    /* Marca todos os usuarios como nao confirmados */

    if(*pkt)
    {
       icqDumpPacket(icq, NULL, "Unexpected contact-list update", sz, (unsigned char *) pkt);
       return -1;
    }

    icqWriteSysLog(icq,PROJECT,"Receiving contact-list update from server");

    pkt++;

    groups = icqv8_convertShort( *( (USHORT *) pkt ) );

    DBGTrace(groups);

    pkt += 2;
    sz  -= 3;

    for(usr = icqQueryNextUser(icq, NULL); usr; usr = icqQueryNextUser(icq, usr))
       usr->c2sFlags |= USRV8_NOTCONFIRMED;

    while(sz > 0 && groups > 0)
    {
       szString  = icqv8_convertShort( *( (USHORT *) pkt ) );
       pkt      += 2;
       sz       -= 2;

       if(szString)
       {
          szString = min(szString,79);
          strncpy(prefix,pkt,szString);
          *(prefix+szString) = 0;

          pkt += szString;
       }
       else
       {
          *prefix = 0;
       }

       tag       = icqv8_convertShort( *( (USHORT *) pkt ) );
       pkt      += 2;
       id        = icqv8_convertShort( *( (USHORT *) pkt ) );
       pkt      += 2;
       type      = icqv8_convertShort( *( (USHORT *) pkt ) );
       pkt      += 2;
       recSize   = icqv8_convertShort( *( (USHORT *) pkt ) );
       pkt      += 2;
       sz       -= 8;

       switch(type)
       {
       case 0x0000:     // A normal contact list entry
          procUser(icq,atol(prefix),tag,id,(const struct userinfo *) pkt);
          break;

       case 0x0001:     // Larger grouping header
          procGroup(icq, gCount++, tag, prefix);
          break;

       default:
          DBGTracex(tag);
          DBGTracex(id);
          DBGTracex(type);
          DBGTracex(recSize);
          icqDumpPacket(icq, NULL, prefix, recSize, (unsigned char *) pkt);
       }


       pkt += recSize;
       sz  -= recSize;

       groups--;
    }

    if(sz < 4)
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected size of contact-list update packet");
       return -1;
    }

    if( ! *((ULONG *)pkt) )
    {
       icqDumpPacket(icq, NULL, "Unexpected ending of contact-list update packet", sz, (unsigned char *) pkt);
       return -1;
    }
    else
    {
       DBGTracex(LONG_VALUE( *( (ULONG *) pkt) ));
#ifdef EXTENDED_LOG
       sprintf(buffer,"Current update level: %08lx",LONG_VALUE( *( (ULONG *) pkt) ) );
       icqWriteSysLog(icq,PROJECT,buffer);
#endif

//       icqSaveValue(icq, "v8.modified", LONG_VALUE( *( (ULONG *) pkt) ));
       icqSaveValue(icq, "SSUL.Update", LONG_VALUE( *( (ULONG *) pkt) ));

       pkt += 4;
       sz  -= 4;
    }

    /* Verifica os usuarios nao confirmados */

    for(usr = icqQueryNextUser(icq, NULL); usr; usr = icqQueryNextUser(icq, usr))
    {
       if(usr->c2sFlags & USRV8_NOTCONFIRMED)
       {
//          if( !(usr->flags & (USRF_TEMPORARY|USRF_DISABLED)) )
          if(usr->flags & USRF_ONLIST)
          {
             icqUpdateUserFlag(icq, usr, TRUE, USRF_WAITING);
             icqWriteUserLog(icq,PROJECT,usr,"Not confirmed by server");
             icqAjustUserIcon(icq, usr, TRUE);
          }
       }
    }


    return 0;
 }

 static void procUser(HICQ icq, unsigned long uin, USHORT group, USHORT id, const struct userinfo *info)
 {
    char        nickName[40];
    char        buffer[0x0100];
    HUSER       usr;
    int         sz;

    if(!uin)
       return;

    sz = min(39,icqv8_convertShort(info->sz));
    strncpy(nickName,(char *) (info+1),sz);
    *(nickName + sz) = 0;

    usr = icqQueryUserHandle(icq,uin);

    if(!usr)
    {
       /* Necessario anexar o usuario */
       usr = icqInsertUser(icq, uin, nickName, "", "", "", "", 0);

       if(!usr)
       {
          icqWriteSysLog(icq,PROJECT,"Error adding new user");
          return;
       }

       icqWriteUserLog(icq,PROJECT,usr,"Added by server request");
    }
    else
    {
       icqWriteUserLog(icq,PROJECT,usr,"Confirmed by server");
    }

    usr->c2sGroup     = group;
    usr->c2sIDNumber  = id;
    usr->flags       |= USRF_ONLIST;

    usr->c2sFlags &= ~USRV8_NOTCONFIRMED;

    if(usr->flags & (USRF_TEMPORARY|USRF_DISABLED) )
    {
       icqWriteUserLog(icq,PROJECT,usr,"Activated by server request");
       icqUserEvent(icq, usr, ICQEVENT_SHOW);
    }

    if(*nickName && strcmp(nickName,icqQueryUserNick(usr)) && icqLoadValue(icq,"updateNick",0))
    {
       sprintf(buffer,"Changing nickname to \"%s\" by server request",nickName);
       icqWriteUserLog(icq,PROJECT,usr,buffer);
       usr = icqUpdateUserInfo(icq, uin, TRUE, nickName, NULL, NULL, NULL, TRUE);

       if(!usr)
       {
          icqWriteSysLog(icq,PROJECT,"Failure updating nickname");
          usr = icqQueryUserHandle(icq,uin);
       }
    }

    if(usr)
    {
       usr->flags &= ~( USRF_TEMPORARY | USRF_DISABLED | USRF_CACHED);
       usr->flags |= USRF_ONLIST;

#ifdef EXTENDED_LOG
       sprintf(buffer,"ICQ#%ld G=%04x ID=%04x F=%08lx",uin,(int) usr->c2sGroup, (int) usr->c2sIDNumber, usr->flags);
       icqWriteSysLog(icq,PROJECT,buffer);
#endif

      icqAjustUserIcon(icq, usr, TRUE);
    }

 }

 static void procGroup(HICQ icq, USHORT pos, USHORT id, const char *title)
 {
    char key[80];
    char buffer[0x0100];

    if(!*title)
       return;

    sprintf(key,"UserGroup%02d", pos);

    sprintf(buffer,"%d,",(int) id);
    strncat(buffer,title,0xFF);

    icqSaveString(icq, key, buffer);

 }

 void icqv8_startUserUpdate(HICQ icq, ICQV8 *cfg, ULONG uin)
 {
    icqv8_sendSingleSnac(icq, cfg, 0x13, 0x11, 0x11);
#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"Starting contact-list update");
#endif
 }

 void icqv8_endUserUpdate(HICQ icq, ICQV8 *cfg, ULONG uin)
 {
    icqv8_sendSingleSnac(icq, cfg, 0x13, 0x12, 0x12);
#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"Terminating contact-list update");
#endif
 }


 int  _System icqv8_addUser(HICQ icq, HUSER usr)
 {
    char        buffer[0x0100];

    ICQV8       *cfg            = icqGetPluginDataBlock(icq, module);
    PACKET      pkt;
    HUSER       tmp;
    char        *ptr;
    USHORT      c2sIDNumber;
    PACKET      subPacket;

    #pragma pack(1)

    struct _prefix
    {
       USHORT tag;      // Tag, Group this contact was added to
       USHORT id;       // ID, Randomly generated
       USHORT type;     // Type, Normal contact list entry
       USHORT Count;    // Count, 11 bytes follow
    } prefix;

    if(usr->flags & USRF_TEMPORARY)
    {
#ifdef EXTENDED_LOG
       icqWriteSysLog(icq,PROJECT,"Temporary user, Server contact-list update rejected");
#endif
       return 0;
    }

    pkt = icqv8_allocatePacket(icq, cfg, 55);

    memset(&prefix,0,sizeof(prefix));

    if(!usr->c2sIDNumber)
    {
       c2sIDNumber      = rand();
       do
       {
#ifdef __OS2__
          DosSleep(0);
#endif
          c2sIDNumber++;
          for(tmp=icqQueryFirstUser(icq);tmp && tmp->c2sIDNumber != c2sIDNumber;tmp=icqQueryNextUser(icq,tmp));

       }
       while(tmp);
       usr->c2sIDNumber = c2sIDNumber;

    }

    DBGTrace(usr->c2sIDNumber);

    sprintf(buffer,"%ld",usr->uin);
    icqv8_insertShort(pkt, 0, strlen(buffer));
    icqv8_insertString(pkt, 0, buffer);

    if(!usr->c2sGroup)
    {
       icqLoadString(icq, "v8.group01", "1,General", buffer, 0xFF);

       ptr = strstr(buffer, ",");

       if(!ptr)
       {
          icqWriteSysLog(icq,PROJECT,"Erro na descricao de grupo");
          icqv8_releasePacket(icq, cfg, pkt);
          return -2;
       }
       *ptr = 0;
       usr->c2sGroup = atoi(buffer);
    }

    /* Verificacao completa, comeco a mandar os pacotes */

    icqv8_startUserUpdate(icq,cfg,usr->uin);

    strncpy(buffer,icqQueryUserNick(usr),0xFF);

    prefix.tag          = SHORT_VALUE(usr->c2sGroup);
    prefix.id           = SHORT_VALUE(usr->c2sIDNumber);
    prefix.type         = 0;

    /* Formata o segundo grupo de TLVs */

    subPacket           = icqv8_allocatePacket(icq,cfg,55);

    icqv8_insertString(subPacket, 0x0131, icqQueryUserNick(usr));

    if(usr->flags & USRF_AUTHORIZE)
    {
       // Autorizacao esta pendente 131748330
       DBGMessage("***** USUARIO PRECISA DE AUTORIZACAO *****");
       icqv8_insertBlock(subPacket, 0x66, 0, NULL);
    }

    /* Anexa o prefixo e segundo grupo */
    prefix.Count = SHORT_VALUE(icqv8_getPacketSize(subPacket));

    icqv8_insertBlock(pkt, 0, sizeof(prefix), (const unsigned char *) &prefix);
    icqv8_insertPacket(icq,cfg,pkt,0,subPacket);

    icqv8_dumpPacket(pkt, icq, "ADD-Packet");
    icqv8_sendSnac(icq, cfg, 0x13, 0x08, 0x00020008, pkt);


/*

        00 09
        31 33 31 37
        34 38 33 33
        30

        00 01
        41 c9

        00 00
        00 0b

        01 31
        00 03
        72 66 73

        00 66
        00 00

*/

/*
> Client: snac(13,11)
> Client: snac(13,8) // add UIN record
> Server: snac(13,E) (000E - what does it mean ????????)
> Client: snac(13,9) // update selected GROUP
> Server: snac(13,E) (0000 - Ok)
> Client: snac(13,12)
*/

//    icqUserEvent(icq, usr, ICQEVENT_UPDATED);

    icqv8_endUserUpdate(icq,cfg,usr->uin);

    return 0;

 }

/*

08/28/2002 15:06:18 icqkrnl    ADD-Packet (32 bytes)
00 08 31 31 34 39 36 38 36 35 00 01 23 5b 00 00   ..11496865..#[..
00 0e 01 31 00 0a 4d 75 6c 68 65 72 47 61 74 6f   ...1..MulherGato

rc=0
*/

 int  _System icqv8_delUser(HICQ icq, HUSER usr)
 {
    ICQV8       *cfg            = icqGetPluginDataBlock(icq, module);
    PACKET      pkt;
    PACKET      subPacket;

/*
Data type  Content  Name  Description
WORD  xx xx   STRLEN  Length of the following UIN in ASCII format.
DATA  ...     UIN  This is the contact's uin in ASCII format.

WORD  xx xx   TAG  This is the Tag ID of the group in the contact list that this contact was a member of.
WORD  xx xx   ID  This is the specific, random, ID that was generated for this contact (and sent in a previous CLI_ADDBUDDY).
WORD  xx xx   TYPE  This field seems to indicate what type of group this is. Seen:

              0x0000 - A normal contact list entry
              0x0001 - A Larger Group header
              0x0004 - Unknown
              0x000E - A contact on the Ignore List
              0x0013 - This group's TLV contains the time(NULL) import time

WORD xx xx BYTELEN If BYTELEN is 0 this is not present, otherwise, A number of TLVs will follow BYTELEN.

Seen:
TLV(00C8) - Sent only with the Group header, this is a list of all IDs in this group
TLV(00CA) - Unknown
TLV(00D4) - Contains the 'Import Time' time(NULL)
TLV(0131) - Nickname of a UIN TLV(013A) - Locally assigned SMS number of a UIN
TLV(0066) - This UIN is still awaiting authorization
*/

    #pragma pack(1)

    struct _prefix
    {
       USHORT tag;      // Tag, Group this contact was added to
       USHORT id;       // ID, Randomly generated
       USHORT type;     // Type, Normal contact list entry
       USHORT Count;    // Count, 11 bytes follow
    } prefix;

    char        buffer[0x0100];

    pkt = icqv8_allocatePacket(icq, cfg, 55);

    memset(&prefix,0,sizeof(prefix));

    CHKPoint();

    sprintf(buffer,"%ld",usr->uin);
    icqv8_insertShort(pkt, 0, strlen(buffer));
    icqv8_insertString(pkt, 0, buffer);

    prefix.tag   = SHORT_VALUE(usr->c2sGroup);
    prefix.id    = SHORT_VALUE(usr->c2sIDNumber);
    prefix.type  = 0;

    DBGTrace(prefix.tag);
    DBGTrace(prefix.id);

    subPacket    = icqv8_allocatePacket(icq,cfg,55);

    /* Formata o pacote auxiliar */

    icqv8_insertString(subPacket, 0x0131, icqQueryUserNick(usr));

    /* Formata prefixo e anexa sub-pacotes */

    prefix.Count = SHORT_VALUE(icqv8_getPacketSize(subPacket));

    DBGTrace(prefix.Count);

    icqv8_insertBlock(pkt, 0, sizeof(prefix), (const unsigned char *) &prefix);
    icqv8_insertPacket(icq,cfg,pkt,0,subPacket);

    icqv8_dumpPacket(pkt, icq, "Pacote de remocao");

    icqv8_sendSnac(icq, cfg, 0x13, 0x0A, 0, pkt);
//    icqv8_sendSnac(icq, cfg, 0x13, 0x0A, usr->uin, pkt);

    return 0;
 }



