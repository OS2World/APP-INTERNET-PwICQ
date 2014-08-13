/*
 * common.c - OS independent network functions
 */

#ifdef __OS2__
   #pragma strings(readonly)
   #define __ssize_t /* To satisfy the TCP-IP 4.21 toolkit */
   #include <types.h>
   #include <sys/socket.h>
#endif

#ifdef linux
   #include <sys/types.h>
   #include <sys/socket.h>
#endif

#ifdef WIN32
   #include <windows.h>
   #include <winsock2.h>
#endif

 #include <icqnetw.h>

/*---[ Implementation ]------------------------------------------------------------------------------------*/

 int ICQAPI icqConnectHost(char *hostname, int port)
 {
    long ip      = icqGetHostByName(hostname);
    if(!ip)
       return -1;
    return icqConnect(ip, port);
 }

 int ICQAPI icqConnect(long ip, int port)
 {
    int sock =  icqOpenTCP(-1);

    if(sock < 0)
       return sock;

    return icqConnectSock(sock,ip,port);
 }

 int ICQAPI icqEnableBroadcast(int sock)
 {
    int iValue = 1;
    return setsockopt(sock,(int) SOL_SOCKET, (int) SO_BROADCAST,(const char *) &iValue, sizeof(iValue));
 }


