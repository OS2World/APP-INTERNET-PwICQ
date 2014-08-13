/*
 * kdepanel.h - Descreve funcoes e classes para o applet pwICQ do KDE
 */
 
 #include <qiconset.h>
 #include <qtimer.h>
 #include <qlabel.h>

 #include "kpanelapplet.h"

 #include <icqtlkt.h>
 #include <icqqueue.h>

 class pwICQApplet : public KPanelApplet
 {
   Q_OBJECT

 public:

   USHORT      iconSize;
   QIconSet    iconPixmap[PWICQICONBARSIZE];
	
   USHORT	   modeIcon;
   UCHAR	   tick;
   BOOL		   clicked;
	
   ICQSHAREMEM *pwICQBlock;
   ICQINSTANCE *pwICQInstance;
   ULONG	   eventCounter;
   ULONG       idleCounter;
   QTimer      *internalTimer;
   QLabel      *base;
	
   /* Funcoes */
	
   pwICQApplet( const QString& configFile, Type t = Stretch, 
                int actions = 0, QWidget *parent = 0, 
                const char *name = 0 );

   ~pwICQApplet();
	
   int  widthForHeight( int ) const;
   int  heightForWidth( int ) const;
   
   void setInstance(ICQINSTANCE *);
   void setModeIcon(USHORT);
   int  sendIPCBlock(ICQQUEUEBLOCK *);
   int  setError(int);

 protected:

   void mousePressEvent(QMouseEvent *);
   void mouseReleaseEvent(QMouseEvent *);

 private slots:

    void timer();


 };


 
