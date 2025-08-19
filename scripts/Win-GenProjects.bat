@echo off

:: ---------------------------------------
:: Check for Python
:: ---------------------------------------
echo === Checking for Python launcher (py) ===
where py >nul 2>nul
if %errorlevel% neq 0 (
    echo Python 3.13+ is required. Please install it.
    pause
    exit /b 1
)
py --version

:: ---------------------------------------
:: Check for CMake
:: ---------------------------------------
echo === Checking for CMake ===
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo CMake 3.30+ is required. Please install it.
    pause
    exit /b 1
)
cmake --version

:: ---------------------------------------
:: Check for Premake
:: ---------------------------------------
echo === Checking for Premake ===
where premake5.exe >nul 2>nul
if %errorlevel% neq 0 (
    echo Premake 5 is required. Please install it.
    pause
    exit /b 1
)
premake5.exe --version

echo === All prerequisites are installed ===

:: ---------------------------------------
:: Sync Shaderc dependencies
:: ---------------------------------------
echo === Getting shaderc dependencies ===
py ..\RodEngine\vendor\shaderc\utils\git-sync-deps

:: ---------------------------------------
:: Run Premake
:: ---------------------------------------
echo === Running Premake ===
pushd ..\
call premake5.exe vs2022
popd

echo === Setup complete ===
pause
