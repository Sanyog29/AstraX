# AstraX

<div align="center">

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![C++](https://img.shields.io/badge/C++-17-orange)
![License](https://img.shields.io/badge/license-MIT-green)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)

**A modern, feature-rich Vim-like terminal text editor built with C++17**

</div>

---

## ✨ Features

- **Vim-Style Modal Editing** - Normal, Insert, Command, Visual, and Search modes
- **Syntax Highlighting** - Built-in support for C/C++, with extensible highlighter system
- **Undo/Redo** - Full edit history with unlimited undo support
- **Cross-Platform** - Native support for Windows, Linux, and macOS
- **Configurable** - JSON configuration with multiple themes (default, light, monokai)
- **Search & Replace** - Regex-powered search with case sensitivity options
- **Line Numbers** - Toggleable line number display
- **Status Bar** - Mode indicator, filename, position, and messages

---

## 🚀 Quick Start

### Building from Source

**Prerequisites:**
- CMake 3.20+
- C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)

```bash
# Clone the repository
git clone https://github.com/yourusername/astrax.git
cd astrax

# Create build directory
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Run
./build/bin/astrax [file]
```

### Command Line Options

```bash
astrax                    # Start with empty buffer
astrax file.cpp           # Open file.cpp
astrax -e file.cpp        # Open in external terminal window
astrax --help             # Show help
astrax --version          # Show version
```

---

## ⌨️ Key Bindings

### Normal Mode

| Key | Action |
|-----|--------|
| `h` `j` `k` `l` | Move left/down/up/right |
| `w` `b` | Move forward/backward by word |
| `0` `$` | Move to start/end of line |
| `g` `G` | Move to start/end of file |
| `i` `a` | Enter Insert mode (before/after cursor) |
| `o` `O` | Open new line below/above |
| `x` | Delete character |
| `dd` | Delete line |
| `yy` | Yank (copy) line |
| `p` `P` | Paste after/before cursor |
| `u` | Undo |
| `Ctrl+R` | Redo |
| `J` | Join lines |
| `:` | Enter Command mode |
| `/` | Enter Search mode |

### Command Mode

| Command | Action |
|---------|--------|
| `:w` | Save file |
| `:q` | Quit (fails if unsaved) |
| `:q!` | Force quit |
| `:wq` `:x` | Save and quit |
| `:e <file>` | Open file |
| `:new` | New buffer |
| `:saveas <file>` | Save as |
| `:set number` | Enable line numbers |
| `:help` | Show available commands |

---

## 🎨 Themes

AstraX includes three built-in themes:

- **default** - Dark theme with vibrant colors
- **light** - Light background theme
- **monokai** - Classic monokai-inspired colors

---

## 📁 Project Structure

```
AstraX/
├── CMakeLists.txt           # Main build configuration
├── include/astrax/          # Header files
│   ├── buffer.h             # Text buffer with undo/redo
│   ├── command.h            # Command pattern & key bindings
│   ├── config.h             # Configuration management
│   ├── editor.h             # Main editor class
│   ├── renderer.h           # Terminal rendering
│   ├── search.h             # Search & replace engine
│   ├── terminal.h           # Abstract terminal interface
│   ├── types.h              # Common types and enums
│   └── syntax/              # Syntax highlighting
├── src/                     # Implementation files
│   ├── terminal_windows.cpp # Windows Console API
│   ├── terminal_unix.cpp    # Unix termios/ANSI
│   └── ...
├── tests/                   # Google Test unit tests
└── config/                  # Configuration files
```

---

## 🧪 Testing

```bash
# Build with tests
cmake -B build -DASTRAX_BUILD_TESTS=ON
cmake --build build

# Run tests
cd build
ctest --output-on-failure
```

---

## 🔧 Configuration

Configuration file location:
- **Windows**: `%APPDATA%\AstraX\config.json`
- **Linux/macOS**: `~/.config/astrax/config.json`

Example configuration:
```json
{
  "showLineNumbers": true,
  "syntaxHighlighting": true,
  "tabSize": 4,
  "expandTabs": true,
  "theme": "default"
}
```

---

## 📄 License

MIT License - see [LICENSE](LICENSE) for details.

---

<div align="center">
Made with ❤️ by the AstraX Team
</div>
