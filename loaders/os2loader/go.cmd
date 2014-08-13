@echo off

SET LIBPATHSTRICT=T
SET BEGINLIBPATH=w:\private\pwicq2

pwicq2 --uin=6838492 --plugins=1 --log=w:\temp\pwicq.log --debug=w:\temp\pwicq.dbg --inifile=..\..\config\pwicq.ini --config-files=..\..\config\

SET LIBPATHSTRICT=F

tail -60 w:\temp\pwicq.dbg


