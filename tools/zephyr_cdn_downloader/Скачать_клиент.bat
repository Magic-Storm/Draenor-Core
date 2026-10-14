@echo off
chcp 65001 >nul
setlocal EnableExtensions
cd /d "%~dp0"

title Zephyr — полный клиент 6.2.4

echo.
echo  ========================================
echo   Полный клиент с https://zephyr.v4y.hu/wow/
echo.
echo   1^) Качаем только exe/DLL ^(быстро^)
echo   2^) Патчим на Zephyr
echo   3^) Запускаем Wow — ОН сам качает
echo      полную Data с CDN в правильном
echo      формате ^(data.000, idx, ...^)
echo.
echo   НЕ зеркалим сырой CDN на диск —
echo   иначе CascLib / экстракторы ломаются.
echo.
echo   Альтернатива без Wow.exe:
echo   Конвертер_Data.bat  (CDN -^> data.NNN/.idx)
echo  ========================================
echo.

set "OUT=%~dp0"
set "OUT=%OUT:~0,-1%"
echo  Папка клиента: %OUT%
echo.
pause

python --version >nul 2>&1
if errorlevel 1 (
  echo [ERROR] Нужен Python 3 в PATH
  pause
  exit /b 1
)

echo.
echo [1/3] exe/DLL с Zephyr...
python download_client.py --out "%OUT%" --region us --install-only
if errorlevel 1 (
  echo [ERROR] download failed
  pause
  exit /b 1
)

set "EXE=%OUT%\Wow-64.exe"
if not exist "%EXE%" set "EXE=%OUT%\Wow.exe"
if not exist "%EXE%" (
  echo [ERROR] Wow.exe не найден в %OUT%
  pause
  exit /b 1
)

echo.
echo [2/3] Патч CAS -^> zephyr.v4y.hu ...
python patch_client.py --exe "%EXE%" --region us
if errorlevel 1 (
  echo [ERROR] patch failed
  pause
  exit /b 1
)

echo.
echo [3/3] Запуск Wow.
echo.
echo  ВАЖНО: оставь клиент работать, пока качает Data.
echo  Нужны десятки GB. Готово, когда Data\data\
echo  вырастет сильно ^(не ~600 MB, а полный объём^).
echo.
echo  После полной Data можно запускать Extract.bat
echo  ^(mapextractor / vmaps / mmaps^).
echo.
pause

start "" "%EXE%"
endlocal
