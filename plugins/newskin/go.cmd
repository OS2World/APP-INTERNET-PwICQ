/* */

   parse arg project

   ok = RxFuncAdd( 'rxICQLoadFuncs', 'icqipc', 'rxICQLoadFuncs' )

   dir = directory();

   'SET LIBPATHSTRICT=T'
   'SET BEGINLIBPATH=w:\private\pwicq2'

   'start \private\pwicq2\pwicq2.exe --uin=6838492 --offline --debug=\temp\pwicq.dbg --log=\temp\pwicq.log --inifile=..\..\config\pwicq.ini --config-files=..\..\config\ --plugins=0 --modules=icqmsg,icqv8,icqv8p,icqusrdb,icqconf,icqsound,icqpmdc --module='||dir||'\'||project||'.dll'

   do while( rxICQQueryAvailable(6838492) <> 6838492 )
      ok = syssleep(1)
   end

   ok = syssleep(1)
   ok = rxICQSetPeerInfo(6838492, 27241234, "127.0.0.1", 2301, 8)   
   ok = syssleep(1)
   ok = rxICQExecute(6838492,"open 27241234")

   'tail -f \temp\pwicq.dbg'


   'SET LIBPATHSTRICT=F'

