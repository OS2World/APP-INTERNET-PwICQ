/*
 * meta.c - Meta Packets
 *
 * All packets are sub-packets of CLI_META and thus channel 2,
 * SNAC (21,2) = SNAC(0x15,0x2) with subcommand 2000.
 *
 * http://www.stud.uni-karlsruhe.de/~uck4/ICQ/index_cli_meta.html
 *
 */


#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <malloc.h>

 #include "icqv8.h"

/*---[ Meta commands ]----------------------------------------------------------------------------------------*/

 #define CLI_METAREQMOREINFO 0x04b2     //      Request information about given UIN.
 #define CLI_METAREQINFO     0x04d0     //      Request information about another user.
 #define CLI_SEARCHBYUIN     0x0569     //      Searches user by UIN.
 #define CLI_SEARCHBYMAIL    0x0573 	//	Searchs using E-Mail
 #define CLI_SEARCHBYPERSINF 0x055F	//	Search for a user by most common options.
 #define CLI_SEARCHRANDOM    0x074e	//	Ask for a random UIN from a user in given chat group.

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 #pragma pack(1)

 struct metaprefix
 {
    USHORT      sz;             // 2 xx xx LENGTH The remaining number of bytes in the packet.
    ULONG       uin;            // 4 xx xx xx xx UIN The user's UIN.
    USHORT      type;           // 2 xx xx COMMAND The command for the ICQ server:
    USHORT      sequence;       // 2 xx xx SEQUENCE A one-up sequence starting at 2. It should be noted tha
    USHORT      subType;        // SubType;
 };

 #define METAPREFIX(x1,subType) { 0, 0, 0, x1, subType }

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 static int metaPacket(HICQ icq, ULONG request, int sz, struct metaprefix *prefix)
 {
    ICQV8  *cfg = icqGetPluginDataBlock(icq, module);
    PACKET pkt  = icqv8_allocatePacket(icq, cfg, sz+8);

    prefix->sz       = sz - 2;
    prefix->uin      = icqQueryUIN(icq);
    prefix->type     = 0x07D0;
    prefix->sequence = cfg->toICQSrvSeq++;

    icqv8_insertBlock(pkt, 0x01, sz, (const unsigned char *) prefix);

    icqv8_dumpPacket(pkt, icq, "Meta packet sent");

    DBGTrace(request);
    return icqv8_sendSnac(icq, cfg, 0x15, 0x02, request, pkt);

 }

 HSEARCH _System icqv8_searchByUIN(HICQ icq, ULONG uin, int searchs)
 {
    int rc;
    struct _pkt
    {
       struct metaprefix        p;
       USHORT                   key;    // WORD.L  36 01  KEY  The key to search for: 0x136 = 310 = UIN number.
       USHORT                   sz;     // WORD.L 04 00 DATALEN Length of the following data.
       ULONG                    uin;    // DWORD.L xx xx xx xx UIN UIN of the user to search to.

    } pkt = { METAPREFIX(0x0040, CLI_SEARCHBYUIN), 0x0136, 0x0004, uin };

    rc = metaPacket(icq, 0, sizeof(struct _pkt), (struct metaprefix *) &pkt);

    DBGTrace(rc);

    DBGTrace(pkt.p.sequence);

    return icqBeginSearch(icq, uin, pkt.p.sequence, 0, 0, 0);
 }

 int _System icqv8_requestUserInfo(HICQ icq, ULONG uin, int searchs)
 {
    int rc;
    struct _pkt
    {
       struct metaprefix        p;
       ULONG                    uin;
    } pkt                               = { METAPREFIX(0x0002, CLI_METAREQINFO), uin } ;

    rc = metaPacket(icq, 0, sizeof(struct _pkt), (struct metaprefix *) &pkt);

    DBGTracex(icqQueryUserHandle(icq,uin));

    icqBeginSearch(icq, uin, pkt.p.sequence, 0, 0, 0);

    return rc;
 }


 int _System icqv8_searchByICQ(HICQ icq, ULONG uin, HWND hwnd, SEARCHCALLBACK callBack)
 {
    int rc;
    struct _pkt
    {
       struct metaprefix        p;
       USHORT                   key;    // WORD.L  36 01  KEY  The key to search for: 0x136 = 310 = UIN number.
       USHORT                   sz;     // WORD.L 04 00 DATALEN Length of the following data.
       ULONG                    uin;    // DWORD.L xx xx xx xx UIN UIN of the user to search to.

    } pkt = { METAPREFIX(0x0040, CLI_SEARCHBYUIN), 0x0136, 0x0004, uin };

    rc = metaPacket(icq, 0, sizeof(struct _pkt), (struct metaprefix *) &pkt);

    DBGTrace(uin);
    DBGTrace(pkt.p.sequence);

    icqBeginSearch(icq, uin, pkt.p.sequence, hwnd, TRUE, callBack);

    return rc;
 }

 int _System icqv8_searchByMail(HICQ icq, const char *key, HWND hwnd, SEARCHCALLBACK callBack)
 {
    int   rc;

    #pragma pack(1)
    struct _pkt
    {
       struct metaprefix        p;
       USHORT			key;	// WORD.L 	5e 01 		KEY The key to search for: 0x15e = 350 = email address.
       USHORT			x1;
       USHORT			sz;	// LLNTS 	xx xx ... 	EMAIL The email address to search for.
       char			mail[0x0101];
    } pkt = { METAPREFIX(0x0040, CLI_SEARCHBYMAIL), 0x015e, 0x000E, strlen(key)+1, "" };

/*
	CLI_SEARCHBYMAIL	Channel: 2 SNAC(0x15,0x2) 2000/1395

	WORD.L 	5e 01 		KEY The key to search for: 0x15e = 350 = email address.
	LLNTS 	xx xx ... 	EMAIL The email address to search for.
*/

    if(pkt.sz > 0xFF)
    {
       icqWriteSysLog(icq,PROJECT,"E-Mail address is too large!");
       pkt.sz = 0xFF;
    }

    strncpy(pkt.mail,key,0x0100);
    icqConvertCodePage(icq, FALSE, pkt.mail, pkt.sz);

    rc = metaPacket(icq, 0, sizeof(struct metaprefix) + 6 + pkt.sz, (struct metaprefix *) &pkt);

    DBGTrace(pkt.p.sequence);

    icqBeginSearch(icq, 0, pkt.p.sequence, hwnd, TRUE, callBack);

    return rc;
 }

 int _System icqv8_searchRandom(HICQ icq, USHORT key, HWND hwnd, SEARCHCALLBACK callBack)
 {
/*
         SNAC (15,2) [CLI_TOICQSRV] flags 0 ref 0
         00 01 00 0e   0c 00 c7 54 - 7f 09 d0 07   02 00 4e 07   .... ..ÇT .Ð. ..N.
         04 00


         00 01 00 0e   0c 00 dc 58   68 00 d0 07   00 00 4e 07   ......ÜXh.Ð...N.
         04 00                                             ..

        CLI_SEARCHRANDOM Channel: 2 SNAC(0x15,0x2) 2000/1870

	WORD.L xx xx GROUP The random chat group to request a UIN from.
	1 - General
	2 - Romance
	3 - Games
	4 - Students
	6 - 20 something
	7 - 30 something
	8 - 40 something
	9 - 50+
	10 - man requesting woman
	11 - woman requesting man
*/
    static const UCHAR table[] = { 1, 2, 3, 4, 6, 7, 8, 9, 10, 11 };

    int rc;

    #pragma pack(1)
    struct _pkt
    {
       struct metaprefix        p;
       USHORT			key;	
    } pkt = { METAPREFIX(0x0040, CLI_SEARCHRANDOM), 0 };

    DBGTrace(sizeof(table));
    DBGTrace(key);

    if(key > sizeof(table))
       return -1;

    DBGTrace(key);

    key = table[key];

    DBGTrace(key);
    pkt.key = key;

    rc = metaPacket(icq, 0, sizeof(pkt), (struct metaprefix *) &pkt);

    DBGTrace(pkt.p.sequence);

    icqBeginSearch(icq, 0, pkt.p.sequence, hwnd, TRUE, callBack);

    return rc;
 }

 int _System icqv8_searchByInformation( HICQ icq, 	HWND		hwnd,
 							const char 	*firstName,
 							const char 	*lastName,
 							const char 	*nickName,
 							const char 	*email,
 							BOOL		online,
 							SEARCHCALLBACK  callBack )
 { 									
/*

	CLI_SEARCHBYPERSINFChannel: 2 SNAC(0x15,0x2) 2000/1375
	
	WORD.L  40 01  	  KEY  The key to search for: 0x140 = 320 = first name.
	LLNTS 	xx xx ... FIRST The first name to search for.
	WORD.L 	4a 01 	  KEY The key to search for: 0x14a = 330 = last name
	LLNTS 	xx xx ... LAST The last name to search for.
	WORD.L 	54 01 	  KEY The key to search for: 0x154 = 340 = nick.
	LNTS 	xx xx ... NICK The nick name to search for.
	WORD.L 	5e 01 	  KEY The key to search for: 0x15e = 350 = email address.
	LNTS 	xx xx ... EMAIL The email address to search for.
	BYTE 01 ONLINE 	  search only online users
	
20:46:55 Outgoing v8 server packet: FLAP seq 000001f0 length 003e channel 2
         SNAC (15,2) [CLI_TOICQSRV] flags 0 ref 0

    USHORT      sz;             // 2 xx xx LENGTH The remaining number of bytes in the packet.
    ULONG       uin;            // 4 xx xx xx xx UIN The user's UIN.
    USHORT      type;           // 2 xx xx COMMAND The command for the ICQ server:
    USHORT      sequence;       // 2 xx xx SEQUENCE A one-up sequence starting at 2. It should be noted tha
    USHORT      subType;        // SubType;

         00 01 00 30   2e 00 8f 69 - e7 04 d0 07   02 00 5f 05   ...0 ...i ç.Ð. .._.
         40 01 0a 00   08 00 52 fc - 64 69 67 65   72 00 4a 01   @... ..Rü dige r.J.
         0b 00 09 00   4b 75 68 6c - 6d 61 6e 6e   00 54 01 03   .... Kuhl mann .T..
         00 01 00 00	

         00 01 00 30

         2e 00		Size
         8f 69 e7 04	UIN

         d0 07		Type	
         02 00		Sequence
         5f 05		Subtype

         40 01
         0a 00
         08 00
         52 fc 64 69	Rüdiger
         67 65 72 00

         4a 01
         0b 00
         09 00
         4b 75 68 6c	Kuhlmann
         6d 61 6e 6e
         00

         54 01		No NickName
         03 00
         01 00
         00

*/

    int			bytes;
    int			f;
    struct metaprefix	*pkt;
    char		*ptr;
    int			sz;
    int			rc;

    struct _keys
    {
    	USHORT		searchKey;
    	USHORT		code;
    	const char	*val;
    }		key[]		= { 	{ 0x0140,	0x000A,		firstName	},
              					{ 0x014a,	0x000B,		lastName	},
              					{ 0x0154,	0x0003,		nickName	},
              					{ 0x015e,	0x000E,		email		}
              				  };	

    DBGTracex(firstName);
    DBGTracex(lastName);
    DBGTracex(nickName);
    DBGTracex(email);

    for(f=bytes=0;f<4;f++)
    {
       if(key[f].val)
          bytes += strlen(key[f].val)+7;
    }
    DBGTrace(bytes);

    bytes += sizeof(struct metaprefix);

    DBGTrace(bytes);

    pkt = malloc(bytes+5);

    if(!pkt)
    {
       icqWriteSysLog(icq,PROJECT,"Memory allocation error when creating search packet");
       return -1;
    }

    memset(pkt,0,bytes);

    pkt->subType = CLI_SEARCHBYPERSINF;
    ptr		 = (char *) (pkt+1);

    for(f=0;f<4;f++)
    {
       if(key[f].val)
       {
          DBGMessage(key[f].val);
          sz = strlen(key[f].val)+1;
          DBGTrace(sz);

          *( (USHORT *) ptr ) = key[f].searchKey;
          ptr += 2;
          *( (USHORT *) ptr ) = key[f].code;
          ptr += 2;
          *( (USHORT *) ptr ) = sz;
          ptr += 2;

          strcpy(ptr,key[f].val);
          icqConvertCodePage(icq, FALSE, ptr, -1);
          ptr += sz;

       }
    }

    rc = metaPacket(icq, 0, bytes, pkt);

    DBGTrace(pkt->sequence);

    icqBeginSearch(icq, 0, pkt->sequence, hwnd, TRUE, callBack);

    free(pkt);

    return rc;
 }


