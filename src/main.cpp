#include "../include/editor.h"
#include <iostream>

int main(int argc, char* argv[]) {
    Editor editor;
    
    if (argc > 1) {
        editor.run(argv[1]);
    } else {
        editor.run();
    }
    
    return 0;
}