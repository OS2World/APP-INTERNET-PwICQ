/*
 * os2bitmap.c - Load Image file
 */

 #pragma strings(readonly)

 #define INCL_DOSMEMMGR
 #define INCL_GPI
 #define INCL_GPILOGCOLORTABLE
 #define INCL_WIN

 #include <string.h>
 #include <io.h>
 #include <os2.h>
 #include <mmioos2.h>
 #include <pwicqgui.h>

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/

#ifdef USE_MMPM
 static HBITMAP LoadBitmap( HAB, HDC, HPS, PSZ );
#endif

 static void erase(HWND, HPS, PRECTL);
 static void paint(HWND);

/*---[ Constants ]----------------------------------------------------------------------------------------*/

 const char *icqBitmapWindowClass = "icqBitmap";

/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 HBITMAP icqskin_loadBitmap(HICQ icq, const char *fileName)
 {
#ifndef USE_MMPM
    DBGMessage("*** NO_MMPM");
    return NO_IMAGE;

#else

    SIZEL              sizl            = { 0, 0 };  /* use same page size as device */
    DEVOPENSTRUC       dop             = { 0, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};
    HAB                hab             = icqQueryAnchorBlock(icq);
    HDC                hdc;
    HPS                hps;
    HBITMAP            ret             = NO_IMAGE;
    char               buffer[0x0100];

    if(icqskin_queryImage(icq,fileName,0xFF,buffer))
       return ret;

    hdc  = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);
    hps  = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);

    if(hps)
    {
       DBGMessage(buffer);
       ret = LoadBitmap(hab,hdc,hps,buffer);
       GpiDestroyPS(hps);
    }

    DevCloseDC(hdc);

    return ret;

#endif
 }

 int icqskin_loadImageFromResource(HICQ icq, USHORT id, USHORT cx, USHORT cy, HBITMAP *img, HBITMAP *msk)
 {
    char               buffer[0x0100];
    SIZEL              sizl            = { 0, 0 };  /* use same page size as device */
    DEVOPENSTRUC       dop             = { 0, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};
    HAB                hab             = icqQueryAnchorBlock(icq);
    HDC                hdc;
    HPS                hps;
    HBITMAP            src;
    int                rc;

    CHKPoint();

    icqskin_deleteImage( *img );
    icqskin_deleteImage( *msk );

    WinLoadString(hab, module, id, 0xFF, buffer);

    DBGMessage(buffer);
    rc = icqskin_loadImage(icq, buffer, img, msk);

    if(*img != NO_IMAGE)
       return rc;

    DBGMessage("File not found, loading default image");

    hdc  = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);
    hps  = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);

    if(hps)
    {
       src = GpiLoadBitmap(hps, module, id, cx, cy);
       if(src != GPI_ERROR)
       {
          icqskin_MakeTransparent(hps,src,img,msk);
          GpiDeleteBitmap(src);
       }
       GpiDestroyPS(hps);
    }

    DevCloseDC(hdc);

    return cy;
 }


 int icqskin_loadImage(HICQ icq, const char *fileName, HBITMAP *img, HBITMAP *msk)
 {
#ifndef USE_MMPM

    return 0;

#else

    char               buffer[0x0100];
    HBITMAP            src             = NO_IMAGE;
    SIZEL              sizl            = { 0, 0 };  /* use same page size as device */
    DEVOPENSTRUC       dop             = { 0, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};
    HAB                hab             = icqQueryAnchorBlock(icq);
    int			       rc              = -1;
    HDC                hdc;
    HPS                hps;

    /* Clean old images */

    icqskin_deleteImage( *img );
    icqskin_deleteImage( *msk );

    if(!fileName)
       return -1;

    if(icqskin_queryImage(icq,fileName,0xFF,buffer))
       return rc;

    DBGMessage(buffer);

    /* And load the new one */

    hdc  = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);
    hps  = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);

    if(hps)
    {
       src = LoadBitmap(hab,hdc,hps,buffer);

       if(src != NO_IMAGE)
       {
          rc = icqskin_MakeTransparent(hps,src,img,msk);
          GpiDeleteBitmap(src);
      }
      GpiDestroyPS(hps);

    }

    DevCloseDC(hdc);

    return rc;

#endif

 }

#ifdef MAKELIB
 int EXPENTRY icqMakeBitmapTransparent(HPS hps, HBITMAP src, HBITMAP *img, HBITMAP *msk)
 {
    return icqskin_MakeTransparent(hps, src, img, msk);
 }
#endif

 int icqskin_MakeTransparent(HPS hps, HBITMAP src, HBITMAP *img, HBITMAP *msk)
 {
    int                rc               = 0;
    long			   pink             = 0x00FF00FF;
    BITMAPINFOHEADER   bmpData;
    BITMAPINFOHEADER2  bmih;
    POINTL             aptlPoints[4];

    if(src == NO_IMAGE)
    {
       *img = *msk = NO_IMAGE;
       return -1;
    }

    GpiQueryBitmapParameters(src, &bmpData);
    rc = bmpData.cy;

    /* Create masking image */

    memset(&bmih,0, sizeof(BITMAPINFOHEADER2));
    bmih.cbFix        = sizeof(bmih);
    bmih.cx           = bmpData.cx;
    bmih.cy           = bmpData.cy;
    bmih.cPlanes      = 1;
    bmih.cBitCount    = 1;

    *msk = GpiCreateBitmap(hps, &bmih, 0L, NULL, NULL);

    GpiSetBitmap(hps,*msk);

    aptlPoints[0].x=0;
    aptlPoints[0].y=0;
    aptlPoints[1].x=aptlPoints[0].x+bmpData.cx-1;
    aptlPoints[1].y=aptlPoints[0].y+bmpData.cy-1;
    aptlPoints[2].x=0;
    aptlPoints[2].y=0;
    aptlPoints[3].x=aptlPoints[2].x+bmpData.cx;
    aptlPoints[3].y=aptlPoints[2].y+bmpData.cy;

/*
#ifdef DEBUG
    GpiQueryLogColorTable(hps, 0L, CLR_PINK, 1, &pink);
    DBGTracex(pink);
    if(pink != 0x00FF00FF)
       DBGMessage("******** UNEXPECTED PINK VALUE");
    pink = 0x00FF00FF;
#endif
*/

    GpiCreateLogColorTable(hps,0L,LCOLF_CONSECRGB,CLR_PINK,1,&pink);

    GpiSetColor(hps,CLR_WHITE);
    GpiSetBackColor(hps,CLR_PINK);
    GpiWCBitBlt(hps,src,4,aptlPoints,ROP_NOTSRCCOPY,BBO_IGNORE);

    /* Create base image */
    *img  = GpiCreateBitmap(  hps, &bmih, 0L, NULL, NULL);

    bmih.cbFix        = sizeof(bmih);
    bmih.cx           = bmpData.cx;
    bmih.cy           = bmpData.cy;
    bmih.cPlanes      = 1;
    bmih.cBitCount    = 24;

    *img  = GpiCreateBitmap(  hps, &bmih, 0L, NULL, NULL);

    GpiSetBitmap(hps,*img);
    GpiSetColor(hps,CLR_WHITE);
    GpiSetBackColor(hps,CLR_BLACK);

    GpiWCBitBlt( hps,*msk,4,aptlPoints,ROP_NOTSRCCOPY,BBO_IGNORE);
    GpiWCBitBlt( hps,src,4,aptlPoints,ROP_SRCAND,BBO_IGNORE);

    GpiSetBitmap(hps,NULLHANDLE);

    return rc;
 }

 HPOINTER icqskin_CreatePointer(SKINDATA *skn, USHORT id)
 {
    SIZEL              sizl            = { 0, 0 };  /* use same page size as device */
    DEVOPENSTRUC       dop             = { 0, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};
    HPOINTER           ret			   = NULLHANDLE;
    HAB                hab             = icqQueryAnchorBlock(skn->icq);
    HDC                hdc;
    HPS                hps;
    HBITMAP            bmp;
    BITMAPINFOHEADER2  bmih;
    RECTL              rcl;
    POINTL			   p;
    USHORT			   fator;

    hdc  = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);
    hps  = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);

    if(hps)
    {
       bmih.cbFix        = sizeof(bmih);
       bmih.cx           = skn->iconSize;
       bmih.cy           = skn->iconSize*2;
       bmih.cPlanes      = 1;
       bmih.cBitCount    = 24;
       bmp               = GpiCreateBitmap(  hps, &bmih, 0L, NULL, NULL);

       GpiSetBitmap(hps,bmp);

/*
       Bit-map handle from which the pointer image is created.

       The bit map must be logically divided into two sections vertically,
       each half representing one of the two images used as the successive
       drawing masks for the pointer.

       For an icon, there are two bit map images.
       The first half is used for the AND mask and the second half is used
       for the XOR mask. For details of bit map formats, see
*/

       rcl.xLeft   =
       rcl.yBottom = 0;

       rcl.yTop    = skn->iconSize*2;
       rcl.xRight  = skn->iconSize;

       WinFillRect(hps, &rcl, CLR_BLACK);

       fator       = skn->iconSize * id;
       rcl.xLeft   = fator;
       rcl.yBottom = 0;
       rcl.yTop    = skn->iconSize;
       rcl.xRight  = fator+skn->iconSize;

       p.x = 0;
       p.y = 0;
       WinDrawBitmap(hps, skn->iconImage, &rcl, &p, CLR_WHITE, CLR_BLACK, DBM_NORMAL);

       p.x = 0;
       p.y = skn->iconSize;
       WinDrawBitmap(hps, skn->iconMasc, &rcl, &p, CLR_WHITE, CLR_BLACK, DBM_NORMAL);

       GpiSetBitmap(hps,NULLHANDLE);

       ret = WinCreatePointer(HWND_DESKTOP, bmp, TRUE, 0, 0);
       DBGTracex(ret);

       GpiDeleteBitmap(bmp);
       GpiDestroyPS(hps);
    }

    DevCloseDC(hdc);
    return ret;

 }

 int icqskin_setBitmap(HICQ icq, hWindow hwnd, const char *fileName)
 {
#ifndef USE_MMPM

    return -1;

#else

    HBITMAP bitmap = (HBITMAP) WinSendMsg(hwnd,SM_QUERYHANDLE,0,0);
    int     rc     = 0;

    WinEnableWindowUpdate(hwnd, FALSE);

    if(bitmap)
       GpiDeleteBitmap(bitmap);

    bitmap = icqskin_loadBitmap(icq, fileName);

    if(bitmap == NO_IMAGE)
    {
       WinSendMsg(hwnd,SM_SETHANDLE,(MPARAM) 0, 0);
       rc = -2;
    }
    else
    {
       WinSendMsg(hwnd,SM_SETHANDLE,(MPARAM) bitmap, 0);
    }

    WinEnableWindowUpdate(hwnd, TRUE);
    WinUpdateWindow(hwnd);

    return rc;

#endif

 }

#ifdef USE_MMPM

 static HBITMAP LoadBitmap( HAB hab, HDC hdc, HPS hps, PSZ pszFileName )
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
          return NO_IMAGE;
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
          return NO_IMAGE;
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
          return NO_IMAGE;
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
          return NO_IMAGE;
     }

     dwReturnCode = mmioQueryHeaderLength ( hmmio,
                                          (PLONG)&ulImageHeaderLength,
                                            0L,
                                            0L);

     if ( ulImageHeaderLength != sizeof ( MMIMAGEHEADER ) )
     {
          /* We have a problem.....possibly incompatible versions */

          ulReturnCode = mmioClose (hmmio, 0L);
          return NO_IMAGE;
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
          return NO_IMAGE;
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
          return NO_IMAGE;
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
          return NO_IMAGE;
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

     return(hbm);
 }

#endif

 int _System icqskin_setItemBitmap(HICQ icq, HWND hwnd, HMODULE hMod, USHORT id, const char *fileName)
 {
    HBITMAP            bitmap          = NO_IMAGE;
    HPS                hps;
    SWP                swp;
    const SKINMGR      *skn            = icqQuerySkinManager(icq);

#ifdef USE_MMPM
    SIZEL              sizl            = { 0, 0 };  /* use same page size as device */
    DEVOPENSTRUC       dop             = { 0, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};
    HAB                hab             = icqQueryAnchorBlock(icq);
    HDC                hdc;
#endif

    DBGTracex(hwnd);
    DBGTracex(module);
    DBGTrace(id);

    if(id)
       hwnd = WinWindowFromID(hwnd,id);


    DBGMessage(fileName);
    DBGTracex(hwnd);

    if(!hwnd)
    {
       icqPrintLog(icq,PROJECT,"Invalid window handle loading %s",fileName);
       return -1;
    }

#ifdef EXTENDED_LOG
    icqPrintLog(icq,PROJECT,"Loading %s",fileName);
#endif

    WinShowWindow(hwnd,FALSE);

    if(skn && skn->LoadBitmap)
    {
       DBGTracex(skn->LoadBitmap);

       bitmap = skn->LoadBitmap(fileName);
       if(!bitmap)
          bitmap == NO_IMAGE;
    }

    DBGTracex(bitmap);

#ifdef USE_MMPM

    if( bitmap == NO_IMAGE && !access(fileName,04))
    {
       hdc    = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);
       hps    = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIT_MICRO | GPIA_ASSOC);

       if(hps)
       {
          bitmap = LoadBitmap(hab,hdc,hps,(PSZ) fileName);
          GpiDestroyPS(hps);
       }

       DevCloseDC(hdc);
    }

#endif

    DBGTracex(bitmap == NO_IMAGE);

    if(bitmap == NO_IMAGE)
    {
       /* Load default image */
#ifdef EXTENDED_LOG
       icqPrintLog(icq,PROJECT,"%s not loaded, reading from resource file",fileName);
#endif
       WinQueryWindowPos(hwnd, &swp);
       hps    = WinGetPS(hwnd);
       bitmap = GpiLoadBitmap(hps, hMod, id, swp.cx, swp.cy);
       DBGTracex(bitmap);
       WinReleasePS(hps);
    }

    WinSendMsg(hwnd,SM_SETHANDLE,(MPARAM) bitmap, 0);

    WinShowWindow(hwnd,TRUE);
    WinInvalidateRect(hwnd,0,TRUE);

    return 0;
 }

 static void erase(HWND hwnd, HPS hps, PRECTL rcl)
 {
    POINTL  ptl  = { 0, 0 } ;
    HBITMAP bmp  = WinQueryWindowULong(hwnd,0);

    hps = WinBeginPaint(hwnd,NULLHANDLE,rcl);
    WinFillRect(hps, rcl, CLR_PALEGRAY);

    WinQueryWindowRect( hwnd, rcl);

    if(bmp)
       WinDrawBitmap(hps, bmp, 0, &ptl, CLR_BLACK, CLR_PALEGRAY, DBM_NORMAL );

    GpiSetColor(hps,CLR_DARKGRAY);
    ptl.x = rcl->xRight-1;
    ptl.y = rcl->yTop-1;
    GpiMove(hps,&ptl);
    ptl.x = rcl->xLeft;
    GpiLine(hps,&ptl);
    ptl.y = rcl->yBottom;
    GpiLine(hps,&ptl);
    GpiSetColor(hps,CLR_WHITE);
    ptl.x = rcl->xRight-1;
    GpiLine(hps,&ptl);
    ptl.y = rcl->yTop-1;
    GpiLine(hps,&ptl);

    WinEndPaint(hps);
 }

 static void paint(HWND hwnd)
 {
    HPS   hps;
    RECTL rcl;
    erase(hwnd,hps,&rcl);
 }

 MRESULT EXPENTRY icqBitmapWindow(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
 {
    switch(msg)
    {
    case WM_CREATE:
       WinSetWindowPtr(hwnd,0,0);
       break;

    case WM_DESTROY:
       if(WinQueryWindowPtr(hwnd,0))
          GpiDeleteBitmap(WinQueryWindowULong(hwnd,0));
       break;

    case WM_ERASEBACKGROUND:
       erase(hwnd,(HPS) mp1, (PRECTL) mp2);
       break;

    case WM_PAINT:
       paint(hwnd);
       break;

    case SM_SETHANDLE:
       if(WinQueryWindowULong(hwnd,0))
          GpiDeleteBitmap(WinQueryWindowULong(hwnd,0));
       WinSetWindowULong(hwnd,0,(ULONG) mp1);
       WinInvalidateRect(hwnd,0,TRUE);
       break;

    case SM_QUERYHANDLE:
       return (MRESULT) WinQueryWindowULong(hwnd,0);

    default:
       return WinDefWindowProc(hwnd,msg,mp1,mp2);
    }
    return 0;

 }

