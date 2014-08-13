/*
 * OS2REXX.H - Funcoes de apoio
 */

/*---[ Defines ]-------------------------------------------------------------------------------------------*/

 #define  INVALID_ROUTINE 40
 #define  VALID_ROUTINE    0
 #define  NULLCHAR        '\0'

 #define REXXFUNCTION(fname)      ULONG EXPENTRY fname(CHAR *name,ULONG numargs,RXSTRING args[],CHAR *queuename,RXSTRING *retstr)
 #define BEGINREXXFUNCTION(fname) ULONG EXPENTRY fname(CHAR *name,ULONG numargs,RXSTRING args[],CHAR *queuename,RXSTRING *retstr) {
 #define ENDREXXFUNCTION(rc)      CopyInteger(rc, retstr ); return( VALID_ROUTINE ); }

 #define REQPARMS(x) 		      if( numargs != x) return(INVALID_ROUTINE);
 #define RETURN_INVALID()         return(INVALID_ROUTINE)
 #define RETURN_VALID(x)          CopyInteger(x, retstr ); return( VALID_ROUTINE );


 #define INTVALUE(x)              atoi(args[x].strptr)
 #define LONGVALUE(x)             atol(args[x].strptr)
 #define STRINGVALUE(x)           args[x].strptr

/*---[ Prototipes ]----------------------------------------------------------------------------------------*/

 void CopyInteger( int, PRXSTRING );
 void CopyString( char *, PRXSTRING );
 void SetNullRXString( PRXSTRING );

