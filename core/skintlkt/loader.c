/*
 * LOADER.C - pwICQ's skin toolkit sample
 */

 #define INCL_WIN
 #include <os2.h>

 #include <malloc.h>
 #include <string.h>

 #include <pwMacros.h>

 #include "skinapi.h"

/*---[ Definicoes ]---------------------------------------------------------------*/

 #define TIMER_TICK 25

/*---[ Prototipos ]---------------------------------------------------------------*/

static void     destroy(HWND);
static void     create(HWND, PCREATESTRUCT);
static MRESULT  buttonAct(HWND, HSKIN, short);
static void     drawItem(HWND,USHORT,POWNERITEM);
static MRESULT  drop(HWND);
static MRESULT  dragLeave(HWND);
static MRESULT  dragOver(HWND, PDRAGINFO);
static MRESULT  accelerator(HWND, HSKIN, USHORT, ULONG, MPARAM, MPARAM);
static void     cicleSkin(HWND,HSKIN);

MRESULT EXPENTRY sknProcedure(HWND, ULONG, MPARAM, MPARAM);

static int _System lbSort(void *, void *);

/*---[ Constantes ]---------------------------------------------------------------*/

static const char *testItens[] =
{
        "Name 8",
        "Name 7",
        "Name 6",
        "Name 5",
        "Name 4",
        "Name 3",
        "Name 2",
        "Name 1",

        NULL
};


static const SKINELEMENT       winElements[] =
{
       STANDARD_SKIN_STUFF

       CMD_ICONBAR,    0, 0,            "Icons",
       CMD_XBITMAP,    0, 0,            "Logo",

       CMD_SLIDER,  1000, 0,            "ScrollBar",

       CMD_BUTTON,  1001, 0,            "Up",
       CMD_BUTTON,  1002, 0,            "Down",

       CMD_FONT,    1003, 0,            "UserListFont",
       CMD_FONT,    101,  0,            "ModeButtonFont",
       CMD_FONT,    102,  0,            "SystemButtonFont",
       CMD_FONT,    103,  0,            "SysMessageFont",

       CMD_LISTBOX, 1003,               WMSKS_CACHEDLB,         "UserList",

       CMD_BUTTON,  101,                WS_VISIBLE,             "ModeButton",
       CMD_BUTTON,  102,                WS_VISIBLE,             "SysButton",
       CMD_BUTTON,  103,                WS_VISIBLE,             "SysMessage",

       0,             0,                0,              NULL
};

/*---[ Statics ]------------------------------------------------------------------*/

 static char		skinPath[0x0100];
 static char		skinName[0x0100];
 static HAB     	hab 		   = NULLHANDLE;
 static HMQ     	hmq;
 static QMSG    	qmsg;
 static PDRAGINFO   	dragInfo	   = NULL;

/*---[ Implementacao ]------------------------------------------------------------*/

int main(int numpar, char *param[])
{
   int          f;
   HWND         hwnd, frame;
   MNUHANDLER   mnu;
   char         buffer[0x0100];

   DBGOpen(PROJECT);
   strcpy(skinPath,"w:\\Public\\pwicq\\skins\\");
   strcpy(skinName,"default.skn");


   if((hab = WinInitialize(0)) == NULLHANDLE)
      return -1;

   if((hmq = WinCreateMsgQueue ( hab , 0 )) != NULLHANDLE)
   {
      if(sknInitialize(hab) && sknRegisterClass(hab,"skintest",sknProcedure,sizeof(void *)))
      {
         strncpy(buffer,skinPath,0xff);
         strncat(buffer,skinName,0xFF);

         hwnd = sknCreateWindow("skintest",buffer,&frame,"SkinTest",(HMODULE) 0, FCF_TASKLIST|FCF_NOMOVEWITHOWNER|FCF_ACCELTABLE|FCF_NOBYTEALIGN, 1000);

         if(hwnd)
         {

            /* Insert listbox and menu test values */
            for(f=0;testItens[f] != NULL;f++)
            {
               sknInsertItemHandle(hwnd,1003,testItens[f]);
               sknInsertMenuItem(hwnd,1003,f,testItens[f]);
               sknInsertMenuItem(hwnd,101,f,testItens[f]);
               sknInsertMenuItem(hwnd,102,f,testItens[f]);
            }

            sknSetSortCallBack(hwnd,1003,lbSort);

            memset(&mnu,0,sizeof(MNUHANDLER));

            mnu.text    = "Teste de menu";
            mnu.after   = 9999;
            mnu.handler = 0;
            mnu.itemID  = 1;

            sknInsertMenuHandler(hwnd,102,&mnu);

            WinSetWindowPos(frame, 0, 10, 10, 150, 375, SWP_SIZE|SWP_MOVE|SWP_SHOW|SWP_ACTIVATE);
            while(WinGetMsg(hab,&qmsg,0,0,0))
               WinDispatchMsg(hab,&qmsg);
         }

      }

      WinDestroyMsgQueue(hmq);
   }
   WinTerminate(hab);

  return 0;
}

static int blink = 0;
static int tick  = 0;

MRESULT EXPENTRY sknProcedure(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch(msg)
   {

   /*---[ Skin messages ]-----------------------------------------*/

   case WMSKN_LOADSKIN:
      sknLoad(hwnd, WinQueryWindowPtr(hwnd,0), winElements, PVOIDFROMMP(mp1));
      break;

   case WMSKN_ACTION:
      DBGMessage("Acao de menu");
      break;

   case WM_SIZE:
      sknResize(hwnd, SHORT1FROMMP(mp2), SHORT2FROMMP(mp2),WinQueryWindowPtr(hwnd,0),winElements);
      break;

    /*---[ Drag&Drop ]---------------------------------------------*/

    case WMSKN_QUERYDRAG:
       if(SHORT1FROMMP(mp2) == 1003)
          return (MRESULT) TRUE;
       return 0;

    case DM_DRAGOVER:
       return dragOver(hwnd, (PDRAGINFO)mp1);

    case DM_DRAGLEAVE:
       return dragLeave(hwnd);

    case DM_DROP:
       return drop(hwnd);

   /*---[ Actions ]-----------------------------------------------*/


   /*---[ Window messages ]---------------------------------------*/


   case WM_CREATE:
      create(hwnd,PVOIDFROMMP(mp2));
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   case WM_DESTROY:
      destroy(hwnd);
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   /* Owner draw controls - Uncomment it to disable the internal skin drawing */

   case WM_TIMER:

      sknDoTimer(hwnd,WinQueryWindowPtr(hwnd,0));

      if(++tick > 40)
      {
         tick  = 0;
         blink = ~blink;
         sknBlinkControl(hwnd,1003,blink);
      }
      break;

   case WM_MEASUREITEM:
      if(SHORT1FROMMP(mp1) == 1003)
         return MRFROM2SHORT( (sknQueryIconSize(WinQueryWindowPtr(hwnd,0))+4),1024);
      return sknDefWindowProc(hwnd, WinQueryWindowPtr(hwnd,0), msg, mp1, mp2);

   case WM_DRAWITEM:
      if(SHORT1FROMMP(mp1) != 1003)
         return sknDefWindowProc(hwnd, WinQueryWindowPtr(hwnd,0), msg, mp1, mp2);
      drawItem(hwnd,SHORT1FROMMP(mp1),PVOIDFROMMP(mp2));
      break;

   case WM_COMMAND:
//      if(SHORT1FROMMP(mp2) != CMDSRC_ACCELERATOR || SHORT1FROMMP(mp1) != 1000)
//         return sknDefWindowProc(hwnd, WinQueryWindowPtr(hwnd,0), msg, mp1, mp2);
//      sknReloadSkin(hwnd,NULL,WinQueryWindowPtr(hwnd,0),winElements);

      if(SHORT1FROMMP(mp2) == CMDSRC_ACCELERATOR)
         return accelerator(hwnd,WinQueryWindowPtr(hwnd,0),SHORT1FROMMP(mp1),msg,mp1,mp2);

      return sknDefWindowProc(hwnd, WinQueryWindowPtr(hwnd,0), msg, mp1, mp2);

      break;

   default:
      return sknDefWindowProc(hwnd, WinQueryWindowPtr(hwnd,0), msg, mp1, mp2);

   }
   return 0;
}

static MRESULT accelerator(HWND hwnd, HSKIN skn, USHORT id, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   char buffer[0x0100];

   switch(id)
   {
   case 1000:   /* Recarrega skin atual */
      strncpy(buffer,skinPath,0xff);
      strncat(buffer,skinName,0xFF);
      sknLoadNewSkin(hwnd,winElements,buffer);
      break;

   case 1001:   /* Recicla skins */
      cicleSkin(hwnd, skn);
      strncpy(buffer,skinPath,0xff);
      strncat(buffer,skinName,0xFF);
      sknLoadNewSkin(hwnd,winElements,buffer);
      break;

   default:
      return sknDefWindowProc(hwnd, skn, msg, mp1, mp2);

   }

   return 0;

}

static void destroy(HWND hwnd)
{
   WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, 1);
   sknDestroy(WinQueryWindowPtr(hwnd,0));
}

static void create(HWND hwnd, PCREATESTRUCT info)
{
   int  f;

   WinSetWindowPtr(hwnd,0,sknCreate(hwnd,winElements,info->cx,info->cy,info->pszText));
   WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, 1, TIMER_TICK);

   /* Interconnect listbox,scrollbar,up and down buttons */
   sknConnectListBox(hwnd,1003,1000,1001,1002);

   /*
    * Create popup menu
    *
    * Parameters:
    *
    * Handle of the main window
    * ID of the listbox/button window
    * ID of the new Menu
    * Style of the new menu
    *
    */
   sknCreatePopupMenu(hwnd, 1003, 2000, WMSKS_SENDACTION);
   sknCreatePopupMenu(hwnd, 101,  2001, WMSKS_SENDACTION);
   sknCreatePopupMenu(hwnd, 102,  2002, WMSKS_SENDACTION);

   WinSetDlgItemText(hwnd, 101, "Button 2");
   WinSetDlgItemText(hwnd, 102, "Button 1");
   WinSetDlgItemText(hwnd, 103, "Button 3");

}

static MRESULT buttonAct(HWND hwnd, HSKIN cfg, short id)
{
   return sknDefaultButtons(hwnd, cfg, id);
}

/*
   Draw control - Supplied by the default procedure, uncomment if you want to draw
                  your controls

*/

static void drawItem(HWND hwnd, USHORT id, POWNERITEM i)
{
   USHORT       left;
   HSKIN        skn     =  WinQueryWindowPtr(hwnd,0);

   if(i->fsState & 0x80000000)
   {
      /* It's drawing in the cache bitmap, draw only images */

      left =  sknDrawIcon(skn, i->hps, 5, i->rclItem.xLeft+4,i->rclItem.yBottom+3);

      if(i->fsState&2)
         sknDrawIcon(skn, i->hps, 0, i->rclItem.xLeft+4,i->rclItem.yBottom+3);

      i->rclItem.xLeft += (left+8);
   }

   if(i->fsState & 0x40000000)
   {
      /* It's drawing on the screen, the images was already draw from the cache bitmap */
      WinDrawText(   i->hps,
                  -1,
                  (char *) i->hItem,
                  &i->rclItem,
                  i->fsState & 1 ? SKC_SELECTEDFOREGROUND : SKC_FOREGROUND,
                  SKC_BACKGROUND,
                  DT_LEFT|DT_VCENTER);
   }



}

static int _System lbSort(void *s1, void *s2)
{
   return strcmp(s1,s2);
}

static MRESULT dragOver(HWND hwnd, PDRAGINFO dInfo)
{
   int         i;
   USHORT      cItems;
   PDRAGITEM   dragItem;
   USHORT      rc              = DOR_NODROPOP;
   char buffer[0x0100];

   dragInfo = dInfo;

   DrgAccessDraginfo(dragInfo);

   cItems = DrgQueryDragitemCount(dragInfo);

   for (i = 0; i < cItems && rc == DOR_NODROPOP; i++)
   {
      dragItem = DrgQueryDragitemPtr(dragInfo, i);

      DrgQueryStrName(dragItem->hstrType, 0xFF, buffer);

      if(!strcmp(buffer,"UniformResourceLocator"))
         rc = DOR_DROP;

   }

   DrgFreeDraginfo(dragInfo);
   return (MRFROM2SHORT(rc, DO_COPY));
}

static MRESULT dragLeave(HWND hwnd)
{
   dragInfo = NULL;
   return (MRESULT) FALSE;
}

static MRESULT drop(HWND hwnd)
{
   char        buffer[0x0100];
   int         i;
   USHORT      cItems;
   PDRAGITEM   dragItem;
   int         sz;

   DBGTracex(dragInfo);

   if(!dragInfo)
      return (MRFROM2SHORT (DOR_NODROPOP, 0));

   DrgAccessDraginfo(dragInfo);

   cItems = DrgQueryDragitemCount(dragInfo);
   DBGTrace(cItems);
   for(i = 0; i < cItems; i++)
   {
      dragItem = DrgQueryDragitemPtr(dragInfo, i);

      DrgQueryStrName(dragItem->hstrType, 0xFF, buffer);
      if(!strcmp(buffer,"UniformResourceLocator"))
      {
         sz = DrgQueryStrName(dragItem->hstrSourceName, 0xFF, buffer);
         DBGTrace(sz);

         if(sz > 0)
         {
            *(buffer+sz) = 0;
            DBGMessage(buffer);
         }
      }
   }

   CHKPoint();

   DrgFreeDraginfo(dragInfo);

   dragLeave(hwnd);

   CHKPoint();
   return (MRFROM2SHORT(DOR_DROP, DO_COPY));

}

static void cicleSkin(HWND hwnd, HSKIN skn)
{
   char                buffer[0x0100];
   FILEFINDBUF3        fl;
   ULONG               fCount;
   HDIR                hDir;
   int		       next    = FALSE;
   APIRET              rc;

   strncpy(buffer,skinPath,0xFF);
   strncat(buffer,"*.skn",0xFF);
   DBGMessage(buffer);

   fCount  = 1;
   hDir    = HDIR_CREATE;
   rc      = DosFindFirst(buffer,&hDir,FILE_NORMAL,&fl,sizeof(FILEFINDBUF3),&fCount,FIL_STANDARD);

   *buffer = 0;

   while(!rc && !*buffer)
   {
      if(next)
         strncpy(buffer,fl.achName,0xFF);

      if(!stricmp(skinName,fl.achName))
         next = TRUE;

      fCount = 1;
      rc     = DosFindNext(hDir,&fl,sizeof(FILEFINDBUF3),&fCount);
   }

   DosFindClose(hDir);

   DBGMessage(buffer);

   if(!*buffer)
   {
      strncpy(buffer,skinPath,0xFF);
      strncat(buffer,"*.skn",0xFF);

      fCount  = 1;
      hDir    = HDIR_CREATE;
      rc      = DosFindFirst(buffer,&hDir,FILE_NORMAL,&fl,sizeof(FILEFINDBUF3),&fCount,FIL_STANDARD);
      strncpy(buffer,fl.achName,0xFF);
      DosFindClose(hDir);
   }

   DBGMessage(buffer);

   if(*buffer)
      strncpy(skinName,buffer,0xFF);


}

