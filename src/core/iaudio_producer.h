#pragma once
#include <cstdint>

namespace kcobain {
/**
 * @brief Audio Producer Interface
 * Defines the contract for audio producers
 */
 class iaudio_producer {
    public:
        virtual ~iaudio_producer() = default;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual bool isRunning() const = 0;
        virtual uint32_t getTotalFramesProduced() const = 0;
        virtual uint32_t getOverrunCount() const = 0;
    };
}