#include "astrax/buffer.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace astrax {

// ============================================================================
// Constructors
// ============================================================================

Buffer::Buffer() {
    lines_.push_back("");
}

Buffer::Buffer(const std::string& content) {
    std::istringstream stream(content);
    std::string line;
    while (std::getline(stream, line)) {
        // Remove carriage return if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines_.push_back(line);
    }
    if (lines_.empty()) {
        lines_.push_back("");
    }
}

// ============================================================================
// Content Access
// ============================================================================

const std::string& Buffer::getLine(size_t index) const {
    static const std::string empty;
    if (index >= lines_.size()) {
        return empty;
    }
    return lines_[index];
}

std::string Buffer::getContent() const {
    std::ostringstream oss;
    for (size_t i = 0; i < lines_.size(); ++i) {
        oss << lines_[i];
        if (i < lines_.size() - 1) {
            oss << '\n';
        }
    }
    return oss.str();
}

// ============================================================================
// Cursor
// ============================================================================

void Buffer::setCursor(Position pos) {
    cursor_.line = std::min(pos.line, lines_.size() > 0 ? lines_.size() - 1 : 0);
    if (cursor_.line < lines_.size()) {
        cursor_.column = std::min(pos.column, lines_[cursor_.line].size());
    } else {
        cursor_.column = 0;
    }
}

void Buffer::moveCursor(int dx, int dy) {
    // Vertical movement
    if (dy != 0) {
        if (dy < 0 && cursor_.line >= static_cast<size_t>(-dy)) {
            cursor_.line += static_cast<size_t>(dy);
        } else if (dy < 0) {
            cursor_.line = 0;
        } else if (cursor_.line + static_cast<size_t>(dy) < lines_.size()) {
            cursor_.line += static_cast<size_t>(dy);
        } else {
            cursor_.line = lines_.size() - 1;
        }
        // Clamp column after vertical move
        cursor_.column = std::min(cursor_.column, lines_[cursor_.line].size());
    }
    
    // Horizontal movement
    if (dx != 0) {
        if (dx < 0 && cursor_.column >= static_cast<size_t>(-dx)) {
            cursor_.column += static_cast<size_t>(dx);
        } else if (dx < 0) {
            cursor_.column = 0;
        } else if (cursor_.column + static_cast<size_t>(dx) <= lines_[cursor_.line].size()) {
            cursor_.column += static_cast<size_t>(dx);
        } else {
            cursor_.column = lines_[cursor_.line].size();
        }
    }
}

void Buffer::moveToLineStart() {
    cursor_.column = 0;
}

void Buffer::moveToLineEnd() {
    cursor_.column = lines_[cursor_.line].size();
}

void Buffer::moveToBufferStart() {
    cursor_.line = 0;
    cursor_.column = 0;
}

void Buffer::moveToBufferEnd() {
    cursor_.line = lines_.size() - 1;
    cursor_.column = lines_[cursor_.line].size();
}

void Buffer::moveForwardWord() {
    const std::string& line = lines_[cursor_.line];
    size_t pos = cursor_.column;
    
    if (pos >= line.size()) {
        // Move to next line
        if (cursor_.line < lines_.size() - 1) {
            cursor_.line++;
            cursor_.column = 0;
        }
        return;
    }
    
    // Skip current word
    while (pos < line.size() && std::isalnum(static_cast<unsigned char>(line[pos]))) {
        pos++;
    }
    
    // Skip spaces
    while (pos < line.size() && !std::isalnum(static_cast<unsigned char>(line[pos]))) {
        pos++;
    }
    
    if (pos >= line.size() && cursor_.line < lines_.size() - 1) {
        cursor_.line++;
        cursor_.column = 0;
    } else {
        cursor_.column = pos;
    }
}

void Buffer::moveBackwardWord() {
    const std::string& line = lines_[cursor_.line];
    int pos = static_cast<int>(cursor_.column) - 1;
    
    if (pos < 0) {
        // Move to previous line
        if (cursor_.line > 0) {
            cursor_.line--;
            cursor_.column = lines_[cursor_.line].size();
        }
        return;
    }
    
    // Skip spaces
    while (pos >= 0 && !std::isalnum(static_cast<unsigned char>(line[static_cast<size_t>(pos)]))) {
        pos--;
    }
    
    // Skip word
    while (pos >= 0 && std::isalnum(static_cast<unsigned char>(line[static_cast<size_t>(pos)]))) {
        pos--;
    }
    
    cursor_.column = static_cast<size_t>(pos + 1);
}

// ============================================================================
// Editing
// ============================================================================

void Buffer::insertChar(char c) {
    pushUndoState();
    clearRedoStack();
    
    if (cursor_.line >= lines_.size()) {
        lines_.push_back("");
    }
    
    lines_[cursor_.line].insert(cursor_.column, 1, c);
    cursor_.column++;
    modified_ = true;
}

void Buffer::insertString(const std::string& text) {
    pushUndoState();
    clearRedoStack();
    
    for (char c : text) {
        if (c == '\n') {
            insertNewline();
        } else {
            if (cursor_.line >= lines_.size()) {
                lines_.push_back("");
            }
            lines_[cursor_.line].insert(cursor_.column, 1, c);
            cursor_.column++;
        }
    }
    modified_ = true;
}

void Buffer::insertNewline() {
    pushUndoState();
    clearRedoStack();
    
    std::string& currentLine = lines_[cursor_.line];
    std::string remainder = currentLine.substr(cursor_.column);
    currentLine = currentLine.substr(0, cursor_.column);
    
    lines_.insert(lines_.begin() + static_cast<long>(cursor_.line) + 1, remainder);
    cursor_.line++;
    cursor_.column = 0;
    modified_ = true;
}

void Buffer::deleteCharBefore() {
    if (cursor_.column > 0) {
        pushUndoState();
        clearRedoStack();
        
        lines_[cursor_.line].erase(cursor_.column - 1, 1);
        cursor_.column--;
        modified_ = true;
    } else if (cursor_.line > 0) {
        pushUndoState();
        clearRedoStack();
        
        cursor_.column = lines_[cursor_.line - 1].size();
        lines_[cursor_.line - 1] += lines_[cursor_.line];
        lines_.erase(lines_.begin() + static_cast<long>(cursor_.line));
        cursor_.line--;
        modified_ = true;
    }
}

void Buffer::deleteCharAt() {
    if (cursor_.column < lines_[cursor_.line].size()) {
        pushUndoState();
        clearRedoStack();
        
        lines_[cursor_.line].erase(cursor_.column, 1);
        modified_ = true;
    } else if (cursor_.line < lines_.size() - 1) {
        pushUndoState();
        clearRedoStack();
        
        lines_[cursor_.line] += lines_[cursor_.line + 1];
        lines_.erase(lines_.begin() + static_cast<long>(cursor_.line) + 1);
        modified_ = true;
    }
}

void Buffer::deleteLine() {
    pushUndoState();
    clearRedoStack();
    
    if (lines_.size() > 1) {
        lines_.erase(lines_.begin() + static_cast<long>(cursor_.line));
        if (cursor_.line >= lines_.size()) {
            cursor_.line = lines_.size() - 1;
        }
    } else {
        lines_[0].clear();
    }
    cursor_.column = 0;
    modified_ = true;
}

void Buffer::deleteToEndOfLine() {
    pushUndoState();
    clearRedoStack();
    
    lines_[cursor_.line] = lines_[cursor_.line].substr(0, cursor_.column);
    modified_ = true;
}

// ============================================================================
// Line Operations
// ============================================================================

void Buffer::insertLineBelow() {
    pushUndoState();
    clearRedoStack();
    
    lines_.insert(lines_.begin() + static_cast<long>(cursor_.line) + 1, "");
    cursor_.line++;
    cursor_.column = 0;
    modified_ = true;
}

void Buffer::insertLineAbove() {
    pushUndoState();
    clearRedoStack();
    
    lines_.insert(lines_.begin() + static_cast<long>(cursor_.line), "");
    cursor_.column = 0;
    modified_ = true;
}

void Buffer::joinLines() {
    if (cursor_.line < lines_.size() - 1) {
        pushUndoState();
        clearRedoStack();
        
        cursor_.column = lines_[cursor_.line].size();
        if (!lines_[cursor_.line].empty() && !lines_[cursor_.line + 1].empty()) {
            lines_[cursor_.line] += " ";
            cursor_.column++;
        }
        lines_[cursor_.line] += lines_[cursor_.line + 1];
        lines_.erase(lines_.begin() + static_cast<long>(cursor_.line) + 1);
        modified_ = true;
    }
}

// ============================================================================
// Undo/Redo
// ============================================================================

void Buffer::pushUndoState() {
    UndoState state;
    state.lines = lines_;
    state.cursor = cursor_;
    
    undoStack_.push_back(std::move(state));
    
    // Limit undo stack size
    while (undoStack_.size() > MAX_UNDO_SIZE) {
        undoStack_.pop_front();
    }
}

void Buffer::clearRedoStack() {
    redoStack_.clear();
}

void Buffer::undo() {
    if (undoStack_.empty()) {
        return;
    }
    
    // Save current state to redo
    UndoState redoState;
    redoState.lines = lines_;
    redoState.cursor = cursor_;
    redoStack_.push_back(std::move(redoState));
    
    // Restore from undo
    UndoState& state = undoStack_.back();
    lines_ = std::move(state.lines);
    cursor_ = state.cursor;
    undoStack_.pop_back();
    
    modified_ = (undoStack_.size() != savedUndoIndex_);
}

void Buffer::redo() {
    if (redoStack_.empty()) {
        return;
    }
    
    // Save current state to undo
    UndoState undoState;
    undoState.lines = lines_;
    undoState.cursor = cursor_;
    undoStack_.push_back(std::move(undoState));
    
    // Restore from redo
    UndoState& state = redoStack_.back();
    lines_ = std::move(state.lines);
    cursor_ = state.cursor;
    redoStack_.pop_back();
    
    modified_ = (undoStack_.size() != savedUndoIndex_);
}

// ============================================================================
// Clipboard
// ============================================================================

void Buffer::yankLine() {
    yankBuffer_ = lines_[cursor_.line];
    yankIsLine_ = true;
}

void Buffer::yankSelection(const Range& range) {
    // TODO: Implement selection yanking
    yankIsLine_ = false;
}

void Buffer::paste() {
    if (yankBuffer_.empty()) {
        return;
    }
    
    pushUndoState();
    clearRedoStack();
    
    if (yankIsLine_) {
        lines_.insert(lines_.begin() + static_cast<long>(cursor_.line) + 1, yankBuffer_);
        cursor_.line++;
        cursor_.column = 0;
    } else {
        lines_[cursor_.line].insert(cursor_.column, yankBuffer_);
        cursor_.column += yankBuffer_.size();
    }
    modified_ = true;
}

void Buffer::pasteBefore() {
    if (yankBuffer_.empty()) {
        return;
    }
    
    pushUndoState();
    clearRedoStack();
    
    if (yankIsLine_) {
        lines_.insert(lines_.begin() + static_cast<long>(cursor_.line), yankBuffer_);
        cursor_.column = 0;
    } else {
        lines_[cursor_.line].insert(cursor_.column, yankBuffer_);
    }
    modified_ = true;
}

// ============================================================================
// File I/O
// ============================================================================

bool Buffer::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    lines_.clear();
    std::string line;
    while (std::getline(file, line)) {
        // Remove carriage return if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines_.push_back(line);
    }
    
    if (lines_.empty()) {
        lines_.push_back("");
    }
    
    filename_ = filename;
    cursor_ = {0, 0};
    modified_ = false;
    undoStack_.clear();
    redoStack_.clear();
    savedUndoIndex_ = 0;
    
    return true;
}

bool Buffer::saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    for (size_t i = 0; i < lines_.size(); ++i) {
        file << lines_[i];
        if (i < lines_.size() - 1) {
            file << '\n';
        }
    }
    
    filename_ = filename;
    markSaved();
    return true;
}

} // namespace astrax
