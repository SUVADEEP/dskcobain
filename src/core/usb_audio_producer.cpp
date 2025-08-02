#include "usb_audio_producer.h"
#include "audio_rb_controller.h"
#include "../../include/kcobain/logger.h"
#include "../../external/miniaudio.h"
#include <cstring>

namespace kcobain {

usb_audio_producer::usb_audio_producer(audio_rb_controller* controller, size_t frameSize, size_t audioDataSize)
    : buffer_controller(controller), running(false), frame_size(frameSize), audio_data_size(audioDataSize),
      gen(rd()), audio_dist(-1.0f, 1.0f), total_frames_produced(0), overrun_count(0) {
    
    if (!buffer_controller || !buffer_controller->isInitialized()) {
        LOG_ERROR("Producer cannot be created - invalid or uninitialized buffer controller");
    }
    
    LOG_INFO("📤 Producer: USB frame=" + std::to_string(frameSize) + " bytes, Audio data=" + 
             std::to_string(audioDataSize) + " bytes per microframe");
}

usb_audio_producer::~usb_audio_producer() {
    stop();
}

void usb_audio_producer::start() {
    if (running.load()) return;
    
    if (!buffer_controller || !buffer_controller->isInitialized()) {
        LOG_ERROR("Cannot start producer - no valid buffer controller");
        return;
    }
    
    running = true;
    LOG_INFO("📤 USB Audio Producer started");
    producer_thread = std::thread([this]() { producerLoop(); });
}

void usb_audio_producer::stop() {
    if (!running.load()) return;
    
    running = false;
    if (producer_thread.joinable()) {
        producer_thread.join();
    }
    LOG_INFO("📤 USB Audio Producer stopped");
}

bool usb_audio_producer::isRunning() const {
    return running.load();
}

uint32_t usb_audio_producer::getTotalFramesProduced() const {
    return total_frames_produced.load();
}

uint32_t usb_audio_producer::getOverrunCount() const {
    return overrun_count.load();
}

void usb_audio_producer::producerLoop() {
    ma_rb* ring_buffer = buffer_controller->getRingBuffer();
    if (!ring_buffer) {
        LOG_ERROR("Producer cannot start - no ring buffer available");
        return;
    }
    
    while (running.load()) {
        // Generate 32-bit float audio data
        size_t numSamples = audio_data_size / sizeof(float);
        std::vector<float> audioSamples(numSamples);
        for (size_t i = 0; i < numSamples; ++i) {
            audioSamples[i] = audio_dist(gen);  // Generate float between -1.0 and 1.0
        }
        
        // Convert float samples to bytes for USB transport
        std::vector<uint8_t> audioData(audio_data_size);
        std::memcpy(audioData.data(), audioSamples.data(), audio_data_size);
        
        // Create USB microframe with padding if needed
        std::vector<uint8_t> usbFrame(frame_size, 0);  // Initialize with zeros
        size_t bytesToCopy = std::min(audio_data_size, frame_size);
        std::copy(audioData.begin(), audioData.begin() + bytesToCopy, usbFrame.begin());
        
        // Write USB frame to miniaudio ring buffer
        size_t bytesToWrite = frame_size;
        void* writeBuffer;
        size_t bytesAcquired = bytesToWrite;
        ma_result result = ma_rb_acquire_write(ring_buffer, &bytesAcquired, &writeBuffer);
        
        // Debug: Log what's happening
        static int writeAttempts = 0;
        writeAttempts++;
        if (writeAttempts % 1000 == 0) {
            LOG_INFO("Write attempt #" + std::to_string(writeAttempts) + 
                     " - result: " + std::to_string(result) + 
                     ", bytesAcquired: " + std::to_string(bytesAcquired));
        }
        
        if (result == MA_SUCCESS && bytesAcquired > 0) {
            memcpy(writeBuffer, usbFrame.data(), bytesAcquired);
            ma_rb_commit_write(ring_buffer, bytesAcquired);
            total_frames_produced.fetch_add(1);
            
            // Check if we're exceeding expected capacity
            uint32_t maxFrames = buffer_controller->getBufferSize() / frame_size;
            if (total_frames_produced.load() > maxFrames) {
                LOG_WARN("Buffer capacity exceeded: " + std::to_string(total_frames_produced.load()) + 
                         " frames produced (max: " + std::to_string(maxFrames) + ")");
            }
        } else if (result != MA_SUCCESS) {
            overrun_count.fetch_add(1);
            LOG_WARN("Overrun detected - buffer full, dropping frame (result: " + std::to_string(result) + ")");
        }
    }
}

} // namespace kcobain