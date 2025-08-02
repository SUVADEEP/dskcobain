#ifndef KCOBAIN_LOGGER_H
#define KCOBAIN_LOGGER_H

#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include <iomanip>

// Platform detection
#ifdef __ANDROID__
    #include <android/log.h>
    #define KCOBAIN_ANDROID_LOGGING
#else
    #define KCOBAIN_STD_LOGGING
#endif

// Logger tag from CMake
#ifndef LOGGER_TAG
    #define LOGGER_TAG "Kcobain"
#endif



namespace kcobain {

// Log levels
enum LogLevel {
    VERBOSE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

// Log level names
inline const char* getLogLevelName(LogLevel level) {
    switch (level) {
        case VERBOSE: return "VERBOSE";
        case DEBUG:   return "DEBUG";
        case INFO:    return "INFO";
        case WARN:    return "WARN";
        case ERROR:   return "ERROR";
        case FATAL:   return "FATAL";
        default:                return "UNKNOWN";
    }
}

// Android log level mapping
#ifdef KCOBAIN_ANDROID_LOGGING
inline int getAndroidLogLevel(LogLevel level) {
    switch (level) {
        case VERBOSE: return ANDROID_LOG_VERBOSE;
        case DEBUG:   return ANDROID_LOG_DEBUG;
        case INFO:    return ANDROID_LOG_INFO;
        case WARN:    return ANDROID_LOG_WARN;
        case ERROR:   return ANDROID_LOG_ERROR;
        case FATAL:   return ANDROID_LOG_FATAL;
        default:                return ANDROID_LOG_INFO;
    }
}
#endif

/**
 * @brief Cross-platform logger for Android and standard C++
 */
class Logger {
private:
    std::string tag;
    LogLevel minLevel;
    bool enabled;
    
    // Get current timestamp
    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_val = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_val), "%H:%M:%S");
        ss << "." << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }
    
    // Format log message
    std::string formatMessage(LogLevel level, const std::string& message) {
        std::stringstream ss;
        ss << "[" << getTimestamp() << "] "
           << "[" << getLogLevelName(level) << "] "
           << "[" << tag << "] "
           << message;
        return ss.str();
    }

public:
    Logger(const std::string& loggerTag = LOGGER_TAG, LogLevel minimumLevel = LogLevel::INFO)
        : tag(loggerTag), minLevel(minimumLevel), enabled(true) {}
    
    ~Logger() = default;
    
    // Enable/disable logging
    void setEnabled(bool enable) { enabled = enable; }
    bool isEnabled() const { return enabled; }
    
    // Set minimum log level
    void setMinLevel(LogLevel level) { minLevel = level; }
    LogLevel getMinLevel() const { return minLevel; }
    
    // Set tag
    void setTag(const std::string& loggerTag) { tag = loggerTag; }
    std::string getTag() const { return tag; }
    
    // Log methods
    void log(LogLevel level, const std::string& message) {
        if (!enabled || level < minLevel) return;
        
        std::string formattedMessage = formatMessage(level, message);
        
#ifdef KCOBAIN_ANDROID_LOGGING
        // Android logging
        __android_log_print(getAndroidLogLevel(level), tag.c_str(), "%s", formattedMessage.c_str());
#else
        // Standard C++ logging
        std::ostream& stream = (level >= LogLevel::ERROR) ? std::cerr : std::cout;
        stream << formattedMessage << std::endl;
#endif
    }
    
    // Convenience methods
    void verbose(const std::string& message) { log(LogLevel::VERBOSE, message); }
    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warn(const std::string& message) { log(LogLevel::WARN, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }
    void fatal(const std::string& message) { log(LogLevel::FATAL, message); }
    
    // Template methods for different data types
    template<typename T>
    void log(LogLevel level, const T& value) {
        std::stringstream ss;
        ss << value;
        log(level, ss.str());
    }
    
    template<typename T>
    void verbose(const T& value) { log(LogLevel::VERBOSE, value); }
    
    template<typename T>
    void debug(const T& value) { log(LogLevel::DEBUG, value); }
    
    template<typename T>
    void info(const T& value) { log(LogLevel::INFO, value); }
    
    template<typename T>
    void warn(const T& value) { log(LogLevel::WARN, value); }
    
    template<typename T>
    void error(const T& value) { log(LogLevel::ERROR, value); }
    
    template<typename T>
    void fatal(const T& value) { log(LogLevel::FATAL, value); }
    
    // Hex dump utility
    void hexDump(LogLevel level, const std::string& label, const uint8_t* data, size_t size, size_t bytesPerLine = 16) {
        if (!enabled || level < minLevel) return;
        
        std::stringstream ss;
        ss << label << " (" << size << " bytes):" << std::endl;
        
        for (size_t i = 0; i < size; i += bytesPerLine) {
            // Offset
            ss << std::hex << std::setfill('0') << std::setw(8) << i << ": ";
            
            // Hex bytes
            for (size_t j = 0; j < bytesPerLine; ++j) {
                if (i + j < size) {
                    ss << std::hex << std::setfill('0') << std::setw(2) << (int)data[i + j] << " ";
                } else {
                    ss << "   ";
                }
            }
            
            // ASCII representation
            ss << " ";
            for (size_t j = 0; j < bytesPerLine && i + j < size; ++j) {
                uint8_t byte = data[i + j];
                ss << (byte >= 32 && byte <= 126 ? (char)byte : '.');
            }
            
            ss << std::endl;
        }
        
        log(level, ss.str());
    }
    

};

// Global logger instance
extern Logger g_logger;

// Convenience macros
#define LOG_VERBOSE(msg) kcobain::g_logger.verbose(msg)
#define LOG_DEBUG(msg) kcobain::g_logger.debug(msg)
#define LOG_INFO(msg) kcobain::g_logger.info(msg)
#define LOG_WARN(msg) kcobain::g_logger.warn(msg)
#define LOG_ERROR(msg) kcobain::g_logger.error(msg)
#define LOG_FATAL(msg) kcobain::g_logger.fatal(msg)

#define LOG_HEXDUMP(label, data, size) kcobain::g_logger.hexDump(LogLevel::INFO, label, data, size)


} // namespace kcobain

#endif // KCOBAIN_LOGGER_H 