@echo off

@echo off

setlocal

  SET LIBPATHSTRICT=T
  SET BEGINLIBPATH=w:\private\pwicq2

   cd \private\pwicq2

   \private\pwicq2\icqtext --offline --uin=6838492 --inifile=w:\data\ibmcpp\sources\icq2\config\pwicq.ini --plugins=0 --module=w:\data\ibmcpp\sources\icq2\plugins\icqusers\icqusrdb.dll

  SET LIBPATHSTRICT=F

endlocal


