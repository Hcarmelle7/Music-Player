#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "audio_engine.h"
#include "error_codes.h"

static GtkMediaStream *media_stream = NULL;
static int initialized = 0;

void init_audio() {
    initialized = 1;
}

gboolean est_en_lecture() {
    if (media_stream) return gtk_media_stream_get_playing(media_stream);
    return FALSE;
}

void reprendre_lecture() {
    if (media_stream) gtk_media_stream_set_playing(media_stream, TRUE);
}

void mettre_en_pause() {
    if (media_stream) {
        // Correction : On force l'état opposé proprement
        gboolean etat = gtk_media_stream_get_playing(media_stream);
        gtk_media_stream_set_playing(media_stream, !etat);
    }
}

// Nettoyage pour éviter les fuites mémoires (saccades)
void cleanup_audio() {
    if (media_stream != NULL) {
        gtk_media_stream_set_playing(media_stream, FALSE);
        g_object_unref(media_stream);
        media_stream = NULL;
    }
}

AppError jouer_musique(const char *chemin_fichier) {
    if (!initialized) init_audio();
    cleanup_audio(); // On nettoie TOUJOURS avant de relancer

    if (access(chemin_fichier, F_OK) == -1) return ERR_FILE_NOT_FOUND;

    GFile *file = g_file_new_for_path(chemin_fichier);
    media_stream = GTK_MEDIA_STREAM(gtk_media_file_new_for_file(file));
    g_object_unref(file);

    if (media_stream) {
        gtk_media_stream_set_volume(media_stream, 1.0);
        gtk_media_stream_set_playing(media_stream, TRUE);
        return SUCCESS;
    }
    return ERR_AUDIO_STREAM;
}

void arreter_musique() { cleanup_audio(); }

// --- CORRECTION DES CALCULS DE TEMPS (Indispensable pour la barre bleue) ---

int64_t obtenir_duree_totale_sec() {
    if (media_stream) {
        int64_t d = gtk_media_stream_get_duration(media_stream);
        return (d > 0) ? d / 1000000 : 0; // Conversion microsecondes -> secondes
    }
    return 0;
}

int64_t obtenir_temps_actuel_sec() {
    if (media_stream) {
        return gtk_media_stream_get_timestamp(media_stream) / 1000000;
    }
    return 0;
}

void definir_progression(double pourcentage) {
    if (media_stream && gtk_media_stream_is_seekable(media_stream)) {
        int64_t duree = gtk_media_stream_get_duration(media_stream);
        if (duree > 0) {
            int64_t cible = (int64_t)(duree * pourcentage);
            gtk_media_stream_seek(media_stream, cible);
        }
    }
}

float obtenir_progression() {
    int64_t tot = obtenir_duree_totale_sec();
    if (tot <= 0) return 0.0f;
    return (float)obtenir_temps_actuel_sec() / (float)tot;
}