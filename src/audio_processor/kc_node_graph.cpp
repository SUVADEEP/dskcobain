#include "kc_node_graph.h"
#include "../../include/kcobain/logger.h"
#include <algorithm>
#include <cstring>

namespace kcobain {
namespace audio_processor {

// Constructor
kc_node_graph::kc_node_graph() : initialized(false), running(false) {
    LOG_INFO("🎵 Creating KC Node Graph");
}

// Destructor
kc_node_graph::~kc_node_graph() {
    shutdown();
}

// Initialize the node graph
bool kc_node_graph::initialize(const kc_node_graph_config& graphConfig) {
    if (initialized) {
        LOG_WARN("Node graph already initialized");
        return true;
    }
    
    config = graphConfig;
    
    LOG_INFO("🚀 Initializing KC Node Graph");
    LOG_INFO("   Max Nodes: " + std::to_string(config.maxNodes));
    LOG_INFO("   Max Connections: " + std::to_string(config.maxConnections));
    LOG_INFO("   Buffer Size: " + std::to_string(config.bufferSize));
    LOG_INFO("   Logging: " + std::string(config.enableLogging ? "Enabled" : "Disabled"));
    
    // Initialize miniaudio node graph with default 2 channels
    ma_node_graph_config maGraphConfig = ma_node_graph_config_init(2);
    ma_result result = ma_node_graph_init(&maGraphConfig, NULL, &graph);
    
    if (result != MA_SUCCESS) {
        LOG_ERROR("Failed to initialize miniaudio node graph: " + std::to_string(result));
        return false;
    }
    
    initialized = true;
    LOG_INFO("✅ KC Node Graph initialized successfully");
    return true;
}



// Shutdown the node graph
void kc_node_graph::shutdown() {
    if (!initialized) {
        return;
    }
    
    LOG_INFO("🛑 Shutting down KC Node Graph");
    
    // Stop if running
    if (running) {
        stop();
    }
    
    // Cleanup nodes
    cleanup_nodes();
    
    // Uninitialize miniaudio graph
    ma_node_graph_uninit(&graph, NULL);
    
    initialized = false;
    LOG_INFO("✅ KC Node Graph shutdown complete");
}

// Check if graph is initialized
bool kc_node_graph::is_initialized() const {
    return initialized;
}

// Add decoder node (placeholder - will be implemented)
bool kc_node_graph::add_decoder_node(const std::string& filename, kc_decoder_node** ppNode) {
    if (!initialized) {
        LOG_ERROR("Cannot add decoder node - graph not initialized");
        return false;
    }
    
    // TODO: File format validation should be done in decoder node
    // For now, accept any filename
    
    LOG_INFO("📁 Adding decoder node for: " + filename);
    
    // TODO: Implement decoder node creation
    // This will be implemented when we create kc_decoder_node
    
    LOG_WARN("Decoder node creation not yet implemented");
    return false;
}

// Add gain node (placeholder - will be implemented)
bool kc_node_graph::add_gain_node(float gain, kc_gain_node** ppNode) {
    if (!initialized) {
        LOG_ERROR("Cannot add gain node - graph not initialized");
        return false;
    }
    
    LOG_INFO("🔊 Adding gain node with gain: " + std::to_string(gain));
    
    // TODO: Implement gain node creation
    // This will be implemented when we create kc_gain_node
    
    LOG_WARN("Gain node creation not yet implemented");
    return false;
}

// Add filter node (placeholder - will be implemented)
bool kc_node_graph::add_filter_node(int filterType, float frequency, float q, kc_filter_node** ppNode) {
    if (!initialized) {
        LOG_ERROR("Cannot add filter node - graph not initialized");
        return false;
    }
    
    LOG_INFO("🎛️ Adding filter node - Type: " + std::to_string(filterType) + 
             ", Freq: " + std::to_string(frequency) + "Hz, Q: " + std::to_string(q));
    
    // TODO: Implement filter node creation
    // This will be implemented when we create kc_filter_node
    
    LOG_WARN("Filter node creation not yet implemented");
    return false;
}

// Connect nodes
bool kc_node_graph::connect_nodes(ma_node* pSourceNode, ma_uint32 sourceBus, ma_node* pTargetNode, ma_uint32 targetBus) {
    if (!initialized) {
        LOG_ERROR("Cannot connect nodes - graph not initialized");
        return false;
    }
    
    if (!pSourceNode || !pTargetNode) {
        LOG_ERROR("Cannot connect nodes - invalid node pointers");
        return false;
    }
    
    if (!validate_node_connection(pSourceNode, sourceBus, pTargetNode, targetBus)) {
        LOG_ERROR("Invalid node connection");
        return false;
    }
    
    ma_result result = ma_node_attach_output_bus(pSourceNode, sourceBus, pTargetNode, targetBus);
    
    if (result != MA_SUCCESS) {
        LOG_ERROR("Failed to connect nodes: " + std::to_string(result));
        return false;
    }
    
    // Track connection
    connections.push_back(std::make_pair(pSourceNode, pTargetNode));
    
    LOG_INFO("🔗 Connected nodes successfully");
    return true;
}

// Disconnect nodes
bool kc_node_graph::disconnect_nodes(ma_node* pSourceNode, ma_uint32 sourceBus, ma_node* pTargetNode, ma_uint32 targetBus) {
    if (!initialized) {
        LOG_ERROR("Cannot disconnect nodes - graph not initialized");
        return false;
    }
    
    // TODO: Implement node disconnection
    // This requires more complex logic to handle miniaudio's node graph
    
    LOG_WARN("Node disconnection not yet implemented");
    return false;
}

// Read PCM frames from the graph
ma_uint32 kc_node_graph::read_pcm_frames(float* pFramesOut, ma_uint32 frameCount) {
    if (!initialized || !running) {
        LOG_ERROR("Cannot read frames - graph not initialized or not running");
        return 0;
    }
    
    if (!pFramesOut) {
        LOG_ERROR("Cannot read frames - invalid output buffer");
        return 0;
    }
    
    ma_uint32 framesRead = ma_node_graph_read_pcm_frames(&graph, pFramesOut, frameCount, NULL);
    
    if (framesRead == 0) {
        LOG_WARN("No frames read from graph");
    }
    
    return framesRead;
}

// Start the graph
bool kc_node_graph::start() {
    if (!initialized) {
        LOG_ERROR("Cannot start graph - not initialized");
        return false;
    }
    
    if (running) {
        LOG_WARN("Graph already running");
        return true;
    }
    
    LOG_INFO("▶️ Starting KC Node Graph");
    running = true;
    return true;
}

// Stop the graph
void kc_node_graph::stop() {
    if (!running) {
        return;
    }
    
    LOG_INFO("⏹️ Stopping KC Node Graph");
    running = false;
}

// Check if graph is running
bool kc_node_graph::is_running() const {
    return running;
}

// Get node count
ma_uint32 kc_node_graph::get_node_count() const {
    return static_cast<ma_uint32>(nodes.size());
}

// Get connection count
ma_uint32 kc_node_graph::get_connection_count() const {
    return static_cast<ma_uint32>(connections.size());
}

// Check if graph is acyclic (no cycles)
bool kc_node_graph::is_acyclic() const {
    // TODO: Implement cycle detection
    // This requires graph traversal algorithms
    
    LOG_WARN("Cycle detection not yet implemented");
    return true; // Assume acyclic for now
}

// Validate node connection
bool kc_node_graph::validate_node_connection(ma_node* pSourceNode, ma_uint32 sourceBus, ma_node* pTargetNode, ma_uint32 targetBus) {
    if (!pSourceNode || !pTargetNode) {
        return false;
    }
    
    // TODO: Implement proper validation
    // Check if source bus exists and target bus exists
    // Check if connection is valid
    
    return true; // Assume valid for now
}

// Cleanup nodes
void kc_node_graph::cleanup_nodes() {
    LOG_INFO("🧹 Cleaning up " + std::to_string(nodes.size()) + " nodes");
    
    // TODO: Implement proper node cleanup
    // This requires calling ma_node_uninit for each node
    
    nodes.clear();
    connections.clear();
}



} // namespace audio_processor
} // namespace kcobain 