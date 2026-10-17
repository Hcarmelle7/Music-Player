#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include <gtk/gtk.h>
#include "error_codes.h" // Important

void init_audio();
void cleanup_audio();

// Prototype mis à jour
AppError jouer_musique(const char *chemin_fichier);

void mettre_en_pause();
void arreter_musique();
void reprendre_lecture();
gboolean est_en_lecture();

void update_audio();
float obtenir_progression();

int64_t obtenir_duree_totale_sec();
int64_t obtenir_temps_actuel_sec();
void definir_progression(double pourcentage);

#endif