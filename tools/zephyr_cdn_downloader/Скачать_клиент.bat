@echo off
chcp 65001 >nul
setlocal EnableExtensions
cd /d "%~dp0"

title Zephyr WoW 6.2.4 — полный клиент

echo.
echo  ========================================
echo   Полный клиент с Zephyr CDN
echo   https://zephyr.v4y.hu/wow/
echo.
echo   Качается СЮДА ЖЕ (папка с этим bat^):
echo     Wow-64.exe
echo     Data\
echo.
echo   Объём большой, можно прерывать и продолжать.
echo  ========================================
echo.

rem Та же папка, где лежит bat / основа
set "OUT_DIR=%~dp0"
set "OUT_DIR=%OUT_DIR:~0,-1%"
echo  Папка: %OUT_DIR%
echo.
pause

python --version >nul 2>&1
if errorlevel 1 (
  echo [ERROR] Нужен Python 3 в PATH
  pause
  exit /b 1
)

python download_client.py --out "%OUT_DIR%" --region us --patch-exe
set "ERR=%ERRORLEVEL%"

echo.
if not "%ERR%"=="0" (
  echo [ERROR] Код %ERR%
  pause
  exit /b %ERR%
)

echo  Готово: %OUT_DIR%\Wow-64.exe
echo          %OUT_DIR%\Data\
echo.
set "EXE=%OUT_DIR%\Wow-64.exe"
if not exist "%EXE%" set "EXE=%OUT_DIR%\Wow.exe"
if exist "%EXE%" (
  echo Запустить сейчас?
  pause
  start "" "%EXE%"
)
endlocal
