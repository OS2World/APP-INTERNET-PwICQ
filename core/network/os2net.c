/*
 * ICQNET.C - Funcoes de rede para o pwICQ
 *
 */

 #define INCL_DOSMISC
 #define INCL_DOSPROCESS
 #include <os2.h>

 #define BSD_SELECT

 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>

#ifdef TCPV40HDRS

 #include <types.h>
 #include <netinet/in.h>
 #include <sys/socket.h>
 #include <sys/select.h>
 #include <netdb.h>

#else

 #define __ssize_t /* To satisfy the TCP-IP 4.21 toolkit */

 #include <sys/time.h>
 #include <types.h>
 #include <unistd.h>
 #include <sys/socket.h>
 #include <sys/sysctl.h>
 #include <netdb.h>
 #include <netinet/in.h>
 #include <net/if.h>
 #include <sys/ioctl.h>
 #include <arpa/inet.h>
 #include <nerrno.h>

#endif

 #include <icqnetw.h>

/*---[ Macros ]--------------------------------------------------------------------------------------------*/

#ifdef OS2SOCKS

// int _System Raccept __TCPPROTO((int, struct sockaddr *, int *));
// int _System Rbind __TCPPROTO((int, struct sockaddr *, int, struct sockaddr *));
// int _System Rconnect __TCPPROTO((int, const struct sockaddr *, int));
// int _System Rgetsockname __TCPPROTO((int, struct sockaddr *, int *));
// int _System Rlisten __TCPPROTO((int, int));

    #define connect(s, n, l)     ERROR(s, n, l)
    #define gethostbyname(n)     Rgethostbyname(n)
    #define getsockname(s, n, l) Rgetsockname(s, n, l)
    #define accept(s, n, l)      Raccept(s, n, l)
    #define listen(s, b)         Rlisten(s, b)

#endif

/*---[ Defines ]-------------------------------------------------------------------------------------------*/

#ifndef loopback
 #define loopback 0x0100007f
#endif

 #define ERROR(x) icqWriteNetworkErrorLog(icq, PROJECT, x)

/*---[ Statics ]-------------------------------------------------------------------------------------------*/


/*---[ Prototipes ]----------------------------------------------------------------------------------------*/

 static int ajustTCPSocket(int);

/*---[ Implementation ]------------------------------------------------------------------------------------*/

 int ICQAPI icqnetw_Configure(HICQ icq, HMODULE mod, char *buffer)
 {
    // Returns TRUE if the detected MPTS version is valid

#ifndef TCPV40HDRS

    int                      mib[4] = { CTL_KERN,KERN_HOSTID,IPPROTO_IP,KERNCTL_INETVER };
    struct inetvers_ctl      vrs;
    size_t                   sz    = sizeof(vrs);
    int                      rc;

    *buffer = 0;

    rc = sysctl(mib, 4, &vrs, &sz, NULL, 0);

    if(rc == -1)
    {
       ERROR("Can't verify network stack level");
       return -1;
    }
    else
    {
       strcpy(buffer,"Detected MPTS ");
       strncat(buffer,vrs.versionstr,79);
       strncat(buffer," and ICQNET32 Build " BUILD,79);
       #ifdef OS2SOCKS
          strncat(buffer," (Socks version)",79);
       #endif
       #ifdef DEBUG
          strncat(buffer," [DEBUG]",79);
       #endif

       rc = atoi(vrs.versionstr);

       if(rc < 6)
          return -1;

    }

#else

    strcpy(buffer,"Using ICQNET16 Build " BUILD);
    #ifdef OS2SOCKS
       strncat(buffer," (Socks version)",79);
    #endif
    #ifdef DEBUG
       strncat(buffer," [DEBUG]",79);
    #endif

#endif


    return 0;
 }

 int ICQAPI icqnetw_Start(HICQ icq, void *nothing, HWND hwnd)
 {
    return -1;
 }

 void ICQAPI icqnetw_ConfigPage(HICQ icq, void *lixo, ULONG uin, USHORT type, HWND hwnd, const DLGINSERT *dlg, char *buffer)
 {
    if(type != 1)
       return;
 }

#ifdef OS2SOCKS
 int ICQAPI icqBindSocket(int sock, int port, BOOL any)
 {
    struct sockaddr_in  in;
    struct sockaddr_in  remote;

    memset(&in, 0, sizeof(in));
    in.sin_family      = AF_INET;
    in.sin_port        = htons(port);
    in.sin_addr.s_addr = INADDR_ANY;

    memset(&remote, 0, sizeof(remote));

    if(!Rbind(sock, (struct sockaddr *) &in, sizeof(in), (struct sockaddr *) &remote))
       return 0;

#ifdef DEBUG
    psock_errno("Bind");
#endif

    memset(&in, 0, sizeof(in));
    in.sin_family      = AF_INET;
    in.sin_port        = 0;
    in.sin_addr.s_addr = INADDR_ANY;

    memset(&remote, 0, sizeof(remote));

    if(!Rbind(sock, (struct sockaddr *) &in, sizeof(in), (struct sockaddr *) &remote))
       return 0;

   return -1;

 }
#else
 int ICQAPI icqBindSocket(int sock, int port, BOOL any)
 {
    struct sockaddr_in  in;
    static int          f = 1;

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &f, sizeof(f));

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
#endif

 static int opensocket(int port, int type)
 {
    int                 sock = socket(AF_INET, type, 0);
    struct timeval      t;

    if(sock < 0)
       return sock;

#ifndef TCPV40HDRS
    t.tv_sec  = 1;
    t.tv_usec = 0;
    if(setsockopt(sock, SOL_SOCKET, 0x1006, (char *) &t, sizeof(t)) != 0)
    {
       soclose(sock);
       return -1;
    }
#endif

    if(port >= 0)
    {
       if(icqBindSocket(sock,port,TRUE) < 0)
       {
          soclose(sock);
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
    return ajustTCPSocket(opensocket(port,SOCK_STREAM));
 }

 static int ajustTCPSocket(int sock)
 {
    struct linger       l;
    int                 kpl;

    if(sock < 0)
       return sock;

    l.l_onoff  = 0;
    l.l_linger = 1;

    if(setsockopt(sock,SOL_SOCKET,SO_LINGER,(char *) &l, sizeof(struct linger)) == -1)
    {
       icqCloseSocket(sock);
       return -1;
    }

    kpl = 1;
    if(setsockopt(sock,SOL_SOCKET,SO_KEEPALIVE,(char *) &kpl, sizeof(kpl)) == -1)
    {
       icqCloseSocket(sock);
       return -1;
    }

    return sock;
 }

 int ICQAPI icqCloseSocket(int sock)
 {
    soclose(sock);
    return 0;
 }

 int ICQAPI icqConnectSock(int sock, long ip, int port)
 {
    struct sockaddr_in  addr;

    memset(&addr,0,sizeof(addr));
    addr.sin_family             = AF_INET;
    addr.sin_addr.s_addr        = ip;
    addr.sin_port               = htons(port);

#ifdef OS2SOCKS
    if(Rconnect(sock,(struct sockaddr *) &addr,sizeof(addr)))
#else
    if(connect(sock,(struct sockaddr *) &addr,sizeof(addr)))
#endif
    {
       soclose(sock);
       return -1;
    }

    return sock;
 }

 int ICQAPI icqGetSockError(void)
 {
#ifdef DEBUG
    psock_errno("Socket error:");
#endif
    return sock_errno();
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
    long ret = htonl(gethostid());

    if(ret == 0 || ret == 0xFFFFFFFF)
       ret = loopback;

    return ret;
 }

 int ICQAPI icqSendTo(int sock, long ip, int port, const void *pkt, int size)
 {
    struct sockaddr_in   to;

    to.sin_family      = AF_INET;
    to.sin_addr.s_addr = ip;
    to.sin_port        = htons(port);

    if(sendto(sock,(char *) pkt,size,0,(struct sockaddr *) &to, sizeof(to)) == -1)
       return icqGetSockError();

    return 0;
 }

 int ICQAPI icqRecvFrom(int sock, long *ip, int *port, void *buffer, int size)
 {
    int                 wrk;
    struct sockaddr_in  addr;

#ifdef TCPV40HDRS

    fd_set              fds;
    struct timeval      tv;

    tv.tv_sec  = 1;
    tv.tv_usec = 0;

    FD_ZERO(&fds);
    FD_SET(sock, &fds);

    select(sock+1, &fds, NULL, NULL, &tv);

    if(!FD_ISSET(sock, &fds))
       return 0;

#endif

    wrk  = sizeof(addr);
    memset(&addr,0,wrk);

    size = recvfrom(     sock,
                         (char *) buffer,
                         size,
                         0,
                         (struct sockaddr *) &addr,
                         &wrk);

    if(size == -1 && sock_errno() == SOCEWOULDBLOCK)
       return 0;

    *ip   = addr.sin_addr.s_addr;
    *port = htons(addr.sin_port);

#ifdef DEBUG
    if(size < 0)
    {
       psock_errno("Socket error:");
    }
#endif

    return size;

 }

 int ICQAPI icqSend(int sock, const void *ptr, int size)
 {
    if(size == -1)
       size = strlen(ptr);

    if(send(sock, (char *) ptr, size, 0) <= 0)
       return icqGetSockError();

    return 0;
 }

 int ICQAPI icqRecv(int sock, void *ptr, int size)
 {
#ifdef TCPV40HDRS

    fd_set              fds;
    struct timeval      tv;

    tv.tv_sec  = 1;
    tv.tv_usec = 0;

    FD_ZERO(&fds);
    FD_SET(sock, &fds);

    select(sock+1, &fds, NULL, NULL, &tv);

    if(!FD_ISSET(sock, &fds))
       return 0;

#endif

    size = recv(sock,(char *) ptr, size, 0);

    if(size == 0)
    {
#ifdef EXTENDED_LOG
       icqWriteSysLog(NULL,PROJECT,"Connection was closed");
#endif
       return -1;
    }

    if(size == -1 && sock_errno() == SOCEWOULDBLOCK)
       return 0;


    return size;

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
    struct timeval       t;

    memset(&addr,0,sizeof(addr));

    size   = sizeof(addr);
    ret    = accept(    sock,
                        (struct sockaddr *) &addr,
                        &size );

    *ip   = addr.sin_addr.s_addr;
    *port = htons(addr.sin_port);

#ifndef TCPV40HDRS
    if(ret > 0)
    {
       t.tv_sec  = 1;
       t.tv_usec = 0;
       if(setsockopt(ret, SOL_SOCKET, 0x1006, (char *) &t, sizeof(t)) != 0)
       {
          soclose(ret);
          return -1;
       }
    }
#endif

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

 ULONG ICQAPI icqString2IPAddress(char *str)
 {
    return inet_addr(str);
 }

 void ICQAPI icqWriteNetworkErrorLog(HICQ icq, const char *id, const char *text)
 {
    UCHAR       *IvTable[2]             = {0};
    ULONG       ulMsgLen                = 0;
    int		    rc                      = icqGetSockError();

    int         err;
    char        *ptr;
    char        wrk[70];
    time_t      ltime;
    char        szOutMsg[0x0100];

#ifndef DEBUG
    if(!rc)
       return;
#endif

    rc -= SOCBASEERR;

    time(&ltime);
    strftime(wrk, 69, "%m/%d/%Y %H:%M:%S", localtime(&ltime));

    if(rc < 0)
    {
       strcpy(szOutMsg,"General failure");
    }
    else
    {
       err = DosGetMessage( IvTable,          /* Message insert pointer array */
                            0,                /* Number of inserts */
                            szOutMsg,         /* Output message */
                            0xFF,             /* Length of output message area */
                            rc,               /* Number of message requested */
                            "SCK.MSG",        /* Message file (created by MKMSGF) */
                            &ulMsgLen);       /* Length of resulting output message */

       if(err)
       {
          sprintf(szOutMsg,"Error %d loading SCK.MSG for message %d",err,rc+SOCBASEERR);
       }
       else if(ulMsgLen < 0x0100)
       {
          *(szOutMsg+ulMsgLen) = 0;
          for(ptr=szOutMsg;*ptr && *ptr != '\n';ptr++);
          *ptr = 0;
       }
    }

    printf("%s %-10s %s %s (rc=%d)\n",wrk,id,text,szOutMsg,rc);

 }

 int ICQAPI icqShutdownSocket(int sock)
 {
    so_cancel(sock);
    return shutdown(sock,2);
 }


 int ICQAPI icqPeekSocket(int sock, int delay)
 {
    int bytes = 0;

    if(ioctl(sock, FIONREAD , (caddr_t) &bytes))
       return -1;

    while(!bytes && delay-- > 0)
    {
       DosSleep(10);
       if(ioctl(sock, FIONREAD , (caddr_t) &bytes))
          return -1;
    }

    return bytes;
 }


