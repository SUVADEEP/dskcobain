#pragma once
#include <cstdint>

namespace kcobain {
/**
 * @brief Audio Consumer Interface
 * Defines the contract for audio consumers
 */
 class iaudio_consumer {
    public:
        virtual ~iaudio_consumer() = default;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual bool isRunning() const = 0;
        virtual uint32_t getTotalFramesConsumed() const = 0;
        virtual uint32_t getUnderrunCount() const = 0;
    };

}