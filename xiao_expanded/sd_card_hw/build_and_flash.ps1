# Build and Flash Script for SD Card Sample

# Clean and build
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
west build -b xiao_nrf54l15/nrf54l15/cpuapp -p

# Flash if build succeeded
if ($LASTEXITCODE -eq 0) {
    Write-Host "`n========================================" -ForegroundColor Green
    Write-Host "Build successful! Flashing..." -ForegroundColor Green
    Write-Host "========================================`n" -ForegroundColor Green
    west flash
} else {
    Write-Host "`n========================================" -ForegroundColor Red
    Write-Host "Build failed!" -ForegroundColor Red
    Write-Host "========================================`n" -ForegroundColor Red
}
