#include "astrax/config.h"
#include <fstream>
#include <sstream>
#include <cstdlib>

// Platform-specific path handling
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#endif

namespace astrax {

// ============================================================================
// Constructor
// ============================================================================

Config::Config() {
    loadDefaults();
}

void Config::loadDefaults() {
    // Editor defaults
    editorConfig_.showLineNumbers = true;
    editorConfig_.showStatusBar = true;
    editorConfig_.syntaxHighlighting = true;
    editorConfig_.autoIndent = true;
    editorConfig_.tabSize = 4;
    editorConfig_.expandTabs = true;
    editorConfig_.theme = "default";
    editorConfig_.colorScheme = "dark";
    
    // Register builtin themes
    registerBuiltinThemes();
    
    // Set default theme
    setTheme("default");
}

void Config::registerBuiltinThemes() {
    // Default dark theme
    Theme defaultTheme;
    defaultTheme.name = "default";
    defaultTheme.normal = {Color::White, Color::Default};
    defaultTheme.cursor = {Color::Black, Color::White};
    defaultTheme.lineNumber = {Color::Yellow, Color::Default};
    defaultTheme.currentLineNumber = {Color::BrightYellow, Color::Default};
    defaultTheme.statusBar = {Color::Black, Color::White};
    defaultTheme.statusBarMode = {Color::White, Color::Blue};
    defaultTheme.keyword = {Color::Blue, Color::Default};
    defaultTheme.type = {Color::Cyan, Color::Default};
    defaultTheme.string = {Color::Green, Color::Default};
    defaultTheme.number = {Color::Magenta, Color::Default};
    defaultTheme.comment = {Color::BrightBlack, Color::Default};
    defaultTheme.preprocessor = {Color::Yellow, Color::Default};
    defaultTheme.function = {Color::BrightBlue, Color::Default};
    defaultTheme.operator_ = {Color::White, Color::Default};
    defaultTheme.searchMatch = {Color::Black, Color::Yellow};
    defaultTheme.searchCurrent = {Color::Black, Color::BrightYellow};
    themes_["default"] = defaultTheme;
    
    // Light theme
    Theme lightTheme = defaultTheme;
    lightTheme.name = "light";
    lightTheme.normal = {Color::Black, Color::Default};
    lightTheme.keyword = {Color::Blue, Color::Default};
    lightTheme.type = {Color::Magenta, Color::Default};
    lightTheme.string = {Color::Red, Color::Default};
    lightTheme.comment = {Color::Green, Color::Default};
    themes_["light"] = lightTheme;
    
    // Monokai theme
    Theme monokaiTheme = defaultTheme;
    monokaiTheme.name = "monokai";
    monokaiTheme.keyword = {Color::Red, Color::Default};
    monokaiTheme.type = {Color::Cyan, Color::Default};
    monokaiTheme.string = {Color::Yellow, Color::Default};
    monokaiTheme.number = {Color::Magenta, Color::Default};
    monokaiTheme.comment = {Color::BrightBlack, Color::Default};
    monokaiTheme.function = {Color::Green, Color::Default};
    themes_["monokai"] = monokaiTheme;
}

// ============================================================================
// Theme Management
// ============================================================================

bool Config::setTheme(const std::string& name) {
    auto it = themes_.find(name);
    if (it == themes_.end()) {
        return false;
    }
    currentTheme_ = it->second;
    editorConfig_.theme = name;
    return true;
}

std::vector<std::string> Config::getAvailableThemes() const {
    std::vector<std::string> names;
    for (const auto& pair : themes_) {
        names.push_back(pair.first);
    }
    return names;
}

// ============================================================================
// File Operations
// ============================================================================

std::string Config::getConfigPath() {
#ifdef _WIN32
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        std::string configPath = std::string(path) + "\\AstraX\\config.json";
        return configPath;
    }
    return "config.json";
#else
    const char* home = std::getenv("HOME");
    if (home) {
        return std::string(home) + "/.config/astrax/config.json";
    }
    return "config.json";
#endif
}

bool Config::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    // Simple JSON parsing (production should use a proper JSON library)
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    // TODO: Implement proper JSON parsing
    // For now, just return true if file exists
    return true;
}

bool Config::save(const std::string& path) const {
    // Create directory if needed (simple approach for cross-platform)
    std::ofstream file(path);
    if (!file.is_open()) {
        // Try to create parent directory
#ifdef _WIN32
        size_t pos = path.find_last_of('\\');
        if (pos != std::string::npos) {
            std::string dir = path.substr(0, pos);
            CreateDirectoryA(dir.c_str(), NULL);
        }
#else
        size_t pos = path.find_last_of('/');
        if (pos != std::string::npos) {
            std::string dir = path.substr(0, pos);
            std::string cmd = "mkdir -p \"" + dir + "\"";
            system(cmd.c_str());
        }
#endif
        file.open(path);
        if (!file.is_open()) {
            return false;
        }
    }
    
    // Simple JSON output
    file << "{\n";
    file << "  \"showLineNumbers\": " << (editorConfig_.showLineNumbers ? "true" : "false") << ",\n";
    file << "  \"showStatusBar\": " << (editorConfig_.showStatusBar ? "true" : "false") << ",\n";
    file << "  \"syntaxHighlighting\": " << (editorConfig_.syntaxHighlighting ? "true" : "false") << ",\n";
    file << "  \"autoIndent\": " << (editorConfig_.autoIndent ? "true" : "false") << ",\n";
    file << "  \"tabSize\": " << editorConfig_.tabSize << ",\n";
    file << "  \"expandTabs\": " << (editorConfig_.expandTabs ? "true" : "false") << ",\n";
    file << "  \"theme\": \"" << editorConfig_.theme << "\",\n";
    file << "  \"colorScheme\": \"" << editorConfig_.colorScheme << "\"\n";
    file << "}\n";
    
    return true;
}

// ============================================================================
// Settings
// ============================================================================

std::string Config::getString(const std::string& key, const std::string& defaultValue) const {
    auto it = settings_.find(key);
    if (it != settings_.end()) {
        return it->second;
    }
    return defaultValue;
}

int Config::getInt(const std::string& key, int defaultValue) const {
    auto it = settings_.find(key);
    if (it != settings_.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

bool Config::getBool(const std::string& key, bool defaultValue) const {
    auto it = settings_.find(key);
    if (it != settings_.end()) {
        return it->second == "true" || it->second == "1";
    }
    return defaultValue;
}

void Config::set(const std::string& key, const std::string& value) {
    settings_[key] = value;
}

void Config::set(const std::string& key, int value) {
    settings_[key] = std::to_string(value);
}

void Config::set(const std::string& key, bool value) {
    settings_[key] = value ? "true" : "false";
}

} // namespace astrax
