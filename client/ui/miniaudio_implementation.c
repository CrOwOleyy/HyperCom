/*
 * Unite de compilation unique de miniaudio.
 *
 * miniaudio est une bibliotheque en un seul en-tete : le corps n'existe que si
 * MINIAUDIO_IMPLEMENTATION est defini, et il ne doit l'etre qu'une seule fois
 * dans tout le programme. D'ou ce fichier, qui ne contient rien d'autre.
 *
 * Il est compile en C et sans avertissements (voir client/ui/CMakeLists.txt) :
 * le code tiers n'est pas soumis a notre norme, et le passer sous -Werror
 * casserait le build a chaque mise a jour amont.
 */

#define MINIAUDIO_IMPLEMENTATION

/* On ne lit qu'un MP3. Retirer les formats et l'encodage inutiles reduit
 * d'autant la surface compilee. */
#define MA_NO_FLAC
#define MA_NO_ENCODING
#define MA_NO_GENERATION

#include <miniaudio.h>
