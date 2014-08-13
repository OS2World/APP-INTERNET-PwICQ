/*
 * loadscheme.cmd - Request loading of a new sound scheme
 */

 parse arg scheme

 ok = RxFuncAdd( 'rxICQLoadFuncs', 'icqipc', 'rxICQLoadFuncs' )
 ok = rxICQLoadFuncs(1)

 instance = rxICQQueryUIN()

 if instance = 0 then
   do
      say "Seens like pwICQ isn't loaded"
      return -1
   end

 say "Using rxICQ Build "||rxICQQueryBuild()||" to talk with instance "||instance

 ok = rxICQPluginRequest(instance,"icqsound","l"||scheme)

 say "Requesting scheme "||scheme||" (rc="||ok||")"


