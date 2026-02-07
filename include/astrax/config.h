#ifndef ASTRAX_CONFIG_H
#define ASTRAX_CONFIG_H

#include "types.h"
#include <string>
#include <unordered_map>

namespace astrax {

/**
 * @brief Theme colors configuration
 */
struct Theme {
    std::string name;
    
    // Editor colors
    ColorPair normal{Color::White, Color::Default};
    ColorPair cursor{Color::Black, Color::White};
    ColorPair lineNumber{Color::Yellow, Color::Default};
    ColorPair currentLineNumber{Color::BrightYellow, Color::Default};
    ColorPair statusBar{Color::Black, Color::White};
    ColorPair statusBarMode{Color::White, Color::Blue};
    
    // Syntax colors
    ColorPair keyword{Color::Blue, Color::Default};
    ColorPair type{Color::Cyan, Color::Default};
    ColorPair string{Color::Green, Color::Default};
    ColorPair number{Color::Magenta, Color::Default};
    ColorPair comment{Color::BrightBlack, Color::Default};
    ColorPair preprocessor{Color::Yellow, Color::Default};
    ColorPair function{Color::BrightBlue, Color::Default};
    ColorPair operator_{Color::White, Color::Default};
    
    // Search
    ColorPair searchMatch{Color::Black, Color::Yellow};
    ColorPair searchCurrent{Color::Black, Color::BrightYellow};
};

/**
 * @brief Configuration manager
 */
class Config {
public:
    Config();
    
    // ========================================================================
    // Loading/Saving
    // ========================================================================
    
    /// Load config from JSON file
    bool load(const std::string& path);
    
    /// Save config to JSON file
    bool save(const std::string& path) const;
    
    /// Load default configuration
    void loadDefaults();
    
    /// Get config file path for current platform
    static std::string getConfigPath();
    
    // ========================================================================
    // Settings
    // ========================================================================
    
    /// Get editor configuration
    EditorConfig& editor() { return editorConfig_; }
    const EditorConfig& editor() const { return editorConfig_; }
    
    /// Get current theme
    Theme& theme() { return currentTheme_; }
    const Theme& theme() const { return currentTheme_; }
    
    /// Set theme by name
    bool setTheme(const std::string& name);
    
    /// Get available themes
    std::vector<std::string> getAvailableThemes() const;
    
    // ========================================================================
    // Individual Settings
    // ========================================================================
    
    /// Get a string setting (returns empty if not found)
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    
    /// Get an int setting
    int getInt(const std::string& key, int defaultValue = 0) const;
    
    /// Get a bool setting
    bool getBool(const std::string& key, bool defaultValue = false) const;
    
    /// Set a value
    void set(const std::string& key, const std::string& value);
    void set(const std::string& key, int value);
    void set(const std::string& key, bool value);
    
private:
    EditorConfig editorConfig_;
    Theme currentTheme_;
    std::unordered_map<std::string, Theme> themes_;
    std::unordered_map<std::string, std::string> settings_;
    
    void registerBuiltinThemes();
};

} // namespace astrax

#endif // ASTRAX_CONFIG_H
