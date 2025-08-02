#include <iostream>
#include <thread>
#include <chrono>
#include "../include/kcobain/logger.h"
#include "core/audio_rb_controller.h"
#include "core/usb_audio_orchestrator.h"


namespace kcobain {
// UsbAudioStreamingOrchestrator is now in its own files
} // namespace kcobain

int main() {
    LOG_INFO("🎵 USB Audio Class Microframe Simulator");
    LOG_INFO("=====================================");
    
    // Create and initialize buffer controller separately
    kcobain::audio_rb_controller buffer_controller;
    size_t bufferSizeBytes = 30720;  // Optimized buffer: 80 microframes × 384 bytes = 30,720 bytes
    
    if (!buffer_controller.initialize(bufferSizeBytes)) {
        LOG_ERROR("Failed to initialize buffer controller");
        return 1;
    }
    
    // Create orchestrator with external buffer controller
    kcobain::usb_audio_orchestrator orchestrator(&buffer_controller, 384);
    orchestrator.startStreaming();
    
    // Run for 1ms (8 microframes)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    orchestrator.stopStreaming();
    orchestrator.printStatistics();
    
    LOG_INFO("✅ Done");
    return 0;
}
