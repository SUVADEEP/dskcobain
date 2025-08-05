#pragma once

#include "../../external/miniaudio.h"
#include <string>
#include <memory>
#include <vector>

namespace kcobain {
namespace audio_processor {

// Forward declarations for our custom nodes
struct kc_decoder_node;
struct kc_gain_node;
struct kc_filter_node;

// Node graph configuration (graph-level settings only)
struct kc_node_graph_config {
    ma_uint32 maxNodes;           // Maximum number of nodes in graph
    ma_uint32 maxConnections;     // Maximum number of connections
    ma_uint32 bufferSize;         // Processing buffer size in frames
    bool enableLogging;           // Enable graph-level logging
    bool enableValidation;        // Enable connection validation
    
    kc_node_graph_config() : maxNodes(16), maxConnections(32), bufferSize(1024), enableLogging(true), enableValidation(true) {}
};



// Main node graph manager
class kc_node_graph {
public:
    kc_node_graph();
    ~kc_node_graph();
    
    // Initialization
    bool initialize(const kc_node_graph_config& config);
    void shutdown();
    bool is_initialized() const;
    
    // Node management
    bool add_decoder_node(const std::string& filename, kc_decoder_node** ppNode);
    bool add_gain_node(float gain, kc_gain_node** ppNode);
    bool add_filter_node(int filterType, float frequency, float q, kc_filter_node** ppNode);
    
    // Node connections
    bool connect_nodes(ma_node* pSourceNode, ma_uint32 sourceBus, ma_node* pTargetNode, ma_uint32 targetBus);
    bool disconnect_nodes(ma_node* pSourceNode, ma_uint32 sourceBus, ma_node* pTargetNode, ma_uint32 targetBus);
    
    // Graph execution
    ma_uint32 read_pcm_frames(float* pFramesOut, ma_uint32 frameCount);
    bool start();
    void stop();
    bool is_running() const;
    

    
    // Graph information
    ma_uint32 get_node_count() const;
    ma_uint32 get_connection_count() const;
    bool is_acyclic() const;
    
    // Access to miniaudio graph
    ma_node_graph* get_ma_graph() { return &graph; }
    const ma_node_graph* get_ma_graph() const { return &graph; }
    
    // Configuration
    const kc_node_graph_config& get_config() const { return config; }
    
private:
    ma_node_graph graph;                     // Miniaudio node graph
    kc_node_graph_config config;             // Graph configuration
    bool initialized;                        // Initialization status
    bool running;                           // Running status
    
    // Node tracking
    std::vector<ma_node*> nodes;            // Track all nodes
    std::vector<std::pair<ma_node*, ma_node*> > connections; // Track connections
    
    // Helper methods
    bool validate_node_connection(ma_node* pSourceNode, ma_uint32 sourceBus, ma_node* pTargetNode, ma_uint32 targetBus);
    void cleanup_nodes();
};



} // namespace audio_processor
} // namespace kcobain 