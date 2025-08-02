# USB Isochronous Transfer Simulation

A pure USB isochronous transfer simulator that mimics real USB Audio Class behavior with accurate timing and callback-based packet handling.

## 🎯 Project Overview

This project implements a **USB isochronous transfer simulation** that behaves like real USB Audio Class devices:

- **125μs microframes** (High Speed USB timing)
- **1ms audio frames** (8 microframes per frame)
- **Callback-based packet handling** (like libusb)
- **Efficient streaming** with error handling
- **No audio format dependencies** - pure USB protocol

## 📁 Project Structure

```
ds_kcobain/
├── include/kcobain/
│   └── usb_iso_transfer.h    # USB transfer simulation header
├── src/
│   ├── main.cpp              # Demo application with usage examples
│   └── usb_iso_transfer.cpp  # USB transfer implementation
├── external/
│   └── miniaudio.h           # (Not used - pure USB simulation)
├── build/                    # Build output (generated)
├── CMakeLists.txt            # Build configuration
└── README.md                # This file
```

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
./bin/kcobain
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

# The binary will be in build_android/bin/kcobain
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

### USB Protocol Simulation

- **High Speed USB (480 Mbps)** timing simulation
- **125μs microframe accuracy** using high-resolution clock
- **1ms frame processing** (8 microframes per audio frame)
- **Isochronous transfer** with proper packet structure

### libusb-Style Interface

```cpp
// Configure endpoint
transfer.configureEndpoint(0x01, 384, 1);  // Endpoint, max packet size, interval

// Set callback
transfer.setPacketCallback(packetCallback);

// Start transfer
transfer.startTransfer();

// Submit data
transfer.submitPacket(data);
transfer.submitFrame(frame);

// Get statistics
transfer.getTransferRate();  // packets per second
transfer.getPacketsSent();
transfer.getErrors();
```

### Error Handling

- **Underrun detection** - when no data is available
- **Overrun protection** - buffer full conditions
- **Timeout handling** - timing violations
- **Error reporting** - detailed error codes and statistics

## 📊 Usage Examples

### Basic Transfer

```cpp
void packetCallback(const UsbIsoTransfer::IsoPacket& packet) {
    if (packet.valid) {
        // Process valid packet data
        std::cout << "Packet: " << packet.data.size() << " bytes" << std::endl;
    } else {
        // Handle error
        std::cout << "Error: " << packet.errorCode << std::endl;
    }
}

UsbIsoTransfer transfer;
transfer.configureEndpoint(0x01, 384, 1);
transfer.setPacketCallback(packetCallback);
transfer.startTransfer();

// Submit data
std::vector<uint8_t> data(384, 0xAA);
transfer.submitPacket(data);
```

### Frame-Based Transfer

```cpp
// Submit multiple packets at once (1ms frame)
std::vector<std::vector<uint8_t> > frame;
for (int i = 0; i < 8; ++i) {
    frame.push_back(generateData(192));
}
transfer.submitFrame(frame);
```

### Error Handling

```cpp
// Check for errors
if (transfer.getErrors() > 0) {
    std::cout << "Last error: " << transfer.getLastError() << std::endl;
    transfer.clearErrors();
}
```

## 🔧 USB Timing Details

### Microframe Timing

- **Base unit**: 125μs (High Speed USB)
- **Audio polling**: 1ms intervals (bInterval = 1)
- **Frame structure**: 8 microframes per 1ms frame

### Packet Processing

```
Time:    0ms    1ms    2ms    3ms
         |      |      |      |
Micro:   0-7    8-15   16-23  24-31
         |      |      |      |
Frame:   0      1      2      3
```

### Bandwidth Calculation

```
Bytes per second = SampleRate × Channels × (BitDepth / 8)
Bytes per millisecond = Bytes per second / 1000
Packets per millisecond = 8 (microframes)
Bytes per packet = Bytes per millisecond / 8
```

## 🧪 Testing Scenarios

The demo application includes:

1. **Basic Transfer** - Single packet submission
2. **Frame Transfer** - Multiple packets per frame
3. **Error Handling** - Underrun simulation
4. **Statistics** - Performance monitoring

## 📈 Performance Characteristics

- **Latency**: ~125μs (microframe timing)
- **Throughput**: Up to 480 Mbps (theoretical)
- **Accuracy**: High-resolution clock timing
- **Efficiency**: No busy waiting, proper synchronization

## 🔍 Troubleshooting

### Common Issues

1. **High error rates**: Check data submission timing
2. **Buffer full errors**: Reduce submission rate
3. **Timing issues**: Verify system clock accuracy

### Debug Information

The application provides detailed output:
- Packet processing details
- Timing information
- Error statistics
- Performance metrics

## 🤝 Contributing

### Development Guidelines

1. **Maintain USB timing accuracy**
2. **Follow libusb patterns**
3. **Add comprehensive error handling**
4. **Test with various packet sizes**

### Adding Features

- **New transfer types**: Bulk, interrupt transfers
- **Multiple endpoints**: Concurrent transfers
- **Advanced timing**: SuperSpeed USB simulation
- **Performance optimization**: Lock-free buffers

## 📄 License

This project is provided as-is for educational and development purposes.

## 🙏 Acknowledgments

- **USB-IF**: USB Audio Class specification
- **libusb**: Reference implementation patterns
- **USB Audio Class**: Protocol documentation

---

**Note**: This is a simulation project for USB protocol development and testing. It does not require actual USB hardware.
