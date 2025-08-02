#include "audio_rb_controller.h"
#include "../../include/kcobain/logger.h"
#include "../../external/miniaudio.h"

namespace kcobain {

audio_rb_controller::audio_rb_controller() 
    : buffer_size_bytes(0), initialized(false) {
}

audio_rb_controller::~audio_rb_controller() {
    if (initialized) {
        ma_rb_uninit(&ring_buffer);
    }
}

bool audio_rb_controller::initialize(size_t bufferSizeBytes) {
    if (initialized) {
        LOG_WARN("Ring buffer already initialized");
        return true;
    }
    
    buffer_size_bytes = bufferSizeBytes;
    ma_result result = ma_rb_init(bufferSizeBytes, NULL, NULL, &ring_buffer);
    
    if (result != MA_SUCCESS) {
        LOG_ERROR("Failed to initialize miniaudio ring buffer: " + std::to_string(result));
        return false;
    }
    
    initialized = true;
    LOG_INFO("✅ Ring buffer initialized: " + std::to_string(bufferSizeBytes) + " bytes");
    return true;
}

ma_rb* audio_rb_controller::getRingBuffer() {
    return initialized ? &ring_buffer : nullptr;
}

bool audio_rb_controller::isInitialized() const { 
    return initialized; 
}

size_t audio_rb_controller::getBufferSize() const { 
    return buffer_size_bytes; 
}

} // namespace kcobain 