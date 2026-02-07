#include "astrax/editor.h"
#include "astrax/syntax/cpp_highlighter.h"
#include <algorithm>

namespace astrax {

// ============================================================================
// Constructor/Destructor
// ============================================================================

Editor::Editor() {
    initialize();
}

Editor::~Editor() = default;

void Editor::initialize() {
    terminal_ = createTerminal();
    buffer_ = std::make_unique<Buffer>();
    renderer_ = std::make_unique<Renderer>(*terminal_);
    commandExecutor_ = std::make_unique<CommandExecutor>();
    
    // Load configuration
    config_.loadDefaults();
    
    // Setup keybindings
    setupKeyBindings();
    
    // Register additional commands
    registerCommands();
}

void Editor::registerCommands() {
    // Any additional commands beyond the builtins
}

void Editor::setupKeyBindings() {
    // Default keybindings are set in KeyBindings constructor
}

// ============================================================================
// Main Loop
// ============================================================================

void Editor::run(const std::string& filename) {
    if (!filename.empty()) {
        openFile(filename);
    }
    
    terminal_->setTitle("AstraX - " + (buffer_->getFilename().empty() ? "[No Name]" : buffer_->getFilename()));
    
    RawModeGuard rawMode(*terminal_);
    
    while (!shouldQuit_) {
        render();
        processInput();
    }
    
    // Cleanup
    terminal_->clearScreen();
    terminal_->setCursor(0, 0);
}

void Editor::quit(bool force) {
    if (!force && buffer_->isModified()) {
        setStatusMessage("No write since last change (use :q! to override)");
        return;
    }
    shouldQuit_ = true;
}

// ============================================================================
// Mode
// ============================================================================

void Editor::setMode(EditorMode mode) {
    mode_ = mode;
    
    switch (mode) {
        case EditorMode::Normal:
            setStatusMessage("");
            break;
        case EditorMode::Insert:
            setStatusMessage("-- INSERT --");
            break;
        case EditorMode::Command:
            commandBuffer_.clear();
            break;
        case EditorMode::Visual:
            setStatusMessage("-- VISUAL --");
            break;
        case EditorMode::Search:
            commandBuffer_.clear();
            break;
    }
}

// ============================================================================
// Buffer Operations
// ============================================================================

void Editor::newBuffer() {
    buffer_ = std::make_unique<Buffer>();
    setMode(EditorMode::Normal);
    setStatusMessage("New buffer");
    terminal_->setTitle("AstraX - [No Name]");
}

bool Editor::openFile(const std::string& filename) {
    if (buffer_->loadFromFile(filename)) {
        setStatusMessage("\"" + filename + "\" loaded");
        terminal_->setTitle("AstraX - " + filename);
        
        // Set up syntax highlighting based on file extension
        auto highlighter = HighlighterFactory::createForFile(filename);
        if (highlighter) {
            renderer_->setHighlighter(std::move(highlighter));
        }
        
        return true;
    } else {
        // New file
        buffer_->setFilename(filename);
        setStatusMessage("\"" + filename + "\" [New File]");
        terminal_->setTitle("AstraX - " + filename);
        return true;
    }
}

bool Editor::saveFile() {
    if (buffer_->getFilename().empty()) {
        setStatusMessage("No filename specified (use :saveas <filename>)");
        return false;
    }
    
    if (buffer_->saveToFile(buffer_->getFilename())) {
        setStatusMessage("\"" + buffer_->getFilename() + "\" written");
        return true;
    } else {
        setStatusMessage("Error: Could not write file");
        return false;
    }
}

bool Editor::saveFileAs(const std::string& filename) {
    if (buffer_->saveToFile(filename)) {
        setStatusMessage("\"" + filename + "\" written");
        terminal_->setTitle("AstraX - " + filename);
        return true;
    } else {
        setStatusMessage("Error: Could not write file");
        return false;
    }
}

// ============================================================================
// Search
// ============================================================================

void Editor::searchForward() {
    // TODO: Implement search
    setStatusMessage("Search forward: " + search_.getPattern());
}

void Editor::searchBackward() {
    // TODO: Implement search
    setStatusMessage("Search backward: " + search_.getPattern());
}

// ============================================================================
// Status
// ============================================================================

void Editor::setStatusMessage(const std::string& message) {
    statusMessage_ = message;
}

// ============================================================================
// Command Execution
// ============================================================================

bool Editor::executeCommand(const std::string& command) {
    return commandExecutor_->execute(*this, command);
}

// ============================================================================
// Rendering
// ============================================================================

void Editor::render() {
    renderer_->setStatusMessage(statusMessage_);
    renderer_->setCommandLine(commandBuffer_);
    renderer_->render(*buffer_, mode_);
}

// ============================================================================
// Input Processing
// ============================================================================

void Editor::processInput() {
    KeyEvent key = terminal_->readKey();
    
    switch (mode_) {
        case EditorMode::Normal:
            processNormalMode(key);
            break;
        case EditorMode::Insert:
            processInsertMode(key);
            break;
        case EditorMode::Command:
            processCommandMode(key);
            break;
        case EditorMode::Visual:
            processVisualMode(key);
            break;
        case EditorMode::Search:
            processSearchMode(key);
            break;
    }
}

void Editor::processNormalMode(const KeyEvent& key) {
    // Try keybindings first
    if (keyBindings_.process(*this, EditorMode::Normal, key)) {
        return;
    }
    
    // Handle arrow keys
    if (key.isSpecial()) {
        switch (key.toSpecial()) {
            case SpecialKey::Up:    buffer_->moveCursor(0, -1); break;
            case SpecialKey::Down:  buffer_->moveCursor(0, 1); break;
            case SpecialKey::Left:  buffer_->moveCursor(-1, 0); break;
            case SpecialKey::Right: buffer_->moveCursor(1, 0); break;
            case SpecialKey::Home:  buffer_->moveToLineStart(); break;
            case SpecialKey::End:   buffer_->moveToLineEnd(); break;
            case SpecialKey::PageUp:   buffer_->moveCursor(0, -20); break;
            case SpecialKey::PageDown: buffer_->moveCursor(0, 20); break;
            default: break;
        }
    }
}

void Editor::processInsertMode(const KeyEvent& key) {
    // Handle Escape key - return to Normal mode
    if (key.isEscape()) {
        setMode(EditorMode::Normal);
        buffer_->moveCursor(-1, 0);  // Move cursor back one position
        return;
    }
    
    // Handle Enter key
    if (key.isEnter()) {
        buffer_->insertNewline();
        return;
    }
    
    // Handle Backspace
    if (key.isBackspace()) {
        buffer_->deleteCharBefore();
        return;
    }
    
    // Handle Tab
    if (key.isTab()) {
        buffer_->insertString("    ");
        return;
    }
    
    // Handle extended/special keys (arrow keys, etc.)
    if (key.isSpecial()) {
        switch (key.toSpecial()) {
            case SpecialKey::Up:        buffer_->moveCursor(0, -1); break;
            case SpecialKey::Down:      buffer_->moveCursor(0, 1); break;
            case SpecialKey::Left:      buffer_->moveCursor(-1, 0); break;
            case SpecialKey::Right:     buffer_->moveCursor(1, 0); break;
            case SpecialKey::Home:      buffer_->moveToLineStart(); break;
            case SpecialKey::End:       buffer_->moveToLineEnd(); break;
            case SpecialKey::Delete:    buffer_->deleteCharAt(); break;
            case SpecialKey::PageUp:    buffer_->moveCursor(0, -20); break;
            case SpecialKey::PageDown:  buffer_->moveCursor(0, 20); break;
            default: break;
        }
        return;
    }
    
    // Handle printable characters
    if (key.isPrintable()) {
        buffer_->insertChar(key.toChar());
    }
}

void Editor::processCommandMode(const KeyEvent& key) {
    if (key.isEscape()) {
        setMode(EditorMode::Normal);
        commandBuffer_.clear();
        return;
    }
    
    if (key.isEnter()) {
        executeCommand(commandBuffer_);
        setMode(EditorMode::Normal);
        commandBuffer_.clear();
        return;
    }
    
    if (key.isBackspace()) {
        if (!commandBuffer_.empty()) {
            commandBuffer_.pop_back();
        } else {
            setMode(EditorMode::Normal);
        }
        return;
    }
    
    if (key.isPrintable()) {
        commandBuffer_ += key.toChar();
    }
}

void Editor::processVisualMode(const KeyEvent& key) {
    if (key.isEscape()) {
        setMode(EditorMode::Normal);
        return;
    }
    
    // TODO: Implement visual mode selection
    processNormalMode(key);
}

void Editor::processSearchMode(const KeyEvent& key) {
    if (key.isEscape()) {
        setMode(EditorMode::Normal);
        commandBuffer_.clear();
        return;
    }
    
    if (key.isEnter()) {
        search_.setPattern(commandBuffer_);
        searchForward();
        setMode(EditorMode::Normal);
        commandBuffer_.clear();
        return;
    }
    
    if (key.isBackspace()) {
        if (!commandBuffer_.empty()) {
            commandBuffer_.pop_back();
        } else {
            setMode(EditorMode::Normal);
        }
        return;
    }
    
    if (key.isPrintable()) {
        commandBuffer_ += key.toChar();
    }
}

} // namespace astrax
