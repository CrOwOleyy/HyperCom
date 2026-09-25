#!/bin/bash
# Return to the repo root if needed
cd "$(dirname "${BASH_SOURCE[0]}")"

# 1. Default passphrase for the local private key
export HYPERCOM_PASSPHRASE="change-me"

# 2. Server key (replace with the key printed at server startup)
export SERVER_KEY="paste-your-server-key-here"

# 3. Quick aliases for the CLI and GUI
if [ -f "./build/linux/bin/hypercom_cli" ]; then
    alias hcli="./build/linux/bin/hypercom_cli --server-key $SERVER_KEY --identity ~/.hypercom.key"
    alias hgui="./build/linux/bin/hypercom_client --server-key $SERVER_KEY --identity ~/.hypercom.key"
else
    alias hcli="./build/bin/hypercom_cli --server-key $SERVER_KEY --identity ~/.hypercom.key"
    alias hgui="./build/bin/hypercom_client --server-key $SERVER_KEY --identity ~/.hypercom.key"
fi

echo "Hypercom environment loaded!"
echo "Use 'hcli <command>' to interact."
