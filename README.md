# USB Audio Class Microframe Simulator

A real-time USB Audio Class simulation that mimics actual USB isochronous transfer behavior with precise 125μs microframe timing and lock-free ring buffers.

## 🎯 Project Overview

This project implements a **USB Audio Class microframe simulation** that accurately reproduces real USB audio streaming behavior:

- **125μs microframes** (High Speed USB timing)
- **384 bytes per microframe** (USB Audio Class specification)
- **Lock-free ring buffers** using miniaudio's `ma_rb` API
- **Producer-Consumer pattern** with separate threads
- **Cross-platform logging** (Android + standard C++)
- **Modular architecture** with separate core and USB libraries

## 📁 Project Structure

```
ds_kcobain/
├── include/kcobain/
│   └── logger.h              # Cross-platform logging system
├── src/
│   ├── main.cpp              # Main application with buffer initialization
│   ├── logger.cpp            # Logger implementation
│   ├── miniaudio_impl.cpp    # Miniaudio implementation
│   └── core/                 # Core audio components
│       ├── audio_rb_controller.h/cpp    # Ring buffer management
│       ├── iaudio_producer.h            # Producer interface
│       ├── iaudio_consumer.h            # Consumer interface
│       ├── usb_audio_producer.h/cpp     # USB audio producer
│       ├── usb_audio_consumer.h/cpp     # USB audio consumer
│       └── usb_audio_orchestrator.h/cpp # Pipeline orchestration
├── external/
│   └── miniaudio.h           # Miniaudio library (single header)
├── build/                    # Build output (generated)
├── CMakeLists.txt            # Build configuration with modular libraries
└── README.md                # This file
```

## 🏗️ Architecture

### **Modular Library Design**

The project uses a modular architecture with separate static libraries:

```
kcobain_core (Static Library)
├── logger.cpp
├── miniaudio_impl.cpp
└── audio_rb_controller.cpp

kcobain_usb (Static Library)
├── usb_audio_producer.cpp
├── usb_audio_consumer.cpp
└── usb_audio_orchestrator.cpp

kcobain (Executable)
└── main.cpp
```

### **Component Responsibilities**

1. **`audio_rb_controller`**: Manages miniaudio's `ma_rb` ring buffer
2. **`usb_audio_producer`**: Generates audio data and writes to buffer
3. **`usb_audio_consumer`**: Reads from buffer at USB timing
4. **`usb_audio_orchestrator`**: Coordinates producer and consumer threads
5. **`main.cpp`**: Initializes buffer and starts the simulation

## 🚀 Building the Project

### Prerequisites

- **CMake** (version 3.10 or higher)
- **C++11** compatible compiler
- **Android NDK** (for Android builds)

### Build Instructions

#### **Standard Build (Linux/macOS/Windows)**

```bash
# Clone the repository
git clone <repository-url>
cd ds_kcobain

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make

# Run the application
./kcobain
```

#### **Custom Logger Tag Build**

```bash
# Build with custom logger tag
cmake -DLOGGER_TAG="MyAudioApp" ..
make

# Build with debug logger tag
cmake -DLOGGER_TAG="KcobainDebug" -DCMAKE_BUILD_TYPE=Debug ..
make
```

#### **Installation Build**

```bash
# Install to system (requires sudo on Linux/macOS)
cmake ..
make
sudo make install

# Install to custom location
cmake -DCMAKE_INSTALL_PREFIX=/opt/kcobain ..
make
make install
```

### **Android Build**

#### **Prerequisites for Android**

1. **Install Android NDK** (version 21 or higher)
2. **Set ANDROID_NDK environment variable**:
   ```bash
   export ANDROID_NDK=/path/to/android-ndk
   ```

#### **Android Build Instructions**

```bash
# Create Android build directory
mkdir build_android && cd build_android

# Configure for Android ARM64
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-33 \
      -DLOGGER_TAG="KcobainAndroid" \
      ..

# Build for Android
make

# The binary will be in build_android/kcobain
```

#### **Android Build Script**

Use the provided build script for convenience:

```bash
# Make script executable
chmod +x build_android.sh

# Build for Android
./build_android.sh

# The binary will be in build_android_arm64-v8a/bin/kcobain
```

#### **Multiple Android Architectures**

```bash
# ARM64 (64-bit)
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-33 \
      ..

# ARM32 (32-bit)
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=armeabi-v7a \
      -DANDROID_PLATFORM=android-33 \
      ..

# x86_64 (for emulator)
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=x86_64 \
      -DANDROID_PLATFORM=android-33 \
      ..
```

## 🎵 Features

### USB Audio Class Simulation

- **125μs microframe timing** using high-resolution clock
- **384 bytes per microframe** (USB Audio Class specification)
- **Lock-free ring buffers** for efficient inter-thread communication
- **Producer-Consumer pattern** with separate threads
- **Underrun/Overrun detection** and statistics

### Core Components

#### **Ring Buffer Controller**
```cpp
kcobain::audio_rb_controller buffer_controller;
buffer_controller.initialize(30720);  // 80 microframes × 384 bytes
```

#### **USB Audio Producer**
- Generates 32-bit float audio data
- Writes to ring buffer at maximum speed
- Tracks overrun conditions

#### **USB Audio Consumer**
- Reads from ring buffer every 125μs
- Simulates USB microframe consumption
- Detects underrun conditions

#### **Orchestrator**
- Manages producer and consumer threads
- Provides statistics and monitoring
- Handles start/stop operations

### Cross-Platform Logging

```cpp
LOG_INFO("🎵 USB Audio Class Microframe Simulator");
LOG_WARN("USB underrun: expected 384 bytes, got " + std::to_string(bytesAcquired));
LOG_ERROR("Failed to initialize buffer controller");
```

## 📊 Usage Examples

### Basic Simulation

```cpp
#include "core/audio_rb_controller.h"
#include "core/usb_audio_orchestrator.h"

int main() {
    // Initialize buffer controller in main
    kcobain::audio_rb_controller buffer_controller;
    size_t bufferSizeBytes = 30720;  // 80 microframes × 384 bytes
    
    if (!buffer_controller.initialize(bufferSizeBytes)) {
        LOG_ERROR("Failed to initialize buffer controller");
        return 1;
    }
    
    // Create orchestrator with external buffer controller
    kcobain::usb_audio_orchestrator orchestrator(&buffer_controller, 384);
    orchestrator.startStreaming();
    
    // Run simulation
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    orchestrator.stopStreaming();
    orchestrator.printStatistics();
    
    return 0;
}
```

### Custom Buffer Configuration

```cpp
// Different buffer sizes for different use cases
size_t smallBuffer = 3072;   // 8 microframes (1ms)
size_t mediumBuffer = 30720; // 80 microframes (10ms)
size_t largeBuffer = 122880; // 320 microframes (40ms)

kcobain::audio_rb_controller buffer_controller;
buffer_controller.initialize(mediumBuffer);
```

### Statistics Monitoring

```cpp
// Get detailed statistics
orchestrator.printStatistics();

// Output includes:
// - Total frames produced/consumed
// - Overrun/underrun counts
// - Timing accuracy metrics
```

## 🔧 USB Timing Details

### Microframe Timing

- **Base unit**: 125μs (High Speed USB)
- **Audio polling**: 1ms intervals (8 microframes per frame)
- **Packet size**: 384 bytes per microframe

### Buffer Sizing

```
Buffer Size = Microframes × Bytes per Microframe
Example: 80 microframes × 384 bytes = 30,720 bytes
```

### Audio Data Calculation

```
Bytes per microframe = (SampleRate × 125μs × BitDepth × Channels) / 8
Example: (96000 × 0.000125 × 32 × 2) / 8 = 96 bytes
```

## 🧪 Testing Scenarios

The simulation includes:

1. **Basic Microframe Transfer** - 384 bytes every 125μs
2. **Buffer Underrun Detection** - When consumer starves
3. **Buffer Overrun Detection** - When producer writes too fast
4. **Timing Accuracy** - High-resolution clock validation
5. **Statistics Collection** - Performance monitoring

## 📈 Performance Characteristics

- **Latency**: 125μs (microframe timing)
- **Throughput**: 384 bytes per microframe
- **Buffer Efficiency**: Lock-free ring buffer operations
- **Thread Safety**: Single producer, single consumer
- **Timing Accuracy**: High-resolution clock synchronization

## 🔍 Troubleshooting

### Common Issues

1. **High underrun rates**: Producer not keeping up with consumer
2. **High overrun rates**: Consumer not reading fast enough
3. **Timing drift**: System clock accuracy issues
4. **Buffer initialization failures**: Insufficient memory

### Debug Information

The application provides detailed logging:
- Buffer initialization status
- Producer/consumer timing
- Underrun/overrun detection
- Performance statistics

## 🤝 Contributing

### Development Guidelines

1. **Maintain USB timing accuracy**
2. **Follow miniaudio naming conventions**
3. **Use lock-free patterns where possible**
4. **Add comprehensive error handling**
5. **Test with various buffer sizes**

### Adding Features

- **New audio formats**: Different sample rates/bit depths
- **Multiple endpoints**: Concurrent audio streams
- **Advanced timing**: SuperSpeed USB simulation
- **Performance optimization**: SIMD operations

## 📄 License

This project is provided as-is for educational and development purposes.

## 🙏 Acknowledgments

- **USB-IF**: USB Audio Class specification
- **Miniaudio**: Lock-free ring buffer implementation
- **USB Audio Class**: Protocol documentation

---

**Note**: This is a simulation project for USB Audio Class development and testing. It accurately reproduces USB timing and behavior without requiring actual USB hardware.
