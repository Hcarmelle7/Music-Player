#ifndef DAO_MUSIQUE_H
#define DAO_MUSIQUE_H

#include "structures.h"
#include "error_codes.h"

// ... (tes autres prototypes existants) ...
AppError ajouter_musique(Musique m);
void recuperer_musiques_triees(int type_tri, Musique *tableau, int *nb_resultats);
void rechercher_musiques(const char *mot_cle, Musique *tableau, int *nb_resultats);
void supprimer_musique(int id);
int musique_existe_deja(const char *titre, const char *artiste, int duree);

// AJOUTE CETTE LIGNE :
void recuperer_musiques_par_artiste(const char *artiste, Musique *tableau, int *nb_resultats);
AppError modifier_infos_musique(int id, const char *nouveau_titre, const char *nouvel_artiste, const char *nouvel_album);

#endif