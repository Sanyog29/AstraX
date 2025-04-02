#include "../include/editor.h"
#include <cctype>

Editor::Editor() : cursorX(0), cursorY(0), mode(NORMAL) {
    // empty line ko initilize karta hai
    buffer.push_back("");
    setStatusMessage("NORMAL MODE");
}

void Editor::clearScreen() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordScreen = { 0, 0 };
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    FillConsoleOutputCharacter(hConsole, ' ', dwConSize, coordScreen, &cCharsWritten);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);
    SetConsoleCursorPosition(hConsole, coordScreen);
}

void Editor::setCursorPosition(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(hConsole, coord);
}

void Editor::refreshScreen() {
    // Clear krne se pehle cursor ko set karte hain
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    
    // clearScreen ko call karte hain
    // Clear the screen and reset cursor position
    clearScreen();
    
    // Display krta hai buffer content ko
    for (size_t i = 0; i < buffer.size(); i++) {
        if (mode == INSERT) {
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        } else {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
        
        setCursorPosition(0, i);
        std::cout << buffer[i];
        
        int charsToErase = csbi.dwSize.X - buffer[i].length();
        if (charsToErase > 0) {
            std::cout << std::string(charsToErase, ' ');
        }
        
        std::cout.flush();
    }
    
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    int width = csbi.dwSize.X;
    int height = csbi.dwSize.Y;
    
    setCursorPosition(0, height - 2);
    std::cout << std::string(width, '-');
    
    std::string modeStr;
    setCursorPosition(0, height - 1);
    
    // Set color based on mode
    switch (mode) {
        case NORMAL:
            SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            modeStr = "NORMAL";
            break;
        case INSERT:
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            modeStr = "INSERT";
            break;
        case COMMAND:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
            modeStr = "COMMAND";
            break;
    }
    
    // Display mode with color
    std::cout << modeStr;
    
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    
    std::string status = " | " + (filename.empty() ? "[No Name]" : filename);
    status += " | " + statusMessage;
    
    if ((modeStr.length() + status.length()) > width) {
        status = status.substr(0, width - modeStr.length());
    }
    
    std::cout << status << std::string(width - (modeStr.length() + status.length()), ' ');
    
    setCursorPosition(cursorX, cursorY);
}

bool Editor::loadFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    buffer.clear();
    std::string line;
    while (std::getline(file, line)) {
        buffer.push_back(line);
    }
    
    if (buffer.empty()) {
        buffer.push_back("");
    }
    
    if (filename.find("templates/") != 0 && filename.find("templates\\") != 0) {
        this->filename = filename;
    }
    
    cursorX = 0;
    cursorY = 0;
    return true;
}

bool Editor::saveFile() {
    if (filename.empty()) {
        setStatusMessage("No filename specified");
        return false;
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        setStatusMessage("Cannot open file for writing");
        return false;
    }
    
    for (const auto& line : buffer) {
        file << line << std::endl;
    }
    
    setStatusMessage("File saved: " + filename);
    return true;
}

void Editor::insertCharacter(char c) {
    if (cursorY >= buffer.size()) {
        buffer.push_back("");
    }
    
    buffer[cursorY].insert(cursorX, 1, c);
    cursorX++;
}

void Editor::deleteCharacter() {
    if (cursorX > 0) {
        buffer[cursorY].erase(cursorX - 1, 1);
        cursorX--;
    } else if (cursorY > 0) {
        cursorX = buffer[cursorY - 1].length();
        buffer[cursorY - 1] += buffer[cursorY];
        buffer.erase(buffer.begin() + cursorY);
        cursorY--;
    }
}

void Editor::processNormalMode(int key) {
    if (buffer.empty()) {
        buffer.push_back("");
    }
    
    // Handle regular keys
    switch (key) {
        case 'h': // Left
            if (cursorX > 0) cursorX--;
            break;
        case 'j': // Down
            if (cursorY < buffer.size() - 1) cursorY++;
            break;
        case 'k': // Up
            if (cursorY > 0) cursorY--;
            break;
        case 'l': // Right
            if (cursorX < buffer[cursorY].length()) cursorX++;
            break;
        case '0': // Move to beginning of line
            cursorX = 0;
            break;
        case '$': // Move to end of line
            cursorX = buffer[cursorY].length();
            break;
        case 'g': // Move to first line
            cursorY = 0;
            break;
        case 'G': // Move to last line
            cursorY = buffer.size() - 1;
            break;
        case 'w': // Move forward one word
            moveForwardWord();
            break;
        case 'b': // Move backward one word
            moveBackwardWord();
            break;
        case 'i': // Enter insert mode
            mode = INSERT;
            setStatusMessage("INSERT MODE");
            break;
        case ':': // Enter command mode
            mode = COMMAND;
            setStatusMessage(":");
            break;
        case 'x': // Delete character under cursor
            if (cursorX < buffer[cursorY].length()) {
                buffer[cursorY].erase(cursorX, 1);
            }
            break;
    }
}

void Editor::processInsertMode(int key) {
    if (key == 224 || key == 0) {
        // Do nothing - arrow keys are handled in the run method
        return;
    } else if (key == 27) {
        mode = NORMAL;
        setStatusMessage("NORMAL MODE");
        if (cursorX > 0) cursorX--;
    } else if (key == 8 || key == 127) { 
        deleteCharacter();
    } else if (key == 13) { 
        std::string currentLine = buffer[cursorY];
        std::string restOfLine = currentLine.substr(cursorX);
        buffer[cursorY] = currentLine.substr(0, cursorX);
        buffer.insert(buffer.begin() + cursorY + 1, restOfLine);
        cursorX = 0;
        cursorY++;
    } else if (key >= 32 && key <= 126) { // Printable ASCII
        insertCharacter(static_cast<char>(key));
    }
    // We don't need to handle arrow keys here as they are handled in the run method
    // The extended key codes (72=Up, 75=Left, 77=Right, 80=Down) should not be processed here
    // as they are part of the sequence that's already handled in the run method
}

void Editor::processCommandMode(int key) {
    static std::string commandBuffer;
    
    if (key == 27) { 
        mode = NORMAL;
        commandBuffer.clear();
        setStatusMessage("NORMAL MODE");
    } else if (key == 13) { 
        setStatusMessage("Processing command: '" + commandBuffer + "'");
        
        if (commandBuffer == "w") {
            saveFile();
        } else if (commandBuffer == "q") {
            exit(0);
        } else if (commandBuffer == "wq") {
            saveFile();
            exit(0);
        } else if (commandBuffer == "run") {
            setStatusMessage("Attempting to run file...");
            compileAndRunFile();
        } else if (commandBuffer.substr(0, 6) == "saveas" && commandBuffer.length() > 7) {
            std::string newFilename = commandBuffer.substr(7);
            saveFileAs(newFilename);
        } else if (commandBuffer == "delete") {
            deleteFile();
        } else if (commandBuffer.substr(0, 6) == "rename" && commandBuffer.length() > 7) {
            std::string newFilename = commandBuffer.substr(7);
            renameFile(newFilename);
        } else {
            setStatusMessage("Unknown command: " + commandBuffer);
        }
        commandBuffer.clear();
        mode = NORMAL;
    } else if (key == 8 || key == 127) { 
        if (!commandBuffer.empty()) {
            commandBuffer.pop_back();
            setStatusMessage(":" + commandBuffer);
        } else {
            mode = NORMAL;
            setStatusMessage("NORMAL MODE");
        }
    } else if (key >= 32 && key <= 126) { 
        commandBuffer += static_cast<char>(key);
        setStatusMessage(":" + commandBuffer);
    }
}

void Editor::setStatusMessage(const std::string& message) {
    statusMessage = message;
}

void Editor::run(const std::string& filename) {
    if (!filename.empty()) {
        if (!loadFile(filename)) {
            std::string ext = filename.substr(filename.find_last_of('.') + 1);
            setStatusMessage("New file: " + filename);
            this->filename = filename;
        }
    }

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
    
    while (true) {
        refreshScreen();
        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        
        int key = _getch();

        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        
        if (key == 224 || key == 0) {
            int extendedKey = _getch();
            
            switch (extendedKey) {
                case 75: 
                    if (cursorX > 0) cursorX--;
                    break;
                case 77: 
                    if (cursorX < buffer[cursorY].length()) cursorX++;
                    break;
                case 72: 
                    if (cursorY > 0) cursorY--;
                    break;
                case 80: 
                    if (cursorY < buffer.size() - 1) cursorY++;
                    break;
                case 71: 
                    cursorX = 0;
                    break;
                case 79: 
                    cursorX = buffer[cursorY].length();
                    break;
            }
        }
       
        else {
            switch (mode) {
                case NORMAL:
                    processNormalMode(key);
                    break;
                case INSERT:
                    processInsertMode(key);
                    break;
                case COMMAND:
                    processCommandMode(key);
                    break;
            }
        }
        
        if (cursorY >= buffer.size()) {
            cursorY = buffer.size() - 1;
        }
        if (cursorX > buffer[cursorY].length()) {
            cursorX = buffer[cursorY].length();
        }
    }
}

std::string Editor::getFileExtension() {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos) {
        return filename.substr(dotPos + 1);
    }
    return "";
}

bool Editor::compileAndRunCpp() {
    if (filename.empty()) {
        setStatusMessage("No file to compile");
        return false;
    }
    
    if (!saveFile()) {
        return false;
    }
    
    std::string baseFilename = filename.substr(0, filename.find_last_of('.'));
    std::string exeFilename = baseFilename + ".exe";
    
    std::string compileCmd = "g++ -std=c++11 \"" + filename + "\" -o \"" + exeFilename + "\"";
    
    clearScreen();
    std::cout << "Compiling " << filename << "...\n";
    

    int compileResult = system(compileCmd.c_str());
    
    if (compileResult != 0) {
        std::cout << "\nCompilation failed with error code: " << compileResult << ". Press any key to return to editor.\n";
        _getch();
        return false;
    }
    
    std::cout << "\nCompilation successful. Running program...\n\n";
    std::string runCmd = "\"" + exeFilename + "\"";
    system(runCmd.c_str());
    
    std::cout << "\nProgram execution completed. Press any key to return to editor.\n";
    _getch();
    return true;
}

bool Editor::compileAndRunJava() {
    if (filename.empty()) {
        setStatusMessage("No file to compile");
        return false;
    }
    
    if (!saveFile()) {
        return false;
    }
    
    std::string className = filename.substr(0, filename.find_last_of('.'));
    size_t lastSlash = className.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        className = className.substr(lastSlash + 1);
    }
    
    if (className == "try") {
        setStatusMessage("Error: 'try' is a reserved keyword in Java and cannot be used as a class name");
        return false;
    }
    std::string directory = filename.substr(0, filename.find_last_of("/\\"));
    if (directory.empty()) {
        directory = ".";
    }
    
    std::string batchFilename = "run_java_temp.bat";
    std::ofstream batchFile(batchFilename);
    
    if (!batchFile.is_open()) {
        setStatusMessage("Failed to create temporary batch file");
        return false;
    }
    
    batchFile << "@echo off" << std::endl;
    batchFile << "cd /d \"" << directory << "\"" << std::endl;
    batchFile << "echo Compiling " << filename << "..." << std::endl;
    batchFile << "javac \"" << filename << "\"" << std::endl;
    batchFile << "if %ERRORLEVEL% neq 0 (" << std::endl;
    batchFile << "  echo Compilation failed." << std::endl;
    batchFile << "  pause" << std::endl;
    batchFile << "  exit /b 1" << std::endl;
    batchFile << ")" << std::endl;
    batchFile << "echo." << std::endl;
    batchFile << "echo Compilation successful. Running program..." << std::endl;
    batchFile << "echo." << std::endl;
    batchFile << "java " << className << std::endl;
    batchFile << "echo." << std::endl;
    batchFile << "echo Program execution completed." << std::endl;
    batchFile << "pause" << std::endl;
    
    batchFile.close();
    
    clearScreen();
    
    system(batchFilename.c_str());
    
    remove(batchFilename.c_str());
    
    return true;
}

bool Editor::compileAndRunFile() {
    std::string ext = getFileExtension();
    
    setStatusMessage("File extension: '" + ext + "'");
    
    if (ext == "cpp" || ext == "cc" || ext == "cxx") {
        return compileAndRunCpp();
    } else if (ext == "java") {
        return compileAndRunJava();
    } else {
        setStatusMessage("Unsupported file type for compilation: " + ext);
        return false;
    }
}

bool Editor::saveFileAs(const std::string& newFilename) {
    std::string oldFilename = filename;
    filename = newFilename;
    
    bool result = saveFile();
    
    if (!result) {
        filename = oldFilename;
    }
    
    return result;
}

bool Editor::deleteFile() {
    if (filename.empty()) {
        setStatusMessage("No file to delete");
        return false;
    }
    
    int result = remove(filename.c_str());
    
    if (result != 0) {
        setStatusMessage("Failed to delete file");
        return false;
    }
    
    setStatusMessage("File deleted: " + filename);
    filename = "";
    buffer.clear();
    buffer.push_back("");
    cursorX = 0;
    cursorY = 0;
    return true;
}

bool Editor::renameFile(const std::string& newFilename) {
    if (filename.empty()) {
        setStatusMessage("No file to rename");
        return false;
    }
    
    int result = rename(filename.c_str(), newFilename.c_str());
    
    if (result != 0) {
        setStatusMessage("Failed to rename file");
        return false;
    }
    
    filename = newFilename;
    setStatusMessage("File renamed to: " + filename);
    return true;
}


void Editor::moveForwardWord() {
    if (cursorY >= buffer.size() || buffer[cursorY].empty()) {
        if (cursorY < buffer.size() - 1) {
            cursorY++;
            cursorX = 0;
        }
        return;
    }
    
    std::string& line = buffer[cursorY];
    size_t pos = cursorX;
    
    if (pos >= line.length()) {
        if (cursorY < buffer.size() - 1) {
            cursorY++;
            cursorX = 0;
        }
        return;
    }
    
    while (pos < line.length() && isalnum(line[pos])) pos++;
    
    while (pos < line.length() && !isalnum(line[pos])) pos++;
    
    if (pos >= line.length()) {
        if (cursorY < buffer.size() - 1) {
            cursorY++;
            cursorX = 0;
        } else {
            cursorX = line.length();
        }
    } else {
        cursorX = pos;
    }
}

void Editor::moveBackwardWord() {
    if (cursorY >= buffer.size()) return;
    
    std::string& line = buffer[cursorY];
    int pos = cursorX - 1;
    
    if (pos < 0) {
        if (cursorY > 0) {
            cursorY--;
            cursorX = buffer[cursorY].length();
        }
        return;
    }
    
    while (pos >= 0 && !isalnum(line[pos])) pos--;
    
    while (pos >= 0 && isalnum(line[pos])) pos--;
    
    cursorX = pos + 1;
}