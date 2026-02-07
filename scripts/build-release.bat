@echo off
:: ============================================================================
:: AstraX Local Release Builder
:: Creates a distributable release package
:: ============================================================================

setlocal enabledelayedexpansion

set VERSION=1.0.0
set PLATFORM=windows

echo.
echo =========================================
echo   AstraX Release Builder v%VERSION%
echo =========================================
echo.

:: Get script directory
set SCRIPT_DIR=%~dp0
set PROJECT_DIR=%SCRIPT_DIR%..

cd /d "%PROJECT_DIR%"

:: Clean and rebuild
echo [1/5] Cleaning build directory...
if exist build rmdir /S /Q build

echo [2/5] Configuring CMake (Release)...
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DASTRAX_BUILD_TESTS=OFF -DASTRAX_ENABLE_WARNINGS=OFF

echo [3/5] Building...
cmake --build build

:: Check if build succeeded
if not exist "build\bin\astrax.exe" (
    echo ERROR: Build failed - astrax.exe not found
    pause
    exit /b 1
)

:: Create release directory
echo [4/5] Creating release package...
set RELEASE_DIR=release\astrax-%VERSION%-%PLATFORM%

if exist release rmdir /S /Q release
mkdir "%RELEASE_DIR%"

:: Copy files
copy build\bin\astrax.exe "%RELEASE_DIR%\"
copy README.md "%RELEASE_DIR%\"
copy LICENSE "%RELEASE_DIR%\"
xcopy config "%RELEASE_DIR%\config\" /E /I

:: Copy installer
copy scripts\install.bat "%RELEASE_DIR%\"

echo [5/5] Creating ZIP archive...
powershell -Command "Compress-Archive -Path '%RELEASE_DIR%' -DestinationPath 'release\astrax-%VERSION%-%PLATFORM%.zip'"

echo.
echo =========================================
echo   Release Build Complete!
echo =========================================
echo.
echo Release package: release\astrax-%VERSION%-%PLATFORM%.zip
echo.
echo Contents:
dir /B "%RELEASE_DIR%"
echo.

pause
