cmake_minimum_required(VERSION 3.20)

project(AstraX
    VERSION 1.0.0
    DESCRIPTION "A modern Vim-like terminal text editor"
    LANGUAGES CXX
)

# ============================================================================
# Global Settings
# ============================================================================
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================================================
# Options
# ============================================================================
option(ASTRAX_BUILD_TESTS "Build unit tests" ON)
option(ASTRAX_ENABLE_SANITIZERS "Enable AddressSanitizer and UBSanitizer" OFF)
option(ASTRAX_ENABLE_WARNINGS "Enable strict compiler warnings" ON)

# ============================================================================
# Compiler Warnings
# ============================================================================
if(ASTRAX_ENABLE_WARNINGS)
    if(MSVC)
        add_compile_options(/W4 /WX /permissive-)
    else()
        add_compile_options(
            -Wall
            -Wextra
            -Wpedantic
            -Werror
            -Wshadow
            -Wconversion
            -Wsign-conversion
            -Wnull-dereference
            -Wdouble-promotion
            -Wformat=2
        )
    endif()
endif()

# ============================================================================
# Sanitizers (Debug builds only)
# ============================================================================
if(ASTRAX_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(NOT MSVC)
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

# ============================================================================
# Platform Detection
# ============================================================================
if(WIN32)
    set(ASTRAX_PLATFORM "WINDOWS")
    add_compile_definitions(ASTRAX_PLATFORM_WINDOWS)
elseif(UNIX AND NOT APPLE)
    set(ASTRAX_PLATFORM "LINUX")
    add_compile_definitions(ASTRAX_PLATFORM_LINUX)
elseif(APPLE)
    set(ASTRAX_PLATFORM "MACOS")
    add_compile_definitions(ASTRAX_PLATFORM_MACOS)
endif()

message(STATUS "Building AstraX ${PROJECT_VERSION} for ${ASTRAX_PLATFORM}")

# ============================================================================
# Source Files
# ============================================================================
set(ASTRAX_HEADERS
    include/astrax/types.h
    include/astrax/terminal.h
    include/astrax/buffer.h
    include/astrax/command.h
    include/astrax/renderer.h
    include/astrax/search.h
    include/astrax/config.h
    include/astrax/editor.h
    include/astrax/syntax/highlighter.h
    include/astrax/syntax/cpp_highlighter.h
)

set(ASTRAX_SOURCES
    src/buffer.cpp
    src/command.cpp
    src/renderer.cpp
    src/search.cpp
    src/config.cpp  
    src/editor.cpp
    src/syntax/highlighter.cpp
    src/syntax/cpp_highlighter.cpp
)

# Platform-specific terminal implementation
if(WIN32)
    list(APPEND ASTRAX_SOURCES src/terminal_windows.cpp)
else()
    list(APPEND ASTRAX_SOURCES src/terminal_unix.cpp)
endif()

# ============================================================================
# Library Target
# ============================================================================
add_library(astrax_core STATIC ${ASTRAX_SOURCES} ${ASTRAX_HEADERS})

target_include_directories(astrax_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Platform-specific libraries
if(WIN32)
    # Windows Console API is part of kernel32, linked automatically
else()
    # Unix needs ncurses or termios (we use raw termios)
    find_package(Threads REQUIRED)
    target_link_libraries(astrax_core PRIVATE Threads::Threads)
endif()

# ============================================================================
# Executable Target
# ============================================================================
add_executable(astrax src/main.cpp)
target_link_libraries(astrax PRIVATE astrax_core)

# ============================================================================
# Testing
# ============================================================================
if(ASTRAX_BUILD_TESTS)
    enable_testing()
    
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # Prevent overriding parent project's compiler/linker settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    add_subdirectory(tests)
endif()

# ============================================================================
# Installation
# ============================================================================
include(GNUInstallDirs)

install(TARGETS astrax
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/astrax
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES config/default.json
    DESTINATION ${CMAKE_INSTALL_DATADIR}/astrax
)

# ============================================================================
# CPack Configuration
# ============================================================================
set(CPACK_PACKAGE_NAME "AstraX")
set(CPACK_PACKAGE_VENDOR "AstraX Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Modern Vim-like terminal text editor")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
endif()

include(CPack)
