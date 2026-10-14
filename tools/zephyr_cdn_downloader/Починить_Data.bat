@echo off
chcp 65001 >nul
setlocal EnableExtensions
cd /d "%~dp0"

title Zephyr — починить ENCODING/ROOT в Data

echo.
echo  ========================================
echo   Починка Data для CascLib / mapextractor
echo.
echo   Добавляет ENCODING / ROOT / DOWNLOAD /
echo   INSTALL если их нет в Data\data\.idx
echo   ^(типичная причина BAD_FORMAT^).
echo  ========================================
echo.

set "OUT=%~dp0"
set "OUT=%OUT:~0,-1%"
set "CACHE=%OUT%\_cdn_cache"

python --version >nul 2>&1
if errorlevel 1 (
  echo [ERROR] Нужен Python 3 в PATH
  pause
  exit /b 1
)

if not exist "%OUT%\Data\data" (
  echo [ERROR] Нет папки Data\data — сначала Конвертер_Data.bat
  pause
  exit /b 1
)

echo Клиент: %OUT%
echo.
python repair_essential.py --out "%OUT%" --region us --cache "%CACHE%"
if errorlevel 1 (
  echo [ERROR] repair failed
  pause
  exit /b 1
)

echo.
echo [+] Также обновляю .build.info ^(CDN Hosts/Path для CascLib^)...
python -c "from download_client import write_build_info; from pathlib import Path; write_build_info(Path(r'%OUT%'), 'us', '451679f16b266632976beb8d5c922d9f', 'eee453d9035886d03a1308de041de88c', '6.2.4.21742', '21742')"

echo.
echo Готово. Снова запусти Extract.bat
pause
endlocal
