# third_party

Ce répertoire accueille les dépendances **non versionnées** du projet. Rien ici
n'est commité : voir `.gitignore`, règle O1.

Les trois seules dépendances autorisées par le brief (§15) :

| Dépendance | Rôle | Emplacement attendu |
|---|---|---|
| **libsodium** | toute la cryptographie | `third_party/libsodium/{include,lib}` |
| **SQLite** | stockage, amalgamation compilée dans le binaire | `third_party/sqlite3/sqlite3.{c,h}` |
| **Dear ImGui** | UI du client natif | `third_party/imgui/` |

## Récupération

```
./scripts/fetch_third_party.sh
```

Le script télécharge, **vérifie l'empreinte SHA-256** de chaque archive, puis
installe. Les empreintes sont figées dans le script ; une non-correspondance
interrompt la récupération plutôt que de continuer.

Sous Windows, `scripts/fetch_third_party.ps1` fait la même chose pour SQLite et
ImGui, et récupère la version binaire précompilée de libsodium pour MSVC.

## Alternative : paquets système

CMake accepte aussi libsodium et SQLite installés par le système
(`pkg-config` / `find_package`). C'est le mode le plus simple pour du
développement :

```
sudo apt install libsodium-dev libsqlite3-dev libglfw3-dev
```

En production, préférer le mode vendorisé : la version est alors figée et
reproductible.

## Note sur le backend ImGui

Dear ImGui n'affiche rien sans backend plateforme/rendu. Le choix retenu évite
toute dépendance vendorisée supplémentaire :

- **Windows** — `imgui_impl_win32` + `imgui_impl_dx11`, qui n'utilisent que le
  SDK Windows. Aucune dépendance ajoutée.
- **Linux** — `imgui_impl_glfw` + `imgui_impl_opengl3`, qui exigent **glfw3**
  comme bibliothèque système. C'est la seule entorse au §15, elle est limitée au
  client graphique Linux et n'affecte ni le serveur ni le client CLI.
