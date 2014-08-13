@echo off

rem   start pwicq2 --uin=6838492 --inifile=w:\data\ibmcpp\sources\icq2\config\pwicq.ini --plugins=1 --config-files=w:\data\ibmcpp\sources\icq2\config\ --debug=w:\temp\pwicq.dbg --log=w:\temp\pwicq.log --skin=default.skn
rem   start pwicq2 --uin=6838492 --inifile=w:\data\ibmcpp\sources\icq2\config\pwicq.ini --plugins=1 --config-files=w:\data\ibmcpp\sources\icq2\config\ --debug=w:\temp\pwicq.dbg --log=w:\temp\pwicq.log --skin=pwicqretro-small.skn
rem    start pwicq2 --uin=6838492 --offline --inifile=w:\data\ibmcpp\sources\icq2\config\pwicq.ini --plugins=0 --config-files=w:\data\ibmcpp\sources\icq2\config\ --debug=w:\temp\pwicq.dbg --log=w:\temp\pwicq.log --modules=icqv8.dll,icqconf.dll,icqusrdb.dll --module=w:\data\ibmcpp\sources\icq2\plugins\icqskmgr\icqskmgr.dll --gui=icqskmgr

   cd \private\pwicq2
   start pwicq2.exe --uin=6838492 --password=oxigenio --save --offline --debug=\temp\pwicq.dbg --log=\temp\pwicq.log --inifile=w:\data\ibmcpp\sources\icq2\config\pwicq.ini --plugins=0 --modules=icqmsg,icqv8,icqusrdb,icqconf --module=%BASEDIR%\icqskmgr.dll

   call rexxtry say syssleep(1) > NUL:
   tail -60 -f w:\temp\pwicq.dbg

cd %BASEDIR%


