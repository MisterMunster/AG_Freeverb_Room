#!/usr/bin/env bash
#
# install.sh - Antigrav Plate Reverb Installer
#
# Builds and installs the VST3 plugin to the system plugin directory
# and places a standalone copy on the Desktop for testing.
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_DIR/build"

PLUGIN_NAME="Antigrav Plate Reverb"

# ─── Colors ───────────────────────────────────────────────────────────────────
BOLD='\033[1m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
CYAN='\033[0;36m'
RED='\033[0;31m'
RESET='\033[0m'

info()  { echo -e "${CYAN}[INFO]${RESET}  $1"; }
ok()    { echo -e "${GREEN}[  OK]${RESET}  $1"; }
warn()  { echo -e "${YELLOW}[WARN]${RESET}  $1"; }
fail()  { echo -e "${RED}[FAIL]${RESET}  $1"; exit 1; }

echo ""
echo -e "${BOLD}╔══════════════════════════════════════════════╗${RESET}"
echo -e "${BOLD}║   Antigrav Plate Reverb  —  Installer        ║${RESET}"
echo -e "${BOLD}╚══════════════════════════════════════════════╝${RESET}"
echo ""

# ─── Detect OS ────────────────────────────────────────────────────────────────
OS="$(uname -s)"
case "$OS" in
    Linux*)     PLATFORM="linux"  ;;
    Darwin*)    PLATFORM="macos"  ;;
    MINGW*|MSYS*|CYGWIN*) PLATFORM="windows" ;;
    *)          fail "Unsupported OS: $OS" ;;
esac
info "Detected platform: ${BOLD}$PLATFORM${RESET}"

# ─── Set platform-specific paths ─────────────────────────────────────────────
case "$PLATFORM" in
    linux)
        VST3_DIR="$HOME/.vst3"
        DESKTOP_DIR="$HOME/Desktop"
        STANDALONE_EXT=""
        ;;
    macos)
        VST3_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
        DESKTOP_DIR="$HOME/Desktop"
        STANDALONE_EXT=".app"
        ;;
    windows)
        VST3_DIR="/c/Program Files/Common Files/VST3"
        DESKTOP_DIR="$HOME/Desktop"
        STANDALONE_EXT=".exe"
        ANTIGRAV_DIR="/c/Users/rvanover/Documents/GitHub/Antigrav"
        ;;
esac

# ─── Step 0: Clean stale CMake caches ────────────────────────────────────────
# Remove any root-level CMakeCache.txt left by other projects
if [ -f "$PROJECT_DIR/CMakeCache.txt" ]; then
    warn "Removing stale CMakeCache.txt from project root..."
    rm -f "$PROJECT_DIR/CMakeCache.txt"
fi

# If the build dir cache points to a different project, nuke it
if [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
    if ! grep -q "AntigravReverb\|AntigravPlateReverb\|AG_Freeverb" "$BUILD_DIR/CMakeCache.txt" 2>/dev/null; then
        warn "Build directory contains cache from a different project. Cleaning..."
        rm -rf "$BUILD_DIR"
    fi
fi

# ─── Step 1: Build ───────────────────────────────────────────────────────────
ARTIFACTS_DIR="$BUILD_DIR/AntigravPlateReverb_artefacts"

if [ ! -d "$ARTIFACTS_DIR/VST3" ]; then
    info "No build found. Building project..."
    cmake -B "$BUILD_DIR" "$PROJECT_DIR" 2>&1 | tail -5
    cmake --build "$BUILD_DIR" --config Release 2>&1 | tail -5
    echo ""
fi

# Check for build artifacts (Release or Debug)
VST3_BUNDLE=""
STANDALONE_BIN=""

for cfg in "" "Release" "Debug"; do
    candidate="$ARTIFACTS_DIR"
    if [ -n "$cfg" ]; then
        candidate="$ARTIFACTS_DIR/$cfg"
    fi
    if [ -d "$candidate/VST3/${PLUGIN_NAME}.vst3" ]; then
        VST3_BUNDLE="$candidate/VST3/${PLUGIN_NAME}.vst3"
        STANDALONE_BIN="$candidate/Standalone/${PLUGIN_NAME}${STANDALONE_EXT}"
        break
    fi
done

if [ -z "$VST3_BUNDLE" ] || [ ! -d "$VST3_BUNDLE" ]; then
    fail "VST3 bundle not found. Run 'cmake --build build' first."
fi

ok "Found VST3 bundle: $VST3_BUNDLE"

if [ -f "$STANDALONE_BIN" ] || [ -d "$STANDALONE_BIN" ]; then
    ok "Found Standalone:   $STANDALONE_BIN"
else
    warn "Standalone binary not found at: $STANDALONE_BIN"
    STANDALONE_BIN=""
fi

# ─── Step 2: Install VST3 ───────────────────────────────────────────────────
echo ""
info "Installing VST3 to: ${BOLD}$VST3_DIR${RESET}"

mkdir -p "$VST3_DIR"
rm -rf "$VST3_DIR/${PLUGIN_NAME}.vst3"
cp -r "$VST3_BUNDLE" "$VST3_DIR/"

ok "VST3 installed to $VST3_DIR/${PLUGIN_NAME}.vst3"

# ─── Step 3: Copy to Desktop ─────────────────────────────────────────────────
echo ""
mkdir -p "$DESKTOP_DIR"

# Copy standalone to Desktop
if [ -n "$STANDALONE_BIN" ]; then
    info "Copying Standalone to Desktop..."
    if [ -d "$STANDALONE_BIN" ]; then
        # macOS .app bundle
        rm -rf "$DESKTOP_DIR/${PLUGIN_NAME}${STANDALONE_EXT}"
        cp -r "$STANDALONE_BIN" "$DESKTOP_DIR/"
    else
        cp "$STANDALONE_BIN" "$DESKTOP_DIR/${PLUGIN_NAME}${STANDALONE_EXT}"
        chmod +x "$DESKTOP_DIR/${PLUGIN_NAME}${STANDALONE_EXT}"
    fi
    ok "Standalone copied to $DESKTOP_DIR/${PLUGIN_NAME}${STANDALONE_EXT}"
fi

# Also copy VST3 bundle to Desktop for easy sharing/testing
info "Copying VST3 bundle to Desktop..."
rm -rf "$DESKTOP_DIR/${PLUGIN_NAME}.vst3"
cp -r "$VST3_BUNDLE" "$DESKTOP_DIR/"
ok "VST3 copied to $DESKTOP_DIR/${PLUGIN_NAME}.vst3"

# ─── Step 4: Copy to Antigrav project folder (Windows) ────────────────────────
if [ -n "${ANTIGRAV_DIR:-}" ]; then
    if [ -d "$ANTIGRAV_DIR" ] || mkdir -p "$ANTIGRAV_DIR" 2>/dev/null; then
        info "Copying to Antigrav project: ${BOLD}$ANTIGRAV_DIR${RESET}"

        rm -rf "$ANTIGRAV_DIR/${PLUGIN_NAME}.vst3"
        cp -r "$VST3_BUNDLE" "$ANTIGRAV_DIR/"
        ok "VST3 copied to $ANTIGRAV_DIR/${PLUGIN_NAME}.vst3"

        if [ -n "$STANDALONE_BIN" ]; then
            cp "$STANDALONE_BIN" "$ANTIGRAV_DIR/${PLUGIN_NAME}${STANDALONE_EXT}"
            ok "Standalone copied to $ANTIGRAV_DIR/${PLUGIN_NAME}${STANDALONE_EXT}"
        fi
    else
        warn "Could not access $ANTIGRAV_DIR — skipping."
    fi
fi

# ─── Summary ─────────────────────────────────────────────────────────────────
echo ""
echo -e "${BOLD}╔══════════════════════════════════════════════╗${RESET}"
echo -e "${BOLD}║   Installation Complete                      ║${RESET}"
echo -e "${BOLD}╚══════════════════════════════════════════════╝${RESET}"
echo ""
echo -e "  VST3 Plugin:  ${GREEN}$VST3_DIR/${PLUGIN_NAME}.vst3${RESET}"
if [ -n "$STANDALONE_BIN" ]; then
echo -e "  Standalone:   ${GREEN}$DESKTOP_DIR/${PLUGIN_NAME}${STANDALONE_EXT}${RESET}"
fi
echo -e "  VST3 (copy):  ${GREEN}$DESKTOP_DIR/${PLUGIN_NAME}.vst3${RESET}"
if [ -n "${ANTIGRAV_DIR:-}" ] && [ -d "$ANTIGRAV_DIR" ]; then
echo -e "  Project:      ${GREEN}$ANTIGRAV_DIR/${PLUGIN_NAME}.vst3${RESET}"
fi
echo ""

case "$PLATFORM" in
    linux)
        echo -e "  ${YELLOW}Tip:${RESET} Open your DAW and scan for plugins in ~/.vst3"
        echo -e "  ${YELLOW}Tip:${RESET} Run the standalone: ${CYAN}\"$DESKTOP_DIR/${PLUGIN_NAME}\"${RESET}"
        ;;
    macos)
        echo -e "  ${YELLOW}Tip:${RESET} Open your DAW — the VST3 should appear in the plugin list."
        echo -e "  ${YELLOW}Tip:${RESET} Double-click the standalone on your Desktop to test."
        ;;
    windows)
        echo -e "  ${YELLOW}Tip:${RESET} Open your DAW and rescan VST3 plugins."
        echo -e "  ${YELLOW}Tip:${RESET} Double-click the standalone on your Desktop to test."
        ;;
esac
echo ""
