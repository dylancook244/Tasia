#!/bin/bash

# Detect OS
OS="$(uname -s)"
ARCH="$(uname -m)"

# Set download URL based on platform
if [[ "$OS" == "Darwin" ]]; then
  PLATFORM="macOS"
elif [[ "$OS" == "Linux" ]]; then
  PLATFORM="Linux"
else
  echo "Unsupported operating system: $OS"
  exit 1
fi

echo "Installing Tasia for $PLATFORM ($ARCH)..."

# Create installation directory
INSTALL_DIR="/usr/local/bin"
if [[ ! -d "$INSTALL_DIR" ]]; then
  echo "Creating installation directory..."
  sudo mkdir -p "$INSTALL_DIR"
fi

# Download the appropriate binary
DOWNLOAD_URL="https://github.com/dylancook244/tasia/releases/latest/download/tasia-$PLATFORM"
curl -L "$DOWNLOAD_URL" -o tasia

# Make it executable
chmod +x tasia

# Move to installation directory
echo "Installing Tasia to $INSTALL_DIR..."
sudo mv tasia "$INSTALL_DIR/"

# Print success message and instructions
echo "Tasia installed successfully!"
echo "You can now use Tasia by running the 'tasia' command."
echo "Try 'tasia help' to get started."