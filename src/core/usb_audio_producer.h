#pragma once

#include <atomic>
#include <thread>
#include <random>
#include <vector>
#include "iaudio_producer.h"

// Forward declaration
namespace kcobain {
    class audio_rb_controller;
}

namespace kcobain {

/**
 * @brief USB Audio Producer Implementation
 * Generates audio data and writes to the ring buffer
 */
class usb_audio_producer : public iaudio_producer {
private:
    audio_rb_controller* buffer_controller;
    std::atomic<bool> running;
    std::thread producer_thread;
    size_t frame_size;  // USB microframe size (384 bytes)
    size_t audio_data_size;  // Actual audio data size per microframe
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> audio_dist;
    std::atomic<uint32_t> total_frames_produced;
    std::atomic<uint32_t> overrun_count;

public:
    usb_audio_producer(audio_rb_controller* controller, size_t frameSize = 384, size_t audioDataSize = 96);
    ~usb_audio_producer();
    
    void start() override;
    void stop() override;
    bool isRunning() const override;
    uint32_t getTotalFramesProduced() const override;
    uint32_t getOverrunCount() const override;

private:
    void producerLoop();
};

} // namespace kcobain 