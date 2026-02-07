#!/bin/bash
# ============================================================================
# AstraX Installer for Linux/macOS
# Version: 1.0.0
# ============================================================================

set -e

APPNAME="astrax"
VERSION="1.0.0"
PREFIX="${PREFIX:-/usr/local}"
INSTALL_DIR="${PREFIX}/bin"

echo ""
echo "========================================="
echo "  AstraX v${VERSION} Installer"
echo "========================================="
echo ""

# Detect OS
OS="$(uname -s)"
case "${OS}" in
    Linux*)     PLATFORM=linux;;
    Darwin*)    PLATFORM=macos;;
    *)          PLATFORM=unknown;;
esac

echo "Platform: ${PLATFORM}"
echo "Install Directory: ${INSTALL_DIR}"
echo ""

# Find source binary
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SOURCE_DIR="${SCRIPT_DIR}"

if [ ! -f "${SOURCE_DIR}/astrax" ]; then
    if [ -f "${SCRIPT_DIR}/../build/bin/astrax" ]; then
        SOURCE_DIR="${SCRIPT_DIR}/../build/bin"
    elif [ -f "${SCRIPT_DIR}/build/bin/astrax" ]; then
        SOURCE_DIR="${SCRIPT_DIR}/build/bin"
    else
        echo "ERROR: astrax binary not found."
        echo "Please build the project first."
        exit 1
    fi
fi

echo "Source: ${SOURCE_DIR}/astrax"
echo ""

# Check if root/sudo needed
if [ ! -w "${INSTALL_DIR}" ]; then
    if [ "$EUID" -ne 0 ]; then
        echo "Installing to ${INSTALL_DIR} requires root privileges."
        echo "Rerunning with sudo..."
        exec sudo "$0" "$@"
    fi
fi

# Install
echo "[1/3] Installing binary..."
cp "${SOURCE_DIR}/astrax" "${INSTALL_DIR}/"
chmod +x "${INSTALL_DIR}/astrax"

# Install config
echo "[2/3] Installing configuration..."
CONFIG_DIR="${PREFIX}/share/astrax"
mkdir -p "${CONFIG_DIR}"
if [ -d "${SCRIPT_DIR}/../config" ]; then
    cp -r "${SCRIPT_DIR}/../config/"* "${CONFIG_DIR}/"
elif [ -d "${SCRIPT_DIR}/config" ]; then
    cp -r "${SCRIPT_DIR}/config/"* "${CONFIG_DIR}/"
fi

# Install man page (if exists)
echo "[3/3] Setting up documentation..."
if [ -f "${SCRIPT_DIR}/../docs/astrax.1" ]; then
    mkdir -p "${PREFIX}/share/man/man1"
    cp "${SCRIPT_DIR}/../docs/astrax.1" "${PREFIX}/share/man/man1/"
fi

echo ""
echo "========================================="
echo "  Installation Complete!"
echo "========================================="
echo ""
echo "AstraX has been installed to: ${INSTALL_DIR}/astrax"
echo ""
echo "Usage:"
echo "  astrax              # Open new file"
echo "  astrax myfile.cpp   # Open existing file"
echo ""
echo "To uninstall:"
echo "  sudo rm ${INSTALL_DIR}/astrax"
echo "  sudo rm -rf ${CONFIG_DIR}"
echo ""
