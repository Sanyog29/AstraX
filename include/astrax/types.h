#ifndef ASTRAX_TYPES_H
#define ASTRAX_TYPES_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace astrax {

// ============================================================================
// Basic Types
// ============================================================================

/// Position in the buffer (0-indexed)
struct Position {
    size_t line = 0;
    size_t column = 0;
    
    bool operator==(const Position& other) const {
        return line == other.line && column == other.column;
    }
    
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
    
    bool operator<(const Position& other) const {
        if (line != other.line) return line < other.line;
        return column < other.column;
    }
};

/// Range in the buffer
struct Range {
    Position start;
    Position end;
    
    bool contains(const Position& pos) const {
        return !(pos < start) && pos < end;
    }
    
    bool isEmpty() const {
        return start == end;
    }
};

/// Terminal size
struct Size {
    int width = 80;
    int height = 24;
};

// ============================================================================
// Editor Modes
// ============================================================================

enum class EditorMode {
    Normal,
    Insert,
    Command,
    Visual,
    Search
};

inline const char* modeToString(EditorMode mode) {
    switch (mode) {
        case EditorMode::Normal:  return "NORMAL";
        case EditorMode::Insert:  return "INSERT";
        case EditorMode::Command: return "COMMAND";
        case EditorMode::Visual:  return "VISUAL";
        case EditorMode::Search:  return "SEARCH";
        default:                   return "UNKNOWN";
    }
}

// ============================================================================
// Colors
// ============================================================================

enum class Color : uint8_t {
    Default = 0,
    Black,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
    BrightBlack,
    BrightRed,
    BrightGreen,
    BrightYellow,
    BrightBlue,
    BrightMagenta,
    BrightCyan,
    BrightWhite
};

struct ColorPair {
    Color foreground = Color::Default;
    Color background = Color::Default;
};

// ============================================================================
// Syntax Highlighting
// ============================================================================

enum class TokenType {
    Default,
    Keyword,
    Type,
    String,
    Number,
    Comment,
    Preprocessor,
    Function,
    Operator,
    Bracket
};

struct Token {
    size_t start;
    size_t length;
    TokenType type;
};

// ============================================================================
// Key Input
// ============================================================================

enum class SpecialKey : int {
    None = 0,
    Escape = 27,
    Enter = 13,
    Backspace = 8,
    Delete = 127,
    Tab = 9,
    
    // Arrow keys (values > 256 to avoid conflict with ASCII)
    Up = 1000,
    Down,
    Left,
    Right,
    Home,
    End,
    PageUp,
    PageDown,
    
    // Function keys
    F1 = 1100,
    F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12
};

struct KeyEvent {
    int key = 0;                    // ASCII char or SpecialKey value
    bool ctrl = false;
    bool alt = false;
    bool shift = false;
    
    /// Check if key is a printable ASCII character (32-126)
    bool isPrintable() const { 
        return key >= 32 && key < 127 && !ctrl && !alt; 
    }
    
    /// Check if key is any character (0-255) without modifiers
    bool isChar() const { return key > 0 && key < 256 && !ctrl && !alt; }
    
    /// Check if key is a special/extended key (arrow keys, function keys, etc.)
    bool isSpecial() const { return key >= 1000; }
    
    /// Check for specific control keys
    bool isEscape() const { return key == 27; }
    bool isEnter() const { return key == 13; }
    bool isBackspace() const { return key == 8 || key == 127; }
    bool isTab() const { return key == 9; }
    
    char toChar() const { return static_cast<char>(key); }
    SpecialKey toSpecial() const { return static_cast<SpecialKey>(key); }
};

// ============================================================================
// Configuration Types
// ============================================================================

struct EditorConfig {
    bool showLineNumbers = true;
    bool showStatusBar = true;
    bool syntaxHighlighting = true;
    bool autoIndent = true;
    int tabSize = 4;
    bool expandTabs = true;
    std::string theme = "default";
    std::string colorScheme = "dark";
};

// ============================================================================
// Utility Functions
// ============================================================================

/// Convert EditorMode to readable string
inline std::string toString(EditorMode mode) {
    return modeToString(mode);
}

} // namespace astrax

#endif // ASTRAX_TYPES_H
