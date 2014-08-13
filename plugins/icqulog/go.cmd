@echo off

@echo off

setlocal

  SET LIBPATHSTRICT=T
  SET BEGINLIBPATH=w:\private\pwicq2

   cd \private\pwicq2

   start pwicq2.exe --uin=6838492 --inifile=w:\data\ibmcpp\sources\icq2\config\pwicq.ini --debug=w:\temp\pwicq.dbg --log=w:\temp\pwicq.log --plugins=0 --config-files=w:\data\ibmcpp\sources\icq2\config\ --module=plugins\icqskmgr.dll --module=plugins\icqmsg.dll --module=plugins\icqv7.dll --module=w:\data\ibmcpp\sources\icq2\plugins\icqulog\icqulog.dll
   call rexxtry say syssleep(1) > NUL:
   tail -60 -f w:\temp\pwicq.dbg

  SET LIBPATHSTRICT=F

endlocal


