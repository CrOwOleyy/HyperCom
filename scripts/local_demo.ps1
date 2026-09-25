# Local demo script for Windows PowerShell
$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$RootDir = Split-Path -Parent $ScriptDir

Write-Host "=== [Hypercom] Automated local demo (native Windows) ===" -ForegroundColor Green

Set-Location $RootDir
. .\env.ps1

# Check whether the binaries exist
$ServerBin = if (Test-Path "$RootDir\build\windows\bin\RelWithDebInfo\hypercom_server.exe") { "$RootDir\build\windows\bin\RelWithDebInfo\hypercom_server.exe" } else { "$RootDir\build-win\bin\RelWithDebInfo\hypercom_server.exe" }
$CliBin    = if (Test-Path "$RootDir\build\windows\bin\RelWithDebInfo\hypercom_cli.exe") { "$RootDir\build\windows\bin\RelWithDebInfo\hypercom_cli.exe" } else { "$RootDir\build-win\bin\RelWithDebInfo\hypercom_cli.exe" }

if (-not (Test-Path $ServerBin)) {
    Write-Host "Binaries not found. Building now..." -ForegroundColor Yellow
    powershell -ExecutionPolicy Bypass -File .\scripts\build_windows.ps1
}

# 1. Run the unit tests
Write-Host "1. Running unit tests..." -ForegroundColor Cyan
$BuildFolder = if (Test-Path "$RootDir\build\windows") { "build\windows" } else { "build-win" }
ctest --test-dir $BuildFolder -C RelWithDebInfo --output-on-failure

Write-Host "=== Local demo completed successfully! ===" -ForegroundColor Green
