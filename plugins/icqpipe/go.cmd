@echo off

@echo off

setlocal

  SET LIBPATHSTRICT=T
  SET BEGINLIBPATH=w:\private\pwicq2

   cd \private\pwicq2

rem   icqtext.exe --uin=6838492 --password=oxigenio --save --plugins=0 --inifile=w:\data\ibmcpp\sources\icq2\config\pwicq.ini --plugins=0 --config-files=w:\data\ibmcpp\sources\icq2\config\ --module=w:\data\ibmcpp\sources\icq2\plugins\icqpipe\icqpipe.dll

   start pwicq2.exe --uin=6838492 --inifile=w:\data\ibmcpp\sources\icq2\config\pwicq.ini --debug=w:\temp\pwicq.dbg --log=w:\temp\pwicq.log --plugins=0 --config-files=w:\data\ibmcpp\sources\icq2\config\ --module=plugins\icqskmgr.dll --module=plugins\icqconf.dll --module=plugins\icqusrdb.dll --module=w:\data\ibmcpp\sources\icq2\plugins\icqpipe\icqpipe.dll
   call rexxtry say syssleep(1) > NUL:
   tail -60 -f w:\temp\pwicq.dbg

  SET LIBPATHSTRICT=F

endlocal


