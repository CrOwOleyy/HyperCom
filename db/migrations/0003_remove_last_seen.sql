-- Suppression du traceur de presence.
--
-- users.last_seen etait ecrit a CHAQUE authentification et servi a N'IMPORTE
-- QUEL utilisateur par profile_get. Concretement, n'importe qui pouvait
-- interroger le profil de quelqu'un toutes les trente secondes et reconstituer
-- ses horaires de connexion, ses habitudes et ses absences.
--
-- C'est exactement ce que le projet existe pour rendre impossible, et ce
-- n'etait meme pas reserve a un administrateur : c'etait une fonctionnalite
-- offerte a tout le monde.
--
-- La colonne disparait plutot que d'etre masquee a l'affichage : ce qui n'est
-- pas ecrit ne peut pas fuiter, ni etre saisi avec le disque.
--
-- Consequence : plus aucun moyen de savoir si quelqu'un est en ligne, ni quand
-- il l'a ete. C'est le comportement recherche, pas un effet de bord.

ALTER TABLE users DROP COLUMN last_seen;
