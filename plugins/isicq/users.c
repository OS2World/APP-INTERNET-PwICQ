/*
 * send.c - I-Share send packets
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <stdio.h>
 #include <stdlib.h>

 #include "ishared.h"

/*---[ Prototipes ]--------------------------------------------------------------------------------------*/


/*---[ Implementation ]----------------------------------------------------------------------------------*/

 ISHARED_USER * ICQAPI ishare_cacheUser(ISHARED_CONFIG *cfg, long ip, USHORT port)
 {
#ifdef PWICQ
    HUSER          pwicqUser;
#endif
    ISHARED_USER   *usr =  NULL;  	
    int            f;

    for(f=0;f<CACHE_USERS && !usr;f++)
    {
       if(cfg->users[f].ip == ip && cfg->users[f].port == port)
       {
          usr       = cfg->users+f;
          usr->idle =
          usr->kpl  = 0;
       }
    }

#ifdef PWICQ
    /*
     * Verify for pwICQ users
     */
    if(cfg->icq)
    {
       for(pwicqUser=icqQueryFirstUser(cfg->icq);pwicqUser;pwicqUser=icqQueryNextUser(cfg->icq,pwicqUser))
       {
          if(icqQueryUserGateway(cfg->icq, pwicqUser) == ip)
          {
             if(usr)
             {
                usr->ip   = 0;
                usr->port = 0;
                usr->idle =
                usr->kpl  = 0;
                usr->seed = 0;
             }
             return NULL;
          }
       }
    }
#endif

    if(usr)
    {
       usr->seed = (rand() % ISHARE_KEEPALIVE_TIMER);
       DBGTrace(usr->seed);
       return usr;
    }

    /* Find empty entry */
    for(f=0;f<CACHE_USERS && !usr;f++)
    {
       if(!cfg->users[f].ip)
          usr = cfg->users+f;
    }

    if(!usr)
    {
       /* Find user more idle */
       usr = cfg->users;
       for(f=1;f<CACHE_USERS;f++)
       {
          if(usr->idle < cfg->users[f].idle)
             usr = cfg->users+f;
       }
    }

    if(usr)
    {
       usr->ip   = ip;
       usr->port = port;
       usr->idle =
       usr->kpl  = 0;
       usr->seed = (rand() % ISHARE_KEEPALIVE_TIMER);
       DBGTrace(usr->seed);
    }

    return usr;

 }

 ISHARED_USER * ICQAPI ishare_getUser(ISHARED_CONFIG *cfg, long ip)
 {
    ISHARED_USER   *usr =  NULL;
    int            f;
      	
    for(f=0;f<CACHE_USERS && !usr;f++)
    {
       if(cfg->users[f].ip == ip)
          usr = cfg->users+f;
    }
    return usr;
 }

