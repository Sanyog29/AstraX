#ifndef ASTRAX_SYNTAX_HIGHLIGHTER_H
#define ASTRAX_SYNTAX_HIGHLIGHTER_H

#include "../types.h"
#include <string>
#include <vector>
#include <memory>

namespace astrax {

/**
 * @brief Abstract interface for syntax highlighting
 */
class ISyntaxHighlighter {
public:
    virtual ~ISyntaxHighlighter() = default;
    
    /// Get the language name
    virtual std::string getLanguage() const = 0;
    
    /// Get file extensions this highlighter handles
    virtual std::vector<std::string> getExtensions() const = 0;
    
    /// Highlight a line of code
    virtual std::vector<Token> highlightLine(const std::string& line, size_t lineIndex) = 0;
    
    /// Reset highlighter state (called at start of file)
    virtual void reset() = 0;
    
    /// Update state for multiline constructs (e.g., block comments)
    virtual void updateState(const std::string& line, size_t lineIndex) = 0;
    
    /// Check if currently in a multiline construct
    virtual bool inMultilineConstruct() const = 0;
};

/**
 * @brief Factory to create highlighters based on file extension
 */
class HighlighterFactory {
public:
    /// Get highlighter for a file
    static std::unique_ptr<ISyntaxHighlighter> createForFile(const std::string& filename);
    
    /// Get highlighter by language name
    static std::unique_ptr<ISyntaxHighlighter> createForLanguage(const std::string& language);
    
    /// Get list of supported languages
    static std::vector<std::string> getSupportedLanguages();
};

/**
 * @brief Color mapping for token types
 */
ColorPair getTokenColor(TokenType type);

} // namespace astrax

#endif // ASTRAX_SYNTAX_HIGHLIGHTER_H
