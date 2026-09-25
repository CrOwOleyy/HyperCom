# Native Windows build and packaging script, under build/windows
$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$RootDir = Split-Path -Parent $ScriptDir
$BuildDir = Join-Path $RootDir "build\windows"
$PkgDir = Join-Path $BuildDir "packages"

Write-Host "=== [Hypercom] Native Windows Build ===" -ForegroundColor Green

if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}
if (-not (Test-Path $PkgDir)) {
    New-Item -ItemType Directory -Path $PkgDir | Out-Null
}

# 1. Fetch Windows dependencies (sodium, sqlite3, glfw)
Set-Location $RootDir
& powershell.exe -File ".\scripts\fetch_third_party.ps1"

# 2. CMake MSVC configuration & build
cmake -S . -B $BuildDir -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build $BuildDir --config RelWithDebInfo -j

# 3. Generate the CPack ZIP package
Set-Location $BuildDir
cpack -G ZIP -C RelWithDebInfo

if (Test-Path "hypercom-*.zip") {
    Move-Item -Path "hypercom-*.zip" -Destination $PkgDir -Force
}

Write-Host "=== [Hypercom] Windows binaries in: $BuildDir\bin\RelWithDebInfo ===" -ForegroundColor Green
Write-Host "=== [Hypercom] ZIP package in: $PkgDir ===" -ForegroundColor Green
