# Deployment Guide

## Quick Start

### Local Installation (Windows)

```cmd
cd c:\Users\sanka\OneDrive\Desktop\AstraX
scripts\install.bat
```

### Create Release Package

```cmd
scripts\build-release.bat
```

---

## Deployment Options

### Option 1: GitHub Releases (Recommended)

1. **Push to GitHub**:
   ```bash
   git add .
   git commit -m "Release v1.0.0"
   git push origin main
   ```

2. **Create a Release Tag**:
   ```bash
   git tag v1.0.0
   git push --tags
   ```

3. **Automated Build**: GitHub Actions will:
   - Build for Windows, Linux, and macOS
   - Create release archives
   - Publish a GitHub Release with download links

---

### Option 2: Manual Release

1. **Build Release**:
   ```cmd
   scripts\build-release.bat
   ```

2. **Distribute**: Share `release/astrax-1.0.0-windows.zip`

---

### Option 3: System Installation

**Windows (Admin required)**:
```cmd
scripts\install.bat
```

Installs to `C:\Program Files\AstraX` and adds to PATH.

**Linux/macOS**:
```bash
chmod +x scripts/install.sh
sudo ./scripts/install.sh
```

Installs to `/usr/local/bin`.

---

## CI/CD Pipeline

### Continuous Integration (`ci.yml`)

Runs on every push/PR:
- Builds on Windows, Linux, macOS
- Runs unit tests
- Runs sanitizers (ASan, UBSan)

### Release Automation (`release.yml`)

Triggered by version tags (`v*`):
- Multi-platform release builds
- Creates archives (ZIP/tar.gz)
- Publishes GitHub Release

---

## Version Management

Update version in:
1. `include/astrax/version.h`
2. `CMakeLists.txt` (project version)
3. Create git tag: `git tag v1.x.x`

---

## Troubleshooting

### Build Errors

```cmd
:: Clean rebuild
rmdir /S /Q build
cmake -B build -G "MinGW Makefiles" -DASTRAX_ENABLE_WARNINGS=OFF
cmake --build build
```

### Permission Denied (Windows)

Run as Administrator or close any running astrax.exe instances.

### PATH Not Updated

Restart terminal or log out/in for PATH changes to take effect.
