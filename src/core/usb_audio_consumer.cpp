#include "usb_audio_consumer.h"
#include "audio_rb_controller.h"
#include "../../include/kcobain/logger.h"
#include "../../external/miniaudio.h"


namespace kcobain {

usb_audio_consumer::usb_audio_consumer(audio_rb_controller* controller)
    : buffer_controller(controller), running(false),
      total_frames_consumed(0), underrun_count(0) {
    
    if (!buffer_controller || !buffer_controller->isInitialized()) {
        LOG_ERROR("Consumer cannot be created - invalid or uninitialized buffer controller");
    }
}

usb_audio_consumer::~usb_audio_consumer() {
    stop();
}

void usb_audio_consumer::start() {
    if (running.load()) return;
    
    if (!buffer_controller || !buffer_controller->isInitialized()) {
        LOG_ERROR("Cannot start consumer - no valid buffer controller");
        return;
    }
    
    running = true;
    LOG_INFO("📥 USB Audio Consumer started");
    consumer_thread = std::thread([this]() { consumerLoop(); });
}

void usb_audio_consumer::stop() {
    if (!running.load()) return;
    
    running = false;
    if (consumer_thread.joinable()) {
        consumer_thread.join();
    }
    LOG_INFO("📥 USB Audio Consumer stopped");
}

bool usb_audio_consumer::isRunning() const {
    return running.load();
}

uint32_t usb_audio_consumer::getTotalFramesConsumed() const {
    return total_frames_consumed.load();
}

uint32_t usb_audio_consumer::getUnderrunCount() const {
    return underrun_count.load();
}

void usb_audio_consumer::consumerLoop() {
    ma_rb* ring_buffer = buffer_controller->getRingBuffer();
    if (!ring_buffer) {
        LOG_ERROR("Consumer cannot start - no ring buffer available");
        return;
    }

    auto microframeStart = std::chrono::high_resolution_clock::now();
    uint64_t microframeCount = 0;
    
    while (running.load()) {
        // Calculate next microframe consumption time
        auto nextMicroframe = microframeStart + std::chrono::microseconds(125);
        
        // Wait until USB consumption time
        std::this_thread::sleep_until(nextMicroframe);
        
        // USB CONSUMES: 384 bytes every 125μs
        size_t bytesToConsume = 384;
        void* readBuffer;
        size_t bytesAcquired = bytesToConsume;
        
        ma_result result = ma_rb_acquire_read(ring_buffer, &bytesAcquired, &readBuffer);
        
        // Performance monitoring: Log every 1000th microframe
        if (microframeCount % 1000 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                now - microframeStart).count();
            auto expectedTime = microframeCount * 125;
            auto timingError = (elapsed > static_cast<int64_t>(expectedTime)) ? 
                              (elapsed - static_cast<int64_t>(expectedTime)) : 
                              (static_cast<int64_t>(expectedTime) - elapsed);
            
            LOG_INFO("USB microframe #" + std::to_string(microframeCount) + 
                     " - Timing error: " + std::to_string(timingError) + "μs" +
                     " - Underruns: " + std::to_string(underrun_count.load()));
        }
        
        if (result == MA_SUCCESS && bytesAcquired == 384) {
            // USB successfully consumed microframe
            ma_rb_commit_read(ring_buffer, bytesAcquired);
            total_frames_consumed.fetch_add(1);
        } else {
            // USB underrun - no data available
            underrun_count.fetch_add(1);
            LOG_WARN("USB underrun: expected 384 bytes, got " + std::to_string(bytesAcquired));
        }
        
        microframeStart = nextMicroframe;
        microframeCount++;
    }
}



} // namespace kcobain