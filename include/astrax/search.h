#ifndef ASTRAX_SEARCH_H
#define ASTRAX_SEARCH_H

#include "types.h"
#include <string>
#include <vector>
#include <regex>

namespace astrax {

/**
 * @brief Search result with match information
 */
struct SearchMatch {
    Position position;
    size_t length;
    std::string text;
    bool valid = false;  // Used instead of optional
    
    SearchMatch() : length(0), valid(false) {}
    SearchMatch(Position pos, size_t len, const std::string& txt)
        : position(pos), length(len), text(txt), valid(true) {}
    
    operator bool() const { return valid; }
};

/**
 * @brief Search direction
 */
enum class SearchDirection {
    Forward,
    Backward
};

/**
 * @brief Search options
 */
struct SearchOptions {
    bool caseSensitive = false;
    bool wholeWord = false;
    bool useRegex = false;
    bool wrapAround = true;
    SearchDirection direction = SearchDirection::Forward;
};

/**
 * @brief Search and replace engine
 */
class Search {
public:
    Search() = default;
    
    // ========================================================================
    // Search
    // ========================================================================
    
    /// Set the search pattern
    void setPattern(const std::string& pattern, const SearchOptions& options = {});
    
    /// Get current pattern
    const std::string& getPattern() const { return pattern_; }
    
    /// Check if pattern is valid (for regex mode)
    bool isPatternValid() const { return patternValid_; }
    
    /// Get error message if pattern is invalid
    const std::string& getError() const { return errorMessage_; }
    
    /// Find next match from position
    SearchMatch findNext(
        const std::vector<std::string>& lines,
        Position from
    ) const;
    
    /// Find previous match from position
    SearchMatch findPrevious(
        const std::vector<std::string>& lines,
        Position from
    ) const;
    
    /// Find all matches
    std::vector<SearchMatch> findAll(
        const std::vector<std::string>& lines
    ) const;
    
    /// Count matches
    size_t countMatches(const std::vector<std::string>& lines) const;
    
    // ========================================================================
    // Replace
    // ========================================================================
    
    /// Replace match at position, returns replaced text
    std::string replace(
        std::vector<std::string>& lines,
        const SearchMatch& match,
        const std::string& replacement
    );
    
    /// Replace all matches, returns number replaced
    size_t replaceAll(
        std::vector<std::string>& lines,
        const std::string& replacement
    );
    
    // ========================================================================
    // History
    // ========================================================================
    
    /// Add pattern to search history
    void addToHistory(const std::string& pattern);
    
    /// Get search history
    const std::vector<std::string>& getHistory() const { return history_; }
    
    /// Navigate history (returns pattern at index, empty string if invalid)
    std::string getHistoryItem(size_t index) const;

private:
    std::string pattern_;
    SearchOptions options_;
    bool patternValid_ = true;
    std::string errorMessage_;
    std::regex compiledRegex_;
    
    std::vector<std::string> history_;
    static const size_t MAX_HISTORY = 100;
    
    // Helper methods
    bool matchesAt(const std::string& line, size_t pos) const;
    size_t patternLength() const;
};

} // namespace astrax

#endif // ASTRAX_SEARCH_H
