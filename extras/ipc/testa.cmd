/* */


 say "Carregando modulo de utilitarios "
 ok = RxFuncAdd( 'rxICQLoadFuncs', 'icqipc', 'rxICQLoadFuncs' )

 ok = rxICQLoadFuncs(1)
 say ok

 ok = RxFuncAdd( 'rxICQPeerConnect', 'icqipc', 'rxICQPeerConnect' )
 say ok


 say "Build: "||rxICQQueryBuild()

 say "Instancia: "||rxICQQueryUIN()

 say "Online mode: "||rxICQQueryOnlineMode()

 say "Codepage: "|| rxICQConvertCodePage(0,1,"αινσϊ")
 say "Codepage: "|| rxICQConvertCodePage(0,0,"αινσϊ")

 say rxICQSetPeerInfo( 27241234, 6838492, "127.0.0.1", 2301, 8)
 say rxICQPeerConnect( 27241234, 6838492)
 say rxICQQueryAvailable(27241234)
 say rxICQQueryAvailable(6838492)

 ok = rxICQDropFuncs()

