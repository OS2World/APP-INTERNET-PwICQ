@echo off

@echo off

setlocal

  SET LIBPATHSTRICT=T
  SET BEGINLIBPATH=w:\private\pwicq2

   cd \private\pwicq2

rem  icqtext.exe --uin=6838492 --password=oxigenio --save --inifile=w:\data\ibmcpp\sources\icq2\config\pwicq.ini --plugins=0 --module=plugins\icqusrdb.dll --module=w:\data\ibmcpp\sources\icq2\plugins\icqv8\icqv8.dll
rem  icqtext.exe --uin=27241234 --inifile=w:\data\ibmcpp\sources\icq2\config\pwicq.ini --plugins=0 --module=plugins\icqusrdb.dll --module=w:\data\ibmcpp\sources\icq2\plugins\icqv8\icqv8.dll

  start pwicq2.exe --uin=6838492 --debug=w:\temp\pwicq.dbg --inifile=w:\data\ibmcpp\sources\icq2\config\pwicq.ini --log=w:\temp\pwicq.log --plugins=0 --modules=icqusrdb.dll,icqconf.dll --module=W:\Public\pwicq\os2\experimental\icqgui.dll --module=w:\data\ibmcpp\sources\icq2\plugins\icqv8\icqv8.dll
  call rexxtry say syssleep(1) > NUL:
  tail -60 -f w:\temp\pwicq.dbg

  SET LIBPATHSTRICT=F

endlocal


