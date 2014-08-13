/*
 * LOADBTM.C - Funcao para carga de uma imagem
 */

#define INCL_DOSMEMMGR
#define INCL_GPI
#define INCL_GPILOGCOLORTABLE
#include "skin.h"

#include <mmioos2.h>

#include <string.h>
#include <malloc.h>

#pragma library("mmpm2.lib")

#define BITCOUNT 24

/* #define WRITE_LOG stdout */

/*---[ Prototipos ]---------------------------------------------------------------*/

static BOOL isMasc(RGB *);

/*---[ Implementacao ]------------------------------------------------------------*/

void loadTransparent( HAB hab, HDC hdc, HPS hps, XBITMAP *x, PSZ FileName)
{
   BITMAPINFOHEADER2    bmih;
   HBITMAP              base            = LoadBitmap(hab,hdc,hps,FileName);
   BITMAPINFOHEADER     pbmpData;       /* bit-map information header           */
   POINTL               aptlPoints[4];

//   DBGMessage(FileName);

   if(x->masc)
      GpiDeleteBitmap(x->masc);

   if(x->image)
      GpiDeleteBitmap(x->image);

   memset(x,0,sizeof(XBITMAP));

   GpiQueryBitmapParameters(base, &pbmpData);

   memset(&bmih,0, sizeof(BITMAPINFOHEADER2));
   bmih.cbFix        = sizeof(bmih);
   bmih.cx           = pbmpData.cx;
   bmih.cy           = pbmpData.cy;
   bmih.cPlanes      = 1;
   bmih.cBitCount    = 1;

   x->masc        = GpiCreateBitmap(   hps,
                                       &bmih,
                                       0L,
                                       NULL,
                                       NULL);

   GpiSetBitmap(hps,x->masc);

   aptlPoints[0].x=0;
   aptlPoints[0].y=0;
   aptlPoints[1].x=aptlPoints[0].x+pbmpData.cx-1;
   aptlPoints[1].y=aptlPoints[0].y+pbmpData.cy-1;
   aptlPoints[2].x=0;
   aptlPoints[2].y=0;
   aptlPoints[3].x=aptlPoints[2].x+pbmpData.cx;
   aptlPoints[3].y=aptlPoints[2].y+pbmpData.cy;

   GpiSetColor(hps,CLR_WHITE);
   GpiSetBackColor(hps,CLR_PINK);

   GpiWCBitBlt( hps,
                base,
                4,
                aptlPoints,
                ROP_NOTSRCCOPY,
                BBO_IGNORE);



   bmih.cbFix        = sizeof(bmih);
   bmih.cx           = pbmpData.cx;
   bmih.cy           = pbmpData.cy;
   bmih.cPlanes      = 1;
   bmih.cBitCount    = 24;

   x->image        = GpiCreateBitmap(  hps,
                                       &bmih,
                                       0L,
                                       NULL,
                                       NULL);

   GpiSetBitmap(hps,x->image);
   GpiSetColor(hps,CLR_WHITE);
   GpiSetBackColor(hps,CLR_BLACK);

   GpiWCBitBlt( hps,
                x->masc,
                4,
                aptlPoints,
                ROP_NOTSRCCOPY,
                BBO_IGNORE);

   GpiWCBitBlt( hps,
                base,
                4,
                aptlPoints,
                ROP_SRCAND,
                BBO_IGNORE);

   GpiSetBitmap(hps,NULLHANDLE);
   GpiDeleteBitmap(base);

}

HBITMAP EXPENTRY sknLoadBitmap( HWND hwnd, PSZ pszFileName )
{
   HBITMAP         ret             = 0;

   SIZEL           sizl            = { 0, 0 };  /* use same page size as device */
   DEVOPENSTRUC    dop             = {0L, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};
   HAB             hab             = WinQueryAnchorBlock(hwnd);
   HDC             hdc;
   HPS             hps;

   hdc  = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);
   hps  = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);

   if(hps)
   {
      ret = LoadBitmap(hab,hdc,hps,pszFileName);
      GpiDestroyPS(hps);
   }

   DevCloseDC(hdc);

   return ret;
}

HBITMAP LoadBitmap ( HAB hab,
                     HDC hdc,
                     HPS hps,
                     PSZ pszFileName )
{
    HBITMAP       hbm;
    MMIOINFO      mmioinfo;
    MMFORMATINFO  mmFormatInfo;
    HMMIO         hmmio;
    ULONG         ulImageHeaderLength;
    MMIMAGEHEADER mmImgHdr;
    ULONG         ulBytesRead;
    ULONG         dwNumRowBytes;
    PBYTE         pRowBuffer;
    ULONG         dwRowCount;
    SIZEL         ImageSize;
    ULONG         dwHeight, dwWidth;
    SHORT         wBitCount;
    FOURCC        fccStorageSystem;
    ULONG         dwPadBytes;
    ULONG         dwRowBits;
    ULONG         ulReturnCode;
    ULONG         dwReturnCode;
    HBITMAP       hbReturnCode;
    LONG          lReturnCode;
    FOURCC        fccIOProc;

//    DBGMessage(pszFileName);

    ulReturnCode = mmioIdentifyFile ( pszFileName,
                                      0L,
                                      &mmFormatInfo,
                                      &fccStorageSystem,
                                      0L,
                                      0L);

    /*
     *  If this file was NOT identified, then this function won't
     *  work, so return an error by indicating an empty bitmap.
     */

    if ( ulReturnCode == MMIO_ERROR )
    {
         DBGMessage("Erro em mmioIdentifyFile");
         return (0L);
    }

    /*
     *  If mmioIdentifyFile did not find a custom-written IO proc which
     *  can understand the image file, then it will return the DOS IO Proc
     *  info because the image file IS a DOS file.
     */

    if( mmFormatInfo.fccIOProc == FOURCC_DOS )
    {

         WinMessageBox( HWND_DESKTOP,
                        HWND_DESKTOP,
                        "Image file could not be interpreted by any permanently or temporarily installed IO procedures.",
                        pszFileName,
                        (HMODULE) NULL,
                        (ULONG) MB_OK | MB_MOVEABLE |
                                MB_ERROR );
         return ( 0L );
    }

    /*
     *  Ensure this is an IMAGE IOproc, and that it can read
     *  translated data
     */

    if ( (mmFormatInfo.ulMediaType != MMIO_MEDIATYPE_IMAGE) ||
         ((mmFormatInfo.ulFlags & MMIO_CANREADTRANSLATED) == 0) )
    {

         WinMessageBox( HWND_DESKTOP,
                        HWND_DESKTOP,
                        "No IMAGE IO Procedures exist which can translate the data in the image file specified.",
                        pszFileName,
                        (HMODULE) NULL,
                        (ULONG) MB_OK | MB_MOVEABLE |
                                MB_ERROR );
         return (0L);
    }
    else
    {
         fccIOProc = mmFormatInfo.fccIOProc;
    }

    /* Clear out and initialize mminfo structure */

    memset ( &mmioinfo,
             0L,
             sizeof ( MMIOINFO ) );

    mmioinfo.fccIOProc = fccIOProc;
    mmioinfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;

    hmmio = mmioOpen ( (PSZ) pszFileName,
                       &mmioinfo,
                       MMIO_READ | MMIO_DENYWRITE | MMIO_NOIDENTIFY );

    if ( ! hmmio )
    {
         /* If file could not be opened, return with error */
         DBGMessage("Erro em mmioOpen");
         return (0L);
    }

    dwReturnCode = mmioQueryHeaderLength ( hmmio,
                                         (PLONG)&ulImageHeaderLength,
                                           0L,
                                           0L);

    if ( ulImageHeaderLength != sizeof ( MMIMAGEHEADER ) )
    {
         /* We have a problem.....possibly incompatible versions */

         ulReturnCode = mmioClose (hmmio, 0L);

         return (0L);
    }

    ulReturnCode = mmioGetHeader ( hmmio,
                                   &mmImgHdr,
                                   (LONG) sizeof ( MMIMAGEHEADER ),
                                   (PLONG)&ulBytesRead,
                                   0L,
                                   0L);

    if ( ulReturnCode != MMIO_SUCCESS )
    {
         /* Header unavailable */
         DBGMessage("Erro em mmioGetHeader");
         ulReturnCode = mmioClose (hmmio, 0L);

         return (0L);
    }

    /*
     *  Determine the number of bytes required, per row.
     *      PLANES MUST ALWAYS BE = 1
     */

    dwHeight = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cy;
    dwWidth = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cx;
    wBitCount = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount;
    dwRowBits = dwWidth * mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount;
    dwNumRowBytes = dwRowBits >> 3;

    /*
     *  Account for odd bits used in 1bpp or 4bpp images that are
     *  NOT on byte boundaries.
     */

    if ( dwRowBits % 8 )
    {
         dwNumRowBytes++;
    }

    /*
     *  Ensure the row length in bytes accounts for byte padding.
     *  All bitmap data rows must are aligned on LONG/4-BYTE boundaries.
     *  The data FROM an IOProc should always appear in this form.
     */

    dwPadBytes = ( dwNumRowBytes % 4 );

    if ( dwPadBytes )
    {
         dwNumRowBytes += 4 - dwPadBytes;
    }

    /* Allocate space for ONE row of pels */

    if ( DosAllocMem( (PPVOID)&pRowBuffer,
                      (ULONG)dwNumRowBytes,
                      fALLOC))
    {
         ulReturnCode = mmioClose (hmmio, 0L);
         DBGMessage("Erro de alocacao de memoria");
         return(0L);
    }

    /* ***************************************************
       Create a memory presentation space that includes
       the memory device context obtained above.
       ***************************************************/

    ImageSize.cx = dwWidth;
    ImageSize.cy = dwHeight;

    /* ***************************************************
       Create an uninitialized bitmap.  This is where we
       will put all of the bits once we read them in.
       ***************************************************/

    hbm = GpiCreateBitmap ( hps,
                            &mmImgHdr.mmXDIBHeader.BMPInfoHeader2,
                            0L,
                            NULL,
                            NULL);

    if ( !hbm )
    {
         ulReturnCode = mmioClose (hmmio, 0L);
         DBGMessage("Erro ao criar bitmap");

         return(0L);
    }

    /* ***************************************************
       Select the bitmap into the memory device context.
       *************************************************** */

    hbReturnCode = GpiSetBitmap ( hps,
                                  hbm );

    /****************************************************************
        LOAD THE BITMAP DATA FROM THE FILE
            One line at a time, starting from the BOTTOM
     *************************************************************** */

    for ( dwRowCount = 0; dwRowCount < dwHeight; dwRowCount++ )
    {
         ulBytesRead = (ULONG) mmioRead ( hmmio,
                                          pRowBuffer,
                                          dwNumRowBytes );

         if ( !ulBytesRead )
         {
              break;
         }

         /*
          *  Allow context switching while previewing.. Couldn't get
          *  it to work. Perhaps will get to it when time is available...
          */

         lReturnCode = GpiSetBitmapBits ( hps,
                                          (LONG) dwRowCount,
                                          (LONG) 1,
                                          (PBYTE) pRowBuffer,
                                          (PBITMAPINFO2) &mmImgHdr.mmXDIBHeader.BMPInfoHeader2);

    }

    ulReturnCode = mmioClose (hmmio, 0L);

    DosFreeMem(pRowBuffer);

//    DBGTracex(hbm);

    return(hbm);
}

USHORT EXPENTRY sknDrawIcon(HSKIN skn, HPS hps, USHORT id, USHORT x, USHORT y)
{
   return drawIcon(hps,skn->iconbar,id,x,y);
}

USHORT EXPENTRY sknQueryIconSize(HSKIN skn)
{
   BITMAPINFOHEADER     bmpData;
   GpiQueryBitmapParameters(skn->iconbar->masc, &bmpData);
   return bmpData.cy;
}

USHORT drawIcon(HPS hps, XBITMAP *b, USHORT id, USHORT x, USHORT y)
{
   BITMAPINFOHEADER     bmpData;
   POINTL               aptl[4];
   USHORT               fator;

   if(!b->masc)
      return 0;

   bmpData.cbFix = sizeof(bmpData);
   GpiQueryBitmapParameters(b->masc, &bmpData);

   aptl[0].x = x;
   aptl[0].y = y;
   aptl[1].x = x+bmpData.cy-1;
   aptl[1].y = y+bmpData.cy-1;

   fator = bmpData.cy * id;

   aptl[2].x = fator;
   aptl[2].y = 0;
   aptl[3].x = fator + bmpData.cy;
   aptl[3].y = bmpData.cy;

   GpiSetColor(hps,CLR_WHITE);
   GpiSetBackColor(hps,CLR_BLACK);

   GpiWCBitBlt( hps,
                b->masc,
                4,
                aptl,
                ROP_SRCAND,
                BBO_IGNORE);

   GpiWCBitBlt( hps,
                b->image,
                4,
                aptl,
                ROP_SRCPAINT,
                BBO_IGNORE);

   return bmpData.cy;

}

void drawTransparent(HPS hps, PPOINTL p, XBITMAP *b)
{
   BITMAPINFOHEADER     bmpData;
   POINTL               aptl[4];

   if(!b->masc)
      return;

   bmpData.cbFix = sizeof(bmpData);
   GpiQueryBitmapParameters(b->masc, &bmpData);

   GpiSetColor(hps,CLR_WHITE);
   GpiSetBackColor(hps,CLR_BLACK);

   // Target - Inclusive
   aptl[0].x = p->x;
   aptl[0].y = p->y;
   aptl[1].x = p->x+bmpData.cx;
   aptl[1].y = p->y+bmpData.cy;

   // Source - Non inclusive
   aptl[2].x = 0;
   aptl[2].y = 0;
   aptl[3].x = bmpData.cx+1;
   aptl[3].y = bmpData.cy+1;

   GpiWCBitBlt( hps,
                b->masc,
                4,
                aptl,
                ROP_SRCAND,
                BBO_IGNORE);

   GpiWCBitBlt( hps,
                b->image,
                4,
                aptl,
                ROP_SRCPAINT,
                BBO_IGNORE);
/*
   GpiWCBitBlt( hps,
                b->image,
                4,
                aptl,
                ROP_SRCCOPY,
                BBO_IGNORE);
*/
}

BOOL validXBitmap(XBITMAP *btm)
{
   return btm->masc != NULLHANDLE;
}

static BOOL isMasc(RGB *r)
{
   return r->bRed == 255 && r->bGreen == 0 && r->bBlue == 255;
}

