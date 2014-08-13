@echo off

setlocal

   SET LIBPATHSTRICT=T
   SET BEGINLIBPATH=w:\public\pwicq2\os2\bin

   cd \private\pwicq2
   start pwicq2.exe --uin=6838492 --inifile=w:\data\ibmcpp\sources\icq2\config\pwicq.ini --plugins=0 --debug=w:\temp\pwicq.dbg --log=w:\temp\pwicq.log --plugins=0 --config-files=w:\data\ibmcpp\sources\icq2\config\ --modules=icqv8,icqusrdb,icqskmgr, --module=%BASEDIR%\icqconf.dll

   SET LIBPATHSTRICT=F

   call rexxtry say syssleep(1) > NUL:
   tail -60 -f w:\temp\pwicq.dbg


endlocal



