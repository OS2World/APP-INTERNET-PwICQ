/* */

   ok = RxFuncAdd( 'rxICQLoadFuncs', 'icqipc', 'rxICQLoadFuncs' )

   dir = directory();

   '@echo off'
   'SET LIBPATHSTRICT=T'
   'SET BEGINLIBPATH=w:\private\pwicq2'

   'start \private\pwicq2\pwicq2.exe --uin=6838492 --debug=\temp\pwicq.dbg --log=\temp\pwicq.log --inifile=..\..\config\pwicq.ini --config-files=..\..\config\ --plugins=0 --modules=icqv8,icqv8p,icqusrdb,icqskmgr --module='||dir||'\icqmsg.dll' 

   'SET LIBPATHSTRICT=F'

   ok = syssleep(3)
   say rxICQSetPeerInfo(6838492, 27241234, 16777343, 2301, 8)   
   ok = syssleep(1)
   ok = rxICQExecute(6838492,"open 27241234")

   'cd '||dir

   'tail -f \temp\pwicq.dbg'

