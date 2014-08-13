@echo off

setlocal

  SET LIBPATHSTRICT=T
  SET BEGINLIBPATH=w:\private\pwicq2

   cd \private\pwicq2

rem  icqtext.exe --uin=6838492 --inifile=w:\data\ibmcpp\sources\icq2\config\pwicq.ini --plugins=0 --module=plugins\icqusrdb.dll --module=plugins\icqv8p.dll  --module=plugins\icqv8.dll --module=w:\data\ibmcpp\sources\icq2\plugins\isicq\isicq.dll

   start pwicq2.exe --uin=6838492 --debug=w:\temp\pwicq.dbg --inifile=w:\data\ibmcpp\sources\icq2\config\pwicq.ini --log=w:\temp\pwicq.log --plugins=0 --module=plugins\icqskmgr.dll --module=plugins\icqusrdb.dll --module=plugins\icqconf.dll --module=plugins\icqv8.dll --module=w:\data\ibmcpp\sources\icq2\plugins\isicq\isicq.dll
   call rexxtry say syssleep(1) > NUL:
   tail -60 -f w:\temp\pwicq.dbg

  SET LIBPATHSTRICT=F

endlocal


