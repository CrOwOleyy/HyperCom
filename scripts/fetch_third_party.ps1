#Requires -Version 5.1
<#
    Windows equivalent of scripts/fetch_third_party.sh.

    Same trust model: no checksum is hardcoded. The script refuses to
    install an archive absent from third_party/checksums.txt, prints
    the computed checksum and the official URL to verify it against,
    then stops.

    usage:
      powershell -ExecutionPolicy Bypass -File scripts\fetch_third_party.ps1 all
#>
param(
    [ValidateSet('all', 'libsodium', 'sqlite3', 'imgui', 'glfw')]
    [string] $Target = 'all'
)

$ErrorActionPreference = 'Stop'
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

$RootDir      = Split-Path -Parent $PSScriptRoot
$VendorDir    = Join-Path $RootDir 'third_party'
$WorkDir      = Join-Path $VendorDir '.download'
$ChecksumFile = Join-Path $VendorDir 'checksums.txt'

$SodiumVersion = if ($env:HYPERCOM_SODIUM_VERSION) { $env:HYPERCOM_SODIUM_VERSION } else { '1.0.20' }
$SqliteYear    = if ($env:HYPERCOM_SQLITE_YEAR)    { $env:HYPERCOM_SQLITE_YEAR }    else { '2024' }
$SqliteStem    = if ($env:HYPERCOM_SQLITE_STEM)    { $env:HYPERCOM_SQLITE_STEM }    else { 'sqlite-amalgamation-3470000' }
$ImguiVersion  = if ($env:HYPERCOM_IMGUI_VERSION)  { $env:HYPERCOM_IMGUI_VERSION }  else { 'v1.91.5' }

function Write-Step([string] $Message) {
    Write-Host "[third_party] $Message" -ForegroundColor Green
}

function Get-PinnedSha256([string] $Key) {
    if (-not (Test-Path $ChecksumFile)) { return $null }
    foreach ($line in Get-Content $ChecksumFile) {
        $trimmed = $line.Trim()
        if ($trimmed -eq '' -or $trimmed.StartsWith('#')) { continue }
        $parts = $trimmed -split '\s+', 2
        if ($parts[0] -eq $Key) { return $parts[1].Trim() }
    }
    return $null
}

function Assert-PinnedSha256([string] $Key, [string] $Path, [string] $VerifyUrl) {
    $actual = (Get-FileHash -Algorithm SHA256 -Path $Path).Hash.ToLower()
    $expected = Get-PinnedSha256 $Key
    if (-not $expected) {
        throw @"
no checksum recorded for '$Key'.

  computed checksum: $actual

  1. compare this value against the one published at:
       $VerifyUrl
  2. if it matches, record the following line in
     third_party\checksums.txt then re-run:

       $Key  $actual

  Until this check is done, the archive isn't installed.
"@
    }
    if ($actual -ne $expected.ToLower()) {
        Remove-Item $Path -Force
        throw "INVALID CHECKSUM for '$Key' -- archive deleted. expected=$expected got=$actual"
    }
    Write-Step "checksum verified: $Key"
}

function Get-VerifiedArchive([string] $Url, [string] $Path, [string] $Key, [string] $VerifyUrl) {
    if (Test-Path $Path) {
        Write-Step "archive already present: $(Split-Path -Leaf $Path)"
    } else {
        Write-Step "downloading $Url"
        Invoke-WebRequest -Uri $Url -OutFile $Path -UseBasicParsing
    }
    Assert-PinnedSha256 $Key $Path $VerifyUrl
}

function Install-Libsodium {
    $prefix = Join-Path $VendorDir 'libsodium'
    if (Test-Path (Join-Path $prefix 'include\sodium.h')) {
        Write-Step 'libsodium already installed'; return
    }
    $name    = "libsodium-$SodiumVersion-stable-msvc.zip"
    $archive = Join-Path $WorkDir $name
    Get-VerifiedArchive "https://download.libsodium.org/libsodium/releases/$name" `
        $archive $name 'https://download.libsodium.org/libsodium/releases/'
    $extract = Join-Path $WorkDir 'libsodium-src'
    if (Test-Path $extract) { Remove-Item $extract -Recurse -Force }
    Expand-Archive -Path $archive -DestinationPath $extract -Force
    $lib = Get-ChildItem -Path $extract -Recurse -Filter 'libsodium.lib' |
        Where-Object { $_.FullName -match 'x64\\Release\\v14.*\\static' } |
        Select-Object -First 1
    if (-not $lib) { throw "libsodium.lib x64/Release not found in the archive" }
    New-Item -ItemType Directory -Force -Path (Join-Path $prefix 'lib') | Out-Null
    Copy-Item $lib.FullName (Join-Path $prefix 'lib\libsodium.lib') -Force
    $include = Get-ChildItem -Path $extract -Recurse -Directory -Filter 'include' |
        Select-Object -First 1
    Copy-Item $include.FullName $prefix -Recurse -Force
    Write-Step 'libsodium installed into third_party\libsodium'
}

function Install-Sqlite3 {
    $prefix = Join-Path $VendorDir 'sqlite3'
    if (Test-Path (Join-Path $prefix 'sqlite3.c')) {
        Write-Step 'sqlite3 already present'; return
    }
    $name    = "$SqliteStem.zip"
    $archive = Join-Path $WorkDir $name
    Get-VerifiedArchive "https://www.sqlite.org/$SqliteYear/$name" `
        $archive $name 'https://www.sqlite.org/download.html'
    $extract = Join-Path $WorkDir 'sqlite-src'
    if (Test-Path $extract) { Remove-Item $extract -Recurse -Force }
    Expand-Archive -Path $archive -DestinationPath $extract -Force
    New-Item -ItemType Directory -Force -Path $prefix | Out-Null
    foreach ($file in @('sqlite3.c', 'sqlite3.h', 'sqlite3ext.h')) {
        Copy-Item (Join-Path $extract "$SqliteStem\$file") $prefix -Force
    }
    Write-Step 'sqlite3 amalgamation installed into third_party\sqlite3'
}

function Install-Imgui {
    $prefix = Join-Path $VendorDir 'imgui'
    if (Test-Path (Join-Path $prefix 'imgui.cpp')) {
        Write-Step 'imgui already present'; return
    }
    $name    = "imgui-$ImguiVersion.zip"
    $archive = Join-Path $WorkDir $name
    Get-VerifiedArchive "https://github.com/ocornut/imgui/archive/refs/tags/$ImguiVersion.zip" `
        $archive $name "https://github.com/ocornut/imgui/releases/tag/$ImguiVersion"
    $extract = Join-Path $WorkDir 'imgui-src'
    if (Test-Path $extract) { Remove-Item $extract -Recurse -Force }
    Expand-Archive -Path $archive -DestinationPath $extract -Force
    $inner = Get-ChildItem -Path $extract -Directory | Select-Object -First 1
    New-Item -ItemType Directory -Force -Path $prefix | Out-Null
    Copy-Item (Join-Path $inner.FullName '*') $prefix -Recurse -Force
    Write-Step "dear imgui $ImguiVersion installed into third_party\imgui"
}

function Install-Glfw {
    $prefix = Join-Path $VendorDir 'glfw'
    if (Test-Path (Join-Path $prefix 'include\GLFW\glfw3.h')) {
        Write-Step 'glfw already present'; return
    }
    $name    = "glfw-3.4.bin.WIN64.zip"
    $archive = Join-Path $WorkDir $name
    Get-VerifiedArchive "https://github.com/glfw/glfw/releases/download/3.4/$name" `
        $archive $name "https://github.com/glfw/glfw/releases/tag/3.4"
    $extract = Join-Path $WorkDir 'glfw-src'
    if (Test-Path $extract) { Remove-Item $extract -Recurse -Force }
    Expand-Archive -Path $archive -DestinationPath $extract -Force
    $inner = Get-ChildItem -Path $extract -Directory | Select-Object -First 1
    New-Item -ItemType Directory -Force -Path $prefix | Out-Null
    Copy-Item (Join-Path $inner.FullName '*') $prefix -Recurse -Force
    Write-Step "glfw 3.4 installed into third_party\glfw"
}

New-Item -ItemType Directory -Force -Path $WorkDir | Out-Null
switch ($Target) {
    'libsodium' { Install-Libsodium }
    'sqlite3'   { Install-Sqlite3 }
    'imgui'     { Install-Imgui }
    'glfw'      { Install-Glfw }
    'all'       { Install-Libsodium; Install-Sqlite3; Install-Imgui; Install-Glfw }
}
Write-Step 'done'
