/*
 * SNAC15.C
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>
 #include <time.h>

 #include "icqv8.h"

/*---[ Defines ]----------------------------------------------------------------------------------------------*/

 #pragma pack(1)

 struct fromICQ
 {
    USHORT  tlv1_prefix;
    USHORT  tlv1_sz;

    USHORT  sz1;

    ULONG   uin;
    USHORT  command;

 };

 struct icqcmd
 {
    USHORT command;
    int    ( * _System exec)(HICQ,ICQV8 *, int, ULONG, void *);
 };

 #define ICQ_CMD_PREFIX (int (* _System)(HICQ,ICQV8 *, int, ULONG, void *))

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

 static int _System fromOldICQ(HICQ, ICQV8 *, int, ULONG, struct fromICQ *);


 static int _System oldICQError(HICQ, ICQV8 *, int, ULONG, UCHAR *);
 static int _System doneOfflineMsgs(HICQ, ICQV8 *, int, ULONG, UCHAR *);
 static int _System offlineMessage(HICQ, ICQV8 *, int, ULONG, UCHAR *);

/*---[ Packet processors ]------------------------------------------------------------------------------------*/

 const SNAC_CMD icqv8_snac15_table[] =
 {
    { 0x01,   SNAC_CMD_PREFIX   oldICQError            },
    { 0x03,   SNAC_CMD_PREFIX   fromOldICQ             },

    { 0x00,                     NULL                   }
 };

 const struct icqcmd icqv8_cmd_table[] =
 {
        { 0x07da,		ICQ_CMD_PREFIX icqv8_searchResult 	},
        { 0x0042,		ICQ_CMD_PREFIX doneOfflineMsgs		},
        { 0x0041,		ICQ_CMD_PREFIX offlineMessage		},

 	{ 0x0000,		NULL					}
 };


/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 static int _System fromOldICQ(HICQ icq, ICQV8 *cfg, int sz, ULONG req, struct fromICQ *pkt)
 {
    char                buffer[0x0100];
    const struct icqcmd *cmd;

    if(SHORT_VALUE(pkt->tlv1_prefix) != 0x0001)
    {
       icqDumpPacket(icq, NULL, "Unexpected TLV in SNAC 15,03", sz, (char *) pkt);
       return -1;
    }

    if(SHORT_VALUE(pkt->tlv1_sz) != (sz-4))
    {
       icqDumpPacket(icq, NULL, "Unexpected TLV size in SNAC 15,03", sz, (char *) pkt);
       return -1;
    }

    for(cmd = icqv8_cmd_table;cmd->exec;cmd++)
    {
       if(pkt->command == cmd->command)
       {
          if(cmd->exec(icq,cfg,sz,req, (void *) (pkt+1)))
          {
             sprintf(buffer,"Error processing SNAC 0x15:0x03 0x%04x",pkt->command);
             icqDumpPacket(icq, NULL, buffer, sz, (char *) pkt);
          }
          return 0;
       }
    }

    sprintf(buffer,"Unexpected SNAC 0x15:0x03 0x%04x",(int) pkt->command);
    icqDumpPacket(icq, NULL, buffer, sz, (char *) pkt);

    return 0;
 }

 static int _System doneOfflineMsgs(HICQ icq, ICQV8 *cfg, int sz, ULONG req, UCHAR *pkt)
 {
    icqWriteSysLog(icq,PROJECT,"End of server's message list");
    icqv8_toICQServer(icq, cfg, 0x003e, 0, 0, NULL);  //  CLI_ACKOFFLINEMSGS
    return 0;
 }

 static int _System oldICQError(HICQ icq, ICQV8 *cfg, int sz, ULONG req, UCHAR *pkt)
 {
    icqDumpPacket(icq, NULL, "OldICQ request was malformed (SNAC 0x15/0x01)", sz, (char *) pkt);
    return 0;
 }

#ifdef DEBUG
 int debug_offlineMessage(HICQ icq, ICQV8 *cfg, int sz, char *pkt)
 {
    return offlineMessage(icq, cfg, sz, 0, (UCHAR *) pkt);
 }
#endif

 static int _System offlineMessage(HICQ icq, ICQV8 *cfg, int sz, ULONG req, UCHAR *pkt)
 {
#ifdef EXTENDED_LOG
    char	logLine[0x0100];
#endif
    struct tm 	tm;
    time_t	msgTime;

    #pragma pack(1)
    struct _header
    {
       USHORT	x1;
       ULONG	uin;		// DWORD.L  xx xx xx xx  UIN  The UIN you're receiving a message from.
       USHORT	year;		// WORD.L xx xx YEAR The year (in UTC time) the message was sent.
       UCHAR    month;		// BYTE xx MONTH The month (in UTC) the message was sent.
       UCHAR	day;		// BYTE xx MDAY The day of the month (in UTC) this message was sent.
       UCHAR    hour;		// BYTE xx HOUR The hour (in UTC) this message was sent.
       UCHAR	min;		// BYTE xx MIN The minutes (in UTC) this message was sent.
       USHORT	type;		// WORD.L xx xx TYPE The type of message sent, like URL message or the like.
       USHORT	msgSize;	// LNTS xx xx ... TEXT The text of the message.
    } *h = (struct _header *) pkt;


    DBGTrace(h->uin);
    DBGTrace(h->type);
    DBGTrace(h->year);
    DBGTrace(h->month);
    DBGTrace(h->day);
    DBGTrace(h->hour);
    DBGTrace(h->min);
    DBGTrace(h->msgSize);
    DBGTrace(strlen((char *)(h+1)));

    memset(&tm,0,sizeof(tm));

//    tm.tm_sec;      		/* seconds after the minute [0-61]        */
    tm.tm_min	= h->min;       /* minutes after the hour [0-59]          */
    tm.tm_hour	= h->hour;      /* hours since midnight [0-23]            */
    tm.tm_mday	= h->day;       /* day of the month [1-31]                */
    tm.tm_mon	= h->month-1;   /* months since January [0-11]            */
    tm.tm_year	= h->year-1900; /* years since 1900                       */
//    tm.tm_wday;     /* days since Sunday [0-6]                */
//    tm.tm_yday;     /* days since January 1 [0-365]           */
//    tm.tm_isdst;    /* Daylight Saving Time flag              */


//    DBGMessage( (h+1) );
    msgTime = mktime(&tm);

    DBGTracex(msgTime);

    DBGMessage(ctime(&msgTime));

#ifdef EXTENDED_LOG
    sprintf(logLine,"Offline message ICQ:%ld %d\\%d\\%d %d:%d T:%d",
    				h->uin,
    				h->month,
    				h->day,
    				h->year,
    				h->hour,
    				h->min,
    				h->type );
    				
    icqDumpPacket(icq, NULL, logLine, sz, pkt);
#endif

    if(h->msgSize > (sz - sizeof(struct _header)) )
    {
       icqDumpPacket(icq, NULL, "Invalid textsize in offline message", sz, pkt);
       return -1;
    }

    icqInsertMessage(icq, h->uin, 0, h->type, msgTime, 0, h->msgSize, (const char *) (h+1));

    return 0;
 }


