@echo off
chcp 65001 >nul
cd /d "%~dp0"
echo Перенаправлено в СТАРТ.bat ^(пункт 4 / exe^)
call "%~dp0СТАРТ.bat" exe
