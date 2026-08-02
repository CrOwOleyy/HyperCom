# ADMIN — exploitation du serveur

Ce document couvre le domaine du collaborateur : configuration, base,
durcissement, annonces. **Rien ici n'exige d'écrire du C++.**

## 1. Démarrage

```
cp hypercom.conf.example hypercom.conf
./hypercom_keygen server keys/server_static.key
chmod 600 keys/server_static.key
./hypercom_server hypercom.conf
```

Au premier lancement, le serveur applique les migrations, génère sa clé
statique si elle manque, puis **affiche sa clé publique**. C'est elle que les
clients épinglent.

> Elle doit être communiquée par un canal de confiance. La faire récupérer
> depuis le serveur lui-même annulerait tout l'intérêt de l'épinglage.

Le serveur **refuse de démarrer sur une configuration invalide** plutôt que de
retomber sur des valeurs par défaut silencieuses. Les erreurs sont accumulées
et affichées ensemble, avec leur numéro de ligne.

## 2. Configuration

Tout est dans `hypercom.conf`, format `clé = valeur` par section. Voir
`hypercom.conf.example`, qui est commenté.

### La politique de journalisation

```ini
[logging]
log_peer_addresses = false     # défaut
```

**Par défaut, le serveur ne journalise aucune adresse IP.** Le passer à `true`
provoque un avertissement explicite au démarrage. C'est volontaire : sur un
réseau qui se dit non surveillé, journaliser des IP doit être une décision
prise, jamais un effet de bord.

Conséquence concrète : une saisie du serveur ne révèle pas qui s'est connecté
ni depuis où.

Le seul chemin vers un journal passe par `logger::redact_peer_address`, qui
rend `[redacted]` tant que la politique n'est pas ouverte. Les appelants n'ont
pas à s'en souvenir.

### Service caché Tor

Le même code sert au clearnet et au `.onion` : Tor relaie simplement vers un
listener en boucle locale.

```ini
[onion]
enabled      = true
bind_address = 127.0.0.1
port         = 7718
```

Côté `torrc` :

```
HiddenServiceDir /var/lib/tor/hypercom/
HiddenServicePort 7717 127.0.0.1:7718
```

Le port de l'oignon ne doit **jamais** être exposé au réseau.

## 3. Migrations

Déposer un fichier dans `db/migrations/NNNN_description.sql`. Il est appliqué
au prochain démarrage, dans une transaction : il passe entièrement ou pas du
tout. Le préfixe numérique de largeur fixe donne l'ordre.

Ne jamais modifier une migration déjà appliquée — en ajouter une nouvelle.

### Si une migration échoue

Le serveur refuse de démarrer et la base n'est **pas** modifiée (la migration
tourne dans une transaction). Le message donne le fichier fautif et l'erreur
SQLite.

Le cas le plus probable est `0002_handle_case_insensitive` sur une base créée
avant elle : si deux comptes ont des pseudos ne différant que par la casse
(`leyy` et `Leyy`), l'index unique ne peut pas être créé. C'est volontaire —
renommer le compte de quelqu'un en silence serait pire que de s'arrêter.

Pour résoudre, il faut choisir quel compte garde le pseudo. Aucune commande
n'existe encore pour ça : il faut passer par `sqlite3`, en ayant arrêté le
serveur.

```
sqlite3 hypercom.db "SELECT id, handle FROM users ORDER BY lower(handle);"
sqlite3 hypercom.db "UPDATE users SET handle = 'leyy_ancien' WHERE id = 4;"
```

Le compte n'est pas perdu : son identité est sa clé publique, pas son pseudo.
Seul le nom affiché change.

## 4. Sauvegarde

```
sqlite3 hypercom.db ".backup /sauvegardes/hypercom-$(date +%F).db"
```

À chaud, sans arrêter le serveur. Une copie brute (`cp`) pendant une écriture
peut produire un fichier incohérent à cause du WAL.

Sauvegarder aussi `keys/server_static.key` : la perdre casse l'épinglage de
tous les clients, qui refuseront alors de se connecter.

## 5. MOTD

La table `motd` porte les annonces, avec un index unique partiel garantissant
un seul actif. Il est poussé aux clients à la connexion.

En attendant la CLI d'administration (voir §7), il se modifie directement :

```sql
UPDATE motd SET active = 0 WHERE active = 1;
INSERT INTO motd (body, active, created_at)
VALUES ('maintenance samedi 14h', 1, strftime('%s','now'));
```

Aucun redémarrage nécessaire : il est relu à chaque connexion.

Plus simple depuis que la CLI existe (voir §7) :

```bash
hypercom_adminctl motd set "maintenance samedi 14h"
```

## 6. Durcissement au déploiement

Ces mesures sont **documentées mais pas implémentées** — elles relèvent de
l'exploitation, pas du code :

- abandon des privilèges après bind ; le serveur ne doit jamais tourner en root ;
- sandbox seccomp-bpf et espaces de noms Linux ;
- unité systemd durcie : `NoNewPrivileges`, `ProtectSystem=strict`,
  `PrivateTmp`, `MemoryDenyWriteExecute` ;
- base et clés en `0600`, propriétaire dédié.

## 7. La CLI d'administration

Elle écoute sur un socket **AF_UNIX** dont le chemin vient de `[paths]
admin_socket`. Jamais de TCP : il n'existe aucun chemin de code capable de
l'exposer au réseau. Le fichier est créé en `0600`, donc seul le compte qui
fait tourner le serveur peut s'y connecter — c'est toute l'authentification,
et elle repose entièrement sur les permissions du système de fichiers.

```bash
hypercom_adminctl help
```

| Commande | Effet |
|---|---|
| `stats` | uptime, connexions, volumétrie de la base |
| `sessions` | connexions en cours |
| `sessions close <descripteur>` | ferme une connexion |
| `motd` | annonce en cours |
| `motd set "texte"` | publie une annonce, effet immédiat |
| `motd clear` | désactive l'annonce |
| `backup <chemin>` | sauvegarde à chaud, instantané cohérent |
| `help` | la liste |

Le chemin du socket se précise avec `--socket` s'il n'est pas au défaut :

```bash
hypercom_adminctl --socket /var/run/hypercom-admin.sock stats
```

**`sessions` ne montre ni pseudo ni adresse, et ce n'est pas réglable.** La
liste donne un descripteur, un état (`handshake`, `hello`, `auth`,
`authentifiée`) et deux durées — de quoi repérer une connexion qui traîne ou un
handshake bloqué, jamais de quoi savoir qui est en ligne. Contrairement à
`log_peer_addresses`, il n'existe aucune option pour réactiver l'identité ici :
un outil qui liste qui se connecte et depuis quand est un outil de
surveillance, pas un outil d'exploitation, et le projet n'en fournit pas.

`motd set` valide le texte comme s'il venait du réseau : UTF-8 correct, pas de
caractère de contrôle. L'annonce sera resservie à tous les clients, un
administrateur distrait n'a pas plus le droit qu'un inconnu d'y glisser
n'importe quoi.

`backup` passe par l'API de sauvegarde de SQLite, pas par une copie de
fichier — en mode WAL, un `cp` pendant une écriture produit un fichier
incohérent.

## 8. Ce qui manque encore

| Élément | État |
|---|---|
| Rechargement à chaud de la configuration | **non implémenté** — changer un listener demanderait de le rouvrir sous les connexions en cours |
| Rotation et purge des journaux | **non implémenté** — `retention_days` est lu et validé, rien ne l'applique |
| Abandon des privilèges après bind | **non implémenté** |
| Sandbox seccomp-bpf | **non implémenté** |

En attendant le rechargement à chaud, tout changement de configuration demande
un redémarrage du serveur.
