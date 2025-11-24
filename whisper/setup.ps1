#!/usr/bin/env pwsh
# Whisper.cpp Setup Script for Windows
# Automates installation and model download

param(
    [Parameter(Mandatory=$false)]
    [ValidateSet("tiny", "base", "small", "medium", "large")]
    [string]$Model = "base",
    
    [Parameter(Mandatory=$false)]
    [switch]$SkipBuild,
    
    [Parameter(Mandatory=$false)]
    [switch]$NoCUDA
)

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Whisper.cpp Setup for BLE Audio System" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check prerequisites
Write-Host "Checking prerequisites..." -ForegroundColor Yellow

# Check CMake
try {
    $cmakeVersion = cmake --version 2>$null
    Write-Host "✓ CMake found: $($cmakeVersion[0])" -ForegroundColor Green
} catch {
    Write-Host "✗ CMake not found. Please install CMake 3.21+" -ForegroundColor Red
    Write-Host "  Download from: https://cmake.org/download/" -ForegroundColor Yellow
    exit 1
}

# Check Visual Studio
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -property installationPath
    if ($vsPath) {
        Write-Host "✓ Visual Studio found at: $vsPath" -ForegroundColor Green
    }
} else {
    Write-Host "⚠ Visual Studio not detected - build may fail" -ForegroundColor Yellow
}

# Check CUDA (optional)
if (-not $NoCUDA) {
    try {
        $nvccVersion = nvcc --version 2>$null | Select-String "release"
        if ($nvccVersion) {
            Write-Host "✓ CUDA Toolkit found: $nvccVersion" -ForegroundColor Green
            $useCUDA = $true
        } else {
            Write-Host "⚠ CUDA not found - will build CPU-only version" -ForegroundColor Yellow
            $useCUDA = $false
        }
    } catch {
        Write-Host "⚠ CUDA not found - will build CPU-only version" -ForegroundColor Yellow
        $useCUDA = $false
    }
} else {
    Write-Host "ℹ CUDA disabled by user (-NoCUDA)" -ForegroundColor Cyan
    $useCUDA = $false
}

Write-Host ""

# Clone whisper.cpp if not exists
$whisperPath = "whisper.cpp"
if (-not (Test-Path $whisperPath)) {
    Write-Host "Cloning whisper.cpp repository..." -ForegroundColor Yellow
    git clone https://github.com/ggerganov/whisper.cpp.git
    if ($LASTEXITCODE -ne 0) {
        Write-Host "✗ Failed to clone repository" -ForegroundColor Red
        exit 1
    }
    Write-Host "✓ Repository cloned" -ForegroundColor Green
} else {
    Write-Host "✓ whisper.cpp already exists" -ForegroundColor Green
}

Write-Host ""

# Build whisper.cpp
if (-not $SkipBuild) {
    Write-Host "Building whisper.cpp..." -ForegroundColor Yellow
    Push-Location $whisperPath
    
    # Create build directory
    if (Test-Path "build") {
        Write-Host "  Cleaning existing build..." -ForegroundColor Cyan
        Remove-Item -Recurse -Force build
    }
    New-Item -ItemType Directory -Path "build" | Out-Null
    
    Push-Location build
    
    # Configure
    Write-Host "  Configuring CMake..." -ForegroundColor Cyan
    if ($useCUDA) {
        Write-Host "    With CUDA acceleration enabled" -ForegroundColor Green
        cmake .. -DGGML_CUDA=ON -DWHISPER_SDL2=ON -DCMAKE_BUILD_TYPE=Release
    } else {
        Write-Host "    CPU-only build" -ForegroundColor Yellow
        cmake .. -DWHISPER_SDL2=ON -DCMAKE_BUILD_TYPE=Release
    }
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "✗ CMake configuration failed" -ForegroundColor Red
        Pop-Location
        Pop-Location
        exit 1
    }
    
    # Build
    Write-Host "  Building (this may take a few minutes)..." -ForegroundColor Cyan
    cmake --build . --config Release
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "✗ Build failed" -ForegroundColor Red
        Pop-Location
        Pop-Location
        exit 1
    }
    
    Pop-Location
    Pop-Location
    
    Write-Host "✓ Build complete" -ForegroundColor Green
} else {
    Write-Host "ℹ Skipping build (-SkipBuild specified)" -ForegroundColor Cyan
}

Write-Host ""

# Download model
Write-Host "Downloading Whisper model: $Model" -ForegroundColor Yellow

$modelPath = "$whisperPath\models\ggml-$Model.bin"
if (Test-Path $modelPath) {
    Write-Host "✓ Model already exists: $modelPath" -ForegroundColor Green
} else {
    Push-Location "$whisperPath\models"
    
    # Use download script
    if (Test-Path "download-ggml-model.cmd") {
        Write-Host "  Downloading... (this may take a while)" -ForegroundColor Cyan
        & .\download-ggml-model.cmd $Model
        
        if ($LASTEXITCODE -ne 0) {
            Write-Host "✗ Model download failed" -ForegroundColor Red
            Pop-Location
            exit 1
        }
    } else {
        Write-Host "⚠ Download script not found, downloading manually..." -ForegroundColor Yellow
        $modelUrl = "https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-$Model.bin"
        Write-Host "  URL: $modelUrl" -ForegroundColor Cyan
        Invoke-WebRequest -Uri $modelUrl -OutFile "ggml-$Model.bin"
    }
    
    Pop-Location
    
    if (Test-Path $modelPath) {
        Write-Host "✓ Model downloaded: $modelPath" -ForegroundColor Green
    } else {
        Write-Host "✗ Model download verification failed" -ForegroundColor Red
        exit 1
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Setup Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "  1. Flash nRF52840 dongle with USB Audio mode:" -ForegroundColor White
Write-Host "     cd M:\nRF54L15\bap_broadcast_sink" -ForegroundColor Cyan
Write-Host "     west flash" -ForegroundColor Cyan
Write-Host ""
Write-Host "  2. Connect dongle to PC" -ForegroundColor White
Write-Host ""
Write-Host "  3. Run transcription:" -ForegroundColor White
Write-Host "     cd M:\nRF54L15\whisper\whisper.cpp" -ForegroundColor Cyan
if ($useCUDA) {
    Write-Host "     .\build\bin\Release\stream.exe -m models\ggml-$Model.bin -t 8 --step 500 --length 5000 -ng 1" -ForegroundColor Cyan
} else {
    Write-Host "     .\build\bin\Release\stream.exe -m models\ggml-$Model.bin -t 8 --step 500 --length 5000" -ForegroundColor Cyan
}
Write-Host ""
Write-Host "For more information, see README.md" -ForegroundColor Yellow
Write-Host ""

# Display model info
$modelSizes = @{
    "tiny" = "~75 MB"
    "base" = "~140 MB"
    "small" = "~460 MB"
    "medium" = "~1.5 GB"
    "large" = "~2.9 GB"
}

Write-Host "Model: $Model ($($modelSizes[$Model]))" -ForegroundColor Cyan
if ($useCUDA) {
    Write-Host "GPU Acceleration: Enabled ✓" -ForegroundColor Green
} else {
    Write-Host "GPU Acceleration: Disabled (CPU only)" -ForegroundColor Yellow
}
Write-Host ""
