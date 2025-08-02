#pragma once

#include <cstddef>
#include "../../include/kcobain/logger.h"
#include "../../external/miniaudio.h"

namespace kcobain {

/**
 * @brief Audio Ring Buffer Controller
 * Manages the miniaudio ring buffer for audio data transfer
 */
class audio_rb_controller {
private:
    ma_rb ring_buffer;
    size_t buffer_size_bytes;
    bool initialized;

public:
    audio_rb_controller();
    ~audio_rb_controller();
    
    bool initialize(size_t bufferSizeBytes);
    ma_rb* getRingBuffer();
    bool isInitialized() const;
    size_t getBufferSize() const;
};

} // namespace kcobain 