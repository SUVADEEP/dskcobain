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


# Decoder Node Graph Architecture

This directory contains a flexible, modular audio decoder implementation using a node graph architecture. The system allows you to build complex audio processing pipelines by connecting different types of nodes together.

## 🏗️ Architecture Overview

The decoder system is built around three main components:

1. **DecoderNode** - Base interface for all processing nodes
2. **DecoderGraph** - Manages the network of connected nodes
3. **Concrete Nodes** - Specific implementations for different audio operations

## 📁 File Structure

```
src/decoder/
├── decoder_node.h              # Base node interface
├── decoder_graph.h             # Graph management
├── decoder_graph.cpp           # Graph implementation
├── decoder_example.cpp         # Usage examples
├── nodes/                      # Concrete node implementations
│   ├── file_source_node.h      # Audio file input
│   ├── audio_processor_node.h  # Audio processing operations
│   └── usb_sink_node.h         # USB audio output
└── README.md                   # This file
```

## 🎯 Key Features

### **Modular Design**
- Each node has a single responsibility
- Easy to add new node types
- Nodes can be connected in any valid configuration

### **Flexible Processing**
- Support for various audio operations (gain, filter, resample, etc.)
- Configurable parameters for each node
- Real-time and batch processing capabilities

### **Graph Management**
- Automatic topological sorting for execution order
- Cycle detection to prevent infinite loops
- Connection validation and error handling

### **USB Integration**
- Direct integration with USB Audio Class
- Precise timing control (125μs microframes)
- Statistics and monitoring capabilities

## 🔧 Node Types

### **FileSourceNode**
- Reads audio files (FLAC, WAV, etc.)
- Supports seeking and frame-by-frame reading
- Configurable buffer sizes

### **AudioProcessorNode**
- **Gain**: Apply volume changes
- **Filter**: Apply various filters (lowpass, highpass, etc.)
- **Resample**: Change sample rate
- **Normalize**: Normalize audio levels
- **Fade**: Apply fade in/out effects
- **Reverse**: Reverse audio playback

### **UsbSinkNode**
- Outputs audio to USB Audio Class devices
- Configurable endpoints and packet sizes
- Precise timing control
- Statistics tracking (underruns, overruns)

## 🚀 Usage Examples

### **Basic Audio Processing Chain**
```cpp
// Create graph
DecoderGraph graph;

// Add nodes
auto fileSource = std::make_shared<FileSourceNode>("AudioFile");
auto processor = std::make_shared<AudioProcessorNode>("GainProcessor");
auto usbSink = std::make_shared<UsbSinkNode>("UsbOutput");

graph.addNode(fileSource);
graph.addNode(processor);
graph.addNode(usbSink);

// Connect nodes
graph.connect("AudioFile", "audio_out", "GainProcessor", "audio_in");
graph.connect("GainProcessor", "audio_out", "UsbOutput", "audio_in");

// Configure and run
fileSource->setParameter("filename", "audio.flac");
processor->setParameter("operation", "gain");
processor->setParameter("gain_value", "1.5");

graph.initialize();
graph.process(input, output);
graph.shutdown();
```

### **Complex Processing Pipeline**
```cpp
// File -> Gain -> Filter -> Resample -> USB
graph.connect("AudioFile", "audio_out", "GainProcessor", "audio_in");
graph.connect("GainProcessor", "audio_out", "FilterProcessor", "audio_in");
graph.connect("FilterProcessor", "audio_out", "ResampleProcessor", "audio_in");
graph.connect("ResampleProcessor", "audio_out", "UsbOutput", "audio_in");
```

## 🔗 Node Connections

### **Connection Types**
- **AUDIO**: Audio data flow between nodes
- **CONTROL**: Control parameter connections
- **METADATA**: Metadata information flow

### **Port System**
Each node defines input and output ports:
- **FileSourceNode**: `audio_out`
- **AudioProcessorNode**: `audio_in`, `audio_out`
- **UsbSinkNode**: `audio_in`

### **Validation**
- Port existence validation
- Type compatibility checking
- Cycle detection

## 📊 Graph Analysis

### **Topological Sorting**
The graph automatically determines the correct execution order:
```cpp
auto order = graph.getTopologicalOrder();
// Returns: ["AudioFile", "GainProcessor", "UsbOutput"]
```

### **Graph Statistics**
```cpp
size_t nodeCount = graph.getNodeCount();
size_t connectionCount = graph.getConnectionCount();
bool isAcyclic = graph.isAcyclic();
```

### **Source and Sink Detection**
```cpp
auto sources = graph.getSourceNodes();  // Nodes with no inputs
auto sinks = graph.getSinkNodes();      // Nodes with no outputs
```

## 🎵 Audio Processing

### **AudioBuffer Structure**
```cpp
struct AudioBuffer {
    std::vector<float> data;     // Audio samples
    uint32_t sampleRate;         // Sample rate (Hz)
    uint32_t channels;           // Number of channels
    uint32_t frameCount;         // Number of frames
};
```

### **Processing Flow**
1. **Input**: AudioBuffer with input data
2. **Processing**: Each node processes the data
3. **Output**: AudioBuffer with processed data

### **Real-time Considerations**
- Lock-free operations where possible
- Minimal latency processing
- Buffer management for smooth playback

## 🔧 Configuration

### **Node Parameters**
Each node supports configurable parameters:
```cpp
node->setParameter("operation", "gain");
node->setParameter("gain_value", "1.5");
node->setParameter("filter_type", "lowpass");
```

### **USB Configuration**
```cpp
usbSink->setParameter("endpoint", "0x01");
usbSink->setParameter("packet_size", "384");
usbSink->setParameter("microframe_interval", "125");
```

## 🧪 Testing and Debugging

### **Graph Validation**
```cpp
if (!graph.isAcyclic()) {
    LOG_ERROR("Graph contains cycles!");
}

auto order = graph.getTopologicalOrder();
if (order.empty()) {
    LOG_ERROR("Cannot determine execution order");
}
```

### **Node State Monitoring**
```cpp
if (!node->isInitialized()) {
    LOG_ERROR("Node not initialized");
}

if (!node->canProcess()) {
    LOG_WARN("Node cannot process");
}
```

## 🔮 Future Enhancements

### **Planned Features**
- **More Node Types**: Effects, analysis, format conversion
- **Dynamic Graph Loading**: Load graphs from configuration files
- **Parallel Processing**: Multi-threaded node execution
- **Visual Editor**: GUI for building graphs
- **Plugin System**: Load custom nodes at runtime

### **Performance Optimizations**
- **SIMD Processing**: Vectorized audio operations
- **Memory Pooling**: Efficient buffer management
- **JIT Compilation**: Runtime optimization of processing chains

## 🤝 Contributing

### **Adding New Nodes**
1. Inherit from `DecoderNode`
2. Implement all virtual methods
3. Register with `DecoderNodeFactory`
4. Add to the build system

### **Node Guidelines**
- Keep nodes focused on single responsibility
- Provide comprehensive parameter validation
- Include proper error handling
- Document all parameters and behaviors

---

**Note**: This decoder system is designed to be extensible and maintainable. The node graph architecture makes it easy to add new processing capabilities while maintaining clean separation of concerns. 