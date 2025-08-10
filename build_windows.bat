@echo off
echo ========================================
echo KCobain Windows Build Script
echo ========================================
echo.

REM Check if GCC is available
gcc --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: GCC not found in PATH
    echo Please install MinGW-w64 or MSYS2 with GCC 6.3.0
    echo.
    echo Download from: https://www.mingw-w64.org/
    echo or install MSYS2 from: https://www.msys2.org/
    pause
    exit /b 1
)

echo GCC version:
gcc --version
echo.

REM Check if CMake is available
cmake --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake not found in PATH
    echo Please install CMake 3.10 or later
    echo Download from: https://cmake.org/download/
    pause
    exit /b 1
)

echo CMake version:
cmake --version
echo.

REM Create build directory
if not exist "build_windows" mkdir build_windows
cd build_windows

REM Copy Windows CMakeLists to standard name
copy ..\CMakeLists_Windows.txt ..\CMakeLists.txt >nul

echo Creating build files...
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..

if errorlevel 1 (
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)

echo.
echo Building project...
cmake --build . --config Release

if errorlevel 1 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo Executable location: build_windows\bin\kcobain.exe
echo.
echo To run the program:
echo   cd build_windows\bin
echo   kcobain.exe <audio_file.wav>
echo.

pause 