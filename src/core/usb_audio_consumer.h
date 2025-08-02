#pragma once

#include <atomic>
#include <thread>
#include "iaudio_consumer.h"

// Forward declaration
namespace kcobain {
    class audio_rb_controller;
}

namespace kcobain {

/**
 * @brief USB Audio Consumer Implementation
 * Reads audio data from the ring buffer and processes it
 */
class usb_audio_consumer : public iaudio_consumer {
private:
    audio_rb_controller* buffer_controller;
    std::atomic<bool> running;
    std::thread consumer_thread;
    std::atomic<uint32_t> total_frames_consumed;
    std::atomic<uint32_t> underrun_count;

public:
    usb_audio_consumer(audio_rb_controller* controller);
    ~usb_audio_consumer();
    
    void start() override;
    void stop() override;
    bool isRunning() const override;
    uint32_t getTotalFramesConsumed() const override;
    uint32_t getUnderrunCount() const override;

private:
    void consumerLoop();
};

} // namespace kcobain 