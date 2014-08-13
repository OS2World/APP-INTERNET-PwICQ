/* */

 parse arg uin

 ok = RxFuncAdd( 'rxICQLoadFuncs', 'icqipc', 'rxICQLoadFuncs' )
 ok = rxICQLoadFuncs()

 say "Adding user "||uin

 ok = rxICQAddUser(0,uin)

 say "Completed (rc="||ok||")"
