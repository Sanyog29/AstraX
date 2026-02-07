#ifndef ASTRAX_CPP_HIGHLIGHTER_H
#define ASTRAX_CPP_HIGHLIGHTER_H

#include "highlighter.h"
#include <unordered_set>

namespace astrax {

/**
 * @brief C/C++ syntax highlighter
 */
class CppHighlighter : public ISyntaxHighlighter {
public:
    CppHighlighter();
    
    std::string getLanguage() const override { return "C++"; }
    
    std::vector<std::string> getExtensions() const override {
        return {".cpp", ".cc", ".cxx", ".c", ".h", ".hpp", ".hxx"};
    }
    
    std::vector<Token> highlightLine(const std::string& line, size_t lineIndex) override;
    
    void reset() override { inBlockComment_ = false; inRawString_ = false; }
    
    void updateState(const std::string& line, size_t lineIndex) override;
    
    bool inMultilineConstruct() const override { 
        return inBlockComment_ || inRawString_; 
    }
    
private:
    bool inBlockComment_ = false;
    bool inRawString_ = false;
    std::string rawStringDelimiter_;
    
    std::unordered_set<std::string> keywords_;
    std::unordered_set<std::string> types_;
    std::unordered_set<std::string> preprocessor_;
    
    void initializeKeywords();
    
    bool isKeyword(const std::string& word) const;
    bool isType(const std::string& word) const;
    bool isNumber(const std::string& word) const;
    
    size_t skipString(const std::string& line, size_t start, std::vector<Token>& tokens);
    size_t skipComment(const std::string& line, size_t start, std::vector<Token>& tokens);
    size_t skipPreprocessor(const std::string& line, size_t start, std::vector<Token>& tokens);
    size_t skipWord(const std::string& line, size_t start, std::vector<Token>& tokens);
    size_t skipNumber(const std::string& line, size_t start, std::vector<Token>& tokens);
};

} // namespace astrax

#endif // ASTRAX_CPP_HIGHLIGHTER_H
