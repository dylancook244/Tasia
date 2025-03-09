import os
import platform
import subprocess
import shutil
import sys

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