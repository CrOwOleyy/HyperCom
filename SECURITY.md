# Politique de sécurité

Hypercom est maintenu par une petite équipe, en dehors de tout cadre
professionnel. Ce document dit honnêtement ce que ça implique.

## Signaler une faille

**Ne pas ouvrir d'issue publique pour une faille de sécurité.** Utiliser le
[signalement privé de vulnérabilités GitHub](../../security/advisories/new)
de ce dépôt (onglet *Security* → *Report a vulnerability*) : le rapport
n'est visible que par les mainteneurs tant qu'un correctif n'est pas publié.

Décrire :

- le fichier et la fonction concernés si possible ;
- les conditions précises pour reproduire le problème ;
- l'impact concret (ce qu'un attaquant obtient, pas seulement « c'est mal
  écrit »).

## Ce qui est dans le périmètre

Toute faille qui rend fausse une des garanties listées dans
[docs/THREAT_MODEL.md](docs/THREAT_MODEL.md) — par exemple : un DM lisible
sans la clé du destinataire, un contournement de l'authentification par
signature, une injection SQL, un crash déclenchable à distance sans
authentification, une fuite mémoire d'un secret.

## Ce qui n'est pas une faille

Le `THREAT_MODEL.md` documente des limites **assumées**, pas des oublis :
le serveur voit qui écrit à qui, « amis uniquement » n'est pas du
chiffrement, un opérateur de serveur malveillant peut mentir à ses propres
utilisateurs. Les rapports sur ces points précis seront fermés en pointant
vers ce document plutôt que traités comme une faille.

## Versions couvertes

Hypercom n'a pas encore de version stable numérotée : seule la branche
`main` reçoit des correctifs de sécurité.

## Délai de réponse

Aucun SLA formel — c'est un projet à deux personnes, pas une entreprise.
En pratique : accusé de réception sous une semaine, correctif ou plan
d'action communiqué avant publication publique du rapport.
