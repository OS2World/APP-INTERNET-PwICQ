/*
 * getset.c - set/get properties
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdio.h>

 #include "peer.h"

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 ULONG _System icqv8_setCookie(HICQ icq,HPEER cfg, ULONG cookie)
 {
    ULONG ret = cfg->cookie;
    cfg->cookie = cookie;
    DBGTracex(cfg->cookie);
    return ret;
 }

 ULONG _System icqv8_getCookie(HICQ icq, HPEER cfg)
 {
    return cfg->cookie;
 }

 long _System icqv8_getPort(HICQ icq, HPEER cfg)
 {
    if(!cfg)
       return 0x2301;

    DBGTrace(icqGetPort(cfg->sock));
    DBGTracex(icqGetPort(cfg->sock));

    if(cfg->sock > 0)
       return icqGetPort(cfg->sock);
    return 0;
 }

 long _System icqv8_getAddress(HICQ icq, HPEER cfg)
 {
	long ip = 0;
	
    if(cfg && cfg->sock > 0)
	{

       DBGTrace(icqGetIPAdress(cfg->sock));
       DBGTracex(icqGetIPAdress(cfg->sock));

       ip = icqGetIPAdress(cfg->sock);
	   
	   if(ip < 0)
	   {
		  ip = 0;
		  icqWriteNetworkErrorLog(icq, PROJECT, "Failure getting local IP address");
	   }
	   
	   DBGTrace(ip);
	}
	
    return ip;
 }

 int icqv8_setPeerSpeed(HICQ icq, PEERSESSION *cfg, ULONG speed)
 {
    cfg->speed = icqLoadValue(icq,"peer.speed",64);

    if( speed < cfg->speed)
       cfg->speed = speed;

    if(cfg->speed > 64)
       cfg->speed = 64;

    cfg->delay  = (64 - cfg->speed) * 5;

    CHKPoint();

    return 0;
 }


