# Script de demonstration locale sous Windows PowerShell
$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$RootDir = Split-Path -Parent $ScriptDir

Write-Host "=== [Hypercom] Demo locale automatisée (Windows Natif) ===" -ForegroundColor Green

Set-Location $RootDir
. .\env.ps1

# Verification de l'existence des binaires
$ServerBin = if (Test-Path "$RootDir\build\windows\bin\RelWithDebInfo\hypercom_server.exe") { "$RootDir\build\windows\bin\RelWithDebInfo\hypercom_server.exe" } else { "$RootDir\build-win\bin\RelWithDebInfo\hypercom_server.exe" }
$CliBin    = if (Test-Path "$RootDir\build\windows\bin\RelWithDebInfo\hypercom_cli.exe") { "$RootDir\build\windows\bin\RelWithDebInfo\hypercom_cli.exe" } else { "$RootDir\build-win\bin\RelWithDebInfo\hypercom_cli.exe" }

if (-not (Test-Path $ServerBin)) {
    Write-Host "Binaires non trouvés. Compilation en cours..." -ForegroundColor Yellow
    powershell -ExecutionPolicy Bypass -File .\scripts\build_windows.ps1
}

# 1. Execution des tests unitaires
Write-Host "1. Verification des tests unitaires..." -ForegroundColor Cyan
$BuildFolder = if (Test-Path "$RootDir\build\windows") { "build\windows" } else { "build-win" }
ctest --test-dir $BuildFolder -C RelWithDebInfo --output-on-failure

Write-Host "=== Demo locale terminee avec succes ! ===" -ForegroundColor Green
