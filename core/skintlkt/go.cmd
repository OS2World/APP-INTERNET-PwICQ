@echo off

start skintest.exe
call rexxtry say syssleep(1) > NUL:
tail -60 -f skintest.dbg


