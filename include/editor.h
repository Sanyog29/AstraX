#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <conio.h>
#include <windows.h>

// Editor modes (Astra Modes !! : These are the core of everything)
enum EditorMode {
    NORMAL,
    INSERT,
    COMMAND
};

class Editor {
private:
    std::vector<std::string> buffer;
    size_t cursorX, cursorY;
    std::string filename;
    EditorMode mode;
    std::string statusMessage;
    
    // Terminal ko handle karta hai
    void clearScreen();
    void setCursorPosition(int x, int y);
    void refreshScreen();
    
    // File dekhta hai ye section
    bool loadFile(const std::string& filename);
    bool saveFile();
    bool saveFileAs(const std::string& newFilename);
    bool deleteFile();
    bool renameFile(const std::string& newFilename);
    
    // Compilation aur execution ke liye
    bool compileAndRunFile();
    bool compileAndRunCpp();
    bool compileAndRunJava();
    std::string getFileExtension();
    
    // Editor operations
    void processNormalMode(int key);
    void processInsertMode(int key);
    void processCommandMode(int key);
    void insertCharacter(char c);
    void deleteCharacter();
    
public:
    Editor();
    void run(const std::string& filename = "");
    void setStatusMessage(const std::string& message);
    
    // Add these to the private section if they're not already there:
    void moveForwardWord();
    void moveBackwardWord();
};

#endif // EDITOR_H