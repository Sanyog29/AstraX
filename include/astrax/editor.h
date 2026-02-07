#ifndef ASTRAX_EDITOR_H
#define ASTRAX_EDITOR_H

#include "types.h"
#include "terminal.h"
#include "buffer.h"
#include "renderer.h"
#include "command.h"
#include "search.h"
#include "config.h"
#include <memory>
#include <string>

namespace astrax {

/**
 * @brief Main editor class - coordinates all components
 */
class Editor {
public:
    Editor();
    ~Editor();
    
    // ========================================================================
    // Main Loop
    // ========================================================================
    
    /// Run the editor with optional file to open
    void run(const std::string& filename = "");
    
    /// Request the editor to quit
    void quit(bool force = false);
    
    /// Check if editor should quit
    bool shouldQuit() const { return shouldQuit_; }
    
    // ========================================================================
    // Mode
    // ========================================================================
    
    /// Get current mode
    EditorMode getMode() const { return mode_; }
    
    /// Set mode
    void setMode(EditorMode mode);
    
    // ========================================================================
    // Buffer Access
    // ========================================================================
    
    /// Get the current buffer
    Buffer& getBuffer() { return *buffer_; }
    const Buffer& getBuffer() const { return *buffer_; }
    
    /// Create a new buffer
    void newBuffer();
    
    /// Open a file
    bool openFile(const std::string& filename);
    
    /// Save current file
    bool saveFile();
    
    /// Save to a new filename
    bool saveFileAs(const std::string& filename);
    
    // ========================================================================
    // Terminal Access
    // ========================================================================
    
    /// Get terminal
    ITerminal& getTerminal() { return *terminal_; }
    
    // ========================================================================
    // Search
    // ========================================================================
    
    /// Get search engine
    Search& getSearch() { return search_; }
    
    /// Search forward
    void searchForward();
    
    /// Search backward
    void searchBackward();
    
    // ========================================================================
    // Status
    // ========================================================================
    
    /// Set status message (shown in status bar)
    void setStatusMessage(const std::string& message);
    
    /// Get status message
    const std::string& getStatusMessage() const { return statusMessage_; }
    
    // ========================================================================
    // Command Mode
    // ========================================================================
    
    /// Get command buffer (for command mode input)
    const std::string& getCommandBuffer() const { return commandBuffer_; }
    
    /// Execute command mode string
    bool executeCommand(const std::string& command);
    
    // ========================================================================
    // Configuration
    // ========================================================================
    
    /// Get configuration
    Config& getConfig() { return config_; }
    const Config& getConfig() const { return config_; }
    
private:
    // ========================================================================
    // Components
    // ========================================================================
    
    std::unique_ptr<ITerminal> terminal_;
    std::unique_ptr<Buffer> buffer_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<CommandExecutor> commandExecutor_;
    KeyBindings keyBindings_;
    Search search_;
    Config config_;
    
    // ========================================================================
    // State
    // ========================================================================
    
    EditorMode mode_ = EditorMode::Normal;
    bool shouldQuit_ = false;
    std::string statusMessage_;
    std::string commandBuffer_;
    
    // ========================================================================
    // Event Processing
    // ========================================================================
    
    void processInput();
    void processNormalMode(const KeyEvent& key);
    void processInsertMode(const KeyEvent& key);
    void processCommandMode(const KeyEvent& key);
    void processVisualMode(const KeyEvent& key);
    void processSearchMode(const KeyEvent& key);
    
    // ========================================================================
    // Rendering
    // ========================================================================
    
    void render();
    
    // ========================================================================
    // Initialization
    // ========================================================================
    
    void initialize();
    void registerCommands();
    void setupKeyBindings();
};

} // namespace astrax

#endif // ASTRAX_EDITOR_H
