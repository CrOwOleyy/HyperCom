$env:HYPERCOM_PASSPHRASE = "change-me"
$env:SERVER_KEY = "paste-your-server-key-here"

function global:hcli {
    $bin = if (Test-Path "$PSScriptRoot\build\windows\bin\RelWithDebInfo\hypercom_cli.exe") { "$PSScriptRoot\build\windows\bin\RelWithDebInfo\hypercom_cli.exe" } else { "$PSScriptRoot\build-win\bin\RelWithDebInfo\hypercom_cli.exe" }
    & $bin --server-key $env:SERVER_KEY --identity "$env:USERPROFILE\.hypercom.key" @args
}

function global:hgui {
    $bin = if (Test-Path "$PSScriptRoot\build\windows\bin\RelWithDebInfo\hypercom_client.exe") { "$PSScriptRoot\build\windows\bin\RelWithDebInfo\hypercom_client.exe" } else { "$PSScriptRoot\build-win\bin\RelWithDebInfo\hypercom_client.exe" }
    & $bin --server-key $env:SERVER_KEY --identity "$env:USERPROFILE\.hypercom.key" @args
}

function global:hserver {
    $bin = if (Test-Path "$PSScriptRoot\build\windows\bin\RelWithDebInfo\hypercom_server.exe") { "$PSScriptRoot\build\windows\bin\RelWithDebInfo\hypercom_server.exe" } else { "$PSScriptRoot\build-win\bin\RelWithDebInfo\hypercom_server.exe" }
    & $bin hypercom.conf @args
}

Write-Host "Environnement Hypercom (Windows Natif) chargé !" -ForegroundColor Green
Write-Host "Commandes disponibles dans votre terminal PowerShell :" -ForegroundColor Yellow
Write-Host "  hserver  -> Lancer le serveur natif Windows"
Write-Host "  hcli     -> Lancer les commandes CLI"
Write-Host "  hgui     -> Lancer l'interface graphique ImGui native Windows"
