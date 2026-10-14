@echo off
chcp 65001 >nul
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"
title Zephyr 6.2.4 — мастер

REM Клиент = эта папка (не чужие инсталлы Midnight и т.п.)
set "CLIENT=%~dp0"
set "CLIENT=%CLIENT:~0,-1%"
set "CACHE=%CLIENT%\_cdn_cache"
set "CACHE_FILL=%CLIENT%\_cdn_cache_fill"
> "%CLIENT%\client_path.txt" echo %CLIENT%

set "MODE="
if not "%~1"=="" set "MODE=%~1"

if /I "%MODE%"=="full" goto FULL
if /I "%MODE%"=="convert" goto CONVERT
if /I "%MODE%"=="fill" goto FILL
if /I "%MODE%"=="exe" goto EXE
if /I "%MODE%"=="repair" goto REPAIR
if /I "%MODE%"=="extract" goto EXTRACT
if /I "%MODE%"=="launch" goto LAUNCH
if not "%MODE%"=="" (
  echo Неизвестный аргумент: %MODE%
  echo Используй: full convert fill exe repair extract launch
  pause
  exit /b 1
)

:MENU
cls
echo.
echo  ========================================
echo   Zephyr WoW 6.2.4.21742
echo   Папка: %CLIENT%
echo  ========================================
echo.
if exist "%CLIENT%\Data\data\data.000" (
  echo   [Data] есть data.000
) else (
  echo   [Data] нет — нужен пункт 2 или 1
)
if exist "%CLIENT%\Wow-64.exe" (echo   [EXE ] Wow-64.exe OK) else if exist "%CLIENT%\Wow.exe" (echo   [EXE ] Wow.exe OK) else (echo   [EXE ] нет — пункт 2 или 4)
if exist "%CLIENT%\mapextractor.exe" (echo   [Ext ] mapextractor OK) else (echo   [Ext ] нет mapextractor.exe)
echo.
echo   1^) ВСЁ ДЛЯ СЕРВЕРА   ^(конверт Data → долить → починить → extract^)
echo   2^) Собрать / докачать Data с CDN
echo   3^) Долить пробелы ^(archive.wow.tools^)
echo   4^) Скачать exe/DLL + патч Zephyr
echo   5^) Починить ENCODING/ROOT
echo   6^) Extract ^(maps / vmaps / mmaps^)
echo   7^) Запустить клиент
echo   0^) Выход
echo.
set "N="
set /p "N=Выбор [0-7]: "
if "%N%"=="1" goto FULL
if "%N%"=="2" goto CONVERT
if "%N%"=="3" goto FILL
if "%N%"=="4" goto EXE
if "%N%"=="5" goto REPAIR
if "%N%"=="6" goto EXTRACT
if "%N%"=="7" goto LAUNCH
if "%N%"=="0" exit /b 0
echo Неверный выбор
timeout /t 2 >nul
goto MENU

:NEED_PY
python --version >nul 2>&1
if errorlevel 1 (
  echo [ERROR] Нужен Python 3 в PATH
  pause
  exit /b 1
)
exit /b 0

:DONE_OR_MENU
if not "%MODE%"=="" (
  pause
  exit /b 0
)
pause
goto MENU

:FULL
call :NEED_PY
if errorlevel 1 exit /b 1
echo.
echo ========== [1/4] CDN → Data ==========
echo Прогресс в этом окне. Можно прервать и снова выбрать 1 или 2 — продолжит.
echo.
python -u convert_cdn_to_data.py --out "%CLIENT%" --region us --cache "%CACHE%" --with-install --patch-exe
if errorlevel 1 goto FAIL
echo.
echo ========== [2/4] Долить усечённые архивы ==========
python -u repair_truncated.py --out "%CLIENT%" --kind both --cache "%CACHE_FILL%"
if errorlevel 1 (
  echo [WARN] fill завершился с ошибкой — смотри лог выше. Продолжаю.
)
echo.
echo ========== [3/4] Починить ENCODING/ROOT ==========
python -u repair_essential.py --out "%CLIENT%" --region us --cache "%CACHE%"
if errorlevel 1 goto FAIL
python -c "from download_client import write_build_info; from pathlib import Path; write_build_info(Path(r'%CLIENT%'), 'us', '451679f16b266632976beb8d5c922d9f', 'eee453d9035886d03a1308de041de88c', '6.2.4.21742', '21742')"
echo.
echo ========== [4/4] Extract ==========
call :DO_EXTRACT_ALL
if errorlevel 1 goto FAIL
echo.
echo Готово: Data + maps/vmaps/mmaps.
goto DONE_OR_MENU

:CONVERT
call :NEED_PY
if errorlevel 1 exit /b 1
echo.
echo  CDN → локальная Data ^(data.NNN + .idx^)
echo  Кэш: %CACHE%
echo  Прервал — запусти снова, докачает.
echo.
if "%MODE%"=="" pause
python -u convert_cdn_to_data.py --out "%CLIENT%" --region us --cache "%CACHE%" --with-install --patch-exe
if errorlevel 1 goto FAIL
echo.
echo Готово. Дальше: пункт 3 ^(долить^), потом 6 ^(extract^) или пункт 1.
goto DONE_OR_MENU

:FILL
call :NEED_PY
if errorlevel 1 exit /b 1
if not exist "%CLIENT%\Data\data\data.000" (
  echo [ERROR] Нет Data — сначала пункт 2
  goto DONE_OR_MENU
)
echo.
echo  Ищем усечённые архивы Zephyr и доливаем с archive.wow.tools
echo.
python -u repair_truncated.py --out "%CLIENT%" --kind both --cache "%CACHE_FILL%"
if errorlevel 1 goto FAIL
echo.
echo Готово.
goto DONE_OR_MENU

:EXE
call :NEED_PY
if errorlevel 1 exit /b 1
echo.
echo [1/2] exe/DLL...
python -u download_client.py --out "%CLIENT%" --region us --install-only
if errorlevel 1 goto FAIL
set "EXE=%CLIENT%\Wow-64.exe"
if not exist "%EXE%" set "EXE=%CLIENT%\Wow.exe"
if not exist "%EXE%" (
  echo [ERROR] Wow.exe не найден
  goto DONE_OR_MENU
)
echo [2/2] Патч Zephyr...
python -u patch_client.py --exe "%EXE%" --region us
if errorlevel 1 goto FAIL
echo Готово.
goto DONE_OR_MENU

:REPAIR
call :NEED_PY
if errorlevel 1 exit /b 1
if not exist "%CLIENT%\Data\data" (
  echo [ERROR] Нет Data\data — сначала пункт 2
  goto DONE_OR_MENU
)
python -u repair_essential.py --out "%CLIENT%" --region us --cache "%CACHE%"
if errorlevel 1 goto FAIL
python -c "from download_client import write_build_info; from pathlib import Path; write_build_info(Path(r'%CLIENT%'), 'us', '451679f16b266632976beb8d5c922d9f', 'eee453d9035886d03a1308de041de88c', '6.2.4.21742', '21742')"
echo Готово.
goto DONE_OR_MENU

:EXTRACT
if not exist "%CLIENT%\mapextractor.exe" (
  echo [ERROR] Нет mapextractor.exe в этой папке
  echo Скопируй из build\bin\RelWithDebInfo
  pause
  goto MENU
)
if not exist "%CLIENT%\Data\data\data.000" (
  echo [ERROR] Нет Data\data\data.000 — пункты 2 и 3
  pause
  goto MENU
)
echo.
echo   1^) mapextractor
echo   2^) vmap4extractor
echo   3^) vmap4assembler
echo   4^) mmaps_generator
echo   5^) всё по очереди
echo   0^) назад
echo.
set "E="
set /p "E=Выбор [0-5]: "
if "%E%"=="1" goto E_MAP
if "%E%"=="2" goto E_VMAPE
if "%E%"=="3" goto E_VMAPA
if "%E%"=="4" goto E_MMAP
if "%E%"=="5" (
  call :DO_EXTRACT_ALL
  if errorlevel 1 goto FAIL
  echo Готово.
  pause
  goto MENU
)
goto MENU

:E_MAP
mapextractor.exe -i "%CLIENT%"
if errorlevel 1 goto FAIL
pause
goto MENU

:E_VMAPE
vmap4extractor.exe -d "%CLIENT%"
if errorlevel 1 goto FAIL
pause
goto MENU

:E_VMAPA
if not exist Buildings (
  echo Сначала vmap4extractor
  pause
  goto MENU
)
if not exist vmaps mkdir vmaps
vmap4assembler.exe Buildings vmaps
if errorlevel 1 goto FAIL
pause
goto MENU

:E_MMAP
mmaps_generator.exe
if errorlevel 1 goto FAIL
pause
goto MENU

:DO_EXTRACT_ALL
if not exist "%CLIENT%\mapextractor.exe" (
  echo [ERROR] Нет mapextractor.exe — extract пропущен
  exit /b 1
)
tasklist /FI "IMAGENAME eq mapextractor.exe" 2>nul | find /I "mapextractor.exe" >nul
if not errorlevel 1 (
  echo [ERROR] mapextractor уже запущен
  exit /b 1
)
echo === mapextractor ===
mapextractor.exe -i "%CLIENT%"
if errorlevel 1 exit /b 1
echo === vmap4extractor ===
vmap4extractor.exe -d "%CLIENT%"
if errorlevel 1 exit /b 1
echo === vmap4assembler ===
if not exist vmaps mkdir vmaps
vmap4assembler.exe Buildings vmaps
if errorlevel 1 exit /b 1
echo === mmaps_generator ===
mmaps_generator.exe
if errorlevel 1 exit /b 1
exit /b 0

:LAUNCH
set "EXE=%CLIENT%\Wow-64.exe"
if not exist "%EXE%" set "EXE=%CLIENT%\Wow.exe"
if not exist "%EXE%" (
  echo [ERROR] Нет Wow — пункт 4
  goto DONE_OR_MENU
)
call :NEED_PY
if not errorlevel 1 python -u patch_client.py --exe "%EXE%" --region us
start "" "%EXE%"
if not "%MODE%"=="" exit /b 0
goto MENU

:FAIL
echo.
echo [ERROR] шаг сорвался. Смотри сообщения выше.
echo Конвертер можно запускать снова — продолжит с кэша.
goto DONE_OR_MENU
