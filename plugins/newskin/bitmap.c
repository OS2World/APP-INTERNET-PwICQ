/*
 * bitmap.c - Common image management
 */

#ifdef __OS2__
 #pragma strings(readonly)
 #include <io.h>
 #define R_OK 04
#endif

#ifdef linux
 #include <unistd.h>
#endif 

 #include <string.h>

 #include <pwicqgui.h>

/*---[ Prototipes ]---------------------------------------------------------------------------------------*/


/*---[ Implementation ]-----------------------------------------------------------------------------------*/

 int icqskin_queryImage(HICQ icq, const char *fileName, int sz, char *buffer)
 {
    /* Make the image file name */
    char *ptr;

#ifdef linux
    static const char *fldr = "/";
#else
    static const char *fldr = "\\";
#endif

    if(!fileName)
       return -1;

    icqLoadString(icq,"SkinImages","",buffer,sz);
    strncat(buffer,fileName,sz);

    if(access(buffer,R_OK))
    {
       /* Can't find in SkinPath */
       icqLoadString(icq,"image-files","",buffer,sz);
       strncat(buffer,fileName,sz);

       if(access(buffer,R_OK))
       {
          /* Can't find in image-path */
          icqQueryPath(icq,"images",buffer,sz-10);
          ptr = buffer+strlen(buffer);
          if(*ptr != *fldr)
             *(ptr++) = *fldr;
          *ptr =0;
          strncat(buffer,fileName,sz);
       }
    }

    return access(buffer,R_OK);

 }

/*

    if(access(fileName,R_OK))
    {

       icqLoadString(icq,"image-files","",buffer,0xFF);

       if(!*buffer)
          icqQueryProgramPath(icq,"images\\",buffer,0xFF);

       strncat(buffer,fileName,0xFF);

       if(access(buffer,04))
       {
          DBGMessage("Can't find the image data file");
          return -1;
       }
    }
    else
    {
       strncpy(buffer,fileName,0xFF);
    }


*/

