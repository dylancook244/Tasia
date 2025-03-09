import os
import platform
import sys
import subprocess
import ctypes
import urllib.request
import shutil

from setuptools import setup, find_packages

from toolchain import cli_commands

def get_package_manager():
    system = platform.system()
    
    if system == "Darwin":  # macOS
        result = subprocess.run(["which", "brew"], capture_output=True, text=True)
        
        if result.returncode != 0:
            print("Installing Homebrew...")
            install_brew = '/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"'
            subprocess.run(install_brew, shell=True)
            
        return "brew", "brew install {}"
            
    elif system == "Windows":
        if not ctypes.windll.shell32.IsUserAnAdmin():
            print("Error: Admin privileges required")
            sys.exit(1)
            
        result = subprocess.run(["where", "choco"], capture_output=True, text=True)
        
        if result.returncode != 0:
            print("Installing Chocolatey...")
            install_choco = '''Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))'''
            subprocess.run(install_choco, shell=True)
            
        return "choco", "choco install {} -y"
    
    elif system == "Linux":
        package_managers = [
            ("apt-get", "sudo apt-get install -y {}"),
            ("apt", "sudo apt install -y {}"),
            ("dnf", "sudo dnf install -y {}"),
            ("yum", "sudo yum install -y {}"),
            ("pacman", "sudo pacman -S --noconfirm {}"),
            ("zypper", "sudo zypper install -y {}")
        ]
                
        for package_manager, command_template in package_managers:
            result = subprocess.run(["which", package_manager], capture_output=True, text=True)
            if result.returncode == 0:
                return package_manager, command_template
                    
    print("Error: No supported package manager found")
    sys.exit(1)

def install_dependencies():
    print("Installing dependencies...")
    
    system = platform.system()
    
    if system not in ["Darwin", "Linux", "Windows"]:
        print(f"OS type not supported: {system}")
        sys.exit(1)
        
    package_manager, command_template = get_package_manager()
    
    if system == "Windows":
        # Check admin privileges
        if not ctypes.windll.shell32.IsUserAnAdmin():
            print("Error: Administrator privileges required for Windows installation")
            sys.exit(1)
        
        # Verify LLVM installation
        try:
            # We need to refresh the PATH environment variable
            os.environ["PATH"] = os.environ["PATH"] + ";C:\\Program Files\\LLVM\\bin"
            
            # Check if clang++ is now available
            result = subprocess.run(["clang++", "--version"], capture_output=True, text=True)
            if result.returncode == 0:
                print("LLVM successfully installed and configured!")
            else:
                print("Warning: LLVM installation may not be correctly configured.")
        except:
            print("Warning: Could not verify LLVM installation.")
            
        # Install make if not already installed
        try:
            subprocess.run(["where", "make.exe"], capture_output=True, check=True)
            print("Make is already installed.")
        except:
            print("Installing Make...")
            try:
                subprocess.run("choco install make -y", shell=True, check=True)
            except:
                print("Failed to install Make. Please install manually.")
                sys.exit(1)
    
    packages = ["llvm", "make"]
    if system == "Linux":
        # Ubuntu needs special handling for newer LLVM versions
        # For Ubuntu, install LLVM 19 specifically
        if os.path.exists("/etc/lsb-release") and "Ubuntu" in open("/etc/lsb-release").read():
            # First, completely remove any existing LLVM installations
            subprocess.run("sudo apt-get remove --purge -y llvm* clang* libllvm*", shell=True)
            subprocess.run("sudo apt-get autoremove -y", shell=True)
            
            # Install LLVM 19 from the official repo
            subprocess.run("wget https://apt.llvm.org/llvm.sh", shell=True, check=True)
            subprocess.run("chmod +x llvm.sh", shell=True, check=True)
            subprocess.run("sudo ./llvm.sh 19", shell=True, check=True)
            
            # Create explicit symlinks to version 19 tools
            subprocess.run("sudo ln -sf /usr/bin/clang-19 /usr/bin/clang", shell=True)
            subprocess.run("sudo ln -sf /usr/bin/clang++-19 /usr/bin/clang++", shell=True)
            subprocess.run("sudo ln -sf /usr/bin/llvm-config-19 /usr/bin/llvm-config", shell=True)
            
            # Double-check the versions are correct
            print("Verifying LLVM installation:")
            subprocess.run("clang++ --version", shell=True)
            subprocess.run("llvm-config --version", shell=True)
            
            # Only install make since LLVM is handled
            packages = ["make"]
        # Fedora/RHEL/CentOS
        elif os.path.exists("/etc/fedora-release") or os.path.exists("/etc/redhat-release"):
            packages.extend(["llvm-devel", "clang-devel"])
        # Arch Linux
        elif os.path.exists("/etc/arch-release"):
            packages.extend(["clang"])
        # Debian (but not Ubuntu, which was handled above)
        elif os.path.exists("/etc/debian_version") and not os.path.exists("/etc/lsb-release"):
            packages.extend(["llvm-dev", "clang"])
        # openSUSE
        elif os.path.exists("/etc/SuSE-release") or os.path.exists("/etc/opensuse-release"):
            packages.extend(["llvm-devel", "clang-devel"])
        # Gentoo
        elif os.path.exists("/etc/gentoo-release"):
            packages = ["sys-devel/llvm", "sys-devel/make", "sys-devel/clang"]
        # Default fallback
        else:
            packages.extend(["llvm-dev", "clang"])
    
    for package in packages:
        print(f"Installing {package}...")
        cmd = command_template.format(package)
        result = subprocess.run(cmd, shell=True)
        if result.returncode != 0:
            print(f"Failed to install {package}")
            sys.exit(1)

def build_tasia():
    print("Building Tasia...")
    
    if os.path.exists("compiler"):
        os.chdir("compiler")
    
    result = subprocess.run("make install", shell=True)
    
    if result.returncode != 0:
        print("Tasia installation failed")
        sys.exit(1)
    
    print("Tasia installed successfully!")

def build():
    print("Starting Tasia installation...")
    install_dependencies()
    build_tasia()


if __name__ == "__main__":
    build_toolchain()