/*
 * DLLMAIN.C - Startup code para DLL
 */

#include <os2.h>

#include <pwMacros.h>

#include "skinapi.h"

unsigned long _System _DLL_InitTerm(unsigned long hModule, unsigned long ulFlag)
{
   switch (ulFlag)
   {
   case 0 :
      if (_CRT_init() == -1)
         return 0UL;
      break;

   case 1 :
      break;

   default  :
      return 0UL;
   }

   return 1UL;
}

const char * EXPENTRY sknQueryBuild(void)
{
   return __DATE__ " " __TIME__;
}

const char * EXPENTRY sknQueryIDString(void)
{
   return PROJECT ":Perry Werneck's skin toolkit version " PWSKIN_VERSION " Build " __DATE__ " " __TIME__ " by " USER "@" HOSTNAME;
}

