#ifndef DAO_PLAYLIST_H
#define DAO_PLAYLIST_H

#include "structures.h"
#include "error_codes.h"

// Gestion des Playlists

int creer_playlist_et_recuperer_id(const char *nom);
AppError supprimer_playlist(int id_playlist);
AppError renommer_playlist(int id_playlist, const char *nouveau_nom);
void recuperer_toutes_les_playlists(Playlist *tableau, int *nb_resultats);

// Gestion du contenu (Liaison Musique <-> Playlist)
AppError ajouter_musique_playlist(int id_playlist, int id_musique);
AppError retirer_musique_playlist(int id_playlist, int id_musique);
void recuperer_musiques_playlist(int id_playlist, Musique *tableau, int *nb_resultats);

#endif