/*
 * icqnetw.c - pwICQ Network module definitions
 */

#ifndef ICQNETW_INCLUDED

   #define ICQNETW_INCLUDED 1

   #ifdef __OS2__
      #include <os2.h>
      #ifndef DLLENTRY
         #define DLLENTRY EXPENTRY
      #endif
      #define OS2_TYPEDEFS_INCLUDED 1
   #endif

   #ifdef WIN32
      #include <windows.h>
      #ifndef DLLENTRY
         #define DLLENTRY __stdcall __export
      #endif
      #define OS2_TYPEDEFS_INCLUDED 1
   #endif

   #ifdef linux
      #define HWND void *
      #ifndef DLLENTRY
         #define DLLENTRY
      #endif
   #endif

   #ifndef ICQAPI
      #define ICQAPI DLLENTRY
   #endif

   #ifndef OS2_TYPEDEFS_INCLUDED

      typedef unsigned short    USHORT;
      typedef unsigned char     UCHAR;
      typedef unsigned long     ULONG;
      typedef char              CHAR;
      typedef unsigned char     BOOL;
      typedef void *            HMODULE;

      #ifndef FALSE
         #define FALSE          0x00
      #endif

      #ifndef TRUE
         #define TRUE          !FALSE
      #endif

      #define _System

      #define sysSleep(x) usleep(x*1000000)

   #endif

   #ifndef HICQ
      #define HICQ void *
      #define DLGINSERT ULONG
   #endif

   void         DLLENTRY icqnetw_ConfigPage(HICQ, void *, ULONG, USHORT, HWND, const DLGINSERT *, char *);
   void         DLLENTRY icqWriteNetworkErrorLog(HICQ, const char *, const char *);

   int          DLLENTRY icqOpenUDP(int);
   int          DLLENTRY icqOpenTCP(int);
   int          DLLENTRY icqConnectSock(int,long,int);
   int          DLLENTRY icqCloseSocket(int);
   int          DLLENTRY icqConnect(long, int);
   int          DLLENTRY icqGetSockError(void);
   long         DLLENTRY icqGetHostByName(char *);
   long         DLLENTRY icqGetHostID(void);
   int          DLLENTRY icqSendTo(int, long, int, const void *, int);
   int          DLLENTRY icqRecvFrom(int, long *, int *, void *, int);
   int          DLLENTRY icqSend(int, const void *, int);
   int          DLLENTRY icqRecv(int, void *, int);
   int          DLLENTRY icqConnectHost(char *, int);
   int          DLLENTRY icqGetPort(int);
   long         DLLENTRY icqGetIPAdress(int);
   int          DLLENTRY icqAcceptConnct(int, long *, int *);
   int          DLLENTRY icqListen(int);
   int          DLLENTRY icqConnectSock(int, long, int);
   int          DLLENTRY icqEnableBroadcast(int);
   int          DLLENTRY icqShutdownSocket(int);
   int          DLLENTRY icqBindSocket(int, int, BOOL);
   int          DLLENTRY icqPeekSocket(int, int);

   const char * DLLENTRY icqFormatIP(long);

#endif


