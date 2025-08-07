#include <iostream>
#include <cstring>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#define MINIAUDIO_IMPLEMENTATION
#include "../external/miniaudio.h"


// Volume control variables
static std::atomic<float> current_volume(1.0f);
static std::atomic<bool> should_exit(false);

// Node graph variables
static ma_node_graph node_graph;
static ma_data_source_node decoder_node;
static ma_loshelf_node low_eq_node;
static ma_peak_node mid_eq_node;
static ma_hishelf_node high_eq_node;
static ma_biquad_node volume_node;
static std::atomic<bool> eq_enabled(true);
static std::atomic<ma_uint32> global_sample_rate(48000); // Default sample rate
static std::atomic<ma_uint32> global_channels(2); // Default channel count

// Legacy EQ variables - these are no longer used since we use the node graph system
// These can be removed in a future cleanup

// Global decoder pointer for seeking
static ma_decoder* global_decoder = nullptr;
static std::atomic<ma_uint64> global_total_frames(0); // Cache total frames to avoid repeated calls
static std::atomic<ma_uint64> decoder_position(0); // Track actual decoder position
static const char* global_filename = nullptr; // Store filename for FLAC detection
static ma_device* global_device = nullptr; // Store device pointer for pausing during seek



// Seek function declaration
bool seek_to_position(float seek_time_seconds);

// Node graph control functions
bool setup_node_graph(ma_decoder* decoder);
void enable_eq_node(bool enable);
void cleanup_node_graph();
void update_low_eq_node(float frequency, float gain);
void update_mid_eq_node(float frequency, float gain, float q);
void update_high_eq_node(float frequency, float gain);
void update_volume(float volume);

// EQ parameters (frequency, gain, Q)
static std::atomic<float> low_freq(80.0f);    // Low shelf frequency
static std::atomic<float> low_gain(0.0f);     // Low shelf gain in dB
static std::atomic<float> mid_freq(1000.0f);  // Mid peak frequency
static std::atomic<float> mid_gain(0.0f);     // Mid peak gain in dB
static std::atomic<float> mid_q(1.0f);        // Mid Q factor
static std::atomic<float> high_freq(8000.0f); // High shelf frequency
static std::atomic<float> high_gain(0.0f);    // High shelf gain in dB

// Simple keyboard input function using cin
char get_key_input() {
    char input;
    std::cin >> input;
    return input;
}



bool seek_to_position(float seek_time_seconds) {
    if (!global_decoder) {
        std::cout << "Decoder not available for seeking" << std::endl;
        return false;
    }
    
    // Calculate total duration
    float total_duration = (float)global_total_frames.load() / global_decoder->outputSampleRate;
    
    // Validate seek time
    if (seek_time_seconds < 0.0f || seek_time_seconds > total_duration) {
        std::cout << "Invalid seek position: " << seek_time_seconds << "s (valid range: 0-" << total_duration << "s)" << std::endl;
        return false;
    }
    
    // Calculate frame position
    ma_uint64 seek_frame = (ma_uint64)(seek_time_seconds * global_decoder->outputSampleRate);
    
    // Ensure we don't seek beyond valid range
    if (seek_frame >= global_total_frames.load()) {
        seek_frame = global_total_frames.load() - 1;
    }
    
    // Pause audio during seeking to prevent noise
    if (global_device) {
        ma_device_stop(global_device);
    }
    
    // Perform the seek operation
    ma_result seek_result = ma_decoder_seek_to_pcm_frame(global_decoder, seek_frame);
    
    if (seek_result == MA_SUCCESS) {
        decoder_position.store(seek_frame);
        std::cout << "Seeked to " << seek_time_seconds << "s (frame " << seek_frame << ")" << std::endl;
        
        // Resume audio after successful seek
        if (global_device) {
            ma_device_start(global_device);
        }
        
        return true;
    } else {
        std::cout << "Failed to seek to position: " << seek_result << std::endl;
        
        // Resume audio even if seek failed
        if (global_device) {
            ma_device_start(global_device);
        }
        
        return false;
    }
}

// Node graph setup function
bool setup_node_graph(ma_decoder* decoder) {
    // Initialize node graph
    ma_node_graph_config nodeGraphConfig = ma_node_graph_config_init(decoder->outputChannels);
    ma_result result = ma_node_graph_init(&nodeGraphConfig, NULL, &node_graph);
    if (result != MA_SUCCESS) {
        std::cout << "Failed to initialize node graph: " << result << std::endl;
        return false;
    }
    
    // All processing in 32-bit float format
    std::cout << "✅ Using 32-bit float format throughout pipeline" << std::endl;
    
    // Initialize decoder node
    ma_data_source_node_config decoderNodeConfig = ma_data_source_node_config_init(decoder);
    result = ma_data_source_node_init(&node_graph, &decoderNodeConfig, NULL, &decoder_node);
    if (result != MA_SUCCESS) {
        std::cout << "Failed to initialize decoder node: " << result << std::endl;
        return false;
    }
    
    // Initialize EQ nodes
    ma_loshelf_node_config lowConfig = ma_loshelf_node_config_init(decoder->outputChannels, decoder->outputSampleRate, low_gain.load(), 0.707f, low_freq.load());
    result = ma_loshelf_node_init(&node_graph, &lowConfig, NULL, &low_eq_node);
    if (result != MA_SUCCESS) {
        std::cout << "Failed to initialize low EQ node: " << result << std::endl;
        return false;
    }
    
    ma_peak_node_config midConfig = ma_peak_node_config_init(decoder->outputChannels, decoder->outputSampleRate, mid_gain.load(), mid_q.load(), mid_freq.load());
    result = ma_peak_node_init(&node_graph, &midConfig, NULL, &mid_eq_node);
    if (result != MA_SUCCESS) {
        std::cout << "Failed to initialize mid EQ node: " << result << std::endl;
        return false;
    }
    
    ma_hishelf_node_config highConfig = ma_hishelf_node_config_init(decoder->outputChannels, decoder->outputSampleRate, high_gain.load(), 0.707f, high_freq.load());
    result = ma_hishelf_node_init(&node_graph, &highConfig, NULL, &high_eq_node);
    if (result != MA_SUCCESS) {
        std::cout << "Failed to initialize high EQ node: " << result << std::endl;
        return false;
    }
    
    // Initialize volume node (simple gain filter)
    ma_biquad_node_config volumeConfig = ma_biquad_node_config_init(decoder->outputChannels, current_volume.load(), 0.0f, 0.0f, 1.0f, 0.0f, 0.0f); // Unity gain initially
    result = ma_biquad_node_init(&node_graph, &volumeConfig, NULL, &volume_node);
    if (result != MA_SUCCESS) {
        std::cout << "Failed to initialize volume node: " << result << std::endl;
        return false;
    }
    
    // Connect nodes: decoder -> low EQ -> mid EQ -> high EQ -> volume -> endpoint
    ma_node_attach_output_bus(&decoder_node, 0, &low_eq_node, 0);
    ma_node_attach_output_bus(&low_eq_node, 0, &mid_eq_node, 0);
    ma_node_attach_output_bus(&mid_eq_node, 0, &high_eq_node, 0);
    ma_node_attach_output_bus(&high_eq_node, 0, &volume_node, 0);
    ma_node_attach_output_bus(&volume_node, 0, ma_node_graph_get_endpoint(&node_graph), 0);
    
    std::cout << "Node graph setup complete" << std::endl;
    return true;
}

// Enable/disable EQ nodes
void enable_eq_node(bool enable) {
    if (enable) {  
        // Enable all nodes by setting their state to started
        ma_node_set_state(&decoder_node, ma_node_state_started);
        ma_node_set_state(&low_eq_node, ma_node_state_started);
        ma_node_set_state(&mid_eq_node, ma_node_state_started);
        ma_node_set_state(&high_eq_node, ma_node_state_started);
        ma_node_set_state(&volume_node, ma_node_state_started);
        std::cout << "EQ chain enabled: decoder -> low EQ -> mid EQ -> high EQ -> volume" << std::endl;
    } else {
        // Bypass EQ nodes by connecting decoder directly to volume
        //ma_node_attach_output_bus(&decoder_node, 0, &volume_node, 0);
        
        // Ensure decoder and volume nodes are enabled
        ma_node_set_state(&decoder_node, ma_node_state_started);
        ma_node_set_state(&volume_node, ma_node_state_started);
        
        // Stop EQ nodes (they're not in the signal path anymore)
        ma_node_set_state(&low_eq_node, ma_node_state_stopped);
        ma_node_set_state(&mid_eq_node, ma_node_state_stopped);
        ma_node_set_state(&high_eq_node, ma_node_state_stopped);
        std::cout << "EQ bypassed: decoder -> volume (EQ nodes stopped)" << std::endl;
    }
}

// Cleanup node graph
void cleanup_node_graph() {
    ma_biquad_node_uninit(&volume_node, NULL);
    ma_hishelf_node_uninit(&high_eq_node, NULL);
    ma_peak_node_uninit(&mid_eq_node, NULL);
    ma_loshelf_node_uninit(&low_eq_node, NULL);
    ma_data_source_node_uninit(&decoder_node, NULL);
    ma_node_graph_uninit(&node_graph, NULL);
}

// Update EQ node parameters
void update_low_eq_node(float frequency, float gain) {
    ma_loshelf2_config config = ma_loshelf2_config_init(ma_format_f32, global_channels.load(), global_sample_rate.load(), gain, 0.707f, frequency);
    ma_loshelf_node_reinit(&config, &low_eq_node);
}

void update_mid_eq_node(float frequency, float gain, float q) {
    ma_peak2_config config = ma_peak2_config_init(ma_format_f32, global_channels.load(), global_sample_rate.load(), gain, q, frequency);
    ma_peak_node_reinit(&config, &mid_eq_node);
}

void update_high_eq_node(float frequency, float gain) {
    ma_hishelf2_config config = ma_hishelf2_config_init(ma_format_f32, global_channels.load(), global_sample_rate.load(), gain, 0.707f, frequency);
    ma_hishelf_node_reinit(&config, &high_eq_node);
}

void update_volume(float volume) {
    current_volume.store(volume);
    
    // Update the volume node with new gain
    // We need to reinitialize the volume node with new coefficients
    ma_biquad_node_config volumeConfig = ma_biquad_node_config_init(
        global_channels.load(), 
        volume,  // b0 coefficient (gain)
        0.0f,    // b1 coefficient
        0.0f,    // b2 coefficient
        1.0f,    // a0 coefficient
        0.0f,    // a1 coefficient
        0.0f     // a2 coefficient
    );
    
    // Reinitialize the volume node with new coefficients
    ma_biquad_node_uninit(&volume_node, NULL);
    ma_result result = ma_biquad_node_init(&node_graph, &volumeConfig, NULL, &volume_node);
    if (result == MA_SUCCESS) {
        // Reconnect the volume node
        ma_node_attach_output_bus(&high_eq_node, 0, &volume_node, 0);
        ma_node_attach_output_bus(&volume_node, 0, ma_node_graph_get_endpoint(&node_graph), 0);
        ma_node_set_state(&volume_node, ma_node_state_started);
    }
}

// Keyboard input handling thread
void keyboard_input_thread() {
    std::cout << "Audio Controls:" << std::endl;
    std::cout << "  U - Volume Up" << std::endl;
    std::cout << "  D - Volume Down" << std::endl;
    std::cout << "  E - Toggle EQ" << std::endl;
    std::cout << "  L - Low EQ (frequency/gain)" << std::endl;
    std::cout << "  M - Mid EQ (frequency/gain/Q)" << std::endl;
    std::cout << "  H - High EQ (frequency/gain)" << std::endl;
    std::cout << "  R - Reset EQ to flat" << std::endl;
    std::cout << "  S - Seek to position (seconds)" << std::endl;
    std::cout << "  I - Show current position" << std::endl;
    std::cout << "  F - Forward 10 seconds" << std::endl;
    std::cout << "  B - Backward 10 seconds" << std::endl;
    std::cout << "  Q - Quit" << std::endl;
    std::cout << "Current Volume: 100%" << std::endl;
    std::cout << "EQ: " << (eq_enabled.load() ? "ON" : "OFF") << std::endl;
    std::cout << "Enter command: ";
    
    while (!should_exit.load()) {
        char input = get_key_input();
        
        switch (toupper(input)) {
            case 'U': {
                float new_volume = std::min(current_volume.load() + 0.1f, 2.0f);
                update_volume(new_volume);
                std::cout << "Volume: " << (int)(new_volume * 100) << "%" << std::endl;
                std::cout << "Enter command: ";
                break;
            }
            case 'D': {
                float new_volume = std::max(current_volume.load() - 0.1f, 0.0f);
                update_volume(new_volume);
                std::cout << "Volume: " << (int)(new_volume * 100) << "%" << std::endl;
                std::cout << "Enter command: ";
                break;
            }
            case 'E': {
                bool new_eq_state = !eq_enabled.load();
                eq_enabled.store(new_eq_state);
                enable_eq_node(new_eq_state);
                std::cout << "EQ: " << (new_eq_state ? "ON" : "OFF") << std::endl;
                std::cout << "Enter command: ";
                break;
            }
            case 'L': {
                std::cout << "Low EQ - Current: " << low_freq.load() << "Hz, " << low_gain.load() << "dB" << std::endl;
                std::cout << "Enter new frequency (20-500Hz): ";
                float freq;
                std::cin >> freq;
                if (freq >= 20.0f && freq <= 500.0f) {
                    low_freq.store(freq);
                    update_low_eq_node(low_freq.load(), low_gain.load());
                }
                std::cout << "Enter new gain (-20 to +20dB): ";
                float gain;
                std::cin >> gain;
                if (gain >= -20.0f && gain <= 20.0f) {
                    low_gain.store(gain);
                    update_low_eq_node(low_freq.load(), low_gain.load());
                }
                std::cout << "Low EQ set to: " << low_freq.load() << "Hz, " << low_gain.load() << "dB" << std::endl;
                std::cout << "Enter command: ";
                break;
            }
            case 'M': {
                std::cout << "Mid EQ - Current: " << mid_freq.load() << "Hz, " << mid_gain.load() << "dB, Q=" << mid_q.load() << std::endl;
                std::cout << "Enter new frequency (100-8000Hz): ";
                float freq;
                std::cin >> freq;
                if (freq >= 100.0f && freq <= 8000.0f) {
                    mid_freq.store(freq);
                    update_mid_eq_node(mid_freq.load(), mid_gain.load(), mid_q.load());
                }
                std::cout << "Enter new gain (-20 to +20dB): ";
                float gain;
                std::cin >> gain;
                if (gain >= -20.0f && gain <= 20.0f) {
                    mid_gain.store(gain);
                    update_mid_eq_node(mid_freq.load(), mid_gain.load(), mid_q.load());
                }
                std::cout << "Enter new Q (0.1-10): ";
                float q;
                std::cin >> q;
                if (q >= 0.1f && q <= 10.0f) {
                    mid_q.store(q);
                    update_mid_eq_node(mid_freq.load(), mid_gain.load(), mid_q.load());
                }
                std::cout << "Mid EQ set to: " << mid_freq.load() << "Hz, " << mid_gain.load() << "dB, Q=" << mid_q.load() << std::endl;
                std::cout << "Enter command: ";
                break;
            }
            case 'H': {
                std::cout << "High EQ - Current: " << high_freq.load() << "Hz, " << high_gain.load() << "dB" << std::endl;
                std::cout << "Enter new frequency (2000-20000Hz): ";
                float freq;
                std::cin >> freq;
                if (freq >= 2000.0f && freq <= 20000.0f) {
                    high_freq.store(freq);
                    update_high_eq_node(high_freq.load(), high_gain.load());
                }
                std::cout << "Enter new gain (-20 to +20dB): ";
                float gain;
                std::cin >> gain;
                if (gain >= -20.0f && gain <= 20.0f) {
                    high_gain.store(gain);
                    update_high_eq_node(high_freq.load(), high_gain.load());
                }
                std::cout << "High EQ set to: " << high_freq.load() << "Hz, " << high_gain.load() << "dB" << std::endl;
                std::cout << "Enter command: ";
                break;
            }
            case 'R': {
                // Reset EQ to flat
                low_gain.store(0.0f);
                mid_gain.store(0.0f);
                high_gain.store(0.0f);
                update_low_eq_node(low_freq.load(), 0.0f);
                update_mid_eq_node(mid_freq.load(), 0.0f, mid_q.load());
                update_high_eq_node(high_freq.load(), 0.0f);
                std::cout << "EQ reset to flat response" << std::endl;
                std::cout << "Enter command: ";
                break;
            }
            case 'S': {
                if (!global_decoder) {
                    std::cout << "Decoder not available for seeking" << std::endl;
                    std::cout << "Enter command: ";
                    break;
                }
                
                // Show current position
                float total_duration = (float)global_total_frames.load() / global_decoder->outputSampleRate;
                float current_time = (float)decoder_position.load() / global_decoder->outputSampleRate;
                std::cout << "Current position: " << current_time << "s / " << total_duration << "s" << std::endl;
                
                // Get seek position from user
                std::cout << "Enter seek position in seconds (0-" << total_duration << "): ";
                float seek_time;
                std::cin >> seek_time;
                
                // Use the seek function
                seek_to_position(seek_time);
                std::cout << "Enter command: ";
                break;
            }
            case 'I': {
                if (!global_decoder) {
                    std::cout << "Decoder not available" << std::endl;
                    std::cout << "Enter command: ";
                    break;
                }
                
                // Use decoder's actual sample rate for calculations
                float total_duration = (float)global_total_frames.load() / global_decoder->outputSampleRate;
                float current_time = (float)decoder_position.load() / global_decoder->outputSampleRate;
                ma_uint64 total_frames = global_total_frames.load();
                
                std::cout << "Position: " << current_time << "s / " << total_duration << "s" << std::endl;
                std::cout << "Frames: " << decoder_position.load() << " / " << total_frames << std::endl;
                std::cout << "Progress: " << (int)((current_time / total_duration) * 100) << "%" << std::endl;
                std::cout << "Enter command: ";
                break;
            }
            case 'F': {
                if (!global_decoder) {
                    std::cout << "Decoder not available for seeking" << std::endl;
                    std::cout << "Enter command: ";
                    break;
                }
                
                // Calculate forward position
                float total_duration = (float)global_total_frames.load() / global_decoder->outputSampleRate;
                float current_time = (float)decoder_position.load() / global_decoder->outputSampleRate;
                float new_time = std::min(current_time + 10.0f, total_duration);
                
                // Use the seek function
                if (seek_to_position(new_time)) {
                    std::cout << "Forwarded to " << new_time << "s" << std::endl;
                }
                std::cout << "Enter command: ";
                break;
            }
            case 'B': {
                if (!global_decoder) {
                    std::cout << "Decoder not available for seeking" << std::endl;
                    std::cout << "Enter command: ";
                    break;
                }
                
                // Calculate backward position
                float current_time = (float)decoder_position.load() / global_decoder->outputSampleRate;
                float new_time = std::max(current_time - 10.0f, 0.0f);
                
                // Use the seek function
                if (seek_to_position(new_time)) {
                    std::cout << "Rewound to " << new_time << "s" << std::endl;
                }
                std::cout << "Enter command: ";
                break;
            }
            case 'Q': {
                should_exit.store(true);
                std::cout << "Exiting..." << std::endl;
                break;
            }
            default: {
                std::cout << "Invalid command. Use U (up), D (down), E (EQ), L (low), M (mid), H (high), R (reset), S (seek), I (info), F (forward), B (backward), or Q (quit)" << std::endl;
                std::cout << "Enter command: ";
                break;
            }
        }
    }
}

// Solution for Seeking Synchronization Issue: Thread-safe callback
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    // Read from the node graph - all processing (including volume) is handled internally
    ma_result result = ma_node_graph_read_pcm_frames(&node_graph, pOutput, frameCount, NULL);
    
    if (result != MA_SUCCESS) {
        // If node graph read fails, output silence
        memset(pOutput, 0, frameCount * pDevice->playback.channels * ma_get_bytes_per_frame(pDevice->playback.format, pDevice->playback.channels));
        // Debug: Print error occasionally
        static int error_count = 0;
        if (++error_count % 1000 == 0) {
            std::cout << "Node graph read error: " << result << std::endl;
        }
    }
    
    // Update statistics (still need this for UI)
    decoder_position.fetch_add(frameCount);
    
    (void)pInput;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <audio_file.wav/flac>" << std::endl;
        return 1;
    }

    const char* filePath = argv[1];

    // Local variables
    ma_result result;
    ma_decoder decoder;
    ma_device device;

    // Init decoder with 32-bit float format for consistency
    ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_f32, 0, 0); // Use 32-bit float
    result = ma_decoder_init_file(filePath, &decoderConfig, &decoder);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize decoder for file: " << filePath << std::endl;
        return -1;
    }
    
    // Note: Volume control is handled through the node graph system
    
    // Update global sample rate and channels
    global_sample_rate.store(decoder.outputSampleRate);
    global_channels.store(decoder.outputChannels);
    global_decoder = &decoder;
    global_filename = filePath; // Store filename for FLAC detection
    decoder_position.store(0); // Initialize decoder position
    
    // Display format information for debugging
    std::cout << "🎵 File Format Information:" << std::endl;
    std::cout << "   Sample Rate: " << decoder.outputSampleRate << " Hz" << std::endl;
    std::cout << "   Channels: " << decoder.outputChannels << std::endl;
    std::cout << "   Format: ";
    switch (decoder.outputFormat) {
        case ma_format_unknown: std::cout << "Unknown"; break;
        case ma_format_u8: std::cout << "8-bit Unsigned"; break;
        case ma_format_s16: std::cout << "16-bit Signed"; break;
        case ma_format_s24: std::cout << "24-bit Signed"; break;
        case ma_format_s32: std::cout << "32-bit Signed"; break;
        case ma_format_f32: std::cout << "32-bit Float"; break;
        default: std::cout << "Other"; break;
    }
    std::cout << std::endl;
    
    // Note: Legacy EQ filters are no longer initialized since we use the node graph system
    // The node graph handles all EQ processing more efficiently
    
    // Get file length for position tracking
    ma_uint64 total_frames = 0;
    ma_decoder_get_length_in_pcm_frames(&decoder, &total_frames);
    global_total_frames.store(total_frames);
    
    // Reset position tracking for new file
    decoder_position.store(0);

    // Set up the node graph - THIS IS CRITICAL FOR AUDIO PLAYBACK
    if (!setup_node_graph(&decoder)) {
        std::cerr << "Failed to set up node graph" << std::endl;
        ma_decoder_uninit(&decoder);
        return -1;
    }
    
    // Enable EQ nodes
    enable_eq_node(eq_enabled.load());

    // Init playback device (will use float32 format from decoder)
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = decoder.outputFormat;  // Will be float32
    deviceConfig.playback.channels = decoder.outputChannels;
    deviceConfig.sampleRate        = decoder.outputSampleRate;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = &decoder;
    
    // Buffer configuration (optional - miniaudio uses defaults if not set)
    deviceConfig.periodSizeInFrames = 0;        // Use default
    deviceConfig.periods = 0;                   // Use default
    deviceConfig.performanceProfile = ma_performance_profile_low_latency;

    result = ma_device_init(NULL, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize playback device" << std::endl;
        ma_decoder_uninit(&decoder);
        return -1;
    }
    
    // Store device pointer for seeking operations
    global_device = &device;

    result = ma_device_start(&device);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to start playback device" << std::endl;
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        return -1;
    }

    // Start keyboard input thread
    std::thread keyboard_thread(keyboard_input_thread);

    // Wait for exit signal
    while (!should_exit.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "🎵 Audio Device Information:" << std::endl;
    std::cout << "   Device Sample Rate: " << device.sampleRate << " Hz" << std::endl;
    std::cout << "   Device Channels: " << device.playback.channels << std::endl;
    std::cout << "   Device Format: " << (device.playback.format == ma_format_f32 ? "F32" : "Other") << std::endl;
    std::cout << "   File Sample Rate: " << decoder.outputSampleRate << " Hz" << std::endl;
    std::cout << "   File Channels: " << decoder.outputChannels << std::endl;
    std::cout << "   File Format: " << (decoder.outputFormat == ma_format_f32 ? "F32" : "Other") << std::endl;
    std::cout << std::endl;
    

    // Clean up
    keyboard_thread.join();
    ma_device_uninit(&device);
    cleanup_node_graph();  // Clean up node graph first
    ma_decoder_uninit(&decoder);
    
    return 0;
}