#ifndef STRUCTURES_H
#define STRUCTURES_H

// Structure représentant une musique
typedef struct {
    int id;
    char titre[256];
    char artiste[256];
    char album[256];
    char genre[100];
    int duree;
    int annee;
    int numero_piste;
    char chemin[512];
    int est_favori;
} Musique;

// Structure représentant une playlist
typedef struct {
    int id;
    char nom[256];
} Playlist;

// Énumération pour les types de tri
// (Vérifiez qu'il n'y a pas d'autre enum ModeTri plus bas dans le fichier)
typedef enum {
    TRI_DEFAUT = 0, // Par Titre
    TRI_TITRE,
    TRI_ARTISTE,
    TRI_ALBUM,
    TRI_GENRE,
    TRI_DUREE,
    TRI_RECENT
} ModeTri;

// Structure pour les paroles (Ajout du module Karaoké)
typedef struct {
    int id;
    int id_musique;
    char texte[4096];
} Paroles;

#endif