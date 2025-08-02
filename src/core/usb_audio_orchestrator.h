#pragma once

#include <memory>
#include "audio_rb_controller.h"
#include "iaudio_producer.h"
#include "iaudio_consumer.h"

namespace kcobain {

/**
 * @brief Audio Streaming Orchestrator
 * Manages the producer and consumer components
 */
class usb_audio_orchestrator {
private:
    audio_rb_controller* buffer_controller;  // Pointer to external controller
    std::unique_ptr<iaudio_producer> producer;
    std::unique_ptr<iaudio_consumer> consumer;
    
    size_t frame_size;

public:
    usb_audio_orchestrator(audio_rb_controller* controller, size_t frameSize = 384);
    ~usb_audio_orchestrator();
    
    void startStreaming();
    void stopStreaming();
    bool isStreaming() const;
    void printStatistics() const;
};

} // namespace kcobain 