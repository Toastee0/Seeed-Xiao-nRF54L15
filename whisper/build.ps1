#!/usr/bin/env pwsh
# Build the audio capture transcribe application

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Building Audio Capture Transcribe" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check if whisper.cpp exists
if (-not (Test-Path "whisper.cpp")) {
    Write-Host "✗ whisper.cpp not found" -ForegroundColor Red
    Write-Host "  Run setup.ps1 first" -ForegroundColor Yellow
    exit 1
}

Write-Host "✓ whisper.cpp found" -ForegroundColor Green
Write-Host ""

# Create build directory
if (Test-Path "build") {
    Write-Host "Cleaning existing build..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force build
}

New-Item -ItemType Directory -Path "build" | Out-Null
Push-Location build

Write-Host "Configuring CMake..." -ForegroundColor Yellow

# Configure - will use whisper.cpp's existing build
cmake .. -DCMAKE_BUILD_TYPE=Release

if ($LASTEXITCODE -ne 0) {
    Write-Host "✗ CMake configuration failed" -ForegroundColor Red
    Pop-Location
    exit 1
}

Write-Host "Building..." -ForegroundColor Yellow
cmake --build . --config Release

if ($LASTEXITCODE -ne 0) {
    Write-Host "✗ Build failed" -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Build Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Run with:" -ForegroundColor Yellow
Write-Host "  .\build\bin\Release\audio_capture_transcribe.exe whisper.cpp\models\ggml-base.bin" -ForegroundColor Cyan
Write-Host ""
