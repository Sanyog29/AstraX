@echo off
:: ============================================================================
:: AstraX Installer for Windows
:: Version: 1.0.0
:: ============================================================================

setlocal enabledelayedexpansion

:: Configuration
set APPNAME=AstraX
set VERSION=1.0.0
set INSTALL_DIR=%ProgramFiles%\%APPNAME%

echo.
echo =========================================
echo   %APPNAME% v%VERSION% Installer
echo =========================================
echo.

:: Check if running as admin
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This installer must be run as Administrator.
    echo Right-click and select "Run as administrator"
    pause
    exit /b 1
)

echo Installation Directory: %INSTALL_DIR%
echo.

:: Get script directory
set SCRIPT_DIR=%~dp0
set SOURCE_DIR=%SCRIPT_DIR%

:: Check if source files exist
if not exist "%SOURCE_DIR%astrax.exe" (
    :: Check if we're in build directory
    if exist "%SCRIPT_DIR%build\bin\astrax.exe" (
        set SOURCE_DIR=%SCRIPT_DIR%build\bin\
    ) else if exist "%SCRIPT_DIR%build\bin\Release\astrax.exe" (
        set SOURCE_DIR=%SCRIPT_DIR%build\bin\Release\
    ) else (
        echo ERROR: astrax.exe not found.
        echo Please build the project first or run from release directory.
        pause
        exit /b 1
    )
)

echo Found AstraX at: %SOURCE_DIR%
echo.

:: Create install directory
echo [1/4] Creating installation directory...
if not exist "%INSTALL_DIR%" (
    mkdir "%INSTALL_DIR%"
    if !errorLevel! neq 0 (
        echo ERROR: Could not create installation directory.
        pause
        exit /b 1
    )
)

:: Copy files
echo [2/4] Copying files...
copy /Y "%SOURCE_DIR%astrax.exe" "%INSTALL_DIR%\" >nul
if exist "%SCRIPT_DIR%README.md" copy /Y "%SCRIPT_DIR%README.md" "%INSTALL_DIR%\" >nul
if exist "%SCRIPT_DIR%LICENSE" copy /Y "%SCRIPT_DIR%LICENSE" "%INSTALL_DIR%\" >nul
if exist "%SCRIPT_DIR%config" xcopy /Y /E /I "%SCRIPT_DIR%config" "%INSTALL_DIR%\config" >nul

:: Add to PATH
echo [3/4] Adding to system PATH...
for /f "skip=2 tokens=3*" %%a in ('reg query "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path') do set CURRENT_PATH=%%a%%b

echo !CURRENT_PATH! | findstr /C:"%INSTALL_DIR%" >nul
if !errorLevel! neq 0 (
    setx /M PATH "!CURRENT_PATH!;%INSTALL_DIR%"
    echo Added %INSTALL_DIR% to system PATH
) else (
    echo AstraX already in PATH
)

:: Create Start Menu shortcut
echo [4/4] Creating Start Menu entry...
set STARTMENU=%ProgramData%\Microsoft\Windows\Start Menu\Programs
if not exist "%STARTMENU%\%APPNAME%" mkdir "%STARTMENU%\%APPNAME%"

:: Create uninstaller
echo @echo off > "%INSTALL_DIR%\uninstall.bat"
echo echo Uninstalling %APPNAME%... >> "%INSTALL_DIR%\uninstall.bat"
echo rmdir /S /Q "%INSTALL_DIR%" >> "%INSTALL_DIR%\uninstall.bat"
echo echo %APPNAME% has been uninstalled. >> "%INSTALL_DIR%\uninstall.bat"
echo pause >> "%INSTALL_DIR%\uninstall.bat"

echo.
echo =========================================
echo   Installation Complete!
echo =========================================
echo.
echo AstraX has been installed to: %INSTALL_DIR%
echo.
echo You can now use AstraX from any terminal:
echo   astrax myfile.cpp
echo.
echo To uninstall, run: %INSTALL_DIR%\uninstall.bat
echo.
pause
