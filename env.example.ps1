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

Write-Host "Hypercom environment (native Windows) loaded!" -ForegroundColor Green
Write-Host "Commands available in your PowerShell terminal:" -ForegroundColor Yellow
Write-Host "  hserver  -> Launch the native Windows server"
Write-Host "  hcli     -> Run CLI commands"
Write-Host "  hgui     -> Launch the native Windows ImGui graphical interface"
