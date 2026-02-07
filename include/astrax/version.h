#ifndef ASTRAX_VERSION_H
#define ASTRAX_VERSION_H

// ============================================================================
// AstraX Version Information
// ============================================================================

#define ASTRAX_VERSION_MAJOR 1
#define ASTRAX_VERSION_MINOR 0
#define ASTRAX_VERSION_PATCH 0

#define ASTRAX_VERSION_STRING "1.0.0"
#define ASTRAX_VERSION_FULL "AstraX v1.0.0"

#define ASTRAX_COPYRIGHT "Copyright (c) 2024 AstraX Team"
#define ASTRAX_LICENSE "MIT License"
#define ASTRAX_WEBSITE "https://github.com/YOUR_USERNAME/AstraX"

// Build information (set by CMake)
#ifndef ASTRAX_BUILD_TYPE
#define ASTRAX_BUILD_TYPE "Release"
#endif

#ifndef ASTRAX_BUILD_DATE
#define ASTRAX_BUILD_DATE __DATE__
#endif

#ifndef ASTRAX_BUILD_TIME
#define ASTRAX_BUILD_TIME __TIME__
#endif

// ============================================================================
// Version Comparison Macros
// ============================================================================

#define ASTRAX_VERSION_CODE(major, minor, patch) \
    ((major) * 10000 + (minor) * 100 + (patch))

#define ASTRAX_VERSION \
    ASTRAX_VERSION_CODE(ASTRAX_VERSION_MAJOR, ASTRAX_VERSION_MINOR, ASTRAX_VERSION_PATCH)

#define ASTRAX_VERSION_AT_LEAST(major, minor, patch) \
    (ASTRAX_VERSION >= ASTRAX_VERSION_CODE(major, minor, patch))

#endif // ASTRAX_VERSION_H
