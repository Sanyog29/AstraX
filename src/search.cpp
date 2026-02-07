#include "astrax/search.h"
#include <algorithm>
#include <cctype>

namespace astrax {

// ============================================================================
// Pattern Setting
// ============================================================================

void Search::setPattern(const std::string& pattern, const SearchOptions& options) {
    pattern_ = pattern;
    options_ = options;
    patternValid_ = true;
    errorMessage_.clear();
    
    if (options_.useRegex) {
        try {
            std::regex_constants::syntax_option_type flags = std::regex::ECMAScript;
            if (!options_.caseSensitive) {
                flags |= std::regex::icase;
            }
            compiledRegex_ = std::regex(pattern_, flags);
        } catch (const std::regex_error& e) {
            patternValid_ = false;
            errorMessage_ = e.what();
        }
    }
}

// ============================================================================
// Search Operations
// ============================================================================

size_t Search::patternLength() const {
    return pattern_.size();
}

bool Search::matchesAt(const std::string& line, size_t pos) const {
    if (pos + pattern_.size() > line.size()) {
        return false;
    }
    
    if (options_.caseSensitive) {
        return line.compare(pos, pattern_.size(), pattern_) == 0;
    } else {
        for (size_t i = 0; i < pattern_.size(); ++i) {
            if (std::tolower(static_cast<unsigned char>(line[pos + i])) !=
                std::tolower(static_cast<unsigned char>(pattern_[i]))) {
                return false;
            }
        }
        return true;
    }
}

SearchMatch Search::findNext(
    const std::vector<std::string>& lines,
    Position from
) const {
    if (pattern_.empty() || !patternValid_) {
        return SearchMatch();
    }
    
    // Start from current position
    size_t startLine = from.line;
    size_t startCol = from.column + 1;  // Start after current position
    
    for (size_t lineIdx = startLine; lineIdx < lines.size(); ++lineIdx) {
        const std::string& line = lines[lineIdx];
        size_t searchStart = (lineIdx == startLine) ? startCol : 0;
        
        if (options_.useRegex) {
            std::smatch match;
            std::string searchStr = line.substr(searchStart);
            if (std::regex_search(searchStr, match, compiledRegex_)) {
                return SearchMatch(
                    {lineIdx, searchStart + static_cast<size_t>(match.position())},
                    static_cast<size_t>(match.length()),
                    match.str()
                );
            }
        } else {
            for (size_t col = searchStart; col + pattern_.size() <= line.size(); ++col) {
                if (matchesAt(line, col)) {
                    return SearchMatch(
                        {lineIdx, col},
                        pattern_.size(),
                        line.substr(col, pattern_.size())
                    );
                }
            }
        }
    }
    
    // Wrap around if enabled
    if (options_.wrapAround) {
        for (size_t lineIdx = 0; lineIdx <= startLine; ++lineIdx) {
            const std::string& line = lines[lineIdx];
            size_t maxCol = (lineIdx == startLine) ? startCol : line.size();
            
            if (options_.useRegex) {
                std::smatch match;
                if (std::regex_search(line, match, compiledRegex_)) {
                    if (lineIdx < startLine || static_cast<size_t>(match.position()) < startCol) {
                        return SearchMatch(
                            {lineIdx, static_cast<size_t>(match.position())},
                            static_cast<size_t>(match.length()),
                            match.str()
                        );
                    }
                }
            } else {
                for (size_t col = 0; col + pattern_.size() <= maxCol; ++col) {
                    if (matchesAt(line, col)) {
                        return SearchMatch(
                            {lineIdx, col},
                            pattern_.size(),
                            line.substr(col, pattern_.size())
                        );
                    }
                }
            }
        }
    }
    
    return SearchMatch();
}

SearchMatch Search::findPrevious(
    const std::vector<std::string>& lines,
    Position from
) const {
    if (pattern_.empty() || !patternValid_) {
        return SearchMatch();
    }
    
    // Search backwards
    for (size_t lineOffset = 0; lineOffset <= from.line; ++lineOffset) {
        size_t lineIdx = from.line - lineOffset;
        const std::string& line = lines[lineIdx];
        
        size_t maxCol = (lineOffset == 0 && from.column > 0) ? from.column - 1 : line.size();
        if (maxCol > line.size()) {
            if (line.empty()) continue;
            maxCol = line.size() - 1;
        }
        
        // Search from end to start
        for (size_t colOffset = 0; colOffset <= maxCol; ++colOffset) {
            size_t col = maxCol - colOffset;
            if (col + pattern_.size() <= line.size() && matchesAt(line, col)) {
                return SearchMatch(
                    {lineIdx, col},
                    pattern_.size(),
                    line.substr(col, pattern_.size())
                );
            }
        }
    }
    
    // Wrap around if enabled
    if (options_.wrapAround) {
        for (size_t lineOffset = 0; lineOffset < lines.size() - from.line; ++lineOffset) {
            size_t lineIdx = lines.size() - 1 - lineOffset;
            if (lineIdx <= from.line) break;
            
            const std::string& line = lines[lineIdx];
            
            for (size_t colOffset = 0; colOffset < line.size(); ++colOffset) {
                size_t col = line.size() - 1 - colOffset;
                if (col + pattern_.size() <= line.size() && matchesAt(line, col)) {
                    return SearchMatch(
                        {lineIdx, col},
                        pattern_.size(),
                        line.substr(col, pattern_.size())
                    );
                }
            }
        }
    }
    
    return SearchMatch();
}

std::vector<SearchMatch> Search::findAll(const std::vector<std::string>& lines) const {
    std::vector<SearchMatch> matches;
    
    if (pattern_.empty() || !patternValid_) {
        return matches;
    }
    
    for (size_t lineIdx = 0; lineIdx < lines.size(); ++lineIdx) {
        const std::string& line = lines[lineIdx];
        
        if (options_.useRegex) {
            std::sregex_iterator it(line.begin(), line.end(), compiledRegex_);
            std::sregex_iterator end;
            
            while (it != end) {
                matches.push_back(SearchMatch(
                    {lineIdx, static_cast<size_t>(it->position())},
                    static_cast<size_t>(it->length()),
                    it->str()
                ));
                ++it;
            }
        } else {
            for (size_t col = 0; col + pattern_.size() <= line.size(); ++col) {
                if (matchesAt(line, col)) {
                    matches.push_back(SearchMatch(
                        {lineIdx, col},
                        pattern_.size(),
                        line.substr(col, pattern_.size())
                    ));
                }
            }
        }
    }
    
    return matches;
}

size_t Search::countMatches(const std::vector<std::string>& lines) const {
    return findAll(lines).size();
}

// ============================================================================
// Replace Operations
// ============================================================================

std::string Search::replace(
    std::vector<std::string>& lines,
    const SearchMatch& match,
    const std::string& replacement
) {
    if (match.position.line >= lines.size()) {
        return "";
    }
    
    std::string& line = lines[match.position.line];
    std::string replaced = line.substr(match.position.column, match.length);
    
    line.replace(match.position.column, match.length, replacement);
    
    return replaced;
}

size_t Search::replaceAll(
    std::vector<std::string>& lines,
    const std::string& replacement
) {
    auto matches = findAll(lines);
    
    // Replace from end to start to preserve positions
    size_t count = 0;
    for (auto it = matches.rbegin(); it != matches.rend(); ++it) {
        replace(lines, *it, replacement);
        ++count;
    }
    
    return count;
}

// ============================================================================
// History
// ============================================================================

void Search::addToHistory(const std::string& pattern) {
    // Remove if already exists
    auto it = std::find(history_.begin(), history_.end(), pattern);
    if (it != history_.end()) {
        history_.erase(it);
    }
    
    // Add to front
    history_.insert(history_.begin(), pattern);
    
    // Limit size
    if (history_.size() > MAX_HISTORY) {
        history_.pop_back();
    }
}

std::string Search::getHistoryItem(size_t index) const {
    if (index >= history_.size()) {
        return "";
    }
    return history_[index];
}

} // namespace astrax
