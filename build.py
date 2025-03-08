import os
import platform
import sys
import subprocess
import ctypes

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
    
    packages = ["llvm", "make"]
    if system == "Linux":
        # Ubuntu needs special handling for newer LLVM versions
        if os.path.exists("/etc/lsb-release") and "Ubuntu" in open("/etc/lsb-release").read():
            # Install LLVM repository script and run it for version 19
            subprocess.run("wget https://apt.llvm.org/llvm.sh", shell=True, check=True)
            subprocess.run("chmod +x llvm.sh", shell=True, check=True)
            subprocess.run("sudo ./llvm.sh 19", shell=True, check=True)
            # No need to explicitly install LLVM packages as the script does that
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
    build()