#include "usb_audio_orchestrator.h"
#include "usb_audio_producer.h"
#include "usb_audio_consumer.h"
#include "../../include/kcobain/logger.h"

namespace kcobain {

usb_audio_orchestrator::usb_audio_orchestrator(audio_rb_controller* controller, size_t frameSize)
    : buffer_controller(controller), frame_size(frameSize) {
    
    if (!buffer_controller || !buffer_controller->isInitialized()) {
        LOG_ERROR("Cannot create orchestrator - buffer controller not initialized");
        return;
    }
    
    // Create producer and consumer instances using concrete classes
    // Calculate audio data size for 32-bit float samples
    // For 96kHz, 32-bit, 2ch: 12 samples × 4 bytes × 2 channels = 96 bytes
    size_t audioDataSize = 96;  // 32-bit float samples per microframe
    producer = std::unique_ptr<iaudio_producer>(new usb_audio_producer(buffer_controller, frameSize, audioDataSize));
    consumer = std::unique_ptr<iaudio_consumer>(new usb_audio_consumer(buffer_controller));
    
    LOG_INFO("🎵 USB Audio Class Simulator: " + std::to_string(frame_size) + " bytes/microframe, " + 
             std::to_string(buffer_controller->getBufferSize()) + " bytes buffer (" + 
             std::to_string(buffer_controller->getBufferSize() / frame_size) + " microframes capacity)");
}

usb_audio_orchestrator::~usb_audio_orchestrator() {
    stopStreaming();
}

void usb_audio_orchestrator::startStreaming() {
    if (!producer || !consumer) {
        LOG_ERROR("Cannot start streaming - producer or consumer not initialized");
        return;
    }
    
    LOG_INFO("🚀 Starting USB Audio Class simulation (125μs microframes)...");
    
    // Start consumer first to avoid initial underruns
    consumer->start();
    producer->start();
}

void usb_audio_orchestrator::stopStreaming() {
    if (producer) producer->stop();
    if (consumer) consumer->stop();
    LOG_INFO("🛑 Streaming stopped");
}

bool usb_audio_orchestrator::isStreaming() const {
    return (producer && producer->isRunning()) || (consumer && consumer->isRunning());
}

void usb_audio_orchestrator::printStatistics() const {
    LOG_INFO("=== USB Audio Statistics ===");
    
    if (producer) {
        LOG_INFO("Total Frames Produced: " + std::to_string(producer->getTotalFramesProduced()));
        LOG_INFO("Overruns: " + std::to_string(producer->getOverrunCount()));
    }
    
    if (consumer) {
        LOG_INFO("Total Frames Consumed: " + std::to_string(consumer->getTotalFramesConsumed()));
        LOG_INFO("Underruns: " + std::to_string(consumer->getUnderrunCount()));
    }
    
    if (producer && consumer) {
        uint32_t produced = producer->getTotalFramesProduced();
        uint32_t consumed = consumer->getTotalFramesConsumed();
        
        if (produced > 0) {
            double underrun_rate = (double)consumer->getUnderrunCount() / produced * 100.0;
            LOG_INFO("Underrun Rate: " + std::to_string(underrun_rate) + "%");
        }
        if (consumed > 0) {
            double overrun_rate = (double)producer->getOverrunCount() / consumed * 100.0;
            LOG_INFO("Overrun Rate: " + std::to_string(overrun_rate) + "%");
        }
    }
}

} // namespace kcobain 