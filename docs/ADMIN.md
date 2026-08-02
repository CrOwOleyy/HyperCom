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

## 6. Durcissement au déploiement

Ces mesures sont **documentées mais pas implémentées** — elles relèvent de
l'exploitation, pas du code :

- abandon des privilèges après bind ; le serveur ne doit jamais tourner en root ;
- sandbox seccomp-bpf et espaces de noms Linux ;
- unité systemd durcie : `NoNewPrivileges`, `ProtectSystem=strict`,
  `PrivateTmp`, `MemoryDenyWriteExecute` ;
- base et clés en `0600`, propriétaire dédié.

## 7. Ce qui manque encore

| Élément | État |
|---|---|
| CLI d'administration sur socket Unix | **non implémenté** — `admin_socket` est lu dans la config mais aucun socket n'est ouvert |
| Rotation et purge des journaux | **non implémenté** — `retention_days` est lu et validé, rien ne l'applique |
| Rechargement à chaud de la configuration | **non implémenté** |
| Statistiques serveur | **non implémenté** |

Le MOTD et la sauvegarde à chaud, eux, sont utilisables dès maintenant par SQL
et `sqlite3`. La CLI reste à écrire : c'est le principal reliquat du §9 du
brief.
