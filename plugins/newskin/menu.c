/*
 * menu.c - Common menu processing
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #define INCL_WIN
#endif

 #include <malloc.h>
 #include <string.h>
 #include <pwicqgui.h>

/*---[ Prototipes ]-----------------------------------------------------------*/


/*---[ Implementation ]-------------------------------------------------------*/

 int _System icqskin_popupMenu(HICQ icq, ULONG uin, USHORT id, USHORT x, USHORT y)
 {
	SKINDATA *skn = icqskin_getDataBlock(icq);

	DBGTrace(id);
	
	if(id >= ICQMNU_COUNTER || !skn->menu[id])
	{
	   DBGMessage("Invalid popup menu request");
	   icqWriteSysLog(icq,PROJECT,"Invalid popup menu request");
	   return -1;
	}
	
    icqskin_showMenu(skn->menu[id],uin,x,y);
	
    return 0;
 }

 int _System icqskin_insertMenu(HICQ icq, USHORT menu, const char *text, USHORT id, MENUCALLBACK *cbk, ULONG cbkParm)
 {
    SKINDATA    *skn   = icqskin_getDataBlock(icq);
    hWindow     hwnd;

	if(menu >= ICQMNU_COUNTER || !skn->menu[menu])
	   return -1;

    if(!skn)
       return 1;

    hwnd = skn->menu[menu];

    if(!hwnd)
       return 2;

    return icqskin_insertMenuOption(hwnd, text, id, cbk, cbkParm);
 }




