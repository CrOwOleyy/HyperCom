
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


## La séquence d'accueil

Elle ne se joue **qu'à la création d'un compte**, jamais aux connexions
suivantes. Pour la revoir sans créer de compte jetable :

hgui --replay-intro

Ou en repartant d'une identité neuve — attention, le fichier ne doit pas déjà
exister, sinon le compte est déjà enregistré et l'intro ne se déclenche pas :

hgui --identity compte_neuf.key

Déroulé, calé sur la durée réelle de `menu.mp3` (~12,5 s) :

| Moment | Ce qui se passe |
|---|---|
| 0 → 6 s | Carte de verre centrée, « Bienvenue dans l'espace HyperCom. » |
| 6 s → fin | Les trois colonnes remontent l'une après l'autre, en bulles |
| ensuite | Interface normale, plus aucune animation |

Remplacer `menu.mp3` recale l'animation tout seul : la durée est lue dans le
fichier. CMake le recopie à côté de l'exécutable à chaque build.

Pas de son ? Ce n'est jamais bloquant — l'intro se déroule à l'identique sur
l'horloge. Le client affiche la raison au démarrage :

    intro : musique, duree retenue 12.5268 s



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


