#include "astrax/renderer.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace astrax {

// ============================================================================
// Viewport
// ============================================================================

void Viewport::ensureVisible(const Position& pos) {
    // Vertical scrolling
    if (pos.line < topLine) {
        topLine = pos.line;
    } else if (pos.line >= topLine + static_cast<size_t>(height) - 2) {  // -2 for status bar
        topLine = pos.line - static_cast<size_t>(height) + 3;
    }
    
    // Horizontal scrolling
    if (pos.column < leftColumn) {
        leftColumn = pos.column;
    } else if (pos.column >= leftColumn + static_cast<size_t>(width) - 6) {  // -6 for line numbers
        leftColumn = pos.column - static_cast<size_t>(width) + 7;
    }
}

// ============================================================================
// Renderer
// ============================================================================

Renderer::Renderer(ITerminal& terminal) : terminal_(terminal) {
    updateViewportSize();
}

void Renderer::updateViewportSize() {
    Size size = terminal_.getSize();
    viewport_.width = size.width;
    viewport_.height = size.height;
}

int Renderer::getLineNumberWidth(size_t totalLines) const {
    int width = 1;
    while (totalLines >= 10) {
        totalLines /= 10;
        width++;
    }
    return std::max(width + 1, 4);  // Minimum 4 chars for line numbers
}

void Renderer::setHighlighter(std::unique_ptr<ISyntaxHighlighter> highlighter) {
    highlighter_ = std::move(highlighter);
}

void Renderer::scrollToCursor(const Position& cursor) {
    viewport_.ensureVisible(cursor);
}

void Renderer::scroll(int lines) {
    if (lines < 0 && viewport_.topLine >= static_cast<size_t>(-lines)) {
        viewport_.topLine -= static_cast<size_t>(-lines);
    } else if (lines < 0) {
        viewport_.topLine = 0;
    } else {
        viewport_.topLine += static_cast<size_t>(lines);
    }
}

void Renderer::render(const Buffer& buffer, EditorMode mode) {
    updateViewportSize();
    
    // Update line number width based on total lines
    lineNumberWidth_ = getLineNumberWidth(buffer.lineCount());
    
    // Ensure cursor is visible
    viewport_.ensureVisible(buffer.getCursor());
    
    terminal_.hideCursor();
    
    // Clear screen on first render or when invalidated
    if (needsFullRedraw_) {
        terminal_.clearScreen();
    }
    
    terminal_.setCursor(0, 0);
    
    int editorHeight = viewport_.height - (showStatusBar_ ? 2 : 0);
    
    // Render each visible line
    for (int screenY = 0; screenY < editorHeight; ++screenY) {
        size_t lineIndex = viewport_.topLine + static_cast<size_t>(screenY);
        
        if (lineIndex < buffer.lineCount()) {
            renderLine(lineIndex, buffer.getLine(lineIndex), screenY);
        } else {
            // Empty line (tilde like vim)
            terminal_.setCursor(0, screenY);
            terminal_.setColor(Color::Blue, Color::Default);
            terminal_.write("~");
            terminal_.resetColor();
            terminal_.clearToEndOfLine();
        }
    }
    
    // Render status bar
    if (showStatusBar_) {
        renderStatusBar(buffer, mode, editorHeight);
        
        // Render command line
        if (mode == EditorMode::Command || mode == EditorMode::Search) {
            renderCommandLine(editorHeight + 1);
        } else {
            terminal_.setCursor(0, editorHeight + 1);
            terminal_.write(statusMessage_);
            terminal_.clearToEndOfLine();
        }
    }
    
    // Position cursor
    Position cursor = buffer.getCursor();
    int cursorScreenX = static_cast<int>(cursor.column - viewport_.leftColumn);
    int cursorScreenY = static_cast<int>(cursor.line - viewport_.topLine);
    
    if (showLineNumbers_) {
        cursorScreenX += lineNumberWidth_ + 1;  // +1 for separator
    }
    
    terminal_.setCursor(cursorScreenX, cursorScreenY);
    terminal_.showCursor();
    
    needsFullRedraw_ = false;
}

void Renderer::refresh(const Buffer& buffer, EditorMode mode) {
    // For now, just do a full render
    // TODO: Implement incremental updates
    render(buffer, mode);
}

void Renderer::renderLine(size_t lineIndex, const std::string& content, int screenY) {
    terminal_.setCursor(0, screenY);
    
    // Render line number
    if (showLineNumbers_) {
        terminal_.setColor(lineNumberColor_.foreground, lineNumberColor_.background);
        
        std::ostringstream oss;
        oss << std::setw(lineNumberWidth_) << (lineIndex + 1);
        terminal_.write(oss.str());
        
        terminal_.setColor(Color::BrightBlack, Color::Default);
        terminal_.write(" ");
        terminal_.resetColor();
    }
    
    // Calculate visible portion of line
    size_t startCol = viewport_.leftColumn;
    size_t visibleWidth = static_cast<size_t>(viewport_.width - lineNumberWidth_ - 1);
    
    if (startCol >= content.size()) {
        terminal_.clearToEndOfLine();
        return;
    }
    
    std::string visibleContent = content.substr(startCol, visibleWidth);
    
    // Apply syntax highlighting if available
    if (highlighter_) {
        std::vector<Token> tokens = highlighter_->highlightLine(content, lineIndex);
        
        size_t pos = 0;
        for (const auto& token : tokens) {
            // Check if token is in visible range
            if (token.start + token.length <= startCol) continue;
            if (token.start >= startCol + visibleWidth) break;
            
            // Output any text before this token
            while (pos < token.start && pos - startCol < visibleWidth) {
                if (pos >= startCol) {
                    terminal_.writeChar(content[pos]);
                }
                pos++;
            }
            
            // Set token color
            ColorPair color = getTokenColor(token.type);
            terminal_.setColor(color.foreground, color.background);
            
            // Output token text
            size_t tokenEnd = token.start + token.length;
            while (pos < tokenEnd && pos - startCol < visibleWidth) {
                if (pos >= startCol) {
                    terminal_.writeChar(content[pos]);
                }
                pos++;
            }
            
            terminal_.resetColor();
        }
        
        // Output remaining text
        while (pos < content.size() && pos - startCol < visibleWidth) {
            if (pos >= startCol) {
                terminal_.writeChar(content[pos]);
            }
            pos++;
        }
    } else {
        // No syntax highlighting, just output text
        terminal_.write(visibleContent);
    }
    
    terminal_.clearToEndOfLine();
}

void Renderer::renderStatusBar(const Buffer& buffer, EditorMode mode, int screenY) {
    terminal_.setCursor(0, screenY);
    
    // Set status bar colors
    ColorPair modeColor = getModeColor(mode);
    terminal_.setColor(modeColor.foreground, Color::Default);
    terminal_.setBold(true);
    
    // Mode indicator
    std::string modeStr = " " + std::string(modeToString(mode)) + " ";
    terminal_.write(modeStr);
    terminal_.setBold(false);
    
    terminal_.setColor(statusBarColor_.foreground, statusBarColor_.background);
    
    // Filename
    std::string filename = buffer.getFilename().empty() ? "[No Name]" : buffer.getFilename();
    if (buffer.isModified()) {
        filename += " [+]";
    }
    terminal_.write(" " + filename + " ");
    
    // Position info
    Position cursor = buffer.getCursor();
    std::ostringstream posInfo;
    posInfo << "Ln " << (cursor.line + 1) << ", Col " << (cursor.column + 1);
    posInfo << " (" << buffer.lineCount() << " lines)";
    
    // Calculate padding
    int usedWidth = static_cast<int>(modeStr.size() + filename.size() + 2 + posInfo.str().size());
    int padding = viewport_.width - usedWidth;
    if (padding > 0) {
        terminal_.write(std::string(static_cast<size_t>(padding), ' '));
    }
    
    terminal_.write(posInfo.str());
    terminal_.resetColor();
}

void Renderer::renderCommandLine(int screenY) {
    terminal_.setCursor(0, screenY);
    terminal_.setColor(Color::White, Color::Default);
    terminal_.write(":" + commandLine_);
    terminal_.clearToEndOfLine();
    terminal_.resetColor();
}

} // namespace astrax
