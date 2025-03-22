# Tasia Installation Script for Windows
Write-Host "Installing Tasia Programming Language..." -ForegroundColor Blue

# Create a temporary directory for installation
$TempDir = [System.IO.Path]::Combine($env:TEMP, "TasiaInstall_" + [System.Guid]::NewGuid().ToString())
Write-Host "Using temporary directory: $TempDir"
New-Item -ItemType Directory -Path $TempDir -Force | Out-Null
Set-Location $TempDir

# Clone the repository
Write-Host "Cloning the Tasia repository..."
git clone https://github.com/dylancook244/tasia.git
Set-Location tasia

# Check for LLVM
if (-not (Test-Path "C:\Program Files\LLVM\bin\llvm-config.exe")) {
    Write-Host "LLVM not found. Installing LLVM..."
    # Download and install LLVM
    $LLVMInstallerUrl = "https://github.com/llvm/llvm-project/releases/download/llvmorg-16.0.0/LLVM-16.0.0-win64.exe"
    $LLVMInstallerPath = "$env:TEMP\LLVM-installer.exe"
    Invoke-WebRequest -Uri $LLVMInstallerUrl -OutFile $LLVMInstallerPath
    Start-Process -FilePath $LLVMInstallerPath -Args "/S" -Wait
    # Add LLVM to PATH
    $env:Path += ";C:\Program Files\LLVM\bin"
    [Environment]::SetEnvironmentVariable("Path", $env:Path, "User")
}

# Check for Clang
if (-not (Get-Command "clang.exe" -ErrorAction SilentlyContinue)) {
    Write-Host "Clang not found. It should be included with LLVM."
    Write-Host "Please ensure LLVM is properly installed." -ForegroundColor Yellow
}

# Navigate to the compiler directory
Write-Host "Navigating to compiler directory..."
Set-Location compiler

# Build and install Tasia
Write-Host "Building and installing Tasia..."
make install

# Clean up the repository
Write-Host "Cleaning up temporary files..."
Set-Location $env:TEMP
Remove-Item -Recurse -Force $TempDir

Write-Host "Tasia has been successfully installed!" -ForegroundColor Green
Write-Host "You can now use the 'tasia' command to compile Tasia programs."