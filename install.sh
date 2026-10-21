#!/usr/bin/env bash
# ==============================================================================
# Laxersaz Organ - Cross-Platform Installer (macOS & Linux)
# Supports clean updates/reinstalls without overwriting user-created soundsets.
# ==============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OS="$(uname -s)"

# Color output helpers
BOLD='\033[1m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BOLD}${BLUE}=== Laxersaz Organ Installer ===${NC}"
echo -e "Platform detected: ${BOLD}${OS}${NC}\n"

# ------------------------------------------------------------------------------
# Target Directories
# ------------------------------------------------------------------------------
if [ "$OS" = "Darwin" ]; then
    VST3_DEST="$HOME/Library/Audio/Plug-Ins/VST3"
    AU_DEST="$HOME/Library/Audio/Plug-Ins/Components"
    LV2_DEST="$HOME/Library/Audio/Plug-Ins/LV2"
    APP_DEST="/Applications"
    if [ ! -w "$APP_DEST" ]; then
        APP_DEST="$HOME/Applications"
    fi
    SOUNDSETS_DEST="$HOME/Music/LaxersazOrgan/Soundsets"
elif [ "$OS" = "Linux" ]; then
    VST3_DEST="$HOME/.vst3"
    LV2_DEST="$HOME/.lv2"
    BIN_DEST="$HOME/.local/bin"
    MUSIC_DIR="${XDG_MUSIC_DIR:-$HOME/Music}"
    SOUNDSETS_DEST="$MUSIC_DIR/LaxersazOrgan/Soundsets"
else
    echo -e "${RED}Error: Unsupported operating system: ${OS}${NC}"
    exit 1
fi

# ------------------------------------------------------------------------------
# Handle --uninstall flag
# ------------------------------------------------------------------------------
if [ "$1" = "--uninstall" ]; then
    echo -e "${YELLOW}Uninstalling Laxersaz Organ...${NC}"
    rm -rf "$VST3_DEST/Laxersaz Organ.vst3" 2>/dev/null || true
    echo -e "  - Removed VST3 from ${VST3_DEST}"

    if [ "$OS" = "Darwin" ]; then
        rm -rf "$AU_DEST/Laxersaz Organ.component" 2>/dev/null || true
        rm -rf "$LV2_DEST/Laxersaz Organ.lv2" 2>/dev/null || true
        rm -rf "$APP_DEST/Laxersaz Organ.app" 2>/dev/null || true
        echo -e "  - Removed AU from ${AU_DEST}"
        echo -e "  - Removed LV2 from ${LV2_DEST}"
        echo -e "  - Removed App from ${APP_DEST}"
        killall -9 AudioComponentRegistrar 2>/dev/null || true
    elif [ "$OS" = "Linux" ]; then
        rm -rf "$LV2_DEST/Laxersaz Organ.lv2" 2>/dev/null || true
        rm -f "$BIN_DEST/Laxersaz Organ" 2>/dev/null || true
        echo -e "  - Removed LV2 from ${LV2_DEST}"
        echo -e "  - Removed Standalone from ${BIN_DEST}"
    fi

    echo -e "\n${YELLOW}Note: Soundsets at '${SOUNDSETS_DEST}' were left intact to preserve any custom sounds.${NC}"
    echo -e "${GREEN}Uninstallation complete!${NC}"
    exit 0
fi

# ------------------------------------------------------------------------------
# Locate Artifacts (searches extracted release package or local build folder)
# ------------------------------------------------------------------------------
find_artifact() {
    local pattern="$1"
    # 1. Search in current script directory (release package layout)
    local found
    found=$(find "$SCRIPT_DIR" -maxdepth 2 -name "$pattern" 2>/dev/null | head -n 1)
    if [ -n "$found" ]; then
        echo "$found"
        return
    fi
    # 2. Search in CMake build directory (local build layout)
    found=$(find "$SCRIPT_DIR/build/ToyOrgan_artefacts" -name "$pattern" 2>/dev/null | head -n 1)
    if [ -n "$found" ]; then
        echo "$found"
        return
    fi
    echo ""
}

VST3_SRC=$(find_artifact "Laxersaz Organ.vst3")
AU_SRC=$(find_artifact "Laxersaz Organ.component")
LV2_SRC=$(find_artifact "Laxersaz Organ.lv2")

SOUNDSETS_SRC=""
if [ -d "$SCRIPT_DIR/Soundsets" ]; then
    SOUNDSETS_SRC="$SCRIPT_DIR/Soundsets"
elif [ -d "$SCRIPT_DIR/../Soundsets" ]; then
    SOUNDSETS_SRC="$SCRIPT_DIR/../Soundsets"
fi

# ------------------------------------------------------------------------------
# Install / Overwrite Plugins
# ------------------------------------------------------------------------------

# 1. VST3
if [ -n "$VST3_SRC" ] && [ -e "$VST3_SRC" ]; then
    mkdir -p "$VST3_DEST"
    rm -rf "$VST3_DEST/Laxersaz Organ.vst3"
    cp -R "$VST3_SRC" "$VST3_DEST/"
    if [ "$OS" = "Darwin" ]; then
        xattr -cr "$VST3_DEST/Laxersaz Organ.vst3" 2>/dev/null || true
    fi
    echo -e "${GREEN}✓ [VST3]${NC} Installed/Updated -> ${BOLD}$VST3_DEST/Laxersaz Organ.vst3${NC}"
else
    echo -e "${YELLOW}⚠ [VST3]${NC} Not found, skipping."
fi

# 2. Audio Unit (macOS only)
if [ "$OS" = "Darwin" ]; then
    if [ -n "$AU_SRC" ] && [ -e "$AU_SRC" ]; then
        mkdir -p "$AU_DEST"
        rm -rf "$AU_DEST/Laxersaz Organ.component"
        cp -R "$AU_SRC" "$AU_DEST/"
        xattr -cr "$AU_DEST/Laxersaz Organ.component" 2>/dev/null || true
        killall -9 AudioComponentRegistrar 2>/dev/null || true
        echo -e "${GREEN}✓ [AU]${NC}   Installed/Updated -> ${BOLD}$AU_DEST/Laxersaz Organ.component${NC}"
    else
        echo -e "${YELLOW}⚠ [AU]${NC}   Not found, skipping."
    fi
fi

# 3. LV2
if [ -n "$LV2_SRC" ] && [ -e "$LV2_SRC" ]; then
    mkdir -p "$LV2_DEST"
    rm -rf "$LV2_DEST/Laxersaz Organ.lv2"
    cp -R "$LV2_SRC" "$LV2_DEST/"
    if [ "$OS" = "Darwin" ]; then
        xattr -cr "$LV2_DEST/Laxersaz Organ.lv2" 2>/dev/null || true
    fi
    echo -e "${GREEN}✓ [LV2]${NC}  Installed/Updated -> ${BOLD}$LV2_DEST/Laxersaz Organ.lv2${NC}"
else
    echo -e "${YELLOW}⚠ [LV2]${NC}  Not found, skipping."
fi

# 4. Standalone Application
if [ "$OS" = "Darwin" ]; then
    APP_SRC=$(find_artifact "Laxersaz Organ.app")
    if [ -n "$APP_SRC" ] && [ -e "$APP_SRC" ]; then
        mkdir -p "$APP_DEST"
        rm -rf "$APP_DEST/Laxersaz Organ.app"
        cp -R "$APP_SRC" "$APP_DEST/"
        xattr -cr "$APP_DEST/Laxersaz Organ.app" 2>/dev/null || true
        echo -e "${GREEN}✓ [App]${NC}  Installed/Updated -> ${BOLD}$APP_DEST/Laxersaz Organ.app${NC}"
    fi
elif [ "$OS" = "Linux" ]; then
    BIN_SRC=$(find "$SCRIPT_DIR" -maxdepth 2 -type f -name "Laxersaz Organ" 2>/dev/null | head -n 1)
    if [ -z "$BIN_SRC" ]; then
        BIN_SRC=$(find "$SCRIPT_DIR/build/ToyOrgan_artefacts" -path "*/Standalone/*" -type f -executable 2>/dev/null | head -n 1)
    fi
    if [ -n "$BIN_SRC" ] && [ -e "$BIN_SRC" ]; then
        mkdir -p "$BIN_DEST"
        rm -f "$BIN_DEST/Laxersaz Organ"
        cp -f "$BIN_SRC" "$BIN_DEST/Laxersaz Organ"
        chmod +x "$BIN_DEST/Laxersaz Organ"
        echo -e "${GREEN}✓ [App]${NC}  Installed/Updated -> ${BOLD}$BIN_DEST/Laxersaz Organ${NC}"
    fi
fi

# ------------------------------------------------------------------------------
# Install / Update Factory Soundsets (Preserving User Custom Folders)
# ------------------------------------------------------------------------------
if [ -n "$SOUNDSETS_SRC" ] && [ -d "$SOUNDSETS_SRC" ]; then
    mkdir -p "$SOUNDSETS_DEST"
    # Copy factory soundset folders cleanly; overwrites factory files while leaving user custom soundsets untouched!
    for fset in "$SOUNDSETS_SRC"/*; do
        if [ -d "$fset" ]; then
            fsetName="$(basename "$fset")"
            destSetFolder="$SOUNDSETS_DEST/$fsetName"
            mkdir -p "$destSetFolder"
            cp -R "$fset/"* "$destSetFolder/"
        fi
    done
    echo -e "${GREEN}✓ [Soundsets]${NC} Updated factory sets in -> ${BOLD}$SOUNDSETS_DEST${NC}"
    echo -e "              ${BLUE}ℹ Note:${NC} Any custom soundset folders you created are preserved."
else
    echo -e "${YELLOW}⚠ [Soundsets]${NC} Factory soundsets folder not found, skipping."
fi

echo -e "\n${BOLD}${GREEN}Installation successful!${NC}"
echo -e "Restart your DAW to scan the new / updated Laxersaz Organ plugin."
