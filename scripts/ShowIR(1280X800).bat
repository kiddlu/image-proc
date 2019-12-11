@echo off

set CUR_DIR=%~dp0

%CUR_DIR%\runtime\python.exe %CUR_DIR%\scripts\ShowIR.py -w 1280 -h 800

pause