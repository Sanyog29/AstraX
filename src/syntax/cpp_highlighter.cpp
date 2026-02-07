#include "astrax/syntax/cpp_highlighter.h"
#include <cctype>

namespace astrax {

// ============================================================================
// Constructor
// ============================================================================

CppHighlighter::CppHighlighter() {
    initializeKeywords();
}

void CppHighlighter::initializeKeywords() {
    // C++ keywords
    keywords_ = {
        "alignas", "alignof", "and", "and_eq", "asm", "auto",
        "bitand", "bitor", "break", "case", "catch", "class",
        "compl", "concept", "const", "consteval", "constexpr", "constinit",
        "const_cast", "continue", "co_await", "co_return", "co_yield",
        "decltype", "default", "delete", "do", "dynamic_cast",
        "else", "enum", "explicit", "export", "extern",
        "false", "for", "friend", "goto", "if", "inline",
        "mutable", "namespace", "new", "noexcept", "not", "not_eq",
        "nullptr", "operator", "or", "or_eq", "private", "protected",
        "public", "register", "reinterpret_cast", "requires", "return",
        "sizeof", "static", "static_assert", "static_cast", "struct",
        "switch", "template", "this", "thread_local", "throw",
        "true", "try", "typedef", "typeid", "typename",
        "union", "using", "virtual", "volatile", "while",
        "xor", "xor_eq", "override", "final"
    };
    
    // Types
    types_ = {
        "void", "bool", "char", "wchar_t", "char8_t", "char16_t", "char32_t",
        "short", "int", "long", "signed", "unsigned", "float", "double",
        "size_t", "int8_t", "int16_t", "int32_t", "int64_t",
        "uint8_t", "uint16_t", "uint32_t", "uint64_t",
        "ptrdiff_t", "intptr_t", "uintptr_t",
        "string", "vector", "map", "unordered_map", "set", "unordered_set",
        "array", "list", "deque", "queue", "stack", "pair", "tuple",
        "unique_ptr", "shared_ptr", "weak_ptr", "optional", "variant",
        "string_view", "span", "any", "function"
    };
    
    // Preprocessor directives
    preprocessor_ = {
        "#include", "#define", "#undef", "#if", "#ifdef", "#ifndef",
        "#else", "#elif", "#endif", "#pragma", "#error", "#warning", "#line"
    };
}

// ============================================================================
// State Management
// ============================================================================

void CppHighlighter::updateState(const std::string& line, size_t /*lineIndex*/) {
    // Check for block comment start/end
    size_t pos = 0;
    while (pos < line.size()) {
        if (inBlockComment_) {
            size_t end = line.find("*/", pos);
            if (end != std::string::npos) {
                inBlockComment_ = false;
                pos = end + 2;
            } else {
                return;
            }
        } else {
            // Skip strings
            if (line[pos] == '"' || line[pos] == '\'') {
                char quote = line[pos];
                pos++;
                while (pos < line.size() && line[pos] != quote) {
                    if (line[pos] == '\\' && pos + 1 < line.size()) {
                        pos += 2;
                    } else {
                        pos++;
                    }
                }
                if (pos < line.size()) pos++;
            }
            // Check for line comment
            else if (pos + 1 < line.size() && line[pos] == '/' && line[pos + 1] == '/') {
                return;
            }
            // Check for block comment start
            else if (pos + 1 < line.size() && line[pos] == '/' && line[pos + 1] == '*') {
                inBlockComment_ = true;
                pos += 2;
                size_t end = line.find("*/", pos);
                if (end != std::string::npos) {
                    inBlockComment_ = false;
                    pos = end + 2;
                } else {
                    return;
                }
            } else {
                pos++;
            }
        }
    }
}

// ============================================================================
// Highlighting
// ============================================================================

std::vector<Token> CppHighlighter::highlightLine(const std::string& line, size_t /*lineIndex*/) {
    std::vector<Token> tokens;
    size_t pos = 0;
    
    // Handle continuation of block comment
    if (inBlockComment_) {
        size_t end = line.find("*/");
        if (end != std::string::npos) {
            tokens.push_back({0, end + 2, TokenType::Comment});
            inBlockComment_ = false;
            pos = end + 2;
        } else {
            tokens.push_back({0, line.size(), TokenType::Comment});
            return tokens;
        }
    }
    
    while (pos < line.size()) {
        // Skip whitespace
        if (std::isspace(static_cast<unsigned char>(line[pos]))) {
            pos++;
            continue;
        }
        
        // Preprocessor
        if (line[pos] == '#') {
            pos = skipPreprocessor(line, pos, tokens);
        }
        // Line comment
        else if (pos + 1 < line.size() && line[pos] == '/' && line[pos + 1] == '/') {
            tokens.push_back({pos, line.size() - pos, TokenType::Comment});
            break;
        }
        // Block comment
        else if (pos + 1 < line.size() && line[pos] == '/' && line[pos + 1] == '*') {
            pos = skipComment(line, pos, tokens);
        }
        // String literal
        else if (line[pos] == '"' || line[pos] == '\'') {
            pos = skipString(line, pos, tokens);
        }
        // Raw string literal
        else if (pos + 1 < line.size() && line[pos] == 'R' && line[pos + 1] == '"') {
            pos = skipString(line, pos, tokens);
        }
        // Number
        else if (std::isdigit(static_cast<unsigned char>(line[pos])) ||
                 (line[pos] == '.' && pos + 1 < line.size() && 
                  std::isdigit(static_cast<unsigned char>(line[pos + 1])))) {
            pos = skipNumber(line, pos, tokens);
        }
        // Identifier/keyword
        else if (std::isalpha(static_cast<unsigned char>(line[pos])) || line[pos] == '_') {
            pos = skipWord(line, pos, tokens);
        }
        // Operators and brackets
        else if (line[pos] == '(' || line[pos] == ')' || 
                 line[pos] == '[' || line[pos] == ']' ||
                 line[pos] == '{' || line[pos] == '}') {
            tokens.push_back({pos, 1, TokenType::Bracket});
            pos++;
        }
        else {
            pos++;
        }
    }
    
    return tokens;
}

size_t CppHighlighter::skipString(const std::string& line, size_t start, std::vector<Token>& tokens) {
    size_t pos = start;
    char quote = line[pos];
    
    // Handle raw strings
    if (quote == 'R' && pos + 1 < line.size() && line[pos + 1] == '"') {
        pos += 2;
        // Find delimiter
        size_t delimEnd = line.find('(', pos);
        if (delimEnd != std::string::npos) {
            std::string delim = ")" + line.substr(pos, delimEnd - pos) + "\"";
            size_t strEnd = line.find(delim, delimEnd);
            if (strEnd != std::string::npos) {
                tokens.push_back({start, strEnd + delim.size() - start, TokenType::String});
                return strEnd + delim.size();
            }
        }
        tokens.push_back({start, line.size() - start, TokenType::String});
        return line.size();
    }
    
    pos++;
    while (pos < line.size()) {
        if (line[pos] == '\\' && pos + 1 < line.size()) {
            pos += 2;
        } else if (line[pos] == quote) {
            pos++;
            break;
        } else {
            pos++;
        }
    }
    
    tokens.push_back({start, pos - start, TokenType::String});
    return pos;
}

size_t CppHighlighter::skipComment(const std::string& line, size_t start, std::vector<Token>& tokens) {
    size_t pos = start + 2;  // Skip /*
    size_t end = line.find("*/", pos);
    
    if (end != std::string::npos) {
        tokens.push_back({start, end + 2 - start, TokenType::Comment});
        return end + 2;
    } else {
        inBlockComment_ = true;
        tokens.push_back({start, line.size() - start, TokenType::Comment});
        return line.size();
    }
}

size_t CppHighlighter::skipPreprocessor(const std::string& line, size_t start, std::vector<Token>& tokens) {
    size_t pos = start;
    
    // Skip #
    pos++;
    
    // Skip whitespace
    while (pos < line.size() && std::isspace(static_cast<unsigned char>(line[pos]))) {
        pos++;
    }
    
    // Read directive name
    size_t wordStart = pos;
    while (pos < line.size() && (std::isalnum(static_cast<unsigned char>(line[pos])) || line[pos] == '_')) {
        pos++;
    }
    
    tokens.push_back({start, pos - start, TokenType::Preprocessor});
    return pos;
}

size_t CppHighlighter::skipWord(const std::string& line, size_t start, std::vector<Token>& tokens) {
    size_t pos = start;
    
    while (pos < line.size() && (std::isalnum(static_cast<unsigned char>(line[pos])) || line[pos] == '_')) {
        pos++;
    }
    
    std::string word = line.substr(start, pos - start);
    
    TokenType type = TokenType::Default;
    if (isKeyword(word)) {
        type = TokenType::Keyword;
    } else if (isType(word)) {
        type = TokenType::Type;
    } else if (pos < line.size() && line[pos] == '(') {
        type = TokenType::Function;
    }
    
    if (type != TokenType::Default) {
        tokens.push_back({start, pos - start, type});
    }
    
    return pos;
}

size_t CppHighlighter::skipNumber(const std::string& line, size_t start, std::vector<Token>& tokens) {
    size_t pos = start;
    
    // Hex, octal, binary
    if (line[pos] == '0' && pos + 1 < line.size()) {
        char next = line[pos + 1];
        if (next == 'x' || next == 'X') {
            pos += 2;
            while (pos < line.size() && std::isxdigit(static_cast<unsigned char>(line[pos]))) {
                pos++;
            }
            tokens.push_back({start, pos - start, TokenType::Number});
            return pos;
        } else if (next == 'b' || next == 'B') {
            pos += 2;
            while (pos < line.size() && (line[pos] == '0' || line[pos] == '1')) {
                pos++;
            }
            tokens.push_back({start, pos - start, TokenType::Number});
            return pos;
        }
    }
    
    // Integer or float
    while (pos < line.size() && std::isdigit(static_cast<unsigned char>(line[pos]))) {
        pos++;
    }
    
    // Decimal point
    if (pos < line.size() && line[pos] == '.') {
        pos++;
        while (pos < line.size() && std::isdigit(static_cast<unsigned char>(line[pos]))) {
            pos++;
        }
    }
    
    // Exponent
    if (pos < line.size() && (line[pos] == 'e' || line[pos] == 'E')) {
        pos++;
        if (pos < line.size() && (line[pos] == '+' || line[pos] == '-')) {
            pos++;
        }
        while (pos < line.size() && std::isdigit(static_cast<unsigned char>(line[pos]))) {
            pos++;
        }
    }
    
    // Suffix (u, l, f, etc.)
    while (pos < line.size() && (line[pos] == 'u' || line[pos] == 'U' ||
                                   line[pos] == 'l' || line[pos] == 'L' ||
                                   line[pos] == 'f' || line[pos] == 'F')) {
        pos++;
    }
    
    tokens.push_back({start, pos - start, TokenType::Number});
    return pos;
}

bool CppHighlighter::isKeyword(const std::string& word) const {
    return keywords_.find(word) != keywords_.end();
}

bool CppHighlighter::isType(const std::string& word) const {
    return types_.find(word) != types_.end();
}

bool CppHighlighter::isNumber(const std::string& word) const {
    if (word.empty()) return false;
    return std::isdigit(static_cast<unsigned char>(word[0]));
}

} // namespace astrax
