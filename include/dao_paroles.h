#ifndef DAO_PAROLES_H
#define DAO_PAROLES_H

#include "structures.h"
#include "error_codes.h"

// Sauvegarde le texte pour une musique donnée
AppError sauvegarder_paroles(int id_musique, const char *texte);

// Récupère le texte (retourne 1 si trouvé, 0 sinon)
int recuperer_paroles(int id_musique, char *buffer_texte, int taille_max);

#endif