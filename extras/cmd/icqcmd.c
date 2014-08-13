/*
 * ICQCMD.C - pwICQ Command line interface
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_ERRORS
 #define INCL_DOSQUEUES
 #define INCL_DOSPROCESS
 #include <os2.h>
#endif

 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>

 #include <pwMacros.h>
 #include <icqtlkt.h>
 #include <icqqueue.h>

/*---[ Public ]--------------------------------------------------------------------------------------------*/

 static int   numpar;
 static char  **param;
 static ULONG to		= 0;

/*---[ Prototipes ]----------------------------------------------------------------------------------------*/

 static ULONG getLong(void);

/*---[ Constants ]-----------------------------------------------------------------------------------------*/

 static int addUser(void);
 static int openURL(void);
 static int display(void);
 static int setmode(void);
 static int toUser(void);
 static int sendURL(void);
 static int play(void);
 static int soundscheme(void);

 static const struct cmd
 {
	const char *key;
	int        parms;
	int        (*exec)(void);
 }
 cmdList[] =
 {
	{ "to", 	      1, toUser      },
	{ "add",	      1, addUser     },
	{ "open",	      1, openURL     },
	{ "set",	      1, setmode     },
	{ "display",      1, display     },
	{ "play",         1, play        },
	{ "soundscheme",  1, soundscheme },
	{ "url",          1, sendURL     }
 };

/*---[ Implementing ]--------------------------------------------------------------------------------------*/

 static const struct cmd *getCmd(void)
 {
	int f;
	
    for(f=0;f< (sizeof(cmdList)/sizeof(struct cmd));f++)
	{
	   if(!strcmp(cmdList[f].key,*param))
		  return &cmdList[f];
	}
	
	printf("Unexpected command \"%s\"\n",*param);
	
	return NULL;
 }

 int main(int np, char *pr[])
 {
	int              rc = 0;
	const struct cmd *cmd;
	
	CHKPoint();
	
	numpar = np;
	param  = pr;
	
    while(--numpar > 0)
    {
	   param++;
	
	   cmd = getCmd();
	
	   DBGTracex(cmd);
	
	   if(cmd)
	   {
	      if(numpar <= cmd->parms)
		  {
		     fprintf(stderr,"Command \"%s\" needs %d parameter(s)\n",*param,cmd->parms);
		     return -1;
          }	
          else
          {
			 rc = cmd->exec();
			 DBGTrace(rc);
          }			
	   }
    }	

    return rc;
 }

 static ULONG getLong(void)
 {
	DBGTrace(numpar);
	if(numpar > 1)
	{
	   param++;
	   numpar--;
	   DBGMessage(*param);
	   return atol(*param);
	}
	return 0;

 }

 static const char *getString(void)
 {
	DBGTrace(numpar);
	if(numpar > 1)
	{
	   param++;
	   numpar--;
	   DBGMessage(*param);
	   return *param;
	}
	return "";
 }

 static int addUser(void)
 {
    ULONG uin = getLong();
    int   rc  = icqIPCAddUser(to,uin);
    printf("Adding ICQ# %ld to contact list (rc=%d)\n",uin,rc);
    return rc;
 }

 static int toUser(void)
 {
    to = getLong();
	return 0;
 }

 static int openURL(void)
 {
    return icqIPCOpenURL(to,getString());
 }

 static int setmode(void)
 {
	const char *mode = getString();
	DBGMessage(mode);
    return icqIPCSetModeByName(to,mode);
 }

 static int display(void)
 {
	return icqIPCPluginRequest(to,"icqosd", getString());
 }

 static int sendURL(void)
 {
    return icqIPCSendURL(to, getString());
 }

 static int play(void)
 {
	char buffer[0x0200];
	*buffer = 'p';
	strncpy(buffer+1,getString(),0x01FF);
    return icqIPCPluginRequest(to,"icqsound", buffer);
 }
 
 static int soundscheme(void)
 {
	char buffer[0x0200];
	*buffer = 'l';
	strncpy(buffer+1,getString(),0x01FF);
    return icqIPCPluginRequest(to,"icqsound", buffer);
 }
