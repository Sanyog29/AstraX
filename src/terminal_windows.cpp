#ifdef ASTRAX_PLATFORM_WINDOWS

#include "astrax/terminal.h"
#include <windows.h>
#include <conio.h>
#include <iostream>

// Windows 10 virtual terminal constants (not defined in older MinGW SDKs)
#ifndef ENABLE_VIRTUAL_TERMINAL_INPUT
#define ENABLE_VIRTUAL_TERMINAL_INPUT 0x0200
#endif
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

namespace astrax {

/**
 * @brief Windows Console API implementation of ITerminal
 */
class WindowsTerminal : public ITerminal {
public:
    WindowsTerminal() {
        hConsole_ = GetStdHandle(STD_OUTPUT_HANDLE);
        hConsoleInput_ = GetStdHandle(STD_INPUT_HANDLE);
        
        // Save original console mode
        GetConsoleMode(hConsoleInput_, &originalMode_);
        GetConsoleScreenBufferInfo(hConsole_, &originalBufferInfo_);
    }
    
    ~WindowsTerminal() override {
        disableRawMode();
    }
    
    // ========================================================================
    // Initialization
    // ========================================================================
    
    void enableRawMode() override {
        if (rawModeEnabled_) return;
        
        DWORD mode = 0;
        GetConsoleMode(hConsoleInput_, &mode);
        mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
        mode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
        SetConsoleMode(hConsoleInput_, mode);
        
        // Enable virtual terminal processing for output (ANSI escape codes)
        GetConsoleMode(hConsole_, &mode);
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT;
        SetConsoleMode(hConsole_, mode);
        
        rawModeEnabled_ = true;
    }
    
    void disableRawMode() override {
        if (!rawModeEnabled_) return;
        
        SetConsoleMode(hConsoleInput_, originalMode_);
        SetConsoleTextAttribute(hConsole_, originalBufferInfo_.wAttributes);
        rawModeEnabled_ = false;
    }
    
    // ========================================================================
    // Screen Operations
    // ========================================================================
    
    void clearScreen() override {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole_, &csbi);
        
        // Calculate visible window dimensions
        SHORT windowWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        SHORT windowHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        DWORD dwConSize = windowWidth * windowHeight;
        
        // Start from top-left of visible window
        COORD coordScreen = {csbi.srWindow.Left, csbi.srWindow.Top};
        DWORD cCharsWritten;
        
        // Clear the visible window area
        for (SHORT y = csbi.srWindow.Top; y <= csbi.srWindow.Bottom; ++y) {
            COORD lineStart = {csbi.srWindow.Left, y};
            FillConsoleOutputCharacter(hConsole_, ' ', windowWidth, lineStart, &cCharsWritten);
            FillConsoleOutputAttribute(hConsole_, csbi.wAttributes, windowWidth, lineStart, &cCharsWritten);
        }
        
        // Position cursor at top-left of visible window
        SetConsoleCursorPosition(hConsole_, coordScreen);
    }
    
    void clearToEndOfScreen() override {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole_, &csbi);
        
        DWORD count = (csbi.dwSize.Y - csbi.dwCursorPosition.Y) * csbi.dwSize.X 
                    - csbi.dwCursorPosition.X;
        DWORD written;
        
        FillConsoleOutputCharacter(hConsole_, ' ', count, csbi.dwCursorPosition, &written);
        FillConsoleOutputAttribute(hConsole_, csbi.wAttributes, count, csbi.dwCursorPosition, &written);
    }
    
    void clearToEndOfLine() override {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole_, &csbi);
        
        // Calculate distance to end of visible window, not buffer
        DWORD count = csbi.srWindow.Right - csbi.dwCursorPosition.X + 1;
        DWORD written;
        
        FillConsoleOutputCharacter(hConsole_, ' ', count, csbi.dwCursorPosition, &written);
        FillConsoleOutputAttribute(hConsole_, csbi.wAttributes, count, csbi.dwCursorPosition, &written);
    }
    
    void setCursor(int x, int y) override {
        // Get window origin to use window-relative coordinates
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole_, &csbi);
        
        COORD coord;
        coord.X = static_cast<SHORT>(csbi.srWindow.Left + x);
        coord.Y = static_cast<SHORT>(csbi.srWindow.Top + y);
        SetConsoleCursorPosition(hConsole_, coord);
    }
    
    void hideCursor() override {
        CONSOLE_CURSOR_INFO info;
        GetConsoleCursorInfo(hConsole_, &info);
        info.bVisible = FALSE;
        SetConsoleCursorInfo(hConsole_, &info);
    }
    
    void showCursor() override {
        CONSOLE_CURSOR_INFO info;
        GetConsoleCursorInfo(hConsole_, &info);
        info.bVisible = TRUE;
        SetConsoleCursorInfo(hConsole_, &info);
    }
    
    Size getSize() override {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole_, &csbi);
        return {
            csbi.srWindow.Right - csbi.srWindow.Left + 1,
            csbi.srWindow.Bottom - csbi.srWindow.Top + 1
        };
    }
    
    // ========================================================================
    // Output
    // ========================================================================
    
    void write(const std::string& text) override {
        DWORD written;
        WriteConsoleA(hConsole_, text.c_str(), static_cast<DWORD>(text.size()), &written, nullptr);
    }
    
    void writeChar(char c) override {
        DWORD written;
        WriteConsoleA(hConsole_, &c, 1, &written, nullptr);
    }
    
    void flush() override {
        // Windows console is synchronous, no need to flush
    }
    
    // ========================================================================
    // Colors
    // ========================================================================
    
    void setColor(Color fg, Color bg) override {
        WORD attr = 0;
        
        // Foreground
        switch (fg) {
            case Color::Black:         attr |= 0; break;
            case Color::Red:           attr |= FOREGROUND_RED; break;
            case Color::Green:         attr |= FOREGROUND_GREEN; break;
            case Color::Yellow:        attr |= FOREGROUND_RED | FOREGROUND_GREEN; break;
            case Color::Blue:          attr |= FOREGROUND_BLUE; break;
            case Color::Magenta:       attr |= FOREGROUND_RED | FOREGROUND_BLUE; break;
            case Color::Cyan:          attr |= FOREGROUND_GREEN | FOREGROUND_BLUE; break;
            case Color::White:         attr |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
            case Color::BrightBlack:   attr |= FOREGROUND_INTENSITY; break;
            case Color::BrightRed:     attr |= FOREGROUND_RED | FOREGROUND_INTENSITY; break;
            case Color::BrightGreen:   attr |= FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
            case Color::BrightYellow:  attr |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
            case Color::BrightBlue:    attr |= FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
            case Color::BrightMagenta: attr |= FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
            case Color::BrightCyan:    attr |= FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
            case Color::BrightWhite:   attr |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
            default:                   attr |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
        }
        
        // Background
        switch (bg) {
            case Color::Red:           attr |= BACKGROUND_RED; break;
            case Color::Green:         attr |= BACKGROUND_GREEN; break;
            case Color::Yellow:        attr |= BACKGROUND_RED | BACKGROUND_GREEN; break;
            case Color::Blue:          attr |= BACKGROUND_BLUE; break;
            case Color::Magenta:       attr |= BACKGROUND_RED | BACKGROUND_BLUE; break;
            case Color::Cyan:          attr |= BACKGROUND_GREEN | BACKGROUND_BLUE; break;
            case Color::White:         attr |= BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE; break;
            case Color::BrightWhite:   attr |= BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY; break;
            default: break;
        }
        
        SetConsoleTextAttribute(hConsole_, attr);
    }
    
    void resetColor() override {
        SetConsoleTextAttribute(hConsole_, originalBufferInfo_.wAttributes);
    }
    
    void setBold(bool enabled) override {
        // Windows console bold is achieved via intensity
        if (enabled) {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(hConsole_, &csbi);
            SetConsoleTextAttribute(hConsole_, csbi.wAttributes | FOREGROUND_INTENSITY);
        }
    }
    
    void setUnderline(bool /*enabled*/) override {
        // Windows console doesn't support underline in traditional mode
        // Could use ENABLE_VIRTUAL_TERMINAL_PROCESSING with ANSI codes
    }
    
    // ========================================================================
    // Input
    // ========================================================================
    
    KeyEvent readKey() override {
        KeyEvent event;
        
        int ch = _getch();
        
        // Handle extended keys (arrow keys, function keys, etc.)
        if (ch == 0 || ch == 224) {
            int ext = _getch();
            switch (ext) {
                case 72: event.key = static_cast<int>(SpecialKey::Up); break;
                case 80: event.key = static_cast<int>(SpecialKey::Down); break;
                case 75: event.key = static_cast<int>(SpecialKey::Left); break;
                case 77: event.key = static_cast<int>(SpecialKey::Right); break;
                case 71: event.key = static_cast<int>(SpecialKey::Home); break;
                case 79: event.key = static_cast<int>(SpecialKey::End); break;
                case 73: event.key = static_cast<int>(SpecialKey::PageUp); break;
                case 81: event.key = static_cast<int>(SpecialKey::PageDown); break;
                case 83: event.key = static_cast<int>(SpecialKey::Delete); break;
                case 59: event.key = static_cast<int>(SpecialKey::F1); break;
                case 60: event.key = static_cast<int>(SpecialKey::F2); break;
                case 61: event.key = static_cast<int>(SpecialKey::F3); break;
                case 62: event.key = static_cast<int>(SpecialKey::F4); break;
                case 63: event.key = static_cast<int>(SpecialKey::F5); break;
                case 64: event.key = static_cast<int>(SpecialKey::F6); break;
                case 65: event.key = static_cast<int>(SpecialKey::F7); break;
                case 66: event.key = static_cast<int>(SpecialKey::F8); break;
                case 67: event.key = static_cast<int>(SpecialKey::F9); break;
                case 68: event.key = static_cast<int>(SpecialKey::F10); break;
                default: event.key = 0; break;
            }
        } else {
            // Check for Ctrl+key combinations
            if (ch >= 1 && ch <= 26 && ch != 9 && ch != 13 && ch != 8) {
                event.key = ch + 'a' - 1;
                event.ctrl = true;
            } else {
                event.key = ch;
            }
        }
        
        return event;
    }
    
    bool hasKey() override {
        return _kbhit() != 0;
    }
    
    // ========================================================================
    // Window Management
    // ========================================================================
    
    void setTitle(const std::string& title) override {
        SetConsoleTitleA(title.c_str());
    }
    
    void openExternalWindow(const std::string& command) override {
        // Check if already in external window
        char buffer[2];
        if (GetEnvironmentVariableA("ASTRAX_EXTERNAL", buffer, sizeof(buffer)) > 0) {
            return; // Already in external window
        }
        
        // Launch in new console
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        
        std::string cmd = "start \"AstraX Editor\" cmd /k \"set ASTRAX_EXTERNAL=1 && \"";
        cmd += exePath;
        cmd += "\"";
        
        if (!command.empty()) {
            cmd += " \"" + command + "\"";
        }
        cmd += "\"";
        
        system(cmd.c_str());
    }
    
private:
    HANDLE hConsole_;
    HANDLE hConsoleInput_;
    DWORD originalMode_ = 0;
    CONSOLE_SCREEN_BUFFER_INFO originalBufferInfo_;
    bool rawModeEnabled_ = false;
};

// Factory function
std::unique_ptr<ITerminal> createTerminal() {
    return std::make_unique<WindowsTerminal>();
}

} // namespace astrax

#endif // ASTRAX_PLATFORM_WINDOWS
