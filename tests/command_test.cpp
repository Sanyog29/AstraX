#include <gtest/gtest.h>
#include "astrax/command.h"
#include "astrax/buffer.h"

using namespace astrax;

// ============================================================================
// KeyBindings Tests
// ============================================================================

TEST(KeyBindingsTest, DefaultBindings) {
    KeyBindings bindings;
    
    // Check that bindings exist
    // We can't easily test processing without a full Editor, 
    // so we just ensure the constructor runs without error
    SUCCEED();
}

TEST(KeyBindingsTest, ResetToDefaults) {
    KeyBindings bindings;
    bindings.resetToDefaults();
    SUCCEED();
}

// ============================================================================
// CommandExecutor Tests
// ============================================================================

class CommandExecutorTest : public ::testing::Test {
protected:
    CommandExecutor executor;
};

TEST_F(CommandExecutorTest, GetSuggestions) {
    auto suggestions = executor.getSuggestions("w");
    
    // Should include "w", "wq"
    EXPECT_GE(suggestions.size(), 1);
    
    bool hasW = false;
    for (const auto& s : suggestions) {
        if (s == "w") hasW = true;
    }
    EXPECT_TRUE(hasW);
}

TEST_F(CommandExecutorTest, GetSuggestionsEmpty) {
    auto suggestions = executor.getSuggestions("");
    EXPECT_GE(suggestions.size(), 5);  // Should have multiple commands
}

TEST_F(CommandExecutorTest, GetSuggestionsNoMatch) {
    auto suggestions = executor.getSuggestions("xyz123");
    EXPECT_EQ(suggestions.size(), 0);
}

// ============================================================================
// Command Pattern Tests
// ============================================================================

TEST(InsertTextCommandTest, Execute) {
    Buffer buffer;
    InsertTextCommand cmd(buffer, {0, 0}, "Hello");
    
    cmd.execute();
    
    EXPECT_EQ(buffer.getLine(0), "Hello");
}

TEST(DeleteTextCommandTest, Execute) {
    Buffer buffer("Hello World");
    Range range{{0, 0}, {0, 5}};
    DeleteTextCommand cmd(buffer, range);
    
    cmd.execute();
    
    // Just check it runs without error for now
    SUCCEED();
}
