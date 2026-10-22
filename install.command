#!/usr/bin/env bash
# ==============================================================================
# Laxersaz Organ - Double-Clickable macOS Installer
# ==============================================================================

# Ensure working directory is the folder where this script lives
cd "$(dirname "$0")"

# Execute the main install script
if [ -f "./install.sh" ]; then
    bash "./install.sh" "$@"
else
    echo "Error: install.sh was not found in $(pwd)"
    exit 1
fi

echo ""
echo "Installation complete. You can close this Terminal window."
read -p "Press [Enter] to exit..."
