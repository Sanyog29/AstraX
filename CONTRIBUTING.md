# Contributing to AstraX

Thank you for your interest in contributing to AstraX! This document provides guidelines and instructions for contributing.

## Code of Conduct

Please be respectful and constructive in all interactions. We're building a welcoming community.

## Getting Started

### Prerequisites

- **Windows**: MinGW-w64 or MSVC, CMake 3.15+
- **Linux**: GCC 7+, CMake 3.15+
- **macOS**: Clang (Xcode Command Line Tools), CMake 3.15+

### Building from Source

```bash
# Clone the repository
git clone https://github.com/YOUR_USERNAME/AstraX.git
cd AstraX

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DASTRAX_BUILD_TESTS=ON
cmake --build build

# Run tests
cd build && ctest --output-on-failure
```

## Development Workflow

### Branch Naming

- `feature/` - New features (e.g., `feature/multiple-buffers`)
- `fix/` - Bug fixes (e.g., `fix/cursor-position`)
- `docs/` - Documentation updates
- `refactor/` - Code refactoring

### Commit Messages

Follow conventional commits:
```
type(scope): description

- feat: New feature
- fix: Bug fix
- docs: Documentation
- refactor: Code refactoring
- test: Adding tests
- chore: Maintenance
```

Example:
```
feat(editor): add multiple buffer support

- Implement buffer switching with :bn and :bp
- Add tab display in status bar
- Update tests for buffer management
```

### Pull Request Process

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run tests: `ctest --output-on-failure`
5. Push to your fork
6. Open a Pull Request

### Code Style

- Use 4 spaces for indentation
- Follow existing naming conventions:
  - Classes: `PascalCase`
  - Functions/Methods: `camelCase`
  - Variables: `camelCase_` (member variables with trailing underscore)
  - Constants: `UPPER_SNAKE_CASE`
- Add header comments for new files
- Use `const` wherever applicable

## Architecture

```
AstraX/
├── include/astrax/     # Public headers
│   ├── editor.h        # Main editor class
│   ├── buffer.h        # Text buffer with gap buffer
│   ├── renderer.h      # Terminal rendering
│   ├── terminal.h      # Platform abstraction
│   └── ...
├── src/                # Implementation
│   ├── editor.cpp
│   ├── buffer.cpp
│   ├── terminal_windows.cpp
│   ├── terminal_unix.cpp
│   └── ...
└── tests/              # Unit tests
```

## Testing

### Running Tests

```bash
cd build
ctest --output-on-failure
```

### Adding Tests

Tests use Google Test. Add new tests to `tests/` directory:

```cpp
#include <gtest/gtest.h>
#include "astrax/buffer.h"

TEST(BufferTest, InsertChar) {
    astrax::Buffer buffer;
    buffer.insertChar('a');
    EXPECT_EQ(buffer.getLine(0), "a");
}
```

## Reporting Issues

When reporting bugs, include:
- OS and version
- Steps to reproduce
- Expected vs actual behavior
- Screenshots if applicable

## License

By contributing, you agree that your contributions will be licensed under the MIT License.
