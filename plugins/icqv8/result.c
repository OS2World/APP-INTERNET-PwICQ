/*
 * SNAC15.C
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>

 #include "icqv8.h"

/*---[ Defines ]----------------------------------------------------------------------------------------------*/

 #pragma pack(1)

 struct responsecmd
 {
    USHORT command;
    int    ( * _System exec)(HICQ,ICQV8 *, int, ULONG, struct searchResponse *);
 };


/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static int _System xmlResponse(HICQ, ICQV8 *, int, ULONG, struct searchResponse *);
 static int _System uknResponse(HICQ, ICQV8 *, int, ULONG, struct searchResponse *);

 static int _System metaGeneral(HICQ, ICQV8 *, int, ULONG, struct searchResponse *);
 static int _System metaMore(HICQ, ICQV8 *, int, ULONG, struct searchResponse *);
 static int _System metaEMail(HICQ, ICQV8 *, int, ULONG, struct searchResponse *);
 static int _System metaWork(HICQ, ICQV8 *, int, ULONG, struct searchResponse *);
 static int _System metaAbout(HICQ, ICQV8 *, int, ULONG, struct searchResponse *);
 static int _System metaInterest(HICQ, ICQV8 *, int, ULONG, struct searchResponse *);
 static int _System metaBackground(HICQ, ICQV8 *, int, ULONG, struct searchResponse *);
 static int _System metaLast(HICQ, ICQV8 *, int, ULONG, struct searchResponse *);
 static int _System metaResponse(HICQ, ICQV8 *, int, ULONG, struct searchResponse *);
 static int _System metaRandom(HICQ, ICQV8 *, int, ULONG, struct searchResponse *);

 static int _System dumpResponse(HICQ, ICQV8 *, int, ULONG, struct searchResponse *);

 static void saveLNTS(HICQ, HUSER, char **, int *, const char *);
 static void sendAutoRemoveReply(HICQ, ULONG);
 static int processSearchResponse(HICQ, ICQV8 *, int, BOOL, struct searchResponse *);


/*---[ SNAC processors ]--------------------------------------------------------------------------------------*/

 static const struct responsecmd procResponse[] =
 {
    { 0x00c8,       metaGeneral             },
    { 0x00dc,       metaMore                },
    { 0x00eb,       metaEMail               },
    { 0x010e,       uknResponse             },
    { 0x00d2,       metaWork                },
    { 0x00e6,       metaAbout               },
    { 0x00f0,       metaInterest            },
    { 0x00fa,       metaBackground          },

    { 0x01a4,	    metaResponse   	    },
    { 0x01ae,       metaLast                },

    { 0x08a2,       xmlResponse             },
    { 0x031b,       uknResponse             },

    { 0x0366,       metaRandom              },

    { 0x0000,       NULL                    }
 };


/*---[ Implement ]--------------------------------------------------------------------------------------------*/

/*
 struct searchResponse
 {
    USHORT  seq;
    USHORT  command;
    UCHAR   result;
 };
*/

 int _System icqv8_searchResult(HICQ icq, ICQV8 *cfg, int sz, ULONG req, struct searchResponse *pkt)
 {
    const struct responsecmd 	*cmd;
    char			buffer[0x0100];

    for(cmd = procResponse;cmd->exec;cmd++)
    {
       if(pkt->command == cmd->command)
       {
          if(cmd->exec(icq,cfg,sz,req,pkt))
          {
             sprintf(buffer,"Error processing response 0x%04x",pkt->command);
             icqDumpPacket(icq, NULL, buffer, sz, (char *) pkt);
          }
          return 0;
       }
    }

    sprintf(buffer,"Unexpected response 0x%04x",(int) pkt->command);
    icqDumpPacket(icq, NULL, buffer, sz, (char *) pkt);

    return 0;
 }

 static int _System uknResponse(HICQ icq, ICQV8 *cfg, int sz, ULONG req, struct searchResponse *response)
 {
    /* Unknown */
    return 0;
 }

 static int _System xmlResponse(HICQ icq, ICQV8 *cfg, int sz, ULONG req, struct searchResponse *response)
 {
    USHORT                      *szKey          = (USHORT *) (response+1);
    char                        *ptr            = (char *)   (szKey+1);
    const struct xmlRequest     *reqxml;
    char                        *terminate;
    char                        key[80];

    if(!req)
       return -1;

    for(reqxml = icqv8_xmltable; reqxml->key && reqxml->id != req; reqxml++);

    if(!reqxml->key)
    {
       icqWriteSysLog(icq,PROJECT,"Unrequested response in SNAC 15,03");
       return -1;
    }

    if(response->result != 0x0A)
       return 0;

    /* Gravar uma chave XML */

    if(memcmp(ptr,"<value>",7))
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected XML Tag in SNAC 15,03");
       return -1;
    }

    ptr += 7;
    terminate = strstr(ptr, "</value>");

    if(!terminate)
    {
       icqWriteSysLog(icq,PROJECT,"Invalid XML Tag in SNAC 15,03");
       return -1;
    }

    *terminate = 0;

    strcpy(key,"v8.xml.");
    strncat(key,reqxml->key,79);
    icqSaveString(icq, key, ptr);

    return 0;
 }

 static int _System dumpResponse(HICQ icq, ICQV8 *cfg, int sz, ULONG req, struct searchResponse *response)
 {
    icqDumpPacket(icq, NULL, "Search response", sz, (char *) response);
    return 0;
 }

 static int _System metaGeneral(HICQ icq, ICQV8 *cfg, int sz, ULONG req, struct searchResponse *response)
 {
/*
snac15.c(180):  f = 0   snac15.c(181):  Trevize
snac15.c(180):  f = 1   snac15.c(181):  Golan
snac15.c(180):  f = 2   snac15.c(181):  Trevize
snac15.c(180):  f = 3   snac15.c(181):  trevize@zaz.com.br
snac15.c(180):  f = 4   snac15.c(181):  Brasilia
snac15.c(180):  f = 5   snac15.c(181):  DF
snac15.c(180):  f = 6   snac15.c(181):  +55-61-3xx-xxxx
snac15.c(180):  f = 7   snac15.c(181):  Not available
snac15.c(180):  f = 8   snac15.c(181):  In my home
snac15.c(180):  f = 9   snac15.c(181):  +55-61-9xx-xxxx
snac15.c(180):  f = 10  snac15.c(181):  70761
*/

    USHORT *szKey = (USHORT *) (response+1);
    char   *str[11];
    int    f;
    ULONG  uin  = icqQuerySearchUIN(icq,response->seq);
    HUSER  usr;

    if(!uin)
    {
       icqWriteSysLog(icq,PROJECT,"Unexpected search response");
       return 0;
    }

    usr = icqQueryUserHandle(icq, uin);
    if(!usr)
       return 0;

    for(f=0;f<11;f++)
    {
       str[f] = (char *) (szKey+1);
       szKey  = (USHORT *) (str[f]+ *szKey);
    }

    icqUpdateUserInfo(icq, uin, FALSE, str[0], str[1], str[2], str[3], (usr->flags & USRF_AUTHORIZE) != 0 );
    icqSearchEvent(icq,uin,ICQSEARCH_BASIC);

    return 0;

 }

 static int _System metaMore(HICQ icq, ICQV8 *cfg, int sz, ULONG req, struct searchResponse *response)
 {
    char        buffer[0x0100];
    HUSER       usr;
    ULONG       uin  = icqQuerySearchUIN(icq,response->seq);

    #pragma pack(1)

    struct _header
    {
       USHORT age;      // WORD.L  xx xx  AGE  The age of the user. 0 or -1 is invalid and means not entered.
       UCHAR  sex;      // BYTE    xx     SEX  The gender of the user. 1 means female, 2 means male, 0 means not entered.
       USHORT hpSize;
    } *header = (struct _header *) (response+1);
/*
   WORD.L  xx xx  AGE  The age of the user. 0 or -1 is invalid and means not entered.
   BYTE  xx       SEX  The gender of the user. 1 means female, 2 means male, 0 means not entered.
   LNTS  xx xx ...  HOMEPAGE  The homepage of the user.
   WORD.L  xx xx  YEAR  The year of birth of the user.
   BYTE  xx  MONTH  The month of birth of the user.
   BYTE  xx  DAY  The day of birth of the user.
   BYTE  xx  LANG1  The language the user speaks fluently, according to a table. *TODO*
   BYTE  xx  LANG2  Another language the user speaks.
   BYTE  xx  LANG3  Another language the user speaks.
   WORD.L  xx xx  UNKNOWN  Unknown: Empty.
*/

    struct _data
    {
       USHORT birthYear;   // WORD.L  xx xx  YEAR  The year of birth of the user.
       UCHAR  birthMonth;  // BYTE  xx  MONTH  The month of birth of the user.
       UCHAR  birthDay;    // BYTE  xx  DAY  The day of birth of the user.
       UCHAR  lang1;       // BYTE  xx  LANG1  The language the user speaks fluently, according to a table. *TODO*
       UCHAR  lang2;       // BYTE  xx  LANG2  Another language the user speaks.
       UCHAR  lang3;       // BYTE  xx  LANG3  Another language the user speaks.
       USHORT x1;          // WORD.L  xx xx  UNKNOWN  Unknown: Empty.
    } *data;

    if(!uin)
       return 0;

    DBGTrace(uin);

    usr = icqQueryUserHandle(icq,uin);
    DBGTracex(usr);

    if(!usr)
       return 0;

    data = (struct _data *) (((char *) (header+1)) + header->hpSize );

    strncpy(buffer,(char *)(header+1), min(header->hpSize,0xFF));
    icqSaveUserDB(icq, usr, "Homepage", buffer);

    DBGTrace(header->age);
    DBGTrace(header->sex);
    DBGTrace(header->hpSize);
    DBGMessage(buffer);

    usr->age = header->age;
    usr->sex = header->sex;

    DBGTrace(data->birthYear);
    DBGTrace(data->birthMonth);
    DBGTrace(data->birthDay);

    sprintf(buffer,"%04d%02d%02d", (int) data->birthYear , (int) data->birthMonth , (int) data->birthDay );
    icqSaveUserDB(icq, usr, "Birth", buffer);

    sprintf(buffer,"%03d%03d%03d", (int) data->lang1, (int) data->lang2, (int) data->lang3);
    icqSaveUserDB(icq, usr, "Language", buffer);

    icqSearchEvent(icq,uin,ICQSEARCH_MORE);

    return 0;
 }

 static int _System metaEMail(HICQ icq, ICQV8 *cfg, int sz, ULONG req, struct searchResponse *response)
 {
    char *ptr                   = (char *) (response+1);
    int  qtd                    = *ptr;
/*
    BYTE  xx  COUNT  The number of email addresses to follow. May be zero.
                     Each consist of the following parameters:
    BYTE  xx  FLAGS  Publish email address? 1 = yes, 0 = no.
    LNTS  xx xx ...  EMAIL  The email address.
*/

    DBGTrace(qtd);

    if(qtd)
       icqDumpPacket(icq, NULL, "E-Mail response packet received", sz, (char *) response);


    return 0;
 }

 static void saveLNTS(HICQ icq, HUSER usr, char **src, int *sz, const char *key)
 {
    char   *ptr    = *src;
    USHORT strSize = *((USHORT *) ptr);
    char   buffer[0x0100];

    ptr+=2;

    if(strSize < 0xFF)
    {
       strncpy(buffer,ptr,strSize);
       *(buffer+strSize) = 0;
//       DBGMessage(buffer);
       icqSaveUserDB(icq, usr, key, buffer);
    }

    *src = (ptr+strSize);
 }

 static int _System metaWork(HICQ icq, ICQV8 *cfg, int sz, ULONG req, struct searchResponse *response)
 {
    HUSER       usr;
    ULONG       uin  = icqQuerySearchUIN(icq,response->seq);
    char        *ptr = (char *) (response+1);
    char        buffer[20];

    /*
       0  LNTS  xx xx ...  WCITY  The user's work place city.
       1  LNTS xx xx ... WSTATE The user's work place state.
       2  LNTS xx xx ... WPHONE The user's work place phone number.
       3  LNTS xx xx ... WFAX The user's work place fax number.
       4  LNTS xx xx ... WADDRESS The user's work place address.
       5  LNTS xx xx ... WZIP The user's work place zip code.
          WORD.L xx xx WCOUNTRY The user's work place country.
       6  LNTS xx xx ... WCOMPANY The user's company.
       7  LNTS xx xx ... WDEPARTMENT The department the user is working in.
       8  LNTS xx xx ... WPOSITION The position the user has at this company.
          WORD.L xx xx WOCCUPATION The occupation the user has at this company.
       10 LNTS xx xx ... WHOMEPAGE The user's work place home page.
    */

    usr = icqQueryUserHandle(icq,uin);
    DBGTracex(usr);

    if(!usr)
       return 0;

    saveLNTS(icq,usr,&ptr,&sz,"Work.City");
    saveLNTS(icq,usr,&ptr,&sz,"Work.State");
    saveLNTS(icq,usr,&ptr,&sz,"Work.Phone");
    saveLNTS(icq,usr,&ptr,&sz,"Work.Fax");
    saveLNTS(icq,usr,&ptr,&sz,"Work.Address");
    saveLNTS(icq,usr,&ptr,&sz,"Work.Zip");

    sprintf(buffer,"%d", (int) *((USHORT *) ptr));
    ptr += 2;
    DBGMessage(buffer);
    icqSaveUserDB(icq, usr, "Work.Country", buffer);

    saveLNTS(icq,usr,&ptr,&sz,"Work.Company");
    saveLNTS(icq,usr,&ptr,&sz,"Work.Department");
    saveLNTS(icq,usr,&ptr,&sz,"Work.Position");

    sprintf(buffer,"%d", (int) *((USHORT *) ptr));
    ptr += 2;
    DBGMessage(buffer);
    icqSaveUserDB(icq, usr, "Work.Occupation", buffer);


    saveLNTS(icq,usr,&ptr,&sz,"Work.Homepage");
    return 0;
 }

 static int _System metaAbout(HICQ icq, ICQV8 *cfg, int sz, ULONG req, struct searchResponse *response)
 {
    HUSER       usr;
    ULONG       uin  = icqQuerySearchUIN(icq,response->seq);
    char        *ptr = (char *) (response+1);

    usr = icqQueryUserHandle(icq,uin);

    if(usr)
       icqSaveUserDB(icq, usr, "About", (ptr+2));

    return 0;
 }

 static int _System metaInterest(HICQ icq, ICQV8 *cfg, int sz, ULONG req, struct searchResponse *response)
 {
 /*
    BYTE  xx  COUNT  The number of following interests. May be zero. Each interest consists of the following parameters:
    WORD.L xx xx INTER The interest area according to a table. *TODO*
    LNTS xx xx ... DESCRIPTION A description of the user's interest in this area.
  */
    ULONG       uin  = icqQuerySearchUIN(icq,response->seq);
    HUSER       usr = icqQueryUserHandle(icq,uin);
    char        *ptr = (char *) (response+1);
    char        buffer[0x0100];
    char        key[20];
    USHORT      szString;
    USHORT      szBuffer;
    int         qtd  = *(ptr++);
    int         f;

    if(!usr)
       return 0;

    DBGTrace(qtd);

    for(f=0;f<qtd;f++)
    {
       sprintf(buffer,"%d,",(int) *( (USHORT *) ptr) );
       ptr += 2;

       szString = * ((USHORT *) ptr);
       DBGTrace(szString);
       szBuffer = strlen(buffer);
       ptr += 2;


       strncpy(buffer+szBuffer,ptr,min((0xFF-szBuffer),szString) );
       *(buffer+0xFF) = 0; // Just in case

       sprintf(key,"Interest.%d",f);

       DBGMessage(key);
       DBGMessage(buffer);

       icqSaveUserDB(icq, usr, key, buffer);

       ptr += szString;

    }

    CHKPoint();
    return 0;
 }

 static int _System metaBackground(HICQ icq, ICQV8 *cfg, int sz, ULONG req, struct searchResponse *response)
 {
 /*
     BYTE  xx  COUNT  The number of background items to follow. May be zero. Each background item consists of the following two parameters:
     WORD.L xx xx PAST The group this background is in, according to a table. *TODO*
     LNTS xx xx ... DESCRIPTION A longer description of this background item.
     BYTE xx COUNT The number of affiliations to follow. May be zero. Each affiliation consists of the following parameters:
     WORD.L xx xx AFFILIATION The group this affiliation is in, according to a table. *TODO*
     LNTS xx xx ... DESCRIPTION A longer description of the affiliation.
  */

  CHKPoint();
  icqEndSearch(icq, response->seq);
  return 0;
 }

 static int _System metaResponse(HICQ icq, ICQV8 *cfg, int sz, ULONG req, struct searchResponse *response)
 {
/*
WORD.L  xx xx   DATALEN  The length of the following data.
DWORD.L  xx xx xx xx  UIN  The user's UIN.
LNTS  xx xx ...  NICK  The user's nick name.
LNTS  xx xx ...  FIRST  The user's first name.
LNTS  xx xx ...  LAST  The user's last name.
LNTS  xx xx ...  EMAIL  The user's email address.
BYTE  xx  FLAGS  Publish email address? 1 = yes, 0 = no.
WORD.L  xx xx  STATUS  0 = Offline, 1 = Online, 2 = not Webaware.
BYTE  xx  SEX  The user's gender. 1 = female, 2 = male, 0 = not specified.
WORD.L  xx xx  AGE  The user's age.
VOID
*/
    return processSearchResponse(icq, cfg, sz, FALSE, response);
 }

 static int _System metaLast(HICQ icq, ICQV8 *cfg, int sz, ULONG req, struct searchResponse *response)
 {
    int rc;
/*
    WORD.L      xx xx           DATALEN  The length of the following data.
    DWORD.L     xx xx xx xx     UIN The user's UIN.
  0 LNTS        xx xx ...       NICK The user's nick name.
  1 LNTS        xx xx ...       FIRST The user's first name.
  2 LNTS        xx xx ...       LAST The user's last name.
  3 LNTS        xx xx ...       EMAIL The user's email address.
    BYTE        xx              FLAGS Authorization flag. 1 = anyone can add to list, 0 = authorization required.
    WORD.L      xx xx           STATUS 0 = Offline, 1 = Online, 2 = not webaware.
    BYTE        xx              SEX The user's gender. 1 = female, 2 = male, 0 = not specified.
    WORD.L      xx xx           AGE The user's age.
    DWORD.L     xx xx xx xx     MISSED The number of users not returned that matched this search.
    VOID
*/
    DBGTrace(response->result);

    rc = processSearchResponse(icq, cfg, sz, TRUE, response);

    icqEndSearch(icq, response->seq);

    return rc;
 }

 static int searchFailed(HICQ icq, ICQSEARCHRESPONSE *blk, USHORT sequence, char *buffer)
 {
    ULONG uin	= icqQuerySearchUIN(icq, sequence);
    HUSER usr;

    DBGTrace(uin);

    DBGMessage("A pesquisa falhou!");

    memset(blk,0,sizeof(ICQSEARCHRESPONSE));
    blk->id    = sizeof(ICQSEARCHRESPONSE);
    blk->uin   = uin;
    blk->flags = ICQSR_LAST|ICQSR_FAILED;
    icqSetSearchResponse(icq, sequence, blk);

    if(!uin)
       return 0;

    usr = icqQueryUserHandle(icq,uin);

    if(usr)
    {
       usr->flags &= ~USRF_REFRESH;

       if(usr->flags & USRF_TEMPORARY)
       {
#ifdef EXTENDED_LOG
          icqWriteSysLog(icq,PROJECT,"Search failed on temporary user!");
#endif
          switch(icqLoadValue(icq,"ifUpdateFails",0))
          {
          case 0:        // Keep user
             break;

          case 1:        // Remove only if no messages
             if(!icqQueryMessage(icq, uin))
             {
                sprintf(buffer,"Rejecting ICQ#%lu (no messages)",uin);
                icqWriteSysLog(icq,PROJECT,buffer);
                sendAutoRemoveReply(icq,uin);
                icqRejectUser(icq,usr,TRUE);
             }
             break;

          case 2:        // Remove with messages
             sprintf(buffer,"Rejecting ICQ#%lu",uin);
             icqWriteSysLog(icq,PROJECT,buffer);
             sendAutoRemoveReply(icq,uin);
             icqRejectUser(icq,usr,TRUE);
             break;
          }
       }
    }

    return 0;
 }

#ifdef DEBUG
 int debug_SearchResponse(HICQ icq, ICQV8 *cfg, int sz, BOOL last, char *response)
 {
    return processSearchResponse(icq, cfg, sz, last, (struct searchResponse *) response);
 }
#endif

 static int processSearchResponse(HICQ icq, ICQV8 *cfg, int sz, BOOL last, struct searchResponse *response)
 {

  #pragma pack(1)

  struct _header
  {
     USHORT     sz;
     ULONG      uin;
  }             *h              = (struct _header *) (response+1);

  struct _part2
  {
     UCHAR      authorize;
     USHORT     status;
     UCHAR      sex;
     USHORT     age;
  }             *d;

  int           	f;
  int           	bytes;
  int           	strSize;
  HUSER         	usr;
  char			buffer[sizeof(ICQSEARCHRESPONSE)+0x0100];
  char			*ptr;
  ICQSEARCHRESPONSE	*blk		= NULL;

#ifdef EXTENDED_LOG
  icqDumpPacket(icq, NULL, "Search response", sz, (char *) response);
#endif

  DBGTrace(sz);
  DBGTrace(last);
  DBGTrace(response->result);

  if(sz <= sizeof(struct searchResponse) || response->result == 50)
  {
     sprintf(buffer,"Search failed (rc=%d)",response->result);
     icqWriteSysLog(icq,PROJECT,buffer);
     return searchFailed(icq,(ICQSEARCHRESPONSE *) buffer, response->seq, buffer);
  }

  if(response->result != 10)
  {
     sprintf(buffer,"Unexpected search result %d",response->result);
     icqWriteSysLog(icq,PROJECT,buffer);
     return -1;
  }

  ptr = (char *) (h+1);
  for(f=bytes=0;f<4;f++)
  {
     strSize  =  *((USHORT *) ptr) + 2;
     ptr     += strSize;
     bytes   += strSize;
     bytes++;
  }

  d = (struct _part2 *) ptr;

  DBGTrace(bytes);

  blk = malloc(bytes + sizeof(ICQSEARCHRESPONSE) +6);

  if(!blk)
  {
     icqWriteSysLog(icq,PROJECT,"Memory allocation error when processing search response");
  }
  else
  {
     memset(blk,0,bytes+sizeof(ICQSEARCHRESPONSE));
     blk->sz     = bytes;
     blk->id     = sizeof(ICQSEARCHRESPONSE);
     blk->uin    = h->uin;
     blk->status = d->status;

     if(last)
        blk->flags = ICQSR_LAST;

     blk->sex	= d->sex;
     blk->age   = d->age;

     if(d->authorize)
        blk->flags |= ICQSR_ALWAYSAUTH;

     ptr = (char *) (h+1);
     for(f=bytes=0;f<4;f++)
     {
        strSize                =  *((USHORT *) ptr);

        ptr                   += 2;
        blk->offset[f]         = bytes;

        DBGTrace(bytes);
        strncpy(blk->text+bytes,ptr,strSize);
        icqConvertCodePage(icq, TRUE, blk->text+bytes, -1);

        ptr                   += strSize;
        bytes                 += strSize+1;

     }

     CHKPoint();
     DBGMessage(getSearchNickname(blk));
     DBGMessage(getSearchFirstname(blk));
     DBGMessage(getSearchLastname(blk));
     DBGMessage(getSearchEMail(blk));

     DBGTrace(d->authorize);
     DBGTracex(d->status);
     DBGTracex(d->sex);
     DBGTrace(d->age);

     CHKPoint();
     DBGTrace(h->uin);

     if(h->uin == icqQueryUIN(icq))
     {
        icqWriteSysLog(icq,PROJECT,"Updating my own information");

        DBGMessage("Atualizar meu proprio cadastro");

        icqSaveValue(icq,  "Authorize",  d->authorize ? 0 : 1);
        icqSaveString(icq, "EMail",      getSearchEMail(blk));
        icqSaveString(icq, "FirstName",  getSearchFirstname(blk));
        icqSaveString(icq, "LastName",   getSearchLastname(blk));
        icqSaveString(icq, "NickName",   getSearchNickname(blk));
        icqSaveValue(icq,  "Sex",        d->sex);
        icqSaveValue(icq,  "Age",        d->age);

     }
     else if(icqQueryUserHandle(icq,h->uin))
     {
        DBGMessage("Atualizar cadastro do usuario");

#ifdef EXTENDED_LOG
        icqWriteSysLog(icq,PROJECT,"Updating contact-list from search result");
#endif
        usr = icqUpdateUserInfo(	icq,
        				h->uin,
        				FALSE,
     					getSearchNickname(blk),
     					getSearchFirstname(blk),
     					getSearchLastname(blk),
     					getSearchEMail(blk),
        				!d->authorize );

        if(usr)
        {
           if(usr->flags & USRF_ONLIST)
              blk->flags |= ICQSR_ONLIST;
           else
              blk->flags |= ICQSR_CACHED;
           usr->sex = d->sex;
           usr->age = d->age;
        }

#ifdef EXTENDED_LOG
        icqWriteSysLog(icq,PROJECT,"Update Ok!");
#endif
     }

     DBGTracex(blk->flags);
     icqSetSearchResponse(icq, response->seq, blk);
     CHKPoint();

     free(blk);
  }


  return 0;

 }

 static void sendAutoRemoveReply(HICQ icq, ULONG uin)
 {
    char buffer[0x0100];

    if(!icqLoadValue(icq,"notifyIfRemoved",0))
       return;

    icqLoadString(icq, "AutoRemoveMsg", "", buffer, 0xFF);

    if(*buffer)
       icqSendMessage(icq, uin, MSG_NORMAL, buffer);

 }

 static int _System metaRandom(HICQ icq, ICQV8 *cfg, int sz, ULONG req, struct searchResponse *response)
 {
    #pragma pack(1)

    struct _header
    {
       ULONG	uin;		// WORD.L  xx xx xx xx  UIN  The UIN found in chat group.
       USHORT   group;		// WORD.L xx 00 GROUP The chat group the UIN is in.
       ULONG	gateway;	// IP xx xx xx xx EXTERNAL IP The external IP of the UIN as the server sees it.
       ULONG	port;		// DWORD.L xx xx xx xx PORT The port the UIN is listening on for direct connections.
       ULONG	ip;		// IP xx xx xx xx INTERNAL IP The internal IP address of the UIN.
       UCHAR	tcpFlag;	// BYTE xx TCPFLAG Guess: Direct connection connectivity.
       USHORT	peerVersion;	// WORD.L xx xx TCPVER The version of the p2p protocol used.		
				// 14 ... UNKNOWN Unknown. Seen:
    } *h = (struct _header *) (response +1);

    HUSER		usr;

    DBGTrace(h->uin);
    DBGTrace(h->group);

    DBGTrace(response->seq);

    usr = icqCacheSearchResult(icq,response->seq,h->uin,TRUE);

    DBGTracex(usr);

    if(usr)
    {
       usr->peerVersion = h->peerVersion;
       icqSetUserIPandPort(icq, h->uin, h->gateway, h->ip, h->port);
#ifdef EXTENDED_LOG
       icqWriteSysLog(icq,PROJECT,"Random search result was cached");
#endif
    }
#ifdef EXTENDED_LOG
    else
    {
       icqWriteSysLog(icq,PROJECT,"Failure caching random search result");
    }
#endif

    return 0;
 }

