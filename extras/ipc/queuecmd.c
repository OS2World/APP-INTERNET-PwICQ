/*
 * QUEUECMD.C - Envia comandos atraves da fila de mensagems
 */


#ifdef __OS2__
 #pragma strings(readonly)
 #include <io.h>
#else
 #include <unistd.h>
#endif

 #include <string.h>
 #include <stdlib.h>
 #include <malloc.h>

 #include <pwMacros.h>

 #include <icqtlkt.h>
 #include <icqqueue.h>

/*---[ Prototipos ]----------------------------------------------------------------------------------------*/


/*---[ Implementacao ]-------------------------------------------------------------------------------------*/

 int EXPENTRY icqIPCSetMode(ULONG uin, ULONG mode)
 {
    struct icqqueuesetmode cmd;

    memset(&cmd,0,sizeof(struct icqqueuesetmode));

    cmd.h.szPrefix      = sizeof(ICQQUEUEBLOCK);                // Size of ICQQUEUEBLOCK (Version check)
    cmd.h.sz            = sizeof(struct icqqueuesetmode);       // Size of datablock
    cmd.h.cmd           = ICQCMD_SETMODE;                       // Command type
    cmd.h.rc            = 0;                                    // Return code
    cmd.h.status        = 0;                                    // Status code

    cmd.mode            = mode;                                 // Mode to set

    return icqIPCSendCommand(uin, (ICQQUEUEBLOCK *) &cmd);
 }

 int EXPENTRY icqIPCSendCMD(ULONG uin, UCHAR cmd)
 {
    ICQQUEUEBLOCK       blk;

    memset(&blk,0,sizeof(ICQQUEUEBLOCK));

    blk.szPrefix        =                               // Size of ICQQUEUEBLOCK (Version check)
    blk.sz              = sizeof(ICQQUEUEBLOCK);        // Size of datablock
    blk.cmd             = cmd;                          // Command type
    blk.rc              = 0;                            // Return code
    blk.status          = 0;                            // Status code

    return icqIPCSendCommand(uin, &blk);

 }

 static int EXPENTRY sendString(ULONG uin, UCHAR cmd,const char *str)
 {
    int            sz   = sizeof(ICQQUEUEBLOCK)+strlen(str)+2;
    ICQQUEUEBLOCK  *blk;
    int            rc;

    blk = malloc(sz);

    if(!blk)
       return -1;

    memset(blk,0,sz);
    blk->szPrefix        = sizeof(ICQQUEUEBLOCK);        // Size of ICQQUEUEBLOCK (Version check)
    blk->sz              = sz;                           // Size of datablock
    blk->cmd             = cmd;                         // Command type
    blk->rc              = 0;                            // Return code
    blk->status          = 0;                            // Status code

    strcpy( (char *) (blk+1), str);

    rc = icqIPCSendCommand(uin, blk);

    free(blk);
    return rc;

 }

 int EXPENTRY icqIPCSetModeByName(ULONG uin,const char *modeName)
 {
    return sendString(uin,ICQCMD_MODEBYNAME,modeName);
 }

 int EXPENTRY icqIPCWriteLog(ULONG uin,const char *txt)
 {
    return sendString(uin,ICQCMD_WRITELOG,txt);
 }

 ULONG EXPENTRY icqIPCQueryBuild(void)
 {
    return XBUILD;
 }

 int EXPENTRY icqIPCQueryString(ULONG uin, const char *key, int szBuffer, char *ret)
 {
    int            strSize  = max(szBuffer,strlen(key))+2;
    int            sz       = sizeof(ICQQUEUEBLOCK)+strSize;
    ICQQUEUEBLOCK  *blk;
    int            rc;

    blk = malloc(sz);

    if(!blk)
       return -1;

    memset(blk,0,sz);
    blk->szPrefix        = sizeof(ICQQUEUEBLOCK);        // Size of ICQQUEUEBLOCK (Version check)
    blk->sz              = sz;                           // Size of datablock
    blk->cmd             = ICQCMD_QUERYCONFIGSTRING;     // Command type
    blk->rc              = 0;                            // Return code
    blk->status          = 0;                            // Status code

    strncpy( (char *) (blk+1), key, strSize);

    rc = icqIPCSendCommand(uin, blk);

    strncpy(ret,(char *)(blk+1),szBuffer);

    free(blk);
    return rc;

 }

 int EXPENTRY icqIPCExecuteCommand(ULONG uin, const char *cmd)
 {
    int                 strSize = strlen(cmd)+1;
    int                 sz      = sizeof(ICQQUEUEBLOCK)+strSize;
    ICQQUEUEBLOCK       *blk;
    int                 rc;

    blk = malloc(sz);

    if(!blk)
       return -1;

    memset(blk,0,sz);
    blk->szPrefix        = sizeof(ICQQUEUEBLOCK);       // Size of ICQQUEUEBLOCK (Version check)
    blk->sz              = sz;                          // Size of datablock
    blk->cmd             = ICQCMD_EXEC;                 // Command type
    blk->rc              = 0;                           // Return code
    blk->status          = 0;                           // Status code

    strncpy( (char *) (blk+1), cmd, strSize);

    rc = icqIPCSendCommand(uin, blk);

    free(blk);
    return rc;

 }

 int EXPENTRY icqIPCShowPopupMenu(ULONG uin, USHORT menu)
 {
    struct icqqueuesetmode cmd;

    memset(&cmd,0,sizeof(struct icqqueuesetmode));

    cmd.h.szPrefix      = sizeof(ICQQUEUEBLOCK);                // Size of ICQQUEUEBLOCK (Version check)
    cmd.h.sz            = sizeof(struct icqqueuesetmode);       // Size of datablock
    cmd.h.cmd           = ICQCMD_OPENMENU;                      // Command type
    cmd.h.rc            = 0;                                    // Return code
    cmd.h.status        = 0;                                    // Status code

    cmd.mode            = (ULONG) menu;                         // Mode to set

    return icqIPCSendCommand(uin, (ICQQUEUEBLOCK *) &cmd);
 }

 int EXPENTRY icqIPCOpenFirstMessage(ULONG uin)
 {
    return icqIPCSendCMD(uin,ICQCMD_OPENFIRST);
 }

 int EXPENTRY icqIPCShowUserList(ULONG uin)
 {
    return icqIPCSendCMD(uin,ICQCMD_POPUPUSERLIST);
 }

 int EXPENTRY icqIPCPluginRequest(ULONG uin,const char *plugin, const char *parm)
 {
    int                 sz;
    int                 rc;
    struct icqpluginipc *req;

    if(!parm)
       parm = "";

    sz = strlen(parm) + sizeof(struct icqpluginipc) + 5;

    req = malloc(sz);
    if(!req)
       return -1;

    memset(req,0,sz);
    req->h.szPrefix      = sizeof(ICQQUEUEBLOCK);                // Size of ICQQUEUEBLOCK (Version check)
    req->h.sz            = sz;                                   // Size of datablock
    req->h.cmd           = ICQCMD_PLUGIN;                        // Command type
    req->h.rc            = 0;                                    // Return code
    req->h.status        = 0;                                    // Status code

    strncpy(req->plugin,plugin,10);
    strcpy( (char *) (req+1), parm);

    rc = icqIPCSendCommand(uin, (ICQQUEUEBLOCK *) req);

    free(req);
	
	DBGTrace(rc);

    return rc;
 }

 int EXPENTRY icqIPCMPTRequest(ULONG uin, UCHAR request, ULONG parm)
 {
    struct icqqueuemptblock cmd;

    memset(&cmd,0,sizeof(struct icqqueuesetmode));

    cmd.h.szPrefix      = sizeof(ICQQUEUEBLOCK);                // Size of ICQQUEUEBLOCK (Version check)
    cmd.h.sz            = sizeof(struct icqqueuesetmode);       // Size of datablock
    cmd.h.cmd           = ICQCMD_MPTREQUEST;                    // Command type
    cmd.h.rc            = 0;                                    // Return code
    cmd.h.status        = 0;                                    // Status code
    cmd.request		= request;
    cmd.uin		= parm;

    return icqIPCSendCommand(uin, (ICQQUEUEBLOCK *) &cmd);

 }

 ULONG EXPENTRY icqIPCQueryOnlineMode(ULONG uin, int sz, char *out)
 {
    struct icqqueuesetmode *cmd = malloc(sizeof(struct icqqueuesetmode *)+sz);
    ULONG		   rc   = 0xFFFFFFFF;

    if(!cmd)
       return rc;

    if(sz > 1 && out)
       *out = 0;

   memset(cmd,0,sizeof(struct icqqueuesetmode)+sz);

   cmd->h.szPrefix      = sizeof(ICQQUEUEBLOCK);                // Size of ICQQUEUEBLOCK (Version che
   cmd->h.sz            = 4+sz;       				// Size of datablock
   cmd->h.cmd           = ICQCMD_QUERYONLINEMODE;               // Command type
   cmd->h.rc            = 0;                                    // Return code
   cmd->h.status        = 0;                                    // Status code
   cmd->mode		= 0xFFFFFFFF;

   icqIPCSendCommand(uin, (ICQQUEUEBLOCK *) cmd);

   if(sz > 1 && out)
      strncpy(out,(char *)(cmd+1),sz);

   rc = cmd->mode;

   free(cmd);

   return rc;

 }

 int  EXPENTRY icqIPCConvertCodePage(ULONG uin, BOOL in, UCHAR *string, int strSize)
 {
    ULONG          rc;
    int            sz;
    ICQQUEUEBLOCK  *blk;
    char           *ptr;

    if(strSize < 0)
       strSize = strlen(string);

    sz = sizeof(ICQQUEUEBLOCK)+strSize+1;

    blk = malloc(sz);

    if(!blk)
       return -1;

    memset(blk,0,sz);
    blk->szPrefix        = sizeof(ICQQUEUEBLOCK);        // Size of ICQQUEUEBLOCK (Version check)
    blk->sz              = sz;                           // Size of datablock
    blk->cmd             = ICQCMD_TRANSLATECODEPAGE;     // Command type
    blk->rc              = 0;                            // Return code
    blk->status          = 0;                            // Status code

    ptr = (char *) (blk+1);

    *(ptr++) = in ? '1' : '0';

    strncpy( ptr, string, strSize);

    rc = icqIPCSendCommand(uin, blk);

    strncpy(string,ptr,strSize);

    free(blk);
    return rc;
 }

 int EXPENTRY icqIPCSetPeerInfo(ULONG uin, ULONG user ,long ip, USHORT port, USHORT version)
 {
    struct icqpeerinfo cmd;

    cmd.h.szPrefix      = sizeof(ICQQUEUEBLOCK);                // Size of ICQQUEUEBLOCK (Version che
    cmd.h.sz            = sizeof(cmd);		                // Size of datablock
    cmd.h.cmd           = ICQCMD_SETPEERINFO;                   // Command type
    cmd.h.rc            = 0;                                    // Return code
    cmd.h.status        = 0;                                    // Status code

    cmd.uin             = user;
    cmd.peerVersion     = version;
    cmd.peerCookie      = 0x19670123;
    cmd.ip              = ip;
    cmd.port            = port;

    return icqIPCSendCommand(uin, &cmd.h);

 }

 int EXPENTRY icqIPCPeerConnect(ULONG uin, ULONG user)
 {
    char           buffer[sizeof(ICQQUEUEBLOCK)+8];
    ICQQUEUEBLOCK  *blk                                = (ICQQUEUEBLOCK *) buffer;

    memset(blk,0,sizeof(ICQQUEUEBLOCK));

    blk->szPrefix          = sizeof(ICQQUEUEBLOCK);        // Size of ICQQUEUEBLOCK (Version check)
    blk->sz                = sizeof(ICQQUEUEBLOCK)+4;      // Size of datablock
    blk->cmd               = ICQCMD_ADDUSER;               // Command type
    *( (ULONG *) (blk+1) ) = user;

    return icqIPCSendCommand(uin, blk);

 }

 int EXPENTRY icqIPCAddUser(ULONG uin, ULONG user)
 {
    char           buffer[sizeof(ICQQUEUEBLOCK)+8];
    ICQQUEUEBLOCK  *blk                                = (ICQQUEUEBLOCK *) buffer;

    memset(blk,0,sizeof(ICQQUEUEBLOCK));

    blk->szPrefix          = sizeof(ICQQUEUEBLOCK);        // Size of ICQQUEUEBLOCK (Version check)
    blk->sz                = sizeof(ICQQUEUEBLOCK)+4;      // Size of datablock
    blk->cmd               = ICQCMD_ADDUSER;           // Command type
    *( (ULONG *) (blk+1) ) = user;

    return icqIPCSendCommand(uin, blk);
 }


 ULONG EXPENTRY icqIPCQueryAvailable(ULONG uin)
 {
    ULONG       rc   = 0;
    int         f;
    ICQSHAREMEM *icq = icqIPCGetSharedBlock();

    DBGTracex(icq);

    if(icq)
    {
       for(f=0;f<PWICQ_MAX_INSTANCES && !rc;f++)
       {
          if( (!uin || (icq->i+f)->uin == uin) && (icq->i+f)->uin)
          {
             DBGTrace((icq->i+f)->uin);
             DBGTracex((icq->i+f)->flags);

             if( (icq->i+f)->flags & ICQINSTANCEFLAG_READY )
                rc = (icq->i+f)->uin;
          }
       }

       icqIPCReleaseSharedBlock(icq);
    }

    return rc;
 }

 int EXPENTRY icqIPCSendFile(ULONG uin, ULONG to, const char *filename, const char *txt)
 {
    ICQQUEUEBLOCK *blk;
    int           sz;
    char	  *ptr;

    if(!txt)
       txt = "";

    if(!filename || access(filename,04))
       return -1;

    sz  = sizeof(ICQQUEUEBLOCK)+strlen(filename)+strlen(txt)+6;
    DBGTrace(sz);

    blk = malloc(sz+2);
    DBGTracex(blk)

    if(!blk)
       return -2;

    memset(blk,0,sz);

    blk->szPrefix = sizeof(ICQQUEUEBLOCK);      // Size of ICQQUEUEBLOCK (Version che
    blk->sz       = sz;		                // Size of datablock
    blk->cmd      = ICQCMD_SENDFILE;            // Command type

    ptr                 = (char *) (blk+1);
    *( (ULONG *) ptr )  = to;
    ptr                += 4;

    strcpy(ptr,filename);
    ptr += strlen(ptr)+1;

    strcpy(ptr,txt);

    sz = icqIPCSendCommand(uin, blk);

    free(blk);

    CHKPoint();
    return sz;

 }

 int EXPENTRY icqIPCOpenURL(ULONG to,const char *url)
 {
    char           buffer[sizeof(ICQQUEUEBLOCK)+0x0100];
    ICQQUEUEBLOCK  *blk                                = (ICQQUEUEBLOCK *) buffer;

    memset(blk,0,sizeof(ICQQUEUEBLOCK));

    blk->szPrefix          = sizeof(ICQQUEUEBLOCK);
    blk->sz                = sizeof(ICQQUEUEBLOCK)+(strlen(url) & 0x00FF);
    blk->cmd               = ICQCMD_OPENURL;
	
	strncpy( (char *) (blk+1), url, 0xFF);

    return icqIPCSendCommand(to, blk);
 }


 int EXPENTRY icqIPCSendURL(ULONG to, const char *url)
 {
    char           buffer[sizeof(ICQQUEUEBLOCK)+0x0100];
    ICQQUEUEBLOCK  *blk                                = (ICQQUEUEBLOCK *) buffer;

    memset(blk,0,sizeof(ICQQUEUEBLOCK));

    blk->szPrefix          = sizeof(ICQQUEUEBLOCK);
    blk->sz                = sizeof(ICQQUEUEBLOCK)+(strlen(url) & 0x00FF);
    blk->cmd               = ICQCMD_SENDURL;
	
	strncpy( (char *) (blk+1), url, 0xFF);

    return icqIPCSendCommand(to, blk);

 }

