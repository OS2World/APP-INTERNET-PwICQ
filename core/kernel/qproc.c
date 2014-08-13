/*
 * QPROC.C - Processa comandos remotos
 */

 #include <string.h>
 #include <errno.h>
 #include <stdlib.h>
 #include <stdio.h>

 #include <pwMacros.h>

 #include <icqkernel.h>
 #include <icqqueue.h>

/*---[ Prototipos ]----------------------------------------------------------------------------------------*/

 static int setMode(HICQ, struct icqqueuesetmode *);
 static int callPlugin(HICQ, struct icqpluginipc *);
 static int show(HICQ, ICQQUEUEBLOCK *);
 static int hide(HICQ, ICQQUEUEBLOCK *);
 static int modeByName(HICQ, ICQQUEUEBLOCK *);
 static int remoteLog(HICQ, ICQQUEUEBLOCK *);
 static int queryBuild(HICQ, ICQQUEUEBLOCK *);
 static int queryString(HICQ, ICQQUEUEBLOCK *);
 static int executeCmd(HICQ, ICQQUEUEBLOCK *);
 static int openFirst(HICQ, ICQQUEUEBLOCK *);
 static int openMenu(HICQ, struct icqqueuesetmode *);
 static int openUserList(HICQ, ICQQUEUEBLOCK *);
 static int mptCommand(HICQ, struct icqqueuemptblock *);
 static int queryMode(HICQ, struct icqqueuesetmode *);
 static int xlatCpg(HICQ, ICQQUEUEBLOCK *);
 static int setpeer(HICQ, struct icqpeerinfo *);
 static int peerConnect(HICQ, ICQQUEUEBLOCK *);
 static int sendFile(HICQ,ICQQUEUEBLOCK *);
 static int newUser(HICQ,ICQQUEUEBLOCK *);
 static int openURL(HICQ,ICQQUEUEBLOCK *);
 static int sendURL(HICQ,ICQQUEUEBLOCK *);

/*---[ Tabela de funcoes ]---------------------------------------------------------------------------------*/

 struct cmd
 {
    UCHAR       id;
    int         (* exec)(HICQ, ICQQUEUEBLOCK *);
 };

 #define PREFIX (int (*)(HICQ,ICQQUEUEBLOCK *))

 static const struct cmd tbl[] = {      { ICQCMD_SETMODE,           PREFIX  setMode         },
                                        { ICQCMD_SHOW,              PREFIX  show            },
                                        { ICQCMD_HIDE,              PREFIX  hide            },
                                        { ICQCMD_PLUGIN,            PREFIX  callPlugin      },
                                        { ICQCMD_MODEBYNAME,        PREFIX  modeByName      },
                                        { ICQCMD_WRITELOG,          PREFIX  remoteLog       },
                                        { ICQCMD_QUERYBUILD,        PREFIX  queryBuild      },
                                        { ICQCMD_QUERYCONFIGSTRING, PREFIX  queryString     },
                                        { ICQCMD_EXEC,              PREFIX  executeCmd      },
                                        { ICQCMD_OPENFIRST,         PREFIX  openFirst       },
                                        { ICQCMD_OPENMENU,          PREFIX  openMenu        },
                                        { ICQCMD_POPUPUSERLIST,     PREFIX  openUserList    },
                                        { ICQCMD_MPTREQUEST,        PREFIX  mptCommand      },
                                        { ICQCMD_QUERYONLINEMODE,   PREFIX  queryMode       },
                                        { ICQCMD_TRANSLATECODEPAGE, PREFIX  xlatCpg         },
                                        { ICQCMD_SETPEERINFO,       PREFIX  setpeer         },
                                        { ICQCMD_PEERCONNECT,       PREFIX  peerConnect     },
                                        { ICQCMD_SENDFILE,          PREFIX  sendFile        },
                                        { ICQCMD_ADDUSER,           PREFIX  newUser         },
										{ ICQCMD_OPENURL,           PREFIX  openURL         },
										{ ICQCMD_SENDURL,           PREFIX  sendURL         },

                                        { 0,                        NULL                    }
                                 };

#ifdef __OS2__
   #define ICQERROR_INVALID           EINVAL
   #define ICQERROR_NOTEXIST          ENOTEXIST
   #define ICQERROR_INVALIDPARAMETER  EBADTYPE
   #define ICQERROR_INVALIDUSER       ENOTEXIST
#endif

#ifdef linux
   #define ICQERROR_INVALID           EINVAL
   #define ICQERROR_NOTEXIST          ENODEV
   #define ICQERROR_INVALIDPARAMETER  EINVAL
   #define ICQERROR_INVALIDUSER       ENODEV
#endif

    #define ICQERROR_NOTONLINE         9999								

/*---[ Implementacao ]-------------------------------------------------------------------------------------*/

 void EXPENTRY icqProcessRemoteBlock(HICQ icq, ICQQUEUEBLOCK *cmd)
 {
#ifdef EXTENDED_LOG
	char				log[0x0100];
#endif	
    const struct cmd    *c;

    if(cmd->szPrefix != sizeof(ICQQUEUEBLOCK) || cmd->sz < sizeof(ICQQUEUEBLOCK))
    {
       icqWriteSysLog(icq,PROJECT,"Invalid IPC block received");
       return;
    }

    cmd->rc = -1;
    DBGTrace(cmd->cmd);
	
#ifdef EXTENDED_LOG
	sprintf(log,"Remote command %d (0x%04x)",cmd->cmd,cmd->cmd);
	DBGMessage(log);
	icqWriteSysLog(icq,PROJECT,log);
#endif	

    for(c=tbl;c->id != cmd->cmd && c->exec; c++);

    if(c->exec)
    {
       cmd->rc = c->exec(icq,cmd);
    }
    else
    {
       cmd->rc = EINVAL;
       icqWriteSysLog(icq,PROJECT,"Unexpected remote command received");
    }

    cmd->status++;

 }

 static int setMode(HICQ icq, struct icqqueuesetmode *cmd)
 {
    int rc;

    if(cmd->h.sz == sizeof(struct icqqueuesetmode))
    {
       icqSetOnlineMode(icq, cmd->mode);
       rc = 0;
    }
    else
    {
       rc = ICQERROR_INVALID;
    }
    return rc;
 }

 static int callPlugin(HICQ icq, struct icqpluginipc *cmd)
 {
    HPLUGIN p;
	
	DBGMessage(cmd->plugin);
	DBGMessage((char *) (cmd+1));

    for( p = icqQueryFirstElement(icq->plugins); p; p = icqQueryNextElement(icq->plugins,p))
    {
       if(!strnicmp(cmd->plugin,(char *)(p+1),11) && p->rpcRequest)
          return p->rpcRequest(icq,cmd);
    }
    return ICQERROR_NOTEXIST;
 }

 static int show(HICQ icq, ICQQUEUEBLOCK *cmd)
 {
    icqSystemEvent(icq,ICQEVENT_SHOW);
    return 0;
 }

 static int hide(HICQ icq, ICQQUEUEBLOCK *cmd)
 {
    icqSystemEvent(icq,ICQEVENT_HIDE);
    return 0;
 }

 static int modeByName(HICQ icq, ICQQUEUEBLOCK *cmd)
 {
    if(cmd->sz < (sizeof(ICQQUEUEBLOCK)+1))
       return ICQERROR_INVALIDPARAMETER;

    DBGMessage( (char *)(cmd+1) );

    icqSetModeByName(icq, (char *)(cmd+1));
    return 0;
 }

 static int remoteLog(HICQ icq, ICQQUEUEBLOCK *cmd)
 {
    if(cmd->sz < (sizeof(ICQQUEUEBLOCK)+1))
       return ICQERROR_INVALIDPARAMETER;

    icqWriteSysLog(icq, "icqipc", (const char *)(cmd+1));
    return 0;
 }

 static int queryBuild(HICQ icq, ICQQUEUEBLOCK *cmd)
 {
    if(cmd->sz < (sizeof(ICQQUEUEBLOCK)+4))
       return ICQERROR_INVALIDPARAMETER;

    *((ULONG *) (cmd+1)) =  icqQueryBuild();

    return 0;
 }

 static int queryString(HICQ icq, ICQQUEUEBLOCK *cmd)
 {
    char buffer[0x0100];

    if(cmd->sz < (sizeof(ICQQUEUEBLOCK)+2))
       return ICQERROR_INVALIDPARAMETER;

    *(buffer+0xFF) = 0;
    strncpy(buffer,(char *)(cmd+1),min(cmd->sz,0xFE));

    DBGMessage(buffer);

    icqLoadString(icq, buffer, "", (char *) (cmd+1), cmd->sz);

    cmd->sz = strlen((char *) (cmd+1));

    DBGMessage( (char *)(cmd+1) );
    DBGTrace(cmd->sz);

    return 0;

 }

 static int executeCmd(HICQ icq, ICQQUEUEBLOCK *cmd)
 {
    return icqExecuteCmd(icq, (char *) (cmd+1));
 }

 static int openFirst(HICQ icq, ICQQUEUEBLOCK *cmd)
 {
    /* Abre a primeira mensagem pendente */
    HMSG  msg   = NULL;
        ULONG uin;

    if(icqQueryFirstMessage(icq,&uin,&msg))
       return 1;

#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"Opening first message by remote request");
#endif

    if(msg)
       icqOpenMessage(icq,uin,msg);

    return 0;
 }

 static int openMenu(HICQ icq, struct icqqueuesetmode *cmd)
 {
#ifdef EXTENDED_LOG
    char buffer[0x0100];
    sprintf(buffer,"Opening popup menu by remote request (id=%d)",(int) cmd->mode);
    icqWriteSysLog(icq,PROJECT,buffer);
    DBGMessage(buffer);
#endif

    return icqShowPopupMenu(icq, 0, (USHORT) cmd->mode, 0, 0);
 }

 static int openUserList(HICQ icq, ICQQUEUEBLOCK *block)
 {
#ifdef EXTENDED_LOG
    icqWriteSysLog(icq,PROJECT,"Opening popup userlist by remote request");
#endif
    return icqPopupUserList(icq);
 }

 static int mptCommand(HICQ icq, struct icqqueuemptblock *block)
 {
    HUSER usr;

    if(block->h.sz != sizeof(struct icqqueuemptblock))
    {
       icqWriteSysLog(icq,PROJECT,"Invalid MPT request");
       return 1;
    }

    switch(block->request)
    {
    case 0:	/* Desativa 	 */
       icq->cntlFlags &= ~ICQFC_MPT;
       for(usr = icqQueryFirstUser(icq);usr;usr = icqQueryNextUser(icq,usr))
       {
          if(usr->u.groupMasc & USRG_MPT)
             icqUserEvent(icq, usr, ICQEVENT_SHOW);
       }
       break;

    case 1:	/* Ativa	 */
       icq->cntlFlags |= ICQFC_MPT;
       for(usr = icqQueryFirstUser(icq);usr;usr = icqQueryNextUser(icq,usr))
       {
          if(usr->u.groupMasc & USRG_MPT)
             icqUserEvent(icq, usr, ICQEVENT_HIDE);
       }
       break;

    case 2:	/* Anexa um UIN	 */
       usr = icqQueryUserHandle(icq,block->uin);
       if(!usr)
          return 2;
       usr->u.groupMasc |= USRG_MPT;
       icq->cntlFlags   |= ICQFC_SAVEUSER;
       break;

    case 3:	/* Retira um UIN */
       usr = icqQueryUserHandle(icq,block->uin);
       if(!usr)
          return 2;
       usr->u.groupMasc &= ~USRG_MPT;
       icq->cntlFlags   |= ICQFC_SAVEUSER;
       break;

    }

    return 0;
 }

 static int queryMode(HICQ icq, struct icqqueuesetmode *mode)
 {
    const ICQMODETABLE *t;

    DBGTrace(mode->h.sz);

    if(mode->h.sz < 4)
       return -1;

    if(icqIsOnline(icq))
    {
       mode->mode = icqQueryOnlineMode(icq);
       if(mode->h.sz > 5)
       {
          for(t = icqQueryModeTable(icq); t && t->descr && t->mode != (mode->mode&0x0000FFFF); t++);
          if(t->descr)
             strncpy( (char *)(mode+1), t->descr, mode->h.sz-4);
       }
    }
    else
    {
       mode->mode = icqQueryOfflineMode(icq);
       if(mode->h.sz > 5)
          strncpy( (char *)(mode+1), "Offline", mode->h.sz-4);
    }


    return 0;
 }

 static int xlatCpg(HICQ icq, ICQQUEUEBLOCK *cmd)
 {
    char *ptr = (char *) (cmd+1);
    int  sz   = cmd->sz - sizeof(ICQQUEUEBLOCK);

    if(sz < 2)
       return ICQERROR_INVALIDPARAMETER;

    icqConvertCodePage(icq, *ptr & 0x0F, ptr+1, sz-1);

    return 0;
 }

 static int setpeer(HICQ icq, struct icqpeerinfo *cmd)
 {
    HUSER usr;
#ifdef EXTENDED_LOG
    char  logBuffer[0x0100];
#endif

    if(cmd->h.sz < sizeof(struct icqpeerinfo))
       return ICQERROR_INVALIDPARAMETER;

#ifdef EXTENDED_LOG
    sprintf(logBuffer,"Forcing %s (%ld) to address ",icqQueryUserNick(icqQueryUserHandle(icq,cmd->uin)),cmd->uin);
    icqIP2String(cmd->ip,logBuffer+strlen(logBuffer));
    sprintf(logBuffer+strlen(logBuffer),":%d",cmd->port);
    icqWriteSysLog(icq,PROJECT,logBuffer);
#endif

    usr = icqSetUserIPandPort(icq, cmd->uin, cmd->ip, cmd->ip, cmd->port);

    if(!usr)
       return ICQERROR_INVALIDUSER;

    usr->u.peerVersion = cmd->peerVersion;
    usr->u.peerCookie  = cmd->peerCookie;

    if( usr->u.mode  == icqQueryOfflineMode(icq))
    {
#ifdef EXTENDED_LOG
       icqWriteSysLog(icq,PROJECT,"Remote request for offline user, icon needs adjust");
#endif
       usr->u.offlineIcon = 7;
       icqAjustUserIcon(icq, usr, TRUE);
    }

    return 0;
 }

 static int peerConnect(HICQ icq, ICQQUEUEBLOCK *cmd)
 {
    HUSER usr;

    DBGTrace(cmd->sz);
    DBGTrace( *((ULONG *)(cmd+1)) );

    if(cmd->sz < sizeof(ICQQUEUEBLOCK)+4)
       return ICQERROR_INVALIDPARAMETER;

    usr = icqQueryUserHandle(icq,*((ULONG *)(cmd+1)) );

    if(!usr)
       return ICQERROR_INVALIDUSER;

    icqBeginPeerSession(icq, usr);

    CHKPoint();

    return 0;

 }

 static int newUser(HICQ icq, ICQQUEUEBLOCK *cmd)
 {
	char  buffer[0x0100];
	ULONG uin;
	
    DBGTrace(cmd->sz);

    if(cmd->sz < sizeof(ICQQUEUEBLOCK)+4)
       return ICQERROR_INVALIDPARAMETER;

	if(!icqIsOnline(icq))
	   return ICQERROR_NOTONLINE;
	
    uin = *((ULONG *)(cmd+1));
	
	sprintf(buffer,"Adding ICQ# %ld by remote request",uin);
    icqWriteSysLog(icq,PROJECT,buffer);	
	
    return icqAddUserInContactList(icq, uin);
 }


 static int sendFile(HICQ icq, ICQQUEUEBLOCK *cmd)
 {
    char  *filename;
    char  *message;

    DBGTrace(cmd->sz);

    if(cmd->sz < sizeof(ICQQUEUEBLOCK)+6)
       return ICQERROR_INVALIDPARAMETER;

    *( ((char *)cmd) + cmd->sz ) = 0;

    message  =
    filename = ( (char *) (cmd+1) ) + 4;

    DBGMessage(filename);

    message += strlen(filename)+1;

    DBGMessage(message);

    return icqSendFile(icq, *((ULONG *)(cmd+1)), filename, message);

 }

 static int openURL(HICQ icq,ICQQUEUEBLOCK *cmd)
 {
	return icqOpenURL(icq,(char *)(cmd+1));
 }

 static int sendURL(HICQ icq,ICQQUEUEBLOCK *cmd)
 {
    icqWriteSysLog(icq,PROJECT,"Send URL Request:");
    icqWriteSysLog(icq,PROJECT,(char *)(cmd+1));

    if(icq->skin)
    {
       if(icq->skin->msgMultDest)
          return icq->skin->msgMultDest(icq,MSG_URL,0,(const char *) (cmd+1));

       if(icq->skin->newMessage)
          return icq->skin->newMessage(icq,0,MSG_URL,0,(const char *)(cmd+1));

       return 102;
    }

    return 101;
 }

