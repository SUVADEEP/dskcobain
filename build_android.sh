#!/bin/bash

# Android NDK Build Script for USB Isochronous Transfer Simulation
# Usage: ./build_android.sh [NDK_VERSION] [API_LEVEL] [ABI]

# # Build for arm64-v8a (default)
# ./build_android.sh

# # Build for armeabi-v7a
# ./build_android.sh 27.0.12077973 33 armeabi-v7a

# # Build for specific NDK version
# ./build_android.sh 29.0.13113456 33 arm64-v8a

# Default values
NDK_VERSION=${1:-"27.0.12077973"}
API_LEVEL=${2:-"33"}
ABI=${3:-"arm64-v8a"}

# Common NDK paths
NDK_PATHS=(
    "$HOME/Library/Android/sdk/ndk/$NDK_VERSION"
    "$HOME/Android/Sdk/ndk/$NDK_VERSION"
    "/opt/android-ndk-$NDK_VERSION"
    "/usr/local/android-ndk-$NDK_VERSION"
)

# Find NDK path
NDK_PATH=""
for path in "${NDK_PATHS[@]}"; do
    if [ -d "$path" ]; then
        NDK_PATH="$path"
        break
    fi
done

if [ -z "$NDK_PATH" ]; then
    echo "❌ Android NDK not found!"
    echo "Searched in:"
    for path in "${NDK_PATHS[@]}"; do
        echo "  $path"
    done
    echo ""
    echo "Please install Android NDK or specify the correct path."
    echo "You can download it from: https://developer.android.com/ndk"
    exit 1
fi

echo "🎚️  Building USB Isochronous Transfer Simulation for Android"
echo "=========================================================="
echo "NDK Version: $NDK_VERSION"
echo "API Level: $API_LEVEL"
echo "ABI: $ABI"
echo "NDK Path: $NDK_PATH"
echo ""

# Create build directory
BUILD_DIR="build_android_${ABI}"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo "🔧 Configuring CMake..."
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$NDK_PATH/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="$ABI" \
    -DANDROID_PLATFORM="android-$API_LEVEL" \
    -DANDROID_NDK="$NDK_PATH" \
    -DANDROID_NATIVE_API_LEVEL="$API_LEVEL" \
    -DCMAKE_BUILD_TYPE=Release \
    -DANDROID_PLATFORM=android-33 \
    -DCMAKE_ANDROID_ARCH_ABI="arm64-v8a;armeabi-v7a"

if [ $? -ne 0 ]; then
    echo "❌ CMake configuration failed!"
    exit 1
fi

# Build
echo "🔨 Building..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

echo ""
echo "✅ Build completed successfully!"
echo "📱 Executable: $BUILD_DIR/bin/kcobain"
echo ""
echo "📊 Build Information:"
echo "  - Target: Android $API_LEVEL"
echo "  - Architecture: $ABI"
echo "  - NDK: $NDK_VERSION"
echo "  - Build Type: Release"
echo ""
echo "🚀 To run on Android device:"
echo "  adb push $BUILD_DIR/bin/kcobain /data/local/tmp/"
echo "  adb shell chmod +x /data/local/tmp/kcobain"
echo "  adb shell /data/local/tmp/kcobain" 