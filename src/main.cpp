/**
 * @file main.cpp
 * @brief AstraX - A modern Vim-like terminal text editor
 * 
 * @author AstraX Team
 * @version 1.0.0
 */

#include "astrax/editor.h"
#include <iostream>
#include <string>
#include <cstdlib>

#ifdef ASTRAX_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace {

void printVersion() {
    std::cout << "AstraX v1.0.0 - A modern Vim-like terminal text editor\n";
    std::cout << "Copyright (c) 2024 AstraX Team\n";
}

void printHelp() {
    std::cout << "Usage: astrax [options] [file]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help       Show this help message\n";
    std::cout << "  -v, --version    Show version information\n";
    std::cout << "  -e, --external   Open in external terminal window\n";
    std::cout << "\nExamples:\n";
    std::cout << "  astrax                  Start with empty buffer\n";
    std::cout << "  astrax file.cpp         Open file.cpp\n";
    std::cout << "  astrax -e file.cpp      Open in external terminal\n";
}

bool isExternalWindow() {
#ifdef ASTRAX_PLATFORM_WINDOWS
    char buffer[2];
    return GetEnvironmentVariableA("ASTRAX_EXTERNAL", buffer, sizeof(buffer)) > 0;
#else
    return std::getenv("ASTRAX_EXTERNAL") != nullptr;
#endif
}

void launchExternalWindow(int argc, char* argv[]) {
#ifdef ASTRAX_PLATFORM_WINDOWS
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    
    std::string cmd = "start \"AstraX Editor\" cmd /k \"set ASTRAX_EXTERNAL=1 && \"";
    cmd += exePath;
    cmd += "\"";
    
    // Pass along file arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg != "-e" && arg != "--external") {
            cmd += " \"" + arg + "\"";
        }
    }
    cmd += "\"";
    
    system(cmd.c_str());
#else
    // Unix: Try to detect terminal emulator
    std::string terminal = "xterm";
    if (std::getenv("GNOME_TERMINAL_SERVICE")) {
        terminal = "gnome-terminal --";
    } else if (std::getenv("KONSOLE_VERSION")) {
        terminal = "konsole -e";
    }
    
    std::string cmd = terminal + " env ASTRAX_EXTERNAL=1 ";
    cmd += argv[0];
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg != "-e" && arg != "--external") {
            cmd += " \"" + arg + "\"";
        }
    }
    cmd += " &";
    
    system(cmd.c_str());
#endif
}

} // anonymous namespace

int main(int argc, char* argv[]) {
    std::string filename;
    bool external = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printHelp();
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            printVersion();
            return 0;
        } else if (arg == "-e" || arg == "--external") {
            external = true;
        } else if (!arg.empty() && arg[0] != '-') {
            filename = arg;
        }
    }
    
    // Launch in external window if requested and not already in one
    if (external && !isExternalWindow()) {
        launchExternalWindow(argc, argv);
        return 0;
    }
    
    // Run the editor
    try {
        astrax::Editor editor;
        editor.run(filename);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
