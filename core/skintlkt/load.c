/*
 * INIMAIN.C - Inicializa a janela principal
 */

 #define INCL_WIN
 #define INCL_GPILOGCOLORTABLE
 #define INCL_GPI
 #define INCL_ERRORS
 #include "skin.h"

 #include <stdio.h>
 #include <io.h>
 #include <malloc.h>
 #include <string.h>
 #include <ctype.h>

/*---[ Definicoes ]---------------------------------------------------------------*/


/*---[ Prototipos ]---------------------------------------------------------------*/

static const char *loadValues(const char *, LONG *, int);
static void loadButton(HWND, HAB, HDC, HPS, char *, SHORT, SHORT, SHORT, char *);
static void processGeneric(SKINCONFIG *, UCHAR, const char *, LONG *,const SKINELEMENT *);
static void loadSlider(HWND, HAB, HDC, HPS, short, short, short, short, char *, char *);
static void loadListBox(HWND, HAB, HDC, HPS, short, short, short, short, char *, char *);
static BOOL loadImages(HWND, HAB, HDC, HPS, int, char *, char *);
static int  load(HWND, HSKIN, const SKINELEMENT *, const char *);

/*---[ Constantes ]---------------------------------------------------------------*/

/*---[ Implementacao ]------------------------------------------------------------*/

int EXPENTRY sknLoad(HWND hwnd, HSKIN cfg, const SKINELEMENT *elements, const char *parmName)
{
   int          rc    = -1;
   SWP          swp;
   HPOINTER     ptr   = WinQueryPointer(HWND_DESKTOP);
   HWND         frame = WinQueryWindow(hwnd,QW_PARENT);

   WinSetPointer(HWND_DESKTOP,WinQuerySysPointer(HWND_DESKTOP,SPTR_WAIT,FALSE));

   WinQueryWindowPos(frame,&swp);

   DBGTrace(swp.cx);
   DBGTrace(swp.cy);
   WinSetWindowPos(frame,0,swp.x,swp.y,0,0,SWP_SIZE);

   WinEnableWindowUpdate(hwnd, FALSE);

   rc = load(hwnd,cfg,elements,parmName);

   DBGTrace(swp.cx);
   DBGTrace(swp.cy);

   if(!cfg->maxSize.x)
      cfg->maxSize.x = 0xFFFF;

   if(!cfg->maxSize.y)
      cfg->maxSize.y = 0xFFFF;

   if(swp.cx > cfg->maxSize.x)
      swp.cx = cfg->maxSize.x;

   if(swp.cy > cfg->maxSize.y)
      swp.cy = cfg->maxSize.y;

   DBGTrace(swp.cx);
   DBGTrace(swp.cy);

   if(swp.cx < cfg->minSize.x)
      swp.cx = cfg->minSize.x;

   if(swp.cy < cfg->minSize.y)
      swp.cy = cfg->minSize.y;

   WinEnableWindowUpdate(hwnd, TRUE);

   DBGTrace(cfg->minSize.x);
   DBGTrace(cfg->minSize.y);
   DBGTrace(cfg->maxSize.x);
   DBGTrace(cfg->maxSize.y);

   DBGTrace(swp.cx);
   DBGTrace(swp.cy);

   WinSetWindowPos(frame,0,swp.x,swp.y,swp.cx,swp.cy,SWP_SIZE);
   WinInvalidateRect(frame, 0, TRUE);

   WinSetPointer(HWND_DESKTOP,ptr);

   return rc;
}

static int load(HWND hwnd, HSKIN cfg, const SKINELEMENT *elements, const char *parmName)
{
   char                 parms[0x0100];
   char                 linha[0x0100];
   HWND                 h;
   LONG                 vlr[5];
   ULONG                *clrPtr;
   ULONG                temp;
   FILE                 *arq;
   char                 *fileName       = parms;
   const SKINELEMENT    *cmd;
   char                 *ptr;
   HDC                  hdc;
   HPS                  hps             = 0L;
   HBITMAP              b;
   HAB                  hab             = WinQueryAnchorBlock(hwnd);
   SIZEL                sizl            = { 0, 0 };  /* use same page size as device */
   DEVOPENSTRUC         dop             = {0L, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};
   int                  rc              = 0;

   if(parmName)
   {
      strncpy(parms,parmName,0x0FF);
   }
   else
   {
      strcpy(parms,".\\skins\\default.skn");
   }

   for(ptr = fileName = parms; *ptr;ptr++)
   {
      if(*ptr == '.')
         fileName = ptr;
   }

   /* Reinicializa controles */
   cfg->maxSize.x = cfg->maxSize.y = 0xFFFF;
   cfg->minSize.x = cfg->minSize.y = 0;

   /* Destroi bitmaps */
   destroyImages(cfg);

   /* Inicializa pallete default */

   cfg->color[SKC_BACKGROUND]                   = 0x00D1D1D1;
   cfg->color[SKC_FOREGROUND]                   = 0x00FFFFFF;
   cfg->color[SKC_SELECTEDBACKGROUND]           = 0x000000FF;
   cfg->color[SKC_SELECTEDFOREGROUND]           = 0x00FFFFFF;
   cfg->color[SKC_BUTTONTEXT]                   = 0x00FFFF00;
   cfg->color[SKC_MENUBACKGROUND]               = 0x00D1D1D1;
   cfg->color[SKC_MENUFOREGROUND]               = 0x00000080;
   cfg->color[SKC_MENUBORDER]                   = 0x00000000;
   cfg->color[SKC_MENUSELECTEDBACKGROUND]       = 0x00000080;
   cfg->color[SKC_MENUSELECTEDFOREGROUND]       = 0xFFFFFFFF;
   cfg->color[SKC_USER1]                        = 0x00FF0000;
   cfg->color[SKC_USER2]                        = 0x0000FF00;
   cfg->color[SKC_USER3]                        = 0x000000FF;
   cfg->color[SKC_USER4]                        = 0x00800000;
   cfg->color[SKC_USER5]                        = 0x00008000;
   cfg->color[SKC_USER6]                        = 0x00000080;



   /* Carrega arquivo de configuracao */

//   CHKPoint();

   hdc  = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);
   hps  = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);

   if(!hps)
   {
      DBGMessage("Erro ao criar o HPS");
      DevCloseDC(hdc);
      return ERROR_INVALID_HANDLE;
   }

//   DBGMessage(parms);

   arq = fopen(parms,"r");

   if(!arq)
   {
      DBGMessage("Nao conseguiu abrir o arquivo de skins");
      rc = -1;
   }
   else
   {
      notifyClients(hwnd,elements,WMSKN_LOADMODE,MPFROMLONG(TRUE),0);

      *(fileName++) = '\\';
//      DBGMessage(parms);

      while(!feof(arq))
      {
         *linha = 0;
         fgets(linha, 0xFF, arq);

         for(ptr=linha;*ptr && *ptr >= ' ';ptr++);
         *ptr = 0;

         for(ptr=linha;*ptr && isspace( ((int) *ptr) );ptr++);

//         DBGMessage(ptr);
         cmd = sknSearchElement(elements,ptr);
//         DBGTracex(cmd);

         if(cmd)
         {
//            DBGTrace(cmd->type);
//            DBGMessage(cmd->keyword);

            while(*ptr && *ptr != '=')
               ptr++;

            if(*ptr)
               ptr++;

//          CHKPoint();

            while(*ptr && isspace( ((int) *ptr)) )
               ptr++;

//          CHKPoint();
//          DBGMessage(parms);
//          DBGMessage(ptr);
//            GpiSelectPalette(hps,lPal);

            *fileName  = 0;
            strncat(parms,ptr,0xFF);

//          DBGMessage(parms);

            switch(cmd->type)
            {
            case CMD_GENERIC:   /* Comando generico */
               processGeneric(cfg,cmd->pos,ptr,vlr,elements);
               break;

            case CMD_BITMAP:    /* Carregar um bitmap */
               if(cfg->b[cmd->pos])
                  GpiDeleteBitmap(cfg->b[cmd->pos]);
               cfg->b[cmd->pos] = LoadBitmap(hab,hdc,hps,parms);
//               DBGTracex(cfg->b[cmd->pos]);
               break;

            case CMD_PALLETE:   // Ajusta pallete
               loadValues(ptr, vlr, 3);
               clrPtr = (cfg->color+cmd->pos);

               *clrPtr = (vlr[0] & 0xFF);
               *clrPtr <<= 8;
               *clrPtr |= (vlr[1] & 0xFF);
               *clrPtr <<= 8;
               *clrPtr |= (vlr[2] & 0xFF);

//               DBGTrace(cmd->pos);

               if(cmd->pos == SKC_MENUBACKGROUND)
               {
                  DBGMessage("**********************************************");
                  DBGTrace(*clrPtr);
               }

               break;


            case CMD_ICONBAR:
               loadTransparent(hab,hdc,hps,cfg->iconbar + cmd->pos,parms);
               break;

            case CMD_XBITMAP:   /* Carrega posicao e imagem */
               ptr = (char *) loadValues(ptr, vlr, 2);
               (cfg->pxbm +cmd->pos)->x    = vlr[0];
               (cfg->pxbm +cmd->pos)->y    = vlr[1];

               *fileName  = 0;
               strncat(parms,ptr,0xFF);

               loadTransparent(hab,hdc,hps,cfg->xbm + cmd->pos,parms);

               break;

            case CMD_BUTTON:    /* Carrega configuracao de botao */
               h   = WinWindowFromID(hwnd,cmd->pos);

               if(h)
               {
                  ptr       = (char *) loadValues(ptr, vlr, 3);
                  *fileName = 0;
                  loadButton(h,hab,hdc,hps,ptr,vlr[0],vlr[1],vlr[2],parms);
               }
               break;

            case CMD_SLIDER:    /* Slider x,y,cx,cy,bitmap1,bitmap2 */
               CHKPoint();
               ptr = (char *) loadValues(ptr, vlr, 4);
               CHKPoint();
               *fileName = 0;
               loadSlider(WinWindowFromID(hwnd,cmd->pos),hab,hdc,hps, vlr[0], vlr[1], vlr[2], vlr[3], ptr, parms);
               break;

            case CMD_LISTBOX:    /* Listbox x,y,cx,cy,bitmap1 */
               ptr = (char *) loadValues(ptr, vlr, 4);
//               DBGTrace(vlr[0]);
//               DBGTrace(vlr[1]);
//               DBGTrace(vlr[2]);
//               DBGTrace(vlr[3]);
//               DBGMessage(ptr);
               *fileName = 0;
               loadListBox(WinWindowFromID(hwnd,cmd->pos),hab,hdc,hps, vlr[0], vlr[1], vlr[2], vlr[3], ptr, parms);
               break;

            case CMD_FONT:
//               DBGMessage(ptr);
               WinSetPresParam(  WinWindowFromID(hwnd,cmd->pos),
                                 PP_FONTNAMESIZE,
                                 (ULONG) strlen(ptr)+1,
                                 (PVOID) ptr);
               break;

            case CMD_MENUFONT:
               if(cfg->menuFont)
                  free(cfg->menuFont);
               cfg->menuFont = strdup(ptr);
               break;

            }
         }
      }

      fclose(arq);

      notifyClients(hwnd,elements,WMSKN_LOADMODE,MPFROMLONG(FALSE),0);
   }

//   CHKPoint();

   GpiDestroyPS(hps);
   DevCloseDC(hdc);
//   CHKPoint();

   return rc;

}

static const char *loadValues(const char *buffer, LONG *vlr, int qtd)
{
   char neg     = 0;

   *vlr = 0;
   while(*buffer && qtd)
   {
      switch(*buffer)
      {
      case '-':
         *vlr = 0;
         neg  = 0xFF;
         break;

      case '$':
         *vlr = POS_CENTRALIZE;
         break;

      case ',':
         if(neg)
            *vlr = -*vlr;
         neg = 0;
         vlr++;
         qtd--;
         if(qtd)
            *vlr = 0;
         break;

      default:
         if(isdigit(*buffer))
         {
            if(*vlr == 0xFFF)
               *vlr = 0;
            else
               *vlr *= 10;
            *vlr += *buffer - '0';
         }
      }
      buffer++;
   }
   if(neg)
      *vlr = -*vlr;

//   DBGMessage(buffer);
   return buffer;
}

static BOOL loadImages(HWND hwnd, HAB hab, HDC hdc, HPS hps, int qtd, char *ptr, char *path)
{
   char         **name  = malloc( qtd * sizeof(char *) );
   int          f       = 0;
   XBITMAP      btm;
   BOOL         ret     = FALSE;
   char         *pos    = path+strlen(path);

   if(!name)
      return FALSE;

   memset(name,0,qtd*sizeof(char*));

   for(name[f] = ptr; *ptr && f < qtd; ptr++)
   {
      if(*ptr == ',')
      {
         *ptr = 0;
         f++;
         *(name+f) = (ptr+1);
      }
   }

//   CHKPoint();

   *pos = 0;
   strncat(path,name[0],0xFF);
//   DBGMessage(path);

   if(name[0] || !access(path,0))
   {
      for(f=0;f<qtd;f++)
      {
         memset(&btm,0,sizeof(XBITMAP));

         if(name[f])
         {
            *pos = 0;
            strncat(path,name[f],0xFF);
//            DBGMessage(path);

            if(!access(path,0))
               loadTransparent(hab, hdc, hps, &btm, path);
         }
         WinSendMsg(hwnd,WMSKN_SETIMAGE,MPFROMP(&btm),MPFROMSHORT(f));
      }
      ret = TRUE;
   }

//   CHKPoint();

   free(name);

   return ret;
}

static void loadButton(HWND hwnd,HAB hab, HDC hdc, HPS hps,char *type,SHORT s0, SHORT s1, SHORT s2, char *path)
{
   static const char    *flags  = "R";
   const char           *fPtr;
   ULONG                masc    = 0;
   ULONG                temp;
   char                 *ptr;

   for(ptr=type;*ptr && *ptr != ',';ptr++);

   if(!*ptr)
      return;

   *(ptr++) = 0;

   while(*type)
   {
      temp = 1;
      for(fPtr = flags;*fPtr && *fPtr != *type;fPtr++)
         temp <<= 1;
      if(*type)
         masc |= temp;
      type++;
   }

   WinSendMsg(hwnd,WMSKN_SETBUTTONFLAGS,MPFROMLONG(masc),MPFROMSHORT(s2));

   if(!loadImages(hwnd, hab, hdc, hps, 3, ptr, path))
      WinShowWindow(hwnd,FALSE);

   WinSendMsg(hwnd,WMSKN_SAVEPOS,MPFROM2SHORT(s0,s1),0);


}

static void processGeneric(SKINCONFIG *cfg, UCHAR cmd, const char *parm, LONG *vlr,const SKINELEMENT *elements)
{
   const SKINELEMENT *el        = NULL;

   switch(cmd)
   {
   case 0:      /* Tamanho da janela */
       DBGMessage(parm);
       loadValues(parm, vlr, 2);
       break;

   case 1:      /* Tamanho minimo */
      loadValues(parm, vlr, 2);
      cfg->minSize.x = vlr[0];
      cfg->minSize.y = vlr[1];
      break;

   case 2:      /* Tamanho maximo */
      loadValues(parm, vlr, 2);
      cfg->maxSize.x = vlr[0];
      cfg->maxSize.y = vlr[1];
      break;

   }
}

const SKINELEMENT * EXPENTRY sknSearchElement(const SKINELEMENT *elements, const char *ptr)
{
   const SKINELEMENT *cmd;
   for(cmd=elements;cmd->type && cmd->keyword && strnicmp(cmd->keyword,ptr,strlen(cmd->keyword));cmd++);
//   DBGTrace(cmd->type);
   return cmd->type ? cmd : NULL;
}


static void loadSlider(HWND hwnd,HAB hab, HDC hdc, HPS hps, short x, short y, short cx, short cy, char *ptr, char *path)
{
   DBGTracex(hwnd);

   if(!hwnd)
      return;

   WinSendMsg(hwnd,WMSKN_AJUSTSIZEPOS,MPFROM2SHORT(x,y),MPFROM2SHORT(cx,cy));

   if(!loadImages(hwnd, hab, hdc, hps, BITMAPS_SLIDER, ptr, path))
      WinShowWindow(hwnd,FALSE);

}

static void loadListBox(HWND hwnd,HAB hab, HDC hdc, HPS hps, short x, short y, short cx, short cy, char *ptr, char *path)
{
//   DBGTracex(hwnd);
//   DBGTrace(cx);
//   DBGTrace(cy);

   if(!hwnd)
      return;

   WinSendMsg(hwnd,WMSKN_AJUSTSIZEPOS,MPFROM2SHORT(x,y),MPFROM2SHORT(cx,cy));

}

