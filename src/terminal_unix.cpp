#if defined(ASTRAX_PLATFORM_LINUX) || defined(ASTRAX_PLATFORM_MACOS)

#include "astrax/terminal.h"
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace astrax {

/**
 * @brief Unix/Linux/macOS implementation of ITerminal using termios and ANSI escape codes
 */
class UnixTerminal : public ITerminal {
public:
    UnixTerminal() {
        tcgetattr(STDIN_FILENO, &originalTermios_);
    }
    
    ~UnixTerminal() override {
        disableRawMode();
    }
    
    // ========================================================================
    // Initialization
    // ========================================================================
    
    void enableRawMode() override {
        if (rawModeEnabled_) return;
        
        struct termios raw = originalTermios_;
        
        // Input flags: disable break, CR to NL, parity, strip, flow control
        raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        
        // Output flags: disable post-processing
        raw.c_oflag &= ~(OPOST);
        
        // Control flags: set 8-bit chars
        raw.c_cflag |= (CS8);
        
        // Local flags: disable echo, canonical mode, signals, extended input
        raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        
        // Control chars: read timeout
        raw.c_cc[VMIN] = 1;   // min chars to read
        raw.c_cc[VTIME] = 0;  // no timeout
        
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        rawModeEnabled_ = true;
    }
    
    void disableRawMode() override {
        if (!rawModeEnabled_) return;
        
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios_);
        rawModeEnabled_ = false;
    }
    
    // ========================================================================
    // Screen Operations
    // ========================================================================
    
    void clearScreen() override {
        write("\x1b[2J\x1b[H");  // Clear screen and move cursor home
    }
    
    void clearToEndOfScreen() override {
        write("\x1b[J");
    }
    
    void clearToEndOfLine() override {
        write("\x1b[K");
    }
    
    void setCursor(int x, int y) override {
        char buf[32];
        snprintf(buf, sizeof(buf), "\x1b[%d;%dH", y + 1, x + 1);
        write(buf);
    }
    
    void hideCursor() override {
        write("\x1b[?25l");
    }
    
    void showCursor() override {
        write("\x1b[?25h");
    }
    
    Size getSize() override {
        struct winsize ws;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
            return {80, 24};  // Default fallback
        }
        return {ws.ws_col, ws.ws_row};
    }
    
    // ========================================================================
    // Output
    // ========================================================================
    
    void write(const std::string& text) override {
        ::write(STDOUT_FILENO, text.c_str(), text.size());
    }
    
    void writeChar(char c) override {
        ::write(STDOUT_FILENO, &c, 1);
    }
    
    void flush() override {
        fflush(stdout);
    }
    
    // ========================================================================
    // Colors (ANSI escape codes)
    // ========================================================================
    
    void setColor(Color fg, Color bg) override {
        char buf[32];
        int fgCode = colorToAnsi(fg, false);
        int bgCode = colorToAnsi(bg, true);
        
        if (fgCode >= 0 && bgCode >= 0) {
            snprintf(buf, sizeof(buf), "\x1b[%d;%dm", fgCode, bgCode);
        } else if (fgCode >= 0) {
            snprintf(buf, sizeof(buf), "\x1b[%dm", fgCode);
        } else if (bgCode >= 0) {
            snprintf(buf, sizeof(buf), "\x1b[%dm", bgCode);
        } else {
            return;
        }
        write(buf);
    }
    
    void resetColor() override {
        write("\x1b[0m");
    }
    
    void setBold(bool enabled) override {
        write(enabled ? "\x1b[1m" : "\x1b[22m");
    }
    
    void setUnderline(bool enabled) override {
        write(enabled ? "\x1b[4m" : "\x1b[24m");
    }
    
    // ========================================================================
    // Input
    // ========================================================================
    
    KeyEvent readKey() override {
        KeyEvent event;
        char c;
        
        if (read(STDIN_FILENO, &c, 1) != 1) {
            return event;
        }
        
        // Check for escape sequence
        if (c == '\x1b') {
            char seq[4];
            if (read(STDIN_FILENO, &seq[0], 1) != 1) {
                event.key = static_cast<int>(SpecialKey::Escape);
                return event;
            }
            
            if (seq[0] == '[') {
                if (read(STDIN_FILENO, &seq[1], 1) != 1) {
                    return event;
                }
                
                if (seq[1] >= '0' && seq[1] <= '9') {
                    if (read(STDIN_FILENO, &seq[2], 1) != 1) {
                        return event;
                    }
                    if (seq[2] == '~') {
                        switch (seq[1]) {
                            case '1': event.key = static_cast<int>(SpecialKey::Home); break;
                            case '3': event.key = static_cast<int>(SpecialKey::Delete); break;
                            case '4': event.key = static_cast<int>(SpecialKey::End); break;
                            case '5': event.key = static_cast<int>(SpecialKey::PageUp); break;
                            case '6': event.key = static_cast<int>(SpecialKey::PageDown); break;
                        }
                    }
                } else {
                    switch (seq[1]) {
                        case 'A': event.key = static_cast<int>(SpecialKey::Up); break;
                        case 'B': event.key = static_cast<int>(SpecialKey::Down); break;
                        case 'C': event.key = static_cast<int>(SpecialKey::Right); break;
                        case 'D': event.key = static_cast<int>(SpecialKey::Left); break;
                        case 'H': event.key = static_cast<int>(SpecialKey::Home); break;
                        case 'F': event.key = static_cast<int>(SpecialKey::End); break;
                    }
                }
            } else if (seq[0] == 'O') {
                if (read(STDIN_FILENO, &seq[1], 1) != 1) {
                    return event;
                }
                switch (seq[1]) {
                    case 'H': event.key = static_cast<int>(SpecialKey::Home); break;
                    case 'F': event.key = static_cast<int>(SpecialKey::End); break;
                    case 'P': event.key = static_cast<int>(SpecialKey::F1); break;
                    case 'Q': event.key = static_cast<int>(SpecialKey::F2); break;
                    case 'R': event.key = static_cast<int>(SpecialKey::F3); break;
                    case 'S': event.key = static_cast<int>(SpecialKey::F4); break;
                }
            } else {
                event.key = static_cast<int>(SpecialKey::Escape);
                event.alt = true;
            }
        } else if (c >= 1 && c <= 26 && c != '\t' && c != '\r' && c != '\n') {
            // Ctrl+key
            event.key = c + 'a' - 1;
            event.ctrl = true;
        } else if (c == 127) {
            event.key = static_cast<int>(SpecialKey::Backspace);
        } else if (c == '\r' || c == '\n') {
            event.key = static_cast<int>(SpecialKey::Enter);
        } else if (c == '\t') {
            event.key = static_cast<int>(SpecialKey::Tab);
        } else {
            event.key = c;
        }
        
        return event;
    }
    
    bool hasKey() override {
        struct timeval tv = {0, 0};
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        return select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0;
    }
    
    // ========================================================================
    // Window Management
    // ========================================================================
    
    void setTitle(const std::string& title) override {
        write("\x1b]0;" + title + "\x07");
    }
    
    void openExternalWindow(const std::string& command) override {
        // Check for common terminal emulators
        const char* term = getenv("TERM");
        if (!term) term = "xterm";
        
        std::string argv0;
        if (getenv("ASTRAX_EXTERNAL")) {
            return; // Already in external window
        }
        
        // Get executable path
        char exePath[1024];
        ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
        if (len == -1) {
            return;
        }
        exePath[len] = '\0';
        
        // Launch in new terminal
        std::string cmd = "ASTRAX_EXTERNAL=1 ";
        if (strstr(term, "gnome")) {
            cmd = "gnome-terminal -- " + cmd;
        } else if (strstr(term, "konsole")) {
            cmd = "konsole -e " + cmd;
        } else {
            cmd = "xterm -e " + cmd;
        }
        cmd += exePath;
        if (!command.empty()) {
            cmd += " \"" + command + "\"";
        }
        cmd += " &";
        
        system(cmd.c_str());
    }
    
private:
    struct termios originalTermios_;
    bool rawModeEnabled_ = false;
    
    int colorToAnsi(Color color, bool background) const {
        int base = background ? 40 : 30;
        int brightBase = background ? 100 : 90;
        
        switch (color) {
            case Color::Default:       return -1;
            case Color::Black:         return base + 0;
            case Color::Red:           return base + 1;
            case Color::Green:         return base + 2;
            case Color::Yellow:        return base + 3;
            case Color::Blue:          return base + 4;
            case Color::Magenta:       return base + 5;
            case Color::Cyan:          return base + 6;
            case Color::White:         return base + 7;
            case Color::BrightBlack:   return brightBase + 0;
            case Color::BrightRed:     return brightBase + 1;
            case Color::BrightGreen:   return brightBase + 2;
            case Color::BrightYellow:  return brightBase + 3;
            case Color::BrightBlue:    return brightBase + 4;
            case Color::BrightMagenta: return brightBase + 5;
            case Color::BrightCyan:    return brightBase + 6;
            case Color::BrightWhite:   return brightBase + 7;
            default:                   return -1;
        }
    }
};

// Factory function
std::unique_ptr<ITerminal> createTerminal() {
    return std::make_unique<UnixTerminal>();
}

} // namespace astrax

#endif // ASTRAX_PLATFORM_LINUX || ASTRAX_PLATFORM_MACOS
