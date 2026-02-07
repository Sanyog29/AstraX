#ifndef ASTRAX_RENDERER_H
#define ASTRAX_RENDERER_H

#include "types.h"
#include "terminal.h"
#include "buffer.h"
#include "syntax/highlighter.h"
#include <memory>
#include <string>

namespace astrax {

/**
 * @brief Viewport for scrolling support
 */
struct Viewport {
    size_t topLine = 0;      // First visible line
    size_t leftColumn = 0;   // First visible column (for horizontal scroll)
    int height = 24;         // Visible lines
    int width = 80;          // Visible columns
    
    /// Adjust viewport to ensure position is visible
    void ensureVisible(const Position& pos);
};

/**
 * @brief Renders the buffer to the terminal
 * 
 * Handles syntax highlighting, line numbers, status bar,
 * and efficient partial screen updates.
 */
class Renderer {
public:
    explicit Renderer(ITerminal& terminal);
    
    // ========================================================================
    // Rendering
    // ========================================================================
    
    /// Full screen render
    void render(const Buffer& buffer, EditorMode mode);
    
    /// Refresh only changed portions (optimization)
    void refresh(const Buffer& buffer, EditorMode mode);
    
    /// Force full redraw on next render
    void invalidate() { needsFullRedraw_ = true; }
    
    // ========================================================================
    // Configuration
    // ========================================================================
    
    /// Set syntax highlighter
    void setHighlighter(std::unique_ptr<ISyntaxHighlighter> highlighter);
    
    /// Enable/disable line numbers
    void showLineNumbers(bool show) { showLineNumbers_ = show; }
    
    /// Enable/disable status bar
    void showStatusBar(bool show) { showStatusBar_ = show; }
    
    /// Set status message (temporary message in status bar)
    void setStatusMessage(const std::string& message) { statusMessage_ = message; }
    
    /// Set command line content (for command mode)
    void setCommandLine(const std::string& content) { commandLine_ = content; }
    
    // ========================================================================
    // Viewport
    // ========================================================================
    
    /// Get current viewport
    const Viewport& getViewport() const { return viewport_; }
    
    /// Scroll viewport
    void scroll(int lines);
    
    /// Scroll to make position visible
    void scrollToCursor(const Position& cursor);
    
private:
    ITerminal& terminal_;
    Viewport viewport_;
    std::unique_ptr<ISyntaxHighlighter> highlighter_;
    
    // Display options
    bool showLineNumbers_ = true;
    bool showStatusBar_ = true;
    int lineNumberWidth_ = 4;
    
    // Status
    std::string statusMessage_;
    std::string commandLine_;
    bool needsFullRedraw_ = true;
    
    // Theme colors
    ColorPair normalColor_{Color::White, Color::Default};
    ColorPair lineNumberColor_{Color::Yellow, Color::Default};
    ColorPair statusBarColor_{Color::Black, Color::White};
    ColorPair modeColors_[5] = {
        {Color::Blue, Color::Default},     // Normal
        {Color::Green, Color::Default},    // Insert
        {Color::Red, Color::Default},      // Command
        {Color::Magenta, Color::Default},  // Visual
        {Color::Cyan, Color::Default}      // Search
    };
    
    // ========================================================================
    // Private Methods
    // ========================================================================
    
    void renderLine(size_t lineIndex, const std::string& content, int screenY);
    void renderStatusBar(const Buffer& buffer, EditorMode mode, int screenY);
    void renderCommandLine(int screenY);
    void updateViewportSize();
    int getLineNumberWidth(size_t totalLines) const;
    
    ColorPair getModeColor(EditorMode mode) const {
        return modeColors_[static_cast<int>(mode)];
    }
};

} // namespace astrax

#endif // ASTRAX_RENDERER_H
