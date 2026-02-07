#include "astrax/command.h"
#include "astrax/editor.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <cstdlib>

namespace astrax {

// ============================================================================
// InsertTextCommand
// ============================================================================

InsertTextCommand::InsertTextCommand(Buffer& buffer, Position pos, const std::string& text)
    : buffer_(buffer), position_(pos), text_(text) {}

void InsertTextCommand::execute() {
    buffer_.setCursor(position_);
    buffer_.insertString(text_);
}

void InsertTextCommand::undo() {
    // TODO: Implement proper undo
}

// ============================================================================
// DeleteTextCommand
// ============================================================================

DeleteTextCommand::DeleteTextCommand(Buffer& buffer, Range range)
    : buffer_(buffer), range_(range) {}

void DeleteTextCommand::execute() {
    // Save text before deleting
    // TODO: Implement proper text extraction
    buffer_.setCursor(range_.start);
}

void DeleteTextCommand::undo() {
    buffer_.setCursor(range_.start);
    buffer_.insertString(deletedText_);
}

// ============================================================================
// CommandExecutor
// ============================================================================

CommandExecutor::CommandExecutor() {
    registerBuiltinCommands();
}

void CommandExecutor::registerBuiltinCommands() {
    // Write (save) command
    registerCommand("w", [](Editor& editor, const std::vector<std::string>& args) {
        if (args.size() > 1) {
            return editor.saveFileAs(args[1]);
        }
        return editor.saveFile();
    });
    
    // Quit command
    registerCommand("q", [](Editor& editor, const std::vector<std::string>& /*args*/) {
        if (editor.getBuffer().isModified()) {
            editor.setStatusMessage("No write since last change (add ! to override)");
            return false;
        }
        editor.quit();
        return true;
    });
    
    // Force quit
    registerCommand("q!", [](Editor& editor, const std::vector<std::string>& /*args*/) {
        editor.quit(true);
        return true;
    });
    
    // Write and quit
    registerCommand("wq", [](Editor& editor, const std::vector<std::string>& args) {
        bool saved = args.size() > 1 ? editor.saveFileAs(args[1]) : editor.saveFile();
        if (saved) {
            editor.quit();
        }
        return saved;
    });
    
    // Exit (same as wq)
    registerCommand("x", [](Editor& editor, const std::vector<std::string>& /*args*/) {
        if (editor.getBuffer().isModified()) {
            if (!editor.saveFile()) {
                return false;
            }
        }
        editor.quit();
        return true;
    });
    
    // Edit (open) file
    registerCommand("e", [](Editor& editor, const std::vector<std::string>& args) {
        if (args.size() < 2) {
            editor.setStatusMessage("Filename required");
            return false;
        }
        return editor.openFile(args[1]);
    });
    
    // New file
    registerCommand("new", [](Editor& editor, const std::vector<std::string>& /*args*/) {
        editor.newBuffer();
        return true;
    });
    
    // Save as
    registerCommand("saveas", [](Editor& editor, const std::vector<std::string>& args) {
        if (args.size() < 2) {
            editor.setStatusMessage("Filename required");
            return false;
        }
        return editor.saveFileAs(args[1]);
    });
    
    // Set option
    registerCommand("set", [](Editor& editor, const std::vector<std::string>& args) {
        if (args.size() < 2) {
            editor.setStatusMessage("Option required");
            return false;
        }
        
        const std::string& option = args[1];
        
        if (option == "number" || option == "nu") {
            // TODO: Toggle line numbers
            editor.setStatusMessage("Line numbers enabled");
            return true;
        } else if (option == "nonumber" || option == "nonu") {
            editor.setStatusMessage("Line numbers disabled");
            return true;
        } else if (option == "syntax") {
            editor.setStatusMessage("Syntax highlighting enabled");
            return true;
        } else if (option == "nosyntax") {
            editor.setStatusMessage("Syntax highlighting disabled");
            return true;
        }
        
        editor.setStatusMessage("Unknown option: " + option);
        return false;
    });
    
    // Run (compile and execute)
    registerCommand("run", [](Editor& editor, const std::vector<std::string>& /*args*/) {
        std::string filename = editor.getBuffer().getFilename();
        
        if (filename.empty()) {
            editor.setStatusMessage("Error: No filename. Save file first with :w <filename>");
            return false;
        }
        
        // Get file extension
        size_t dotPos = filename.find_last_of('.');
        if (dotPos == std::string::npos) {
            editor.setStatusMessage("Error: Unknown file type. Need .cpp or .c extension");
            return false;
        }
        
        std::string ext = filename.substr(dotPos + 1);
        
        // Only support C/C++ for now
        if (ext != "cpp" && ext != "c" && ext != "cc" && ext != "cxx") {
            editor.setStatusMessage("Error: Only C/C++ files supported (.cpp, .c, .cc, .cxx)");
            return false;
        }
        
        // Save file first
        if (editor.getBuffer().isModified()) {
            if (!editor.saveFile()) {
                editor.setStatusMessage("Error: Could not save file before compiling");
                return false;
            }
        }
        
        // Get base filename (without extension) for output
        std::string baseName = filename.substr(0, dotPos);
        
        // Handle paths with spaces
        std::string quotedFilename = "\"" + filename + "\"";
        std::string outputName = baseName + ".exe";
        std::string quotedOutput = "\"" + outputName + "\"";
        
        // Build compile command
        std::string compiler = (ext == "c") ? "gcc" : "g++";
        std::string compileCmd = compiler + " -std=c++11 " + quotedFilename + " -o " + quotedOutput + " 2>&1";
        
        editor.setStatusMessage("Compiling " + filename + "...");
        
        // Disable raw mode temporarily to run the command
        editor.getTerminal().disableRawMode();
        
        // Clear screen first
        system("cls");
        
        // Show compile output
        std::cout << "\n=========================================" << std::endl;
        std::cout << "  Compiling: " << filename << std::endl;
        std::cout << "=========================================\n" << std::endl;
        
        int compileResult = system(compileCmd.c_str());
        
        if (compileResult != 0) {
            std::cout << "\n=========================================" << std::endl;
            std::cout << "  COMPILATION FAILED" << std::endl;
            std::cout << "=========================================\n" << std::endl;
            std::cout << "Press Enter to return to editor..." << std::endl;
            std::cin.get();
            
            // Clear screen and re-enable raw mode BEFORE returning
            system("cls");
            editor.getTerminal().enableRawMode();
            editor.getTerminal().clearScreen();
            
            editor.setStatusMessage("Compilation failed");
            return false;
        }
        
        std::cout << "\n=========================================" << std::endl;
        std::cout << "  Compilation SUCCESSFUL!" << std::endl;
        std::cout << "  Running: " << outputName << std::endl;
        std::cout << "=========================================\n" << std::endl;
        
        // Run the compiled executable
        std::string runCmd = quotedOutput;
        int runResult = system(runCmd.c_str());
        
        std::cout << "\n=========================================" << std::endl;
        std::cout << "  Program exited with code: " << runResult << std::endl;
        std::cout << "=========================================\n" << std::endl;
        std::cout << "Press Enter to return to editor..." << std::endl;
        std::cin.get();
        
        // Clear screen and re-enable raw mode
        system("cls");
        editor.getTerminal().enableRawMode();
        editor.getTerminal().clearScreen();
        
        editor.setStatusMessage("Program finished (exit code " + std::to_string(runResult) + ")");
        return true;
    });
    
    // Help
    registerCommand("help", [](Editor& editor, const std::vector<std::string>& /*args*/) {
        editor.setStatusMessage("Commands: :w :q :wq :e <file> :new :saveas <file> :set <opt> :run");
        return true;
    });
}

void CommandExecutor::registerCommand(const std::string& name, CommandHandler handler) {
    commands_[name] = std::move(handler);
}

std::vector<std::string> CommandExecutor::parseCommand(const std::string& input) const {
    std::vector<std::string> parts;
    std::istringstream iss(input);
    std::string part;
    
    while (iss >> part) {
        parts.push_back(part);
    }
    
    return parts;
}

bool CommandExecutor::execute(Editor& editor, const std::string& command) {
    if (command.empty()) {
        return true;
    }
    
    std::vector<std::string> parts = parseCommand(command);
    if (parts.empty()) {
        return true;
    }
    
    const std::string& cmd = parts[0];
    
    auto it = commands_.find(cmd);
    if (it != commands_.end()) {
        return it->second(editor, parts);
    }
    
    // Check for line number
    try {
        size_t lineNum = std::stoul(cmd);
        Buffer& buffer = editor.getBuffer();
        if (lineNum > 0 && lineNum <= buffer.lineCount()) {
            buffer.setCursor({lineNum - 1, 0});
            return true;
        }
    } catch (...) {
        // Not a number
    }
    
    lastError_ = "Unknown command: " + cmd;
    editor.setStatusMessage(lastError_);
    return false;
}

std::vector<std::string> CommandExecutor::getSuggestions(const std::string& prefix) const {
    std::vector<std::string> suggestions;
    
    for (const auto& pair : commands_) {
        if (pair.first.find(prefix) == 0) {
            suggestions.push_back(pair.first);
        }
    }
    
    std::sort(suggestions.begin(), suggestions.end());
    return suggestions;
}

// ============================================================================
// KeyBindings
// ============================================================================

KeyBindings::KeyBindings() {
    setupDefaultBindings();
}

void KeyBindings::setupDefaultBindings() {
    // Normal mode bindings
    bind(EditorMode::Normal, {static_cast<int>('h')}, [](Editor& e) {
        e.getBuffer().moveCursor(-1, 0);
    });
    
    bind(EditorMode::Normal, {static_cast<int>('j')}, [](Editor& e) {
        e.getBuffer().moveCursor(0, 1);
    });
    
    bind(EditorMode::Normal, {static_cast<int>('k')}, [](Editor& e) {
        e.getBuffer().moveCursor(0, -1);
    });
    
    bind(EditorMode::Normal, {static_cast<int>('l')}, [](Editor& e) {
        e.getBuffer().moveCursor(1, 0);
    });
    
    bind(EditorMode::Normal, {static_cast<int>('w')}, [](Editor& e) {
        e.getBuffer().moveForwardWord();
    });
    
    bind(EditorMode::Normal, {static_cast<int>('b')}, [](Editor& e) {
        e.getBuffer().moveBackwardWord();
    });
    
    bind(EditorMode::Normal, {static_cast<int>('0')}, [](Editor& e) {
        e.getBuffer().moveToLineStart();
    });
    
    bind(EditorMode::Normal, {static_cast<int>('$')}, [](Editor& e) {
        e.getBuffer().moveToLineEnd();
    });
    
    bind(EditorMode::Normal, {static_cast<int>('g')}, [](Editor& e) {
        e.getBuffer().moveToBufferStart();
    });
    
    bind(EditorMode::Normal, {static_cast<int>('G')}, [](Editor& e) {
        e.getBuffer().moveToBufferEnd();
    });
    
    bind(EditorMode::Normal, {static_cast<int>('i')}, [](Editor& e) {
        e.setMode(EditorMode::Insert);
    });
    
    bind(EditorMode::Normal, {static_cast<int>('a')}, [](Editor& e) {
        e.getBuffer().moveCursor(1, 0);
        e.setMode(EditorMode::Insert);
    });
    
    bind(EditorMode::Normal, {static_cast<int>('o')}, [](Editor& e) {
        e.getBuffer().insertLineBelow();
        e.setMode(EditorMode::Insert);
    });
    
    bind(EditorMode::Normal, {static_cast<int>('O')}, [](Editor& e) {
        e.getBuffer().insertLineAbove();
        e.setMode(EditorMode::Insert);
    });
    
    bind(EditorMode::Normal, {static_cast<int>('x')}, [](Editor& e) {
        e.getBuffer().deleteCharAt();
    });
    
    bind(EditorMode::Normal, {static_cast<int>('d'), false, false, false}, [](Editor& e) {
        // dd - delete line (would need multi-key handling)
        e.getBuffer().deleteLine();
    });
    
    bind(EditorMode::Normal, {static_cast<int>('y')}, [](Editor& e) {
        e.getBuffer().yankLine();
        e.setStatusMessage("Line yanked");
    });
    
    bind(EditorMode::Normal, {static_cast<int>('p')}, [](Editor& e) {
        e.getBuffer().paste();
    });
    
    bind(EditorMode::Normal, {static_cast<int>('P')}, [](Editor& e) {
        e.getBuffer().pasteBefore();
    });
    
    bind(EditorMode::Normal, {static_cast<int>('u')}, [](Editor& e) {
        e.getBuffer().undo();
        e.setStatusMessage("Undo");
    });
    
    bind(EditorMode::Normal, {static_cast<int>('r'), true}, [](Editor& e) {  // Ctrl+R
        e.getBuffer().redo();
        e.setStatusMessage("Redo");
    });
    
    bind(EditorMode::Normal, {static_cast<int>(':')}, [](Editor& e) {
        e.setMode(EditorMode::Command);
    });
    
    bind(EditorMode::Normal, {static_cast<int>('/')}, [](Editor& e) {
        e.setMode(EditorMode::Search);
    });
    
    bind(EditorMode::Normal, {static_cast<int>('J')}, [](Editor& e) {
        e.getBuffer().joinLines();
    });
}

void KeyBindings::bind(EditorMode mode, const KeyEvent& key, Action action) {
    bindings_[mode].push_back({key, std::move(action)});
}

void KeyBindings::unbind(EditorMode mode, const KeyEvent& key) {
    auto& modeBindings = bindings_[mode];
    modeBindings.erase(
        std::remove_if(modeBindings.begin(), modeBindings.end(),
            [&key](const KeyBinding& b) { return keysMatch(b.key, key); }),
        modeBindings.end()
    );
}

bool KeyBindings::keysMatch(const KeyEvent& a, const KeyEvent& b) {
    // For printable characters, only match the key value
    // This handles cases where 'G' comes through as ASCII 71 without shift flag
    if (a.key > 0 && a.key < 256 && !a.ctrl && !a.alt &&
        b.key > 0 && b.key < 256 && !b.ctrl && !b.alt) {
        return a.key == b.key;
    }
    
    // For special keys or keys with modifiers, require exact match
    return a.key == b.key && a.ctrl == b.ctrl && a.alt == b.alt && a.shift == b.shift;
}

bool KeyBindings::process(Editor& editor, EditorMode mode, const KeyEvent& key) {
    auto it = bindings_.find(mode);
    if (it == bindings_.end()) {
        return false;
    }
    
    for (const auto& binding : it->second) {
        if (keysMatch(binding.key, key)) {
            binding.action(editor);
            return true;
        }
    }
    
    return false;
}

void KeyBindings::resetToDefaults() {
    bindings_.clear();
    setupDefaultBindings();
}

} // namespace astrax
