# Tasia Installation Script for Windows
Write-Host "Installing Tasia Programming Language..." -ForegroundColor Blue

# Create installation directory
$InstallDir = "$env:LOCALAPPDATA\Programs\Tasia"
if (-not (Test-Path $InstallDir)) {
    Write-Host "Creating installation directory..."
    New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
}

# Download Tasia executable
Write-Host "Downloading Tasia executable..."
$DownloadUrl = "https://github.com/dylancook244/tasia/releases/latest/download/tasia-Windows.exe"
$ExePath = "$InstallDir\tasia.exe"
Invoke-WebRequest -Uri $DownloadUrl -OutFile $ExePath

# Add to PATH
$CurrentPath = [Environment]::GetEnvironmentVariable("Path", "User")
if (-not $CurrentPath.Contains($InstallDir)) {
    Write-Host "Adding Tasia to your PATH..."
    [Environment]::SetEnvironmentVariable("Path", "$CurrentPath;$InstallDir", "User")
    # Refresh PATH in current session
    $env:Path = [Environment]::GetEnvironmentVariable("Path", "User")
}

Write-Host "Tasia installed successfully!" -ForegroundColor Green
Write-Host "You can now use Tasia by running the 'tasia' command."
Write-Host "Try 'tasia help' to get started."