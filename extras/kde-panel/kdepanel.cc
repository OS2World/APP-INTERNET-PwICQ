/*
 * kdepanel.cc - Cria um panel KDE para o pwICQ
 */

 #include <unistd.h>
 
 #include <klocale.h>
 #include <kglobal.h>
 #include <qimage.h>
 #include <qlabel.h>
 #include <qbutton.h>
 #include <qpushbutton.h>
 #include <qlayout.h>
 
 #define DEBUGFILE "/tmp/kdepanel.dbg"
 #include <pwmacros.h>
 
 #include <sys/ipc.h>
 #include <sys/shm.h>
 #include <sys/msg.h>

 #include <icqtlkt.h>
 #include <icqqueue.h>

 #include "kdepanel.h"

// #include <icqqueue.h>

/*---[ Implementacao ]---------------------------------------------------------------*/

 extern "C" 
 {
   KPanelApplet* init( QWidget *parent, const QString& configFile )
   {
      KGlobal::locale()->insertCatalogue( "pwICQApplet");

	  OPEN_DEBUG();
	  DBGMessage( "---[ " __DATE__ " " __TIME__ " ]----------------------------------------------------------");
	  CLOSE_DEBUG();
	  
      return new pwICQApplet( configFile, KPanelApplet::Normal, 0, parent, "pwICQApplet");
   }
 }

 
 pwICQApplet::pwICQApplet( const QString& configFile,
                                  Type type, int actions,
                                  QWidget *parent, const char *name )

   : KPanelApplet( configFile, type, actions, parent, name )

 {
	char buffer[0x0100];
    OPEN_DEBUG();
	DBGMessage( PROJECT " - Creating");
	
	pwICQBlock    = NULL;
	pwICQInstance = NULL;
	modeIcon      = 0xFFFF;
	clicked       = FALSE;
	
    /* Carrega o arquivo de imagem */
    strcpy(buffer,"/usr/share/pwicq/images/icons.gif"); // O CERTO SERIA PEDIR DO .conf

    DBGMessage(buffer);
    QImage *icons = new QImage(buffer);

    iconSize = icons->height();
    DBGTrace(iconSize);

    int c;
    int f;
    for(f = c = 0; f < PWICQICONBARSIZE; f++)
    {
	  iconPixmap[f] = QIconSet(QPixmap( icons->copy(c,0,iconSize,iconSize) ));
	  c += iconSize;
    }

    delete icons;

	base = new QLabel(this);

    base->setAlignment(Qt::AlignCenter);
    base->setFrameRect( QRect( 0, 0, iconSize+4, iconSize+4 ));
	
    /* Final configurations */

    setInstance(NULL); 

	internalTimer = new QTimer( this ); 
    connect( internalTimer, SIGNAL(timeout()), SLOT(timer()) );
    internalTimer->start( 1000 );   

	CLOSE_DEBUG();
 }

 pwICQApplet::~pwICQApplet()
 {
    OPEN_DEBUG();
	DBGMessage( PROJECT " - Destroying");

	if(pwICQBlock)
	   shmdt(pwICQBlock);
       
	CLOSE_DEBUG();
 }
 
 int pwICQApplet::widthForHeight( int h ) const
 {
    return iconSize+4; 
 }

 int pwICQApplet::heightForWidth( int w ) const
 {
    return iconSize+4; 
 }
 
 void pwICQApplet::timer()
 {

	if(!pwICQBlock)
	{
       /* Try to find the pwICQ's shared-memory block */
       OPEN_DEBUG();
	   
       key_t memKey 	=  ftok(PWICQ_SHAREMEM,0x230167);
       int   shmID      = shmget(memKey,sizeof(sizeof(ICQSHAREMEM)),0660);

	   DBGTrace(memKey);
	   DBGTrace(shmID);
	   
       if( ((int) memKey) != -1 && ((int) shmID) != -1 )
	   {
 	      pwICQBlock = (ICQSHAREMEM *) shmat(shmID,0,0);
		  DBGTracex(pwICQBlock);
		  
	      if(pwICQBlock->sz != sizeof(ICQSHAREMEM) || pwICQBlock->si != sizeof(ICQINSTANCE))
	      {
			 DBGMessage("Invalid shared-memory block");
             shmdt(pwICQBlock); 
			 pwICQBlock = NULL;
			 internalTimer->stop();
	      }
   	   }
	   CLOSE_DEBUG();
	   return;
	}
	
	if(!pwICQInstance)
	{
	   /* Try to find a valid pwICQ instance */
       OPEN_DEBUG();
	   DBGMessage("Searching for a valid pwICQ instance");
	   
	   int f;

#ifdef DEBUG	   
       for(f=0;f<PWICQ_MAX_INSTANCES && !(pwICQBlock->i+f)->uin;f++);
#else
       for(f=0;f<PWICQ_MAX_INSTANCES && !(pwICQBlock->i+f)->uin;f++)
	   {
	      DBGTrace(f);
		  DBGtrace((pwICQBlock->i+f)->uin);
	   }
#endif	   
	   
       if(f < PWICQ_MAX_INSTANCES)
		  setInstance(pwICQBlock->i+f);
	   
	   DBGTracex(pwICQInstance);
	   
	   CLOSE_DEBUG();
	   
	   return;
	}
	
	if(!pwICQInstance->uin)
	{
	   setInstance(NULL);
	   return;
	}
	
	/* All the data is valid, process-it */
	if(pwICQInstance->eventCounter == eventCounter)
    {
       if(idleCounter++ > 5)
       {
           pwICQInstance->uin = 0;
           pwICQInstance->pid = 0;
       }
	   return;
    }

    OPEN_DEBUG();
    
    idleCounter = 0;
	
	if(pwICQInstance->eventIcon != 0xFF)
	{
	   setModeIcon( clicked || ((++tick & 1) == 1) ? pwICQInstance->eventIcon : pwICQInstance->modeIcon);
	}
	else
	{
	   tick = 0;
       setModeIcon(pwICQInstance->modeIcon);
	}

    eventCounter = pwICQInstance->eventCounter;

	CLOSE_DEBUG();
 }

 void pwICQApplet::setInstance(ICQINSTANCE *vlr)
 {
    OPEN_DEBUG();
	
	if(!vlr)
	{
	   DBGMessage("Removing instance indicator, going to inactive mode");
	   eventCounter = 0;
       setModeIcon(ICQICON_INACTIVE);
 	   shmdt(pwICQBlock);
	   pwICQBlock = NULL;
    }
	else
	{
	   DBGMessage("Activating instance indicator");
	   DBGTrace(vlr->uin);
	   eventCounter = vlr->eventCounter;
	   setModeIcon(vlr->modeIcon);
	}

    idleCounter   = 0;
	pwICQInstance = vlr;
	
	DBGTracex(pwICQInstance);
	DBGTrace(modeIcon);
	
	
    CLOSE_DEBUG();
 }

 void pwICQApplet::setModeIcon(USHORT code)
 {
	if(modeIcon == code)
       return;

	base->setPixmap( iconPixmap[code].pixmap() ); 
	modeIcon = code;
 }

 void pwICQApplet::mousePressEvent(QMouseEvent *event)
 {
    OPEN_DEBUG();
    clicked = TRUE;
    CLOSE_DEBUG();
 }
   
 void pwICQApplet::mouseReleaseEvent(QMouseEvent *event)
 {
    struct icqqueuesetmode cmd;
	   
    OPEN_DEBUG();

	CHKPoint();
	
	memset(&cmd,0,sizeof(cmd));
	cmd.h.sz = sizeof(cmd);
	
	DBGTrace(event->x());
	DBGTrace(event->y());
	DBGTracex(event->state());
	
	if( event->x() > (iconSize+4) 
			     || event->y() > (iconSize+4) 
		         || event->x() < 0 
	             || event->y() < 0)
	{
	   DBGMessage("Released out of the window");
	   clicked = FALSE;
	}
	else if(!pwICQInstance)
	{
	   DBGMessage("Invalid instance");
	   clicked = FALSE;
	}
	else if(event->state() & Qt::MidButton)
	{
	   DBGMessage("Middle Button");
       cmd.h.cmd = ICQCMD_OPENMENU;
	   cmd.mode  = ICQMNU_SYSTEM;
	   sendIPCBlock((ICQQUEUEBLOCK *) &cmd);
	}
	else if(event->state() & Qt::LeftButton)
	{
	   DBGMessage("Left Button");
       cmd.h.cmd = (pwICQInstance->eventIcon != 0xFF) ? ICQCMD_OPENFIRST : ICQCMD_POPUPUSERLIST;
	   DBGTracex(pwICQInstance->eventIcon);
	   DBGTrace(cmd.h.cmd);
	   sendIPCBlock((ICQQUEUEBLOCK *) &cmd);

#ifdef DEBUG	
	   if(pwICQInstance)
	      DBGTrace(pwICQInstance->uin);
#endif	   
	}
	else if(event->state() & Qt::RightButton)
	{
	   DBGMessage("Right Button");
       cmd.h.cmd = ICQCMD_OPENMENU;
	   cmd.mode  = ICQMNU_MODES;
	   sendIPCBlock((ICQQUEUEBLOCK *) &cmd);
	}
#ifdef DEBUG	
	else if(event->state() & Qt::MouseButtonMask)
	{
	   DBGMessage("Combinated Button");
	}
#endif
	
	clicked = FALSE;
	
    CLOSE_DEBUG();
 }

 int pwICQApplet::setError(int rc)
 {
	OPEN_DEBUG();
	DBGTrace(rc);
	CLOSE_DEBUG();
	
	if(rc)
       setInstance(NULL);

	return rc;
 }

 int pwICQApplet::sendIPCBlock(ICQQUEUEBLOCK *cmd)
 {
    /* Send an IPC Request, wait (not too much) for response */
    ICQQUE_PACKET  packet;
    ICQQUEUEBLOCK  *req;
    int			   rc		= 0;
    key_t		   key;
    int			   queue;
    int			   f;

	if(!pwICQInstance || !pwICQInstance->uin)
	{
	   OPEN_DEBUG();
	   DBGMessage("Can't send IPC request, no valid instance");
	   CLOSE_DEBUG();
	   return setError(-1);
	}

    cmd->szPrefix = sizeof(ICQQUEUEBLOCK);

    memset(&packet,0,sizeof(ICQQUE_PACKET));
    packet.uin    = pwICQInstance->uin;
    packet.msg.sz = sizeof(ICQQUE_MESSAGE);

    if( (packet.msg.shmID = shmget(IPC_PRIVATE,cmd->sz,IPC_CREAT | 0660)) == -1)
	   return setError(-2);

 	req = (ICQQUEUEBLOCK *) shmat(packet.msg.shmID,0,0);
 	if( ((int) req) == -1)
 	{
	   return setError(-3);
 	}
 	else
 	{
 	   memcpy(req,cmd,cmd->sz);

       key = ftok("/tmp/pwicq.queue",0x230167); 	    
       
       if( ((int) key) == -2)
       {
          rc = -4;
       }
       else
       {
          queue = msgget(key,0660);
          
          if(queue == -1)
          {
             rc = -5;
          }
          else
          {
             if(msgsnd(queue, (struct msgbuf *) &packet, sizeof(ICQQUE_PACKET), 0) == -1)
             {
                rc = -6;
             }
             else
             {
                for(f=0;f<100 && req->status == cmd->status;f++)
                   usleep(1000);
 	            memcpy(cmd,req,cmd->sz);
             }
          }
       }
 	   shmdt(req);
 	}
	
    return setError(rc);
 }
   
  
