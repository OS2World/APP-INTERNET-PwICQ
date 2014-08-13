/* */

   ok = RxFuncAdd( 'rxICQLoadFuncs', 'icqipc', 'rxICQLoadFuncs' )

   dir = directory();

   '@echo off'
   '@echo > w:\temp\x'

   'SET LIBPATHSTRICT=T'
   'SET BEGINLIBPATH=w:\private\pwicq2'

   'start \private\pwicq2\pwicq2.exe --uin=6838492 --offline --debug=\temp\pwicq.dbg --log=\temp\pwicq.log --inifile=..\..\config\pwicq.ini --config-files=..\..\config\ --plugins=0 --modules=icqmsg,icqv8,icqusrdb,icqskmgr,icqconf --module='||dir||'\icqv8p.dll' 

   'SET LIBPATHSTRICT=F'

   do while( rxICQQueryAvailable(6838492) <> 6838492 )
      ok = syssleep(1)
   end

   ok = syssleep(1)

   ok = rxICQSetPeerInfo( 6838492,  27241234,  16777343, 2301, 8)   
   ok = rxICQPeerConnect( 6838492, 27241234 )

   ok = syssleep(1)
   ok = rxICQSetPeerInfo( 27241234,  6838492,  16777343, 0, 8)   

   ok = syssleep(1)
   ok = rxICQSendFile( 6838492, 27241234, "w:\temp\x", "Teste de envio de arquivo") 

/*   ok = rxICQSendFile( 27241234, 6838492, "w:\temp\x", "Teste de envio de arquivo") */

   'tail -f \temp\pwicq.dbg'


