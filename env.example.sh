#!/bin/bash
# Revenir à la racine si besoin
cd "$(dirname "${BASH_SOURCE[0]}")"

# 1. Passphrase par défaut pour la clé privée localement
export HYPERCOM_PASSPHRASE="change-me"

# 2. Clé du serveur (remplacez par la clé affichée au démarrage du serveur)
export SERVER_KEY="paste-your-server-key-here"

# 3. Alias rapides pour le CLI et GUI
if [ -f "./build/linux/bin/hypercom_cli" ]; then
    alias hcli="./build/linux/bin/hypercom_cli --server-key $SERVER_KEY --identity ~/.hypercom.key"
    alias hgui="./build/linux/bin/hypercom_client --server-key $SERVER_KEY --identity ~/.hypercom.key"
else
    alias hcli="./build/bin/hypercom_cli --server-key $SERVER_KEY --identity ~/.hypercom.key"
    alias hgui="./build/bin/hypercom_client --server-key $SERVER_KEY --identity ~/.hypercom.key"
fi

echo "Environnement Hypercom chargé !"
echo "Utilisez 'hcli <commande>' pour interagir."
