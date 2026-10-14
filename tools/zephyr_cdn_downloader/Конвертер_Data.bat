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
echo   После каждого архива файл из кэша
echo   удаляется ^(экономия места^).
echo   Индексы в _cdn_cache остаются — можно
echo   прервать и продолжить.
echo.
echo   Нужно свободно ~40–60+ ГБ на этом диске.
echo  ========================================
echo.

set "OUT=%~dp0"
set "OUT=%OUT:~0,-1%"
set "CACHE=%OUT%\_cdn_cache"

echo  Клиент : %OUT%
echo  Кэш CDN: %CACHE%
echo.
echo  Если ошибка "No space left on device":
echo   - удали папку _test_out ^(тесты^)
echo   - или перенеси конвертер на диск с местом
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
python convert_cdn_to_data.py --out "%OUT%" --region us --cache "%CACHE%" --with-install --patch-exe
if errorlevel 1 (
  echo [ERROR] convert failed
  pause
  exit /b 1
)

echo.
echo [2/2] Готово.
echo  Проверь: Data\data\data.000 и *.idx
echo  Папку _cdn_cache можно удалить.
echo  Дальше: Extract.bat / mapextractor
echo.
pause
endlocal
