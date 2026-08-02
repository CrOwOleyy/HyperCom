-- Suppression des metadonnees qui n'existaient que pour observer.
--
-- Ce qui disparait, et pourquoi :
--
--   users.created_at         age d'un compte. Permet de correler les comptes
--                            crees au meme moment, ne sert a rien d'autre.
--   friends.created_at       date a laquelle deux personnes se sont liees.
--                            C'est une chronologie de relations sociales.
--   dm_envelopes.created_at  quand quelqu'un a recu un message prive. Le
--                            serveur ne peut pas lire le contenu, mais il
--                            tenait le registre horodate des echanges. La date
--                            vit desormais A L'INTERIEUR du chiffre : le
--                            client l'affiche, le serveur ne la voit plus.
--   dm_envelopes.delivered   colonne morte, jamais lue par le code.
--
-- CE QUI RESTE, sciemment : posts.created_at, comments.created_at et
-- forums.created_at. Un forum ou l'on ne sait pas si un fil date d'aujourd'hui
-- ou d'il y a deux ans n'est plus utilisable, et ces contenus sont publics par
-- nature -- leur horodatage est deja visible de tous ceux qui les lisent.
-- L'enlever ne protegerait personne, ca degraderait seulement l'outil.

DROP INDEX idx_dm_recipient_undelivered;
CREATE INDEX idx_dm_recipient ON dm_envelopes(recipient_id, id);

ALTER TABLE dm_envelopes DROP COLUMN delivered;
ALTER TABLE dm_envelopes DROP COLUMN created_at;
ALTER TABLE users DROP COLUMN created_at;
ALTER TABLE friends DROP COLUMN created_at;
