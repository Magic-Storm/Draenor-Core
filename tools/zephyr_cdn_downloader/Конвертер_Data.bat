@echo off
chcp 65001 >nul
setlocal EnableExtensions
cd /d "%~dp0"

title Zephyr — CDN → локальная Data (CASC)

echo.
echo  ========================================
echo   Конвертер CDN → Data\data\
echo.
echo   Качает архивы с zephyr.v4y.hu и пишет
echo   нормальный локальный CASC:
echo     data.000, data.001, … + .idx
echo.
echo   Это то, что открывают CascLib /
echo   mapextractor / vmap4extractor.
echo.
echo   Нужны десятки GB и много времени.
echo   Можно прервать и продолжить с --cache.
echo  ========================================
echo.

set "OUT=%~dp0"
set "OUT=%OUT:~0,-1%"
set "CACHE=%OUT%\_cdn_cache"

echo  Клиент : %OUT%
echo  Кэш CDN: %CACHE%
echo.
pause

python --version >nul 2>&1
if errorlevel 1 (
  echo [ERROR] Нужен Python 3 в PATH
  pause
  exit /b 1
)

echo.
echo [1/2] Конвертация архивов → Data\data\ ...
python convert_cdn_to_data.py --out "%OUT%" --region us --cache "%CACHE%" --keep-cache --with-install --patch-exe
if errorlevel 1 (
  echo [ERROR] convert failed
  pause
  exit /b 1
)

echo.
echo [2/2] Готово.
echo  Проверь: Data\data\data.000 и *.idx
echo  Дальше: Extract.bat / mapextractor
echo.
pause
endlocal
