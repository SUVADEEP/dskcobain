# KC Audio Processor

A flexible, real-time audio processing pipeline built on miniaudio's node graph system.

## 🎯 Overview

The KC Audio Processor provides a modular audio processing architecture that allows you to build complex audio processing chains using a node-based approach. It leverages miniaudio's built-in node graph system for efficient, real-time audio processing.

## 🏗️ Architecture

```
Decoder Node → Gain Node → Filter Node → Ring Buffer → USB Consumer
```

### Core Components

1. **kc_node_graph** - Main graph manager and orchestrator
2. **kc_decoder_node** - File decoding node (FLAC, WAV, MP3, OGG)
3. **kc_gain_node** - Volume control node
4. **kc_filter_node** - Audio filtering node (lowpass, highpass, etc.)

## 📁 File Structure

```
src/audio_processor/
├── kc_node_graph.h              # Main node graph interface
├── kc_node_graph.cpp            # Node graph implementation
├── node_graph_example.cpp       # Usage example
├── decoder/                     # Decoder node implementation
│   ├── kc_decoder_node.h        # Decoder node interface
│   ├── kc_decoder_node.cpp      # Decoder node implementation
│   └── decoder_example.cpp      # Decoder usage example
├── nodes/                       # Processing nodes
│   ├── kc_gain_node.h           # Gain node interface
│   ├── kc_gain_node.cpp         # Gain node implementation
│   ├── kc_filter_node.h         # Filter node interface
│   └── kc_filter_node.cpp       # Filter node implementation
└── README.md                    # This file
```

## 🚀 Quick Start

### Basic Usage

```cpp
#include "kc_node_graph.h"

using namespace kcobain::audio_processor;

// Create configuration
kc_node_graph_config config;
config.channels = 2;           // Stereo
config.sampleRate = 96000;     // 96kHz
config.format = ma_format_f32; // 32-bit float
config.bufferSize = 1024;      // Buffer size

// Initialize graph
kc_node_graph graph;
graph.initialize(config);

// Add nodes
kc_decoder_node* decoder = nullptr;
graph.add_decoder_node("audio.flac", &decoder);

kc_gain_node* gain = nullptr;
graph.add_gain_node(1.5f, &gain);

kc_filter_node* filter = nullptr;
graph.add_filter_node(0, 8000.0f, 1.0f, &filter); // Lowpass filter

// Connect nodes
graph.connect_nodes(decoder, 0, gain, 0);
graph.connect_nodes(gain, 0, filter, 0);

// Start processing
graph.start();

// Read processed audio
float buffer[1024 * 2];
ma_uint32 framesRead = graph.read_pcm_frames(buffer, 1024);

// Cleanup
graph.shutdown();
```

## 🎛️ Supported Features

### File Formats
- **FLAC** - Lossless compression
- **WAV** - Uncompressed PCM
- **MP3** - Lossy compression
- **OGG** - Open container format

### Audio Processing
- **Gain Control** - Volume adjustment
- **Filters** - Lowpass, highpass, bandpass, notch
- **Real-time Processing** - Low latency audio pipeline
- **Format Conversion** - Automatic sample rate and format conversion

### Performance
- **Lock-free Processing** - Uses miniaudio's optimized node graph
- **Real-time Ready** - Designed for low-latency applications
- **Memory Efficient** - Streaming audio processing
- **Cross-platform** - Works on Windows, macOS, Linux, Android

## 🔧 Configuration

### Node Graph Configuration

```cpp
struct kc_node_graph_config {
    ma_uint32 channels;           // Number of audio channels
    ma_uint32 sampleRate;         // Sample rate in Hz
    ma_format format;             // Audio format (f32, s16, etc.)
    ma_uint32 bufferSize;         // Buffer size in frames
};
```

### Performance Tuning

- **Buffer Size**: Smaller buffers = lower latency, higher CPU usage
- **Sample Rate**: Higher rates = better quality, more processing
- **Format**: f32 = best quality, s16 = smaller memory usage

## 🧪 Testing

Run the example to test the foundation:

```bash
# Build and run the example
make node_graph_example
./node_graph_example
```

## 🔮 Roadmap

### Phase 1: Foundation ✅
- [x] Node graph foundation
- [x] Basic configuration
- [x] Node connection system

### Phase 2: Core Nodes 🚧
- [ ] Decoder node implementation
- [ ] Gain node implementation
- [ ] Filter node implementation
- [ ] Node integration testing

### Phase 3: Advanced Features 📋
- [ ] Parametric equalizer
- [ ] Effects processing
- [ ] Sample rate conversion
- [ ] Audio monitoring

### Phase 4: Integration 📋
- [ ] USB audio consumer integration
- [ ] Real-time performance optimization
- [ ] Cross-platform testing

## 🐛 Troubleshooting

### Common Issues

1. **"File format not supported"**
   - Ensure file has supported extension (.flac, .wav, .mp3, .ogg)
   - Check file is not corrupted

2. **"Graph not initialized"**
   - Call `graph.initialize(config)` before adding nodes
   - Ensure configuration is valid

3. **"No frames read"**
   - Check if graph is running (`graph.start()`)
   - Verify nodes are properly connected
   - Ensure input file exists and is valid

### Debug Logging

Enable debug logging by setting the log level:

```cpp
// In your main function
LOG_INFO("Starting audio processing...");
```

## 📚 API Reference

### kc_node_graph

Main graph manager class.

#### Methods
- `initialize(config)` - Initialize the graph
- `shutdown()` - Cleanup resources
- `add_decoder_node(filename, &node)` - Add decoder node
- `add_gain_node(gain, &node)` - Add gain node
- `add_filter_node(type, freq, q, &node)` - Add filter node
- `connect_nodes(source, sourceBus, target, targetBus)` - Connect nodes
- `start()` - Start processing
- `stop()` - Stop processing
- `read_pcm_frames(buffer, frameCount)` - Read processed audio

## 🤝 Contributing

1. Follow the existing code style
2. Add tests for new features
3. Update documentation
4. Ensure cross-platform compatibility

## 📄 License

This project is part of the KCobain audio processing framework. 