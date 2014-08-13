@echo off

rem   cd \private\pwicq2

   SET LIBPATHSTRICT=T
   SET BEGINLIBPATH=w:\data\ibmcpp\sources\icq2\core\network;w:\private\pwicq2

   \private\pwicq2\icqtext --offline --uin=6838492 --inifile=w:\data\ibmcpp\sources\icq2\config\pwicq.ini --plugins=1 --config-files=w:\data\ibmcpp\sources\icq2\config\

rem   start \private\pwicq2\pwicq2 --uin=6838492 --inifile=w:\data\ibmcpp\sources\icq2\config\pwicq.ini --plugins=1 --config-files=w:\data\ibmcpp\sources\icq2\config\ --debug=w:\temp\pwicq.dbg --log=w:\temp\pwicq.log --skin=w:\public\pwicq2\os2\bin\skins\default.skn
rem   start \private\pwicq2\pwicq2 --uin=27241234 --inifile=w:\data\ibmcpp\sources\icq2\config\pwicq.ini --plugins=1 --config-files=w:\data\ibmcpp\sources\icq2\config\ --debug=w:\temp\pwicq.dbg --log=w:\temp\pwicq.log --skin=w:\public\pwicq2\os2\bin\skins\default.skn
rem   call rexxtry say syssleep(1) > NUL:
rem   tail -60 -f w:\temp\pwicq.dbg

   SET LIBPATHSTRICT=F

rem   cd \data\ibmcpp\sources\icq2\core\kernel



