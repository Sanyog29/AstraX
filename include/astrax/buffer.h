#ifndef ASTRAX_BUFFER_H
#define ASTRAX_BUFFER_H

#include "types.h"
#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <functional>

namespace astrax {

/**
 * @brief Text buffer with undo/redo support
 * 
 * Stores document content as a vector of lines with efficient
 * insertion/deletion and full undo/redo history.
 */
class Buffer {
public:
    Buffer();
    explicit Buffer(const std::string& content);
    
    // ========================================================================
    // Content Access
    // ========================================================================
    
    /// Get number of lines
    size_t lineCount() const { return lines_.empty() ? 1 : lines_.size(); }
    
    /// Get a specific line (0-indexed)
    const std::string& getLine(size_t index) const;
    
    /// Get all content as a single string
    std::string getContent() const;
    
    /// Check if buffer is empty
    bool isEmpty() const { return lines_.empty() || (lines_.size() == 1 && lines_[0].empty()); }
    
    /// Check if buffer has been modified
    bool isModified() const { return modified_; }
    
    /// Mark buffer as saved
    void markSaved() { modified_ = false; savedUndoIndex_ = undoStack_.size(); }
    
    // ========================================================================
    // Cursor
    // ========================================================================
    
    /// Get current cursor position
    Position getCursor() const { return cursor_; }
    
    /// Set cursor position (clamped to valid range)
    void setCursor(Position pos);
    
    /// Move cursor by delta
    void moveCursor(int dx, int dy);
    
    /// Move cursor to start of line
    void moveToLineStart();
    
    /// Move cursor to end of line
    void moveToLineEnd();
    
    /// Move cursor to start of buffer
    void moveToBufferStart();
    
    /// Move cursor to end of buffer
    void moveToBufferEnd();
    
    /// Move cursor forward one word
    void moveForwardWord();
    
    /// Move cursor backward one word
    void moveBackwardWord();
    
    // ========================================================================
    // Editing
    // ========================================================================
    
    /// Insert a character at cursor
    void insertChar(char c);
    
    /// Insert a string at cursor
    void insertString(const std::string& text);
    
    /// Insert a newline at cursor
    void insertNewline();
    
    /// Delete character before cursor (backspace)
    void deleteCharBefore();
    
    /// Delete character at cursor (delete key)
    void deleteCharAt();
    
    /// Delete current line
    void deleteLine();
    
    /// Delete from cursor to end of line
    void deleteToEndOfLine();
    
    // ========================================================================
    // Line Operations
    // ========================================================================
    
    /// Insert a new line below current and move cursor
    void insertLineBelow();
    
    /// Insert a new line above current and move cursor
    void insertLineAbove();
    
    /// Join current line with next line
    void joinLines();
    
    // ========================================================================
    // Undo/Redo
    // ========================================================================
    
    /// Undo last operation
    void undo();
    
    /// Redo last undone operation
    void redo();
    
    /// Check if undo is available
    bool canUndo() const { return !undoStack_.empty(); }
    
    /// Check if redo is available
    bool canRedo() const { return !redoStack_.empty(); }
    
    // ========================================================================
    // Clipboard
    // ========================================================================
    
    /// Yank (copy) current line
    void yankLine();
    
    /// Yank selection or word
    void yankSelection(const Range& range);
    
    /// Paste yanked content
    void paste();
    
    /// Paste yanked content before cursor
    void pasteBefore();
    
    /// Get yanked content
    const std::string& getYanked() const { return yankBuffer_; }
    
    // ========================================================================
    // File I/O
    // ========================================================================
    
    /// Load content from file
    bool loadFromFile(const std::string& filename);
    
    /// Save content to file
    bool saveToFile(const std::string& filename);
    
    /// Get current filename
    const std::string& getFilename() const { return filename_; }
    
    /// Set filename
    void setFilename(const std::string& filename) { filename_ = filename; }
    
private:
    // ========================================================================
    // Undo System
    // ========================================================================
    
    struct UndoState {
        std::vector<std::string> lines;
        Position cursor;
    };
    
    void pushUndoState();
    void clearRedoStack();
    
    // ========================================================================
    // Data Members
    // ========================================================================
    
    std::vector<std::string> lines_;
    Position cursor_{0, 0};
    std::string filename_;
    bool modified_ = false;
    
    // Undo/redo
    std::deque<UndoState> undoStack_;
    std::deque<UndoState> redoStack_;
    size_t savedUndoIndex_ = 0;
    static constexpr size_t MAX_UNDO_SIZE = 1000;
    
    // Clipboard
    std::string yankBuffer_;
    bool yankIsLine_ = false;  // Was a full line yanked?
};

} // namespace astrax

#endif // ASTRAX_BUFFER_H
