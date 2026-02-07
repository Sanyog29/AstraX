#include "astrax/syntax/highlighter.h"
#include "astrax/syntax/cpp_highlighter.h"
#include <algorithm>

namespace astrax {

// ============================================================================
// Factory
// ============================================================================

std::unique_ptr<ISyntaxHighlighter> HighlighterFactory::createForFile(const std::string& filename) {
    // Find extension
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) {
        return nullptr;
    }
    
    std::string ext = filename.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    // C/C++
    if (ext == ".cpp" || ext == ".cc" || ext == ".cxx" || ext == ".c" ||
        ext == ".h" || ext == ".hpp" || ext == ".hxx") {
        return std::make_unique<CppHighlighter>();
    }
    
    // TODO: Add more language highlighters
    // Python: .py
    // JavaScript: .js, .jsx, .ts, .tsx
    // Java: .java
    // Rust: .rs
    // Go: .go
    
    return nullptr;
}

std::unique_ptr<ISyntaxHighlighter> HighlighterFactory::createForLanguage(const std::string& language) {
    std::string lower = language;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "c++" || lower == "cpp" || lower == "c") {
        return std::make_unique<CppHighlighter>();
    }
    
    return nullptr;
}

std::vector<std::string> HighlighterFactory::getSupportedLanguages() {
    return {"C++", "C"};
}

// ============================================================================
// Token Colors
// ============================================================================

ColorPair getTokenColor(TokenType type) {
    switch (type) {
        case TokenType::Keyword:      return {Color::Blue, Color::Default};
        case TokenType::Type:         return {Color::Cyan, Color::Default};
        case TokenType::String:       return {Color::Green, Color::Default};
        case TokenType::Number:       return {Color::Magenta, Color::Default};
        case TokenType::Comment:      return {Color::BrightBlack, Color::Default};
        case TokenType::Preprocessor: return {Color::Yellow, Color::Default};
        case TokenType::Function:     return {Color::BrightBlue, Color::Default};
        case TokenType::Operator:     return {Color::White, Color::Default};
        case TokenType::Bracket:      return {Color::BrightYellow, Color::Default};
        default:                      return {Color::White, Color::Default};
    }
}

} // namespace astrax
