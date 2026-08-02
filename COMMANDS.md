
## Sur Windows (PowerShell)

### 1. Installer les dépendances
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\scripts\fetch_third_party.ps1

### 2. Compiler (Générer l'exécutable)

cmake -S . -B build/windows

### 3. Compiler le projet
cmake --build build/windows --config RelWithDebInfo -j

### 3. Lancer le projet
### 1. Charger l'environnement
. .\env.ps1

### Pour lancer le serveur localement dans le terminal 1
hserver

### 2. Lancer le client interface graphique (GUI)
hgui

### Pour lancer un 2ème client (pour tester avec un 2ème compte)
hgui --identity compte2.key



## Sur Linux / WSL

### 1. Installer les dépendances
./scripts/fetch_third_party.sh

### 2. Compiler (Générer l'exécutable)
# Générer les fichiers de construction
cmake -S . -B build/linux

# Compiler le projet
cmake --build build/linux -j

### 3. Lancer le projet
*(Charge les raccourcis `hgui`, `hcli` et `hserver`)*

### 1. Pour lancer le serveur localement (terminal 1)
hserver

### 2. Charger l'environnement (terminal 2)
source env.sh

### 3. Lancer le client interface graphique (GUI) (terminal 2)
hgui


