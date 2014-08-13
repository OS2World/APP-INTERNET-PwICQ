/*
 * ICQNET.C - Funcoes de rede para o pwICQ
 */

#ifndef WIN32
   #error Windows only
#endif

 #include <stdio.h>
 #include <string.h>
 #include <sys/types.h>
 #include <errno.h>
 #include <unistd.h>
 #include <time.h>

 #include <icqnetw.h>
 #include <winsock2.h>

/*---[ Macros ]-----------------------------------------------------------------*/

/*---[ Constants ]--------------------------------------------------------------*/

/*---[ Implementation ]---------------------------------------------------------*/

 int DLLENTRY icqnetw_Configure(HICQ icq, HMODULE mod)
 {
    return 0;
 }

 int DLLENTRY icqnetw_Start(HICQ icq, void *nothing, HWND hwnd)
 {
    return -1;
 }

 void DLLENTRY icqnetw_ConfigPage(HICQ icq, void *lixo, ULONG uin, USHORT type, HWND hwnd, const DLGINSERT *dlg, char *buffer)
 {
    return;
 }

 int DLLENTRY icqBindSocket(int sock, int port, BOOL any)
 {
    struct sockaddr_in  in;
    static const int	f    = 1;

    setsockopt(sock, (int) SOL_SOCKET, (int) SO_REUSEADDR, (const char *) &f, sizeof(f));

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

 int DLLENTRY icqOpenUDP(int port)
 {
    return opensocket(port,SOCK_DGRAM);
 }

 int DLLENTRY icqOpenTCP(int port)
 {
    return opensocket(port,SOCK_STREAM);
 }

 int DLLENTRY icqCloseSocket(int sock)
 {
    close(sock);
    return 0;
 }

 int DLLENTRY icqConnectSock(int sock, long ip, int port)
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

 int DLLENTRY icqGetSockError(void)
 {
    return errno;
 }

 long DLLENTRY icqGetHostByName(char *hostname)
 {
    struct hostent *host = gethostbyname(hostname);
    if(host == NULL)
       return 0;

    return *( (long *) host->h_addr );
 }

 long DLLENTRY icqGetHostID(void)
 {
    return 0x0100007f;
 }

 int DLLENTRY icqSendTo(int sock, long ip, int port, const void *pkt, int size)
 {
    struct sockaddr_in   to;

    to.sin_family      = AF_INET;
    to.sin_addr.s_addr = ip;
    to.sin_port        = htons(port);

    if(sendto(sock,(char *) pkt,size,0,(struct sockaddr *) &to, sizeof(to)) == -1)
       return icqGetSockError();

    return 0;
 }

 int DLLENTRY icqRecvFrom(int sock, long *ip, int *port, void *buffer, int size)
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

 int DLLENTRY icqSend(int sock, const void *ptr, int size)
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

 int DLLENTRY icqRecv(int sock, void *ptr, int size)
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

 int DLLENTRY icqGetPort(int sock)
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

 long DLLENTRY icqGetIPAdress(int sock)
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

 int DLLENTRY icqAcceptConnct(int sock, long *ip, int *port)
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

 int DLLENTRY icqListen(int sock)
 {
    return listen(sock,5);
 }

 const char * DLLENTRY icqFormatIP(long ip)
 {
    struct in_addr in;
    in.s_addr = ip;
    return inet_ntoa(in);
 }

 void DLLENTRY icqWriteNetworkErrorLog(HICQ icq, const char *id, const char *text)
 {
    char        wrk[70];
    time_t      ltime;
    int		rc		= icqGetSockError();
	
    time(&ltime);
    strftime(wrk, 69, "%m/%d/%Y %H:%M:%S", localtime(&ltime));
    printf("%s %-10s %s: %s (rc=%d)\n",wrk,id,text,strerror(rc),rc);
	
 }

 int DLLENTRY icqShutdownSocket(int sock)
 {
    return shutdown(sock,2);
 }

