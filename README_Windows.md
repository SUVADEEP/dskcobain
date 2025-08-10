# KCobain Windows Build Guide

This guide explains how to build KCobain on Windows using GCC 6.3.0.

## Prerequisites

### 1. GCC 6.3.0 Compiler
You need GCC 6.3.0 or compatible version. Recommended options:

**Option A: MinGW-w64**
- Download from: https://www.mingw-w64.org/
- Install with GCC 6.3.0 or later
- Add to PATH: `C:\mingw64\bin`

**Option B: MSYS2 (Recommended)**
- Download from: https://www.msys2.org/
- Install and update packages
- Install GCC: `pacman -S mingw-w64-x86_64-gcc`
- Add to PATH: `C:\msys64\mingw64\bin`

### 2. CMake 3.10 or later
- Download from: https://cmake.org/download/
- Install and add to PATH
- Verify: `cmake --version`

### 3. Make sure both are in PATH
```cmd
gcc --version
cmake --version
```

## Quick Build (Recommended)

1. **Run the build script:**
   ```cmd
   build_windows.bat
   ```

2. **The script will:**
   - Check for required tools
   - Create build directory
   - Configure with CMake
   - Build the project
   - Show executable location

3. **Run the program:**
   ```cmd
   cd build_windows\bin
   kcobain.exe assets\Sample_BeeMoved_96kHz24bit.flac
   ```

## Manual Build

If you prefer manual build:

1. **Create build directory:**
   ```cmd
   mkdir build_windows
   cd build_windows
   ```

2. **Configure with CMake:**
   ```cmd
   cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -f ../CMakeLists_Windows.txt ..
   ```

3. **Build the project:**
   ```cmd
   cmake --build . --config Release
   ```

4. **Run the program:**
   ```cmd
   cd bin
   kcobain.exe <audio_file>
   ```

## Build Types

### Release Build (Default)
```cmd
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -f ../CMakeLists_Windows.txt ..
```

### Debug Build
```cmd
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -f ../CMakeLists_Windows.txt ..
```

### RelWithDebInfo Build
```cmd
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo -f ../CMakeLists_Windows.txt ..
```

## Troubleshooting

### Common Issues

1. **"GCC not found"**
   - Make sure MinGW-w64 or MSYS2 is installed
   - Add `C:\mingw64\bin` or `C:\msys64\mingw64\bin` to PATH
   - Restart command prompt after PATH changes

2. **"CMake not found"**
   - Install CMake from https://cmake.org/download/
   - Add CMake to PATH during installation

3. **"Make not found"**
   - Install MinGW-w64 with MSYS2: `pacman -S mingw-w64-x86_64-make`
   - Or use `cmake --build .` instead of `make`

4. **Compilation errors**
   - Make sure you're using GCC 6.3.0 or later
   - Check that all source files are present
   - Verify miniaudio.h is in the external directory

5. **Linking errors**
   - Make sure Windows system libraries are available
   - Try running as Administrator if needed

### Build Output

Successful build creates:
```
build_windows/
├── bin/
│   └── kcobain.exe          # Main executable
├── lib/
│   ├── libkcobain_core.a    # Core library
│   └── libkcobain_usb.a     # USB library
└── [other build files]
```

## Audio File Support

The program supports:
- **WAV** - Uncompressed PCM
- **FLAC** - Lossless compression
- **MP3** - Lossy compression (if supported by miniaudio)
- **OGG** - Open container format

## Usage

```cmd
kcobain.exe <audio_file>
```

**Controls:**
- **U/D** - Volume up/down
- **E** - Toggle EQ
- **L/M/H** - Adjust low/mid/high EQ
- **R** - Reset EQ to flat
- **S** - Seek to position
- **I** - Show current position
- **F/B** - Forward/backward 10 seconds
- **Q** - Quit

## Development

### Adding New Files
1. Add source files to appropriate directories
2. Update `CMakeLists_Windows.txt` with new files
3. Rebuild the project

### Debugging
1. Use Debug build type
2. Run with debugger: `gdb bin/kcobain.exe`
3. Set breakpoints and examine variables

### Performance
- Release builds are optimized with `-O3`
- Debug builds include symbols with `-g -O0`
- Profile with tools like `gprof` or `perf`

## Platform Notes

- **Windows Subsystem**: Console application (not GUI)
- **Audio Backend**: miniaudio (WASAPI on Windows)
- **Threading**: Native Windows threads
- **File I/O**: Standard C++ file operations
- **Memory**: Standard C++ memory management

## Support

For issues specific to Windows builds:
1. Check this README first
2. Verify GCC and CMake versions
3. Check PATH environment variable
4. Try clean rebuild: delete `build_windows` and rebuild 