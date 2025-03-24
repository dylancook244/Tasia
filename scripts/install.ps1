# Tasia Installation Script for Windows
Write-Host "Installing Tasia for Windows..." -ForegroundColor Green

# Create a temporary directory for installation
$TEMP_DIR = New-Item -ItemType Directory -Path "$env:TEMP\tasia_install_$(Get-Random)" -Force
Write-Host "Using temporary directory: $TEMP_DIR" -ForegroundColor Cyan
Set-Location -Path $TEMP_DIR

# Check for git
if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Host "Git is not installed. Please install git and try again." -ForegroundColor Red
    exit 1
}

# Check for Visual Studio with C++ tools
$VS_PATH = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath -ErrorAction SilentlyContinue
if (-not $VS_PATH) {
    Write-Host "Visual Studio with C++ tools is not installed. Please install it and try again." -ForegroundColor Red
    exit 1
}

# Clone the repository
Write-Host "Cloning the Tasia repository..." -ForegroundColor Cyan
git clone https://github.com/dylancook244/tasia.git
Set-Location -Path "$TEMP_DIR\tasia\compiler"

# Install LLVM using Chocolatey if not present
if (-not (Get-Command llvm-config -ErrorAction SilentlyContinue)) {
    Write-Host "LLVM not found. Installing LLVM..." -ForegroundColor Cyan
    
    # Check for Chocolatey
    if (-not (Get-Command choco -ErrorAction SilentlyContinue)) {
        Write-Host "Chocolatey not found. Installing Chocolatey..." -ForegroundColor Yellow
        Set-ExecutionPolicy Bypass -Scope Process -Force
        [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
        Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
    }
    
    # Install LLVM
    choco install llvm -y
}

# Get LLVM path
$LLVM_PATH = (Get-Command llvm-config -ErrorAction SilentlyContinue).Source
if ($LLVM_PATH) {
    $LLVM_PATH = Split-Path -Parent $LLVM_PATH
    $LLVM_PATH = Split-Path -Parent $LLVM_PATH
} else {
    $LLVM_PATH = "C:\Program Files\LLVM"
    if (-not (Test-Path $LLVM_PATH)) {
        Write-Host "LLVM installation path not found. Please install LLVM and try again." -ForegroundColor Red
        exit 1
    }
}

Write-Host "LLVM is installed at: $LLVM_PATH" -ForegroundColor Green

# Define source files
$SOURCES = @(
    ".\parser\parser.c",
    ".\codegen\codegen.c", 
    ".\ast\ast.c", 
    ".\scanner\scanner.c", 
    ".\scanner\token.c", 
    ".\compile\compile.c", 
    ".\compile\cli.c", 
    ".\symbol_table\symbol_table.c", 
    "main.c"
)

# Find Visual Studio Developer Command Prompt
$VCVARS_PATH = Get-ChildItem -Path "$VS_PATH\VC\Auxiliary\Build\vcvars64.bat" -ErrorAction SilentlyContinue
if (-not $VCVARS_PATH) {
    Write-Host "Visual Studio Developer Command Prompt not found." -ForegroundColor Red
    exit 1
}

# Compile Tasia using a temporary batch file to ensure Visual Studio environment is loaded
$BUILD_SCRIPT = @"
@echo off
call "$($VCVARS_PATH.FullName)"
clang -std=c11 -g -O2 -I"$LLVM_PATH\include" $($SOURCES -join ' ') -o tasia.exe -L"$LLVM_PATH\lib" -lLLVM
exit %ERRORLEVEL%
"@

$BUILD_SCRIPT | Out-File -FilePath "$TEMP_DIR\build_tasia.bat" -Encoding ASCII
Write-Host "Compiling Tasia..." -ForegroundColor Cyan
$BUILD_RESULT = Start-Process -FilePath "cmd.exe" -ArgumentList "/c $TEMP_DIR\build_tasia.bat" -NoNewWindow -Wait -PassThru

if ($BUILD_RESULT.ExitCode -eq 0) {
    Write-Host "Compilation successful! Installing Tasia..." -ForegroundColor Green
    
    # Create destination directory
    $INSTALL_DIR = "$env:ProgramFiles\Tasia"
    New-Item -ItemType Directory -Path $INSTALL_DIR -Force | Out-Null
    
    # Copy the executable
    Copy-Item -Path ".\tasia.exe" -Destination "$INSTALL_DIR\tasia.exe" -Force
    
    # Add to PATH
    $PATH = [Environment]::GetEnvironmentVariable("PATH", "Machine")
    if (-not $PATH.Contains($INSTALL_DIR)) {
        [Environment]::SetEnvironmentVariable("PATH", "$PATH;$INSTALL_DIR", "Machine")
        Write-Host "Added Tasia to system PATH" -ForegroundColor Green
    }
    
    Write-Host "Installation complete!" -ForegroundColor Green
} else {
    Write-Host "Compilation failed." -ForegroundColor Red
    exit 1
}

# Clean up
Write-Host "Cleaning up temporary files..." -ForegroundColor Cyan
Set-Location -Path $env:TEMP
Remove-Item -Path $TEMP_DIR -Recurse -Force

Write-Host "Tasia has been successfully installed!" -ForegroundColor Green
Write-Host "You can now use the 'tasia' command to compile Tasia programs." -ForegroundColor Green