/*
 * OS2CONFIG.C - Acesso ao arquivo de INI em OS/2
 */

 #define INCL_WINSHELLDATA
 #define INCL_WIN
 #include <stdio.h>
 #include <string.h>
 #include <icqkernel.h>

/*---[ Definicoes ]----------------------------------------------------------------*/

 #pragma pack(1)
 typedef struct winpos
 {
    USHORT      sz;
    USHORT      x;
    USHORT      y;
    USHORT      cx;
    USHORT      cy;
 } WINPOS;


/*---[ Implementacao ]-------------------------------------------------------------*/

 void icqChkConfigFile(HICQ icq, char *buffer)
 {
    if(icq->ini)
       return;
    strncpy(buffer,icq->programPath,0xFF);
    strncat(buffer,"pwicq.ini",0xFF);
    icqLoadConfigFile(icq,buffer);
 }

 int EXPENTRY icqLoadConfigFile(HICQ icq, char *name)
 {
    /* Abro o arquivo de configuracao */
    /*
    char buffer[0x0100];
    icqQueryPath(icq, "pwicq.ini", buffer, 0xFF);
    icq->ini   = PrfOpenProfile(icq->hab,buffer);

    strncpy(buffer,icq->programPath,0xFF);
    strncat(buffer,ext,0xFF);
 */
    if(icq->ini)
    {
       PrfCloseProfile(icq->ini);
       icq->ini = NULLHANDLE;
    }
    DBGMessage(name);
    icq->ini = PrfOpenProfile(icq->hab,name);

    if(!icq->uin)
       icq->uin = icqLoadProfileValue(icq,"MAIN","uin",0);

    return 0;
 }

 char * EXPENTRY icqLoadProfileString(HICQ icq, const char *aplic, const char *key, const char *def, char *buffer, int size)
 {
    ULONG sz;
    sz = size-1;
    if(PrfQueryProfileData(      icq->ini,
                                 (PSZ) aplic,
                                 (PSZ) key,
                                 buffer,
                                 &sz ) )
    {
       *(buffer+sz) = 0;
    }
    else
    {
       strncpy(buffer,def,size);
    }
    return buffer;
 }

 void EXPENTRY icqSaveProfileString(HICQ icq, const char *aplic, const char *key, const char *string)
 {
    if(!string)
    {
       PrfWriteProfileData(     icq->ini,
                                (PSZ) aplic,
                                (PSZ) key,
                                0,
                                0);
       return;
    }

    PrfWriteProfileData(      icq->ini,
                              (PSZ) aplic,
                              (PSZ) key,
                              (PVOID) string,
                              strlen(string));
 }

 int EXPENTRY icqSaveConfigFile(HICQ icq)
 {
    /* Sem sentido em OS/2 */
    return 0;
 }

/*
 void EXPENTRY WinCentralize(HWND hwnd)
 {
     SWP win;
     SWP dkt;

     WinQueryWindowPos(HWND_DESKTOP,&dkt);
     WinQueryWindowPos(hwnd,&win);

     dkt.cx >>= 1;
     dkt.cy >>= 1;

     DBGTrace(dkt.cx);
     DBGTrace(dkt.cy);

     win.cx >>= 1;
     win.cy >>= 1;

     dkt.cx -= win.cx;
     dkt.cy -= win.cy;

     WinSetWindowPos(    hwnd,
                         0,
                         dkt.cx, dkt.cy,
                         0,0,
                         SWP_MOVE );

 }
*/

 void EXPENTRY icqStoreWindow(HWND hwnd, HICQ icq, ULONG uin, const char *key)
 {
    WINPOS      pos;
    SWP         swp;
    char        work[40];

    WinQueryWindowPos(hwnd,&swp);

    if(swp.fl & SWP_MINIMIZE)
       return;

    sprintf(work,"%d",icq->uin);

    memset(&pos,0,sizeof(WINPOS));
    pos.sz = sizeof(WINPOS);
    pos.x  = swp.x;
    pos.y  = swp.y;
    pos.cx = swp.cx;
    pos.cy = swp.cy;

    PrfWriteProfileData(icq->ini, work, (PSZ) key, &pos, sizeof(WINPOS));

 }

 void EXPENTRY icqRestoreWindow(HWND hwnd, HICQ icq, ULONG uin, const char *key, short cx, short cy)
 {
    char        work[40];
    WINPOS      pos;
    ULONG       sz              = sizeof(WINPOS);
    SWP         dkt;
    ULONG       flags           = SWP_SHOW|SWP_MOVE;

    if(cx >= 0 && cy >= 0)
    {
       flags |= SWP_SIZE;
    }
    else
    {
       WinQueryWindowPos(HWND_DESKTOP,&dkt);
       cx = dkt.cx;
       cy = dkt.cy;
    }

    WinQueryWindowPos(HWND_DESKTOP,&dkt);

    sprintf(work,"%d",icq->uin);

    if(PrfQueryProfileData(icq->ini, work, (PSZ) key, &pos, &sz))
    {
       if(sz == sizeof(pos) && pos.sz == sizeof(WINPOS) && pos.cx < dkt.cx && pos.cy < dkt.cy && pos.cx && pos.cy)
       {
          DBGTracex(pos.cx);
          DBGTracex(pos.cy);
          WinSetWindowPos(hwnd,0,pos.x,pos.y,pos.cx, pos.cy,flags);
          return;
       }
    }

    dkt.cx >>= 1;
    dkt.cy >>= 1;

    dkt.cx -= (cx >> 1);
    dkt.cy -= (cy >> 1);

    WinSetWindowPos(   hwnd,
                       0,
                       dkt.cx, dkt.cy,
                       cx,     cy,
                       flags);

 }

 void EXPENTRY icqDeleteProfile(HICQ icq,const char *aplic)
 {
    if(aplic)
       PrfWriteProfileData(icq->ini,(PSZ) aplic,0,0,0);
 }


