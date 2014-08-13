/*
 * ICQNET.C - Funcoes de rede para o pwICQ
 */

#ifndef linux
   #error Linux only
#endif

 #include <stdio.h>
 #include <string.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netdb.h>
 #include <errno.h>
 #include <unistd.h>
 #include <time.h>
 #include <netinet/in.h>
 #include <net/if.h>
 #include <sys/ioctl.h>


 #include <icqnetw.h>

/*---[ Macros ]-----------------------------------------------------------------*/

/*---[ Constants ]--------------------------------------------------------------*/

/*---[ Implementation ]---------------------------------------------------------*/

 int ICQAPI icqnetw_Configure(HICQ icq, HMODULE mod)
 {
    return 0;
 }

 int ICQAPI icqnetw_Start(HICQ icq, void *nothing, HWND hwnd)
 {
    return -1;
 }

 void ICQAPI icqnetw_ConfigPage(HICQ icq, void *lixo, ULONG uin, USHORT type, HWND hwnd, const DLGINSERT *dlg, char *buffer)
 {
    return;
 }

 int ICQAPI icqBindSocket(int sock, int port, BOOL any)
 {
    struct sockaddr_in  in;
    static const int	f    = 1;

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &f, sizeof(f));

    memset(&in, 0, sizeof(in));
    in.sin_family      = AF_INET;
    in.sin_port        = htons(port);
    in.sin_addr.s_addr = INADDR_ANY;

    if(!bind(sock, (struct sockaddr *) &in, sizeof(in)))
       return 0;

    if(any)
    {
       in.sin_port = 0;
       return bind(sock, (struct sockaddr *) &in, sizeof(in));
    }

    return -1;
 }

 static int opensocket(int port, int type)
 {
    int sock = socket(PF_INET, type, 0);

    if(sock < 0)
       return sock;

   if(port >= 0)
   {
      if(icqBindSocket(sock, port, TRUE) < 0)
      {
         close(sock);
         return -1;
      }
   }
   return sock;
 }

 int ICQAPI icqOpenUDP(int port)
 {
    return opensocket(port,SOCK_DGRAM);
 }

 int ICQAPI icqOpenTCP(int port)
 {
    return opensocket(port,SOCK_STREAM);
 }

 int ICQAPI icqCloseSocket(int sock)
 {
    close(sock);
    return 0;
 }

 int ICQAPI icqConnectSock(int sock, long ip, int port)
 {
    struct sockaddr_in  addr;

    memset(&addr,0,sizeof(addr));
    addr.sin_family             = PF_INET;
    addr.sin_addr.s_addr        = ip;
    addr.sin_port               = htons(port);

    if(connect(sock,(struct sockaddr *) & addr,sizeof(addr)))
    {
       close(sock);
       return -1;
    }

    return sock;
 }

 int ICQAPI icqGetSockError(void)
 {
    return errno;
 }

 long ICQAPI icqGetHostByName(char *hostname)
 {
    struct hostent *host = gethostbyname(hostname);
    if(host == NULL)
       return 0;

    return *( (long *) host->h_addr );
 }

 long ICQAPI icqGetHostID(void)
 {
    struct ifreq       i;
    struct sockaddr_in *a;
    long               ret  = 0x0100007f;
    int                sock = socket(PF_INET,SOCK_DGRAM,0);

    if(sock > 0)
    {
       strncpy(i.ifr_name,"ppp0",IFNAMSIZ);
       if(ioctl(sock,SIOCGIFADDR,(caddr_t) &i, sizeof(i)) != -1)
       {
          a   = (struct sockaddr_in *) &i.ifr_addr;
	  ret = a->sin_addr.s_addr;
       }
       else
       {
          strncpy(i.ifr_name,"eth0",IFNAMSIZ);
          if(ioctl(sock,SIOCGIFADDR,(caddr_t) &i, sizeof(i)) != -1)
          {
             a   = (struct sockaddr_in *) &i.ifr_addr;
	     ret = a->sin_addr.s_addr;
          }
       }
       close(sock);
    }
    return ret;
 }

 int ICQAPI icqSendTo(int sock, long ip, int port, const void *pkt, int size)
 {
    struct sockaddr_in   to;

    to.sin_family      = AF_INET;
    to.sin_addr.s_addr = ip;
    to.sin_port        = htons(port);

    if(sendto(sock,(char *) pkt,size,MSG_NOSIGNAL,(struct sockaddr *) &to, sizeof(to)) == -1)
       return icqGetSockError();

    return 0;
 }

 int ICQAPI icqRecvFrom(int sock, long *ip, int *port, void *buffer, int size)
 {
    struct sockaddr_in  addr;
    int                 wrk;
    fd_set              fds;
    struct timeval      tv;

    tv.tv_sec  = 1;
    tv.tv_usec = 0;

    FD_ZERO(&fds);
    FD_SET(sock, &fds);

    select(sock+1, &fds, NULL, NULL, &tv);

    if(!FD_ISSET(sock, &fds))
       return 0;

    wrk  = sizeof(addr);
    memset(&addr,0,wrk);

    size = recvfrom(     sock,
                         (char *) buffer,
                         size,
                         0,
                         (struct sockaddr *) &addr,
                         &wrk);

    *ip   = addr.sin_addr.s_addr;
    *port = htons(addr.sin_port);

    return size;

 }

 int ICQAPI icqSend(int sock, const void *ptr, int size)
 {
    int bytes;

    if(size == -1)
       size = strlen(ptr);

    bytes = send(sock, (char *) ptr, size, 0);

    if(bytes <= 0)
    {
       perror("icqSend");
       return icqGetSockError();
    }

    return 0;
 }

 int ICQAPI icqRecv(int sock, void *ptr, int size)
 {
    fd_set              rfds;
//    fd_set              efds;
    struct timeval      tv;
    int                 bytes;

    tv.tv_sec  = 1;
    tv.tv_usec = 0;

//    FD_ZERO(&efds);
//    FD_SET(sock, &efds);

    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);

    if(select(sock+1, &rfds, NULL, NULL, &tv) < 0)
       return -1;

    if(!FD_ISSET(sock, &rfds))
       return 0;

    bytes = recv(sock,(char *) ptr, size, 0);

    if(bytes <= 0)
       perror("icqRecv");

    return bytes == 0 ? -1 : bytes;

 }

 int ICQAPI icqGetPort(int sock)
 {
    struct sockaddr_in   in;
    int                  size;

    if(sock < 1)
       return 0;

    size = sizeof(in);
    if(getsockname(sock,(struct sockaddr *) &in, &size) == -1)
       return -1;

    return htons(in.sin_port);
 }

 long ICQAPI icqGetIPAdress(int sock)
 {
    struct sockaddr_in   in;
    int                  size;

    if(sock < 1)
       return 0;

    size = sizeof(in);
    if(getsockname(sock,(struct sockaddr *) &in, &size) == -1)
       return -1;

    return in.sin_addr.s_addr;
 }

 int ICQAPI icqAcceptConnct(int sock, long *ip, int *port)
 {
    int                  size;
    int                  ret;
    struct sockaddr_in   addr;

    memset(&addr,0,sizeof(addr));

    size   = sizeof(addr);
    ret    = accept(    sock,
                        (struct sockaddr *) &addr,
                        &size );

    *ip   = addr.sin_addr.s_addr;
    *port = htons(addr.sin_port);

    return ret;

 }

 int ICQAPI icqListen(int sock)
 {
    return listen(sock,5);
 }

 const char * ICQAPI icqFormatIP(long ip)
 {
    struct in_addr in;
    in.s_addr = ip;
    return inet_ntoa(in);
 }

 void ICQAPI icqWriteNetworkErrorLog(HICQ icq, const char *id, const char *text)
 {
    char        wrk[70];
    time_t      ltime;
	int 		rc		= icqGetSockError();
	
    time(&ltime);
    strftime(wrk, 69, "%m/%d/%Y %H:%M:%S", localtime(&ltime));
    printf("%s %-10s %s: %s (rc=%d)\n",wrk,id,text,strerror(rc),rc);
	
 }

 int ICQAPI icqShutdownSocket(int sock)
 {
    return shutdown(sock,SHUT_RDWR);
 }

 int ICQAPI icqPeekSocket(int sock, int delay)
 {
    int bytes = 0;

    if(ioctl(sock, FIONREAD , (caddr_t) &bytes))
       return -1;

    while(!bytes && delay-- > 0)
    {
       usleep(1000);
       if(ioctl(sock, FIONREAD , (caddr_t) &bytes))
          return -1;
    }

    return bytes;
 }
