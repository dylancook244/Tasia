#!/bin/bash

# Tasia Installation Script for macOS and Linux
echo "Installing Tasia Programming Language..."

# Determine OS for download URL
if [[ "$OSTYPE" == "darwin"* ]]; then
  PLATFORM="macOS"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
  PLATFORM="Linux"
else
  echo "Unsupported operating system: $OSTYPE"
  exit 1
fi

echo "Detected $PLATFORM system"

# Create installation directory
INSTALL_DIR="/usr/local/bin"
if [[ ! -d "$INSTALL_DIR" ]]; then
  echo "Creating installation directory..."
  sudo mkdir -p "$INSTALL_DIR"
fi

# Download the appropriate Tasia executable
echo "Downloading Tasia executable..."
DOWNLOAD_URL="https://github.com/dylancook244/tasia/releases/latest/download/tasia-$PLATFORM"
curl -L "$DOWNLOAD_URL" -o tasia

# Make it executable
chmod +x tasia

# Move to installation directory
echo "Installing Tasia to $INSTALL_DIR..."
sudo mv tasia "$INSTALL_DIR/"

echo "Tasia installed successfully!"
echo "You can now use Tasia by running the 'tasia' command."
echo "Try 'tasia help' to get started."