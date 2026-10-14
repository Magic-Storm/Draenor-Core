@echo off
chcp 65001 >nul
cd /d "%~dp0"
echo Перенаправлено в СТАРТ.bat ^(пункт 2 / convert^)
call "%~dp0СТАРТ.bat" convert
