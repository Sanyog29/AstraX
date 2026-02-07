#ifndef ASTRAX_TERMINAL_H
#define ASTRAX_TERMINAL_H

#include "types.h"
#include <memory>
#include <string>

namespace astrax {

/**
 * @brief Abstract interface for terminal operations
 * 
 * This interface abstracts platform-specific terminal handling,
 * allowing the editor to work on Windows, Linux, and macOS.
 */
class ITerminal {
public:
    virtual ~ITerminal() = default;
    
    // ========================================================================
    // Initialization
    // ========================================================================
    
    /// Enable raw mode for direct input handling
    virtual void enableRawMode() = 0;
    
    /// Restore original terminal settings
    virtual void disableRawMode() = 0;
    
    // ========================================================================
    // Screen Operations
    // ========================================================================
    
    /// Clear the entire screen
    virtual void clearScreen() = 0;
    
    /// Clear from cursor to end of screen
    virtual void clearToEndOfScreen() = 0;
    
    /// Clear from cursor to end of line
    virtual void clearToEndOfLine() = 0;
    
    /// Set cursor position (0-indexed)
    virtual void setCursor(int x, int y) = 0;
    
    /// Hide the cursor
    virtual void hideCursor() = 0;
    
    /// Show the cursor
    virtual void showCursor() = 0;
    
    /// Get terminal size
    virtual Size getSize() = 0;
    
    // ========================================================================
    // Output
    // ========================================================================
    
    /// Write a string at current cursor position
    virtual void write(const std::string& text) = 0;
    
    /// Write a character at current cursor position
    virtual void writeChar(char c) = 0;
    
    /// Flush output buffer
    virtual void flush() = 0;
    
    // ========================================================================
    // Colors
    // ========================================================================
    
    /// Set foreground and background color
    virtual void setColor(Color fg, Color bg = Color::Default) = 0;
    
    /// Reset colors to default
    virtual void resetColor() = 0;
    
    /// Set bold text
    virtual void setBold(bool enabled) = 0;
    
    /// Set underlined text
    virtual void setUnderline(bool enabled) = 0;
    
    // ========================================================================
    // Input
    // ========================================================================
    
    /// Read a key event (blocking)
    virtual KeyEvent readKey() = 0;
    
    /// Check if a key is available (non-blocking)
    virtual bool hasKey() = 0;
    
    // ========================================================================
    // Window Management
    // ========================================================================
    
    /// Set the terminal window title
    virtual void setTitle(const std::string& title) = 0;
    
    /// Request editor to open in external window
    virtual void openExternalWindow(const std::string& command) = 0;
};

/**
 * @brief Factory function to create platform-specific terminal
 */
std::unique_ptr<ITerminal> createTerminal();

/**
 * @brief RAII wrapper for terminal raw mode
 */
class RawModeGuard {
public:
    explicit RawModeGuard(ITerminal& terminal) : terminal_(terminal) {
        terminal_.enableRawMode();
    }
    
    ~RawModeGuard() {
        terminal_.disableRawMode();
    }
    
    // Non-copyable
    RawModeGuard(const RawModeGuard&) = delete;
    RawModeGuard& operator=(const RawModeGuard&) = delete;
    
private:
    ITerminal& terminal_;
};

} // namespace astrax

#endif // ASTRAX_TERMINAL_H
