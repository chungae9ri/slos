@echo off
if "%1" == "" goto USAGE
if "%2" == "" goto USAGE
if "%3" == "" goto USAGE
if "%4" == "" goto USAGE

python "%~dp0ramparse.py" -w -d -t -i -q -s %*

goto QUIT

: USAGE
echo Usage: ramparse.bat -a [dump path] -v [vmlinux file]
echo    ex) ramparse.bat -a . -v vmlinux

: QUIT
