#include <gtest/gtest.h>
#include "astrax/buffer.h"

using namespace astrax;

// ============================================================================
// Buffer Creation Tests
// ============================================================================

TEST(BufferTest, DefaultConstructor) {
    Buffer buffer;
    EXPECT_EQ(buffer.lineCount(), 1);
    EXPECT_TRUE(buffer.isEmpty());
    EXPECT_FALSE(buffer.isModified());
}

TEST(BufferTest, StringConstructor) {
    Buffer buffer("Hello\nWorld");
    EXPECT_EQ(buffer.lineCount(), 2);
    EXPECT_EQ(buffer.getLine(0), "Hello");
    EXPECT_EQ(buffer.getLine(1), "World");
    EXPECT_FALSE(buffer.isEmpty());
}

TEST(BufferTest, EmptyStringConstructor) {
    Buffer buffer("");
    EXPECT_EQ(buffer.lineCount(), 1);
    EXPECT_TRUE(buffer.isEmpty());
}

// ============================================================================
// Cursor Movement Tests
// ============================================================================

TEST(BufferTest, CursorMovement) {
    Buffer buffer("Hello\nWorld\nTest");
    
    // Initial position
    EXPECT_EQ(buffer.getCursor().line, 0);
    EXPECT_EQ(buffer.getCursor().column, 0);
    
    // Move right
    buffer.moveCursor(1, 0);
    EXPECT_EQ(buffer.getCursor().column, 1);
    
    // Move down
    buffer.moveCursor(0, 1);
    EXPECT_EQ(buffer.getCursor().line, 1);
    
    // Move left
    buffer.moveCursor(-1, 0);
    EXPECT_EQ(buffer.getCursor().column, 0);
    
    // Move up
    buffer.moveCursor(0, -1);
    EXPECT_EQ(buffer.getCursor().line, 0);
}

TEST(BufferTest, CursorClamp) {
    Buffer buffer("Hello");
    
    // Try to move past end of line
    buffer.moveCursor(100, 0);
    EXPECT_EQ(buffer.getCursor().column, 5);
    
    // Try to move before start
    buffer.moveCursor(-100, 0);
    EXPECT_EQ(buffer.getCursor().column, 0);
    
    // Try to move past last line
    buffer.moveCursor(0, 100);
    EXPECT_EQ(buffer.getCursor().line, 0);
}

TEST(BufferTest, LineStartEnd) {
    Buffer buffer("Hello World");
    buffer.setCursor({0, 5});
    
    buffer.moveToLineStart();
    EXPECT_EQ(buffer.getCursor().column, 0);
    
    buffer.moveToLineEnd();
    EXPECT_EQ(buffer.getCursor().column, 11);
}

TEST(BufferTest, WordMovement) {
    Buffer buffer("Hello World Test");
    
    buffer.moveForwardWord();
    EXPECT_EQ(buffer.getCursor().column, 6);
    
    buffer.moveForwardWord();
    EXPECT_EQ(buffer.getCursor().column, 12);
    
    buffer.moveBackwardWord();
    EXPECT_EQ(buffer.getCursor().column, 6);
}

// ============================================================================
// Editing Tests
// ============================================================================

TEST(BufferTest, InsertChar) {
    Buffer buffer;
    
    buffer.insertChar('H');
    buffer.insertChar('i');
    
    EXPECT_EQ(buffer.getLine(0), "Hi");
    EXPECT_EQ(buffer.getCursor().column, 2);
    EXPECT_TRUE(buffer.isModified());
}

TEST(BufferTest, InsertString) {
    Buffer buffer;
    
    buffer.insertString("Hello");
    
    EXPECT_EQ(buffer.getLine(0), "Hello");
    EXPECT_EQ(buffer.getCursor().column, 5);
}

TEST(BufferTest, InsertNewline) {
    Buffer buffer("Hello World");
    buffer.setCursor({0, 5});
    
    buffer.insertNewline();
    
    EXPECT_EQ(buffer.lineCount(), 2);
    EXPECT_EQ(buffer.getLine(0), "Hello");
    EXPECT_EQ(buffer.getLine(1), " World");
    EXPECT_EQ(buffer.getCursor().line, 1);
    EXPECT_EQ(buffer.getCursor().column, 0);
}

TEST(BufferTest, DeleteCharBefore) {
    Buffer buffer("Hello");
    buffer.setCursor({0, 5});
    
    buffer.deleteCharBefore();
    
    EXPECT_EQ(buffer.getLine(0), "Hell");
    EXPECT_EQ(buffer.getCursor().column, 4);
}

TEST(BufferTest, DeleteCharAt) {
    Buffer buffer("Hello");
    buffer.setCursor({0, 0});
    
    buffer.deleteCharAt();
    
    EXPECT_EQ(buffer.getLine(0), "ello");
}

TEST(BufferTest, DeleteLine) {
    Buffer buffer("Hello\nWorld\nTest");
    buffer.setCursor({1, 0});
    
    buffer.deleteLine();
    
    EXPECT_EQ(buffer.lineCount(), 2);
    EXPECT_EQ(buffer.getLine(0), "Hello");
    EXPECT_EQ(buffer.getLine(1), "Test");
}

TEST(BufferTest, JoinLines) {
    Buffer buffer("Hello\nWorld");
    
    buffer.joinLines();
    
    EXPECT_EQ(buffer.lineCount(), 1);
    EXPECT_EQ(buffer.getLine(0), "Hello World");
}

// ============================================================================
// Undo/Redo Tests
// ============================================================================

TEST(BufferTest, UndoRedo) {
    Buffer buffer;
    
    buffer.insertString("Hello");
    EXPECT_EQ(buffer.getLine(0), "Hello");
    EXPECT_TRUE(buffer.canUndo());
    
    buffer.undo();
    EXPECT_EQ(buffer.getLine(0), "");
    EXPECT_TRUE(buffer.canRedo());
    
    buffer.redo();
    EXPECT_EQ(buffer.getLine(0), "Hello");
}

TEST(BufferTest, MultipleUndo) {
    Buffer buffer;
    
    buffer.insertChar('A');
    buffer.insertChar('B');
    buffer.insertChar('C');
    
    EXPECT_EQ(buffer.getLine(0), "ABC");
    
    buffer.undo();
    EXPECT_EQ(buffer.getLine(0), "AB");
    
    buffer.undo();
    EXPECT_EQ(buffer.getLine(0), "A");
    
    buffer.undo();
    EXPECT_EQ(buffer.getLine(0), "");
}

// ============================================================================
// Clipboard Tests
// ============================================================================

TEST(BufferTest, YankAndPaste) {
    Buffer buffer("Hello\nWorld");
    
    buffer.yankLine();
    EXPECT_EQ(buffer.getYanked(), "Hello");
    
    buffer.paste();
    
    EXPECT_EQ(buffer.lineCount(), 3);
    EXPECT_EQ(buffer.getLine(1), "Hello");
    EXPECT_EQ(buffer.getLine(2), "World");
}

// ============================================================================
// File I/O Tests (if applicable)
// ============================================================================

TEST(BufferTest, GetContent) {
    Buffer buffer("Hello\nWorld");
    
    std::string content = buffer.getContent();
    EXPECT_EQ(content, "Hello\nWorld");
}

TEST(BufferTest, ModifiedFlag) {
    Buffer buffer;
    EXPECT_FALSE(buffer.isModified());
    
    buffer.insertChar('A');
    EXPECT_TRUE(buffer.isModified());
    
    buffer.markSaved();
    EXPECT_FALSE(buffer.isModified());
}
