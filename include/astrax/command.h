#ifndef ASTRAX_COMMAND_H
#define ASTRAX_COMMAND_H

#include "types.h"
#include "buffer.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace astrax {

class Editor;  // Forward declaration

/**
 * @brief Abstract command interface for undo/redo pattern
 */
class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string description() const = 0;
};

/**
 * @brief Insert text command
 */
class InsertTextCommand : public ICommand {
public:
    InsertTextCommand(Buffer& buffer, Position pos, const std::string& text);
    void execute() override;
    void undo() override;
    std::string description() const override { return "Insert text"; }
    
private:
    Buffer& buffer_;
    Position position_;
    std::string text_;
};

/**
 * @brief Delete text command
 */
class DeleteTextCommand : public ICommand {
public:
    DeleteTextCommand(Buffer& buffer, Range range);
    void execute() override;
    void undo() override;
    std::string description() const override { return "Delete text"; }
    
private:
    Buffer& buffer_;
    Range range_;
    std::string deletedText_;
};

/**
 * @brief Command mode handler - parses and executes ex commands like :w, :q
 */
class CommandExecutor {
public:
    using CommandHandler = std::function<bool(Editor&, const std::vector<std::string>&)>;
    
    CommandExecutor();
    
    /// Register a command handler
    void registerCommand(const std::string& name, CommandHandler handler);
    
    /// Execute a command string (e.g., "w", "q", "wq", "saveas filename")
    bool execute(Editor& editor, const std::string& command);
    
    /// Get command suggestions for autocomplete
    std::vector<std::string> getSuggestions(const std::string& prefix) const;
    
    /// Get last error message
    const std::string& getLastError() const { return lastError_; }
    
private:
    std::unordered_map<std::string, CommandHandler> commands_;
    std::string lastError_;
    
    void registerBuiltinCommands();
    std::vector<std::string> parseCommand(const std::string& input) const;
};

/**
 * @brief Keybinding manager - maps keys to actions
 */
class KeyBindings {
public:
    using Action = std::function<void(Editor&)>;
    
    KeyBindings();
    
    /// Bind a key to an action in a specific mode
    void bind(EditorMode mode, const KeyEvent& key, Action action);
    
    /// Unbind a key
    void unbind(EditorMode mode, const KeyEvent& key);
    
    /// Process a key event, returns true if handled
    bool process(Editor& editor, EditorMode mode, const KeyEvent& key);
    
    /// Reset to default keybindings
    void resetToDefaults();
    
private:
    struct KeyBinding {
        KeyEvent key;
        Action action;
    };
    
    std::unordered_map<EditorMode, std::vector<KeyBinding>> bindings_;
    
    void setupDefaultBindings();
    
    // Helper to match keys
    static bool keysMatch(const KeyEvent& a, const KeyEvent& b);
};

} // namespace astrax

#endif // ASTRAX_COMMAND_H
