@echo off
REM Build script for Switch-Case Compiler
REM This script helps set up and build the project on Windows

echo ======================================
echo Switch-Case Compiler Build Script
echo ======================================
echo.

REM Check if build directory exists
if not exist build (
    echo Creating build directory...
    mkdir build
)

cd build

REM Configure with CMake
echo.
echo Configuring project with CMake...
echo.

REM Try to find Qt automatically (adjust paths as needed)
REM Common Qt installation paths:
REM C:\Qt\6.5.0\msvc2019_64
REM C:\Qt\5.15.2\msvc2019_64

REM You may need to modify this path to match your Qt installation
set QT_PATH=C:\Qt\6.8.3\msvc2022_64

if exist "%QT_PATH%" (
    echo Found Qt at: %QT_PATH%
    cmake -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH="%QT_PATH%" ..
) else (
    echo Warning: Qt path not found at %QT_PATH%
    echo Trying CMake without explicit Qt path...
    cmake -G "Visual Studio 17 2022" ..
)

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: CMake configuration failed!
    echo.
    echo Please ensure:
    echo  1. CMake is installed and in PATH
    echo  2. Qt is installed
    echo  3. Visual Studio is installed
    echo.
    echo You may need to edit this script to set the correct QT_PATH
    pause
    exit /b 1
)

echo.
echo ======================================
echo Configuration successful!
echo ======================================
echo.
echo Building project...
echo.

REM Build the project
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo ======================================
echo Build successful!
echo ======================================
echo.
echo Deploying Qt runtime files...

if exist "%QT_PATH%\bin\windeployqt.exe" (
    "%QT_PATH%\bin\windeployqt.exe" --release --compiler-runtime Release\SwitchCaseCompiler.exe
    if %ERRORLEVEL% NEQ 0 (
        echo Warning: windeployqt reported an issue. The app may still need Qt DLLs in PATH.
    ) else (
        echo Qt runtime deployment completed.
    )
) else (
    echo Warning: windeployqt not found. Skipping runtime deployment.
)

echo.
echo Executable location: build\Release\SwitchCaseCompiler.exe
echo.
echo To run the application:
echo   cd Release
echo   SwitchCaseCompiler.exe
echo.
echo Note: You may need to copy Qt DLLs using windeployqt:
echo   windeployqt.exe Release\SwitchCaseCompiler.exe
echo.

pause
