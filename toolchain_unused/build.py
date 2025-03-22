import os
import platform
import subprocess
import shutil
import sys

# After building with PyInstaller
def install_to_bin():
    """Install the executable to a bin directory in PATH"""
    # Get the path to the generated executable
    executable_path = os.path.join(os.getcwd(), 'dist', 'tasia')
    print(f"Looking for executable at: {executable_path}")
    
    if not os.path.exists(executable_path):
        print(f"ERROR: Executable not found at {executable_path}")
        return False
    else:
        print(f"Found executable at {executable_path}")
    
    # Choose installation directory
    if sys.platform.startswith('win'):
        # Windows - prefer user's Python scripts directory
        script_dir = os.path.join(sys.prefix, 'Scripts')
        bin_dir = script_dir if os.path.exists(script_dir) else os.path.join(os.environ.get('USERPROFILE', ''), 'bin')
    else:
        # Unix-like - use /usr/local/bin if possible
        bin_dir = '/usr/local/bin'
        print(f"Checking if we have write access to {bin_dir}: {os.access(bin_dir, os.W_OK)}")
        # Fallback to user's local bin if we don't have permissions
        if not os.access(bin_dir, os.W_OK):
            bin_dir = os.path.expanduser('~/.local/bin')
            print(f"Using fallback directory: {bin_dir}")
            os.makedirs(bin_dir, exist_ok=True)
    
    print(f"Selected bin directory: {bin_dir}")
    
    # Make sure the directory exists
    os.makedirs(bin_dir, exist_ok=True)
    
    # Determine the destination path
    dest_path = os.path.join(bin_dir, 'tasia')
    print(f"Destination path: {dest_path}")
    
    # Check if an existing executable exists and remove it
    if os.path.exists(dest_path):
        print(f"Found existing tasia executable at {dest_path}")
        if bin_dir == '/usr/local/bin' and not os.access(bin_dir, os.W_OK):
            print("Using sudo to remove existing executable")
            try:
                result = subprocess.run(['sudo', 'rm', '-f', dest_path], check=True, capture_output=True, text=True)
                print(f"sudo rm result: {result.returncode}")
                if result.stderr:
                    print(f"sudo rm stderr: {result.stderr}")
            except Exception as e:
                print(f"Error removing existing executable: {e}")
        else:
            print("Removing existing executable directly")
            try:
                os.remove(dest_path)
            except Exception as e:
                print(f"Error removing existing executable: {e}")
    
    # Copy the executable to the bin directory
    print(f"Copying {executable_path} to {dest_path}")
    
    # On Unix systems, need to use sudo if writing to system directories
    if bin_dir == '/usr/local/bin' and not os.access(bin_dir, os.W_OK):
        print("Using sudo to copy executable")
        try:
            cp_result = subprocess.run(['sudo', 'cp', executable_path, dest_path], check=True, capture_output=True, text=True)
            print(f"sudo cp result: {cp_result.returncode}")
            if cp_result.stderr:
                print(f"sudo cp stderr: {cp_result.stderr}")
                
            chmod_result = subprocess.run(['sudo', 'chmod', '+x', dest_path], check=True, capture_output=True, text=True)
            print(f"sudo chmod result: {chmod_result.returncode}")
            if chmod_result.stderr:
                print(f"sudo chmod stderr: {chmod_result.stderr}")
        except Exception as e:
            print(f"Error copying executable: {e}")
    else:
        print("Copying executable directly")
        try:
            shutil.copy2(executable_path, dest_path)
            if not sys.platform.startswith('win'):
                os.chmod(dest_path, 0o755)  # Make executable
        except Exception as e:
            print(f"Error copying executable: {e}")
    
    # Verify installation
    if os.path.exists(dest_path):
        print(f"Verified: tasia executable is now at {dest_path}")
        print("You can now run 'tasia' from any directory")
        return True
    else:
        print(f"ERROR: tasia executable was not installed to {dest_path}")
        return False

def clean_output_directories():
    """Clean up build and dist directories if they exist"""
    print("Cleaning output directories...")
    for dir_name in ['build', 'dist']:
        if os.path.exists(dir_name):
            shutil.rmtree(dir_name)

def build_executable():
    """Build the executable for the current platform"""
    system = platform.system()
    print(f"Building Tasia executable for {system}...")
    
    # Find the main script
    script_path = "tasia.py"
    if not os.path.exists(script_path):
        print(f"Error: Could not find tasia.py in the current directory")
        return False
    
    # Call PyInstaller directly
    pyinstaller_cmd = [
        "pyinstaller",
        "--onefile",
        "--name=tasia",
        "--clean",
        script_path
    ]
    
    print(f"Running PyInstaller with command: {' '.join(pyinstaller_cmd)}")
    
    # Run PyInstaller
    try:
        subprocess.check_call(pyinstaller_cmd)
    except subprocess.CalledProcessError as e:
        print(f"Error: PyInstaller failed to build executable: {e}")
        return False
    
    # Get the resulting executable name
    executable = "tasia.exe" if system == "Windows" else "tasia"
    executable_path = os.path.join("dist", executable)
    
    if not os.path.exists(executable_path):
        print(f"Error: Could not find executable at {executable_path}")
        return False
    
    print(f"Successfully built executable: {executable_path}")
    
    # Create a platform-specific release package
    release_dir = f"release-{system.lower()}"
    os.makedirs(release_dir, exist_ok=True)
    
    # Copy the executable 
    shutil.copy2(executable_path, os.path.join(release_dir, executable))
    
    # Look for README.md
    if os.path.exists("README.md"):
        shutil.copy2("README.md", release_dir)
    else:
        print("Warning: README.md not found, skipping this file in the release package")
    
    print(f"Release package created in: {release_dir}")
    install_to_bin()
    return True

def main():
    """Main build function that orchestrates the process"""
    print("Starting Tasia build process...")
    
    # Clean output directories
    clean_output_directories()
    
    # Build the executable
    success = build_executable()
    
    if success:
        print("Tasia build completed successfully!")
    else:
        print("Tasia build failed.")
        sys.exit(1)

if __name__ == "__main__":
    main()