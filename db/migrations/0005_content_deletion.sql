-- Suppression de son propre contenu par son auteur.
--
-- Le reseau n'a aucune moderation : personne ne peut effacer le contenu de
-- quelqu'un d'autre, ni un utilisateur, ni un administrateur. En revanche
-- chacun reste maitre de ce qu'il a lui-meme publie -- c'est la seule
-- suppression que le protocole autorise, et elle est verifiee cote serveur
-- par un WHERE author_id qui rend l'usurpation impossible plutot
-- qu'arbitrable.
--
-- Pourquoi une colonne et pas un DELETE : la ligne doit survivre pour que
-- l'arborescence des commentaires tienne. Effacer un commentaire au milieu
-- d'un fil emporterait toutes les reponses qui s'y rattachent
-- (ON DELETE CASCADE), donc le contenu d'autres personnes -- exactement ce
-- que le projet interdit.
--
-- Le texte lui-meme est reellement efface a la suppression (title et body mis
-- a la chaine vide par le serveur), pas seulement masque a l'affichage. Un
-- masquage cote client laisserait le contenu dans la base et dans chaque
-- sauvegarde, ce qui ne serait pas une suppression.
--
-- deleted_at porte la date plutot qu'un simple booleen : meme cout de
-- stockage, et l'information est utile a l'administrateur qui inspecte la
-- base. NULL signifie "visible".

ALTER TABLE posts ADD COLUMN deleted_at INTEGER;
ALTER TABLE comments ADD COLUMN deleted_at INTEGER;
