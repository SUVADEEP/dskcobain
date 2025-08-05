#pragma once

#include "../../../external/miniaudio.h"
#include <string>

namespace kcobain {
namespace audio_processor {

// Decoder node configuration
struct kc_decoder_node_config {
    std::string filename;           // Input file path
    ma_uint32 outputChannels;       // Output channels (default: 2)
    ma_format outputFormat;         // Output format (default: f32)
    bool preserveSampleRate;        // Whether to preserve input sample rate
    ma_uint32 outputSampleRate;     // Target sample rate (if not preserving)
    ma_uint32 bufferSize;           // Processing buffer size
    
    kc_decoder_node_config() : 
        outputChannels(2), 
        outputFormat(ma_format_f32), 
        preserveSampleRate(true),
        outputSampleRate(0),
        bufferSize(1024) {}
};

// Decoder node structure
struct kc_decoder_node {
    ma_node_base base;              // Must be first (miniaudio requirement)
    
    // Configuration
    kc_decoder_node_config config;
    
    // Miniaudio components
    ma_decoder decoder;             // File decoder
    ma_resampler resampler;         // Sample rate converter (if needed)
    ma_channel_converter channelConverter; // Channel converter (if needed)
    ma_data_converter dataConverter;       // Data converter (if needed)
    
    // Audio information
    ma_uint32 inputChannels;        // Input file channels
    ma_uint32 inputSampleRate;      // Input file sample rate
    ma_format inputFormat;          // Input file format
    
    // Output information (fixed format)
    ma_uint32 outputChannels;       // Always 2 channels
    ma_format outputFormat;         // Always 32-bit float
    ma_uint32 outputSampleRate;     // Preserved from input
    
    // Processing buffers
    float* inputBuffer;             // Raw decoded data
    float* outputBuffer;            // Converted data
    ma_uint32 bufferSize;           // Buffer size in frames
    
    // Playback state
    enum class PlaybackState {
        STOPPED,
        PLAYING,
        PAUSED,
        SEEKING
    };
    
    PlaybackState state;            // Current playback state
    ma_uint32 currentFrame;         // Current frame position
    ma_uint32 totalFrames;          // Total frames in file
    bool isEOF;                     // End of file flag
    bool isInitialized;             // Initialization status
    
    // Statistics
    ma_uint64 totalFramesProcessed;
    ma_uint64 totalProcessingTime;
};

// Node vtable (miniaudio requirement)
extern ma_node_vtable kc_decoder_node_vtable;

// Functions
bool kc_decoder_node_init(const kc_decoder_node_config* pConfig, kc_decoder_node* pNode);
void kc_decoder_node_uninit(kc_decoder_node* pNode, const ma_allocation_callbacks* pAllocationCallbacks);

// Processing functions
ma_uint32 kc_decoder_node_read_pcm_frames(kc_decoder_node* pNode, float* pFramesOut, ma_uint32 frameCount);
bool kc_decoder_node_is_eof(kc_decoder_node* pNode);

// Playback control functions
bool kc_decoder_node_play(kc_decoder_node* pNode);
bool kc_decoder_node_pause(kc_decoder_node* pNode);
bool kc_decoder_node_stop(kc_decoder_node* pNode);
bool kc_decoder_node_seek_to_frame(kc_decoder_node* pNode, ma_uint32 frame);
bool kc_decoder_node_seek_to_time(kc_decoder_node* pNode, float timeInSeconds);
bool kc_decoder_node_eject(kc_decoder_node* pNode);

// Playback state functions
kc_decoder_node::PlaybackState kc_decoder_node_get_state(kc_decoder_node* pNode);
ma_uint32 kc_decoder_node_get_current_frame(kc_decoder_node* pNode);
float kc_decoder_node_get_current_time(kc_decoder_node* pNode);
float kc_decoder_node_get_duration(kc_decoder_node* pNode);

// Format conversion functions
bool kc_decoder_node_convert_format(kc_decoder_node* pNode, const float* pInput, float* pOutput, ma_uint32 frameCount);
bool kc_decoder_node_convert_channels(kc_decoder_node* pNode, const float* pInput, float* pOutput, ma_uint32 frameCount);
bool kc_decoder_node_convert_sample_rate(kc_decoder_node* pNode, const float* pInput, float* pOutput, ma_uint32 frameCount);

// Utility functions
bool kc_decoder_node_requires_conversion(kc_decoder_node* pNode);
ma_uint32 kc_decoder_node_get_output_frame_count(kc_decoder_node* pNode, ma_uint32 inputFrameCount);

} // namespace audio_processor
} // namespace kcobain 