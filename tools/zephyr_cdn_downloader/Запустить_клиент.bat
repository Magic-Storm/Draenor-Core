@echo off
chcp 65001 >nul
setlocal
cd /d "%~dp0"

set "EXE=%~dp0Wow-64.exe"
if not exist "%EXE%" set "EXE=%~dp0Wow.exe"
if not exist "%EXE%" (
  echo Нет Wow-64.exe в этой папке. Сначала Скачать_клиент.bat
  pause
  exit /b 1
)

python "%~dp0patch_client.py" --exe "%EXE%" --region us
start "" "%EXE%"
endlocal
