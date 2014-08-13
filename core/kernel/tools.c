/*
 * tools.c - Ferramentas diversas
 */

#ifdef __OS2__

 #pragma strings(readonly)
#endif

 #include <icqkernel.h>


 /*---[ Implementacao ]-------------------------------------------------------------------------------*/

 const char * EXPENTRY icqIP2String(long IP, char *buffer)
 {
    unsigned char        *src    = (unsigned char *) &IP;
    unsigned char        *dst    = buffer;
    unsigned char        tmp[4];
    unsigned int         f,v,p;


    for(f=0;f<4;f++)
    {
       v = *(src++);
       if(v)
       {
          for(p=0;p<3&&v;p++)
          {
             *(tmp+p) = (v%10);
             v /= 10;
          }
          while(p--)
             *(dst++) = '0'+ *(tmp+p);
       }
       else
       {
          *(dst++) = '0';
       }
       if(f < 3)
          *(dst++) = '.';

    }
    *dst = 0;
    return buffer;
 }

 HWND EXPENTRY icqOpenConfigDialog(HICQ icq, ULONG uin, USHORT id)
 {
    if(icq->skin && icq->skin->openConfig)
       return icq->skin->openConfig(icq,uin,id);
    return 0;
 }

 int EXPENTRY icqPopupUserList(HICQ icq)
 {
    if(icq->skin && icq->skin->popupUserList)
       return icq->skin->popupUserList(icq);
    return -1;
 }

 const char * EXPENTRY icqQuerySkinManagerName(HICQ icq)
 {
	if(!icq->skin)
	   return "none";
	
	if(!icq->skin->id)
	   return "no-name";
	
	return icq->skin->id;
 }


