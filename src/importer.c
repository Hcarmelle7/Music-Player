/**
 * @file importer.c
 * @brief Logique métier pour le scan de fichiers et l'extraction de métadonnées.
 * Ce module assure l'importation sécurisée depuis des supports locaux ou externes (USB).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <strings.h> 
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h> 
#include "importer.h"
#include "dao_musique.h"
#include "structures.h"
#include "error_codes.h"

/**
 * @brief Vérifie si l'extension du fichier fait partie des formats autorisés.
 * Supporte : MP3, WAV, FLAC, OGG, M4A.
 */
static int est_fichier_audio(const char *nom_fichier) {
    const char *ext = strrchr(nom_fichier, '.');
    if (!ext) return 0;
    if (strcasecmp(ext, ".mp3") == 0) return 1;
    if (strcasecmp(ext, ".wav") == 0) return 1;
    if (strcasecmp(ext, ".flac") == 0) return 1;
    if (strcasecmp(ext, ".ogg") == 0) return 1;
    if (strcasecmp(ext, ".m4a") == 0) return 1;
    return 0;
}

/**
 * @brief Analyse le nom du fichier pour deviner l'artiste et le titre.
 * Utilisé en Fallback si les métadonnées ID3 sont absentes[cite: 79].
 */
static void extraire_infos_fallback(const char *filename, Musique *m) {
    char temp[256];
    snprintf(temp, 256, "%s", filename);
    
    char *dot = strrchr(temp, '.');
    if (dot) *dot = '\0';

    char *sep = strstr(temp, " - ");
    if (sep) {
        *sep = '\0'; 
        if (strlen(m->artiste) == 0 || strcmp(m->artiste, "Inconnu") == 0) 
            snprintf(m->artiste, 256, "%s", temp);
        if (strlen(m->titre) == 0 || strcmp(m->titre, "Inconnu") == 0)
            snprintf(m->titre, 256, "%s", sep + 3); 
    } else {
        if (strlen(m->titre) == 0 || strcmp(m->titre, "Inconnu") == 0)
            snprintf(m->titre, 256, "%s", temp);
    }
}

/**
 * @brief Utilise GStreamer Discoverer pour extraire les tags techniques et ID3.
 * Gère la durée, le titre, l'artiste, l'album, le genre et l'année[cite: 78].
 */
static void extraire_tags_gstreamer(const char *path, Musique *m) {
    GError *err = NULL;
    GstDiscoverer *dc;
    GstDiscovererInfo *info;
    
    if (!gst_is_initialized()) gst_init(NULL, NULL);

    dc = gst_discoverer_new(5 * GST_SECOND, &err);
    if (!dc) return;

    // Conversion sécurisée du chemin en URI file://
    char *uri = g_filename_to_uri(path, NULL, NULL);
    if (!uri) {
        g_object_unref(dc);
        return;
    }

    info = gst_discoverer_discover_uri(dc, uri, &err);
    g_free(uri); // Libération immédiate après usage

    if (!info) {
        g_object_unref(dc);
        return;
    }

    // Extraction de la durée en secondes
    GstClockTime dur = gst_discoverer_info_get_duration(info);
    if (dur != GST_CLOCK_TIME_NONE) {
        m->duree = (int)(dur / GST_SECOND);
    }

    const GstTagList *tags = gst_discoverer_info_get_tags(info);
    if (tags) {
        char *str_val = NULL;
        
        // Extraction sécurisée des chaînes de caractères avec libération g_free
        if (gst_tag_list_get_string(tags, GST_TAG_TITLE, &str_val) && str_val) {
            snprintf(m->titre, 256, "%s", str_val);
            g_free(str_val);
        }
        if (gst_tag_list_get_string(tags, GST_TAG_ARTIST, &str_val) && str_val) {
            snprintf(m->artiste, 256, "%s", str_val);
            g_free(str_val);
        }
        if (gst_tag_list_get_string(tags, GST_TAG_ALBUM, &str_val) && str_val) {
            snprintf(m->album, 256, "%s", str_val);
            g_free(str_val);
        }
        if (gst_tag_list_get_string(tags, GST_TAG_GENRE, &str_val) && str_val) {
            snprintf(m->genre, 100, "%s", str_val);
            g_free(str_val);
        }

        GstDateTime *dt = NULL;
        if (gst_tag_list_get_date_time(tags, GST_TAG_DATE_TIME, &dt)) {
            m->annee = gst_date_time_get_year(dt);
            gst_date_time_unref(dt);
        }
        
        guint track_num = 0;
        if (gst_tag_list_get_uint(tags, GST_TAG_TRACK_NUMBER, &track_num)) {
            m->numero_piste = (int)track_num;
        }
    }

    // Libération des ressources GStreamer [cite: 180]
    gst_discoverer_info_unref(info);
    g_object_unref(dc);
}

/**
 * @brief Parcourt récursivement un répertoire pour importer les fichiers audio.
 * Gère l'insertion en base de données et la détection des doublons[cite: 72, 95].
 */
void scanner_dossier(const char *base_path, int *added, int *duplicates) {
    DIR *dir;
    struct dirent *entry;
    char path[1024];

    if (!(dir = opendir(base_path))) return;

    while ((entry = readdir(dir)) != NULL) {
        // Ignorer les répertoires parents et actuels
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);
        struct stat info;
        if (stat(path, &info) != 0) continue;

        if (S_ISDIR(info.st_mode)) {
            scanner_dossier(path, added, duplicates); // Récursion pour sous-dossiers
        } 
        else if (S_ISREG(info.st_mode) && est_fichier_audio(entry->d_name)) {
            
            Musique m;
            // Initialisation avec des valeurs par défaut [cite: 79]
            snprintf(m.chemin, 512, "%s", path);
            snprintf(m.titre, 256, "Inconnu");
            snprintf(m.artiste, 256, "Inconnu");
            snprintf(m.album, 256, "Inconnu");
            snprintf(m.genre, 100, "Inconnu");
            m.duree = 0;
            m.annee = 0;
            m.numero_piste = 0;
            m.est_favori = 0;

            extraire_tags_gstreamer(path, &m);

            if (strcmp(m.titre, "Inconnu") == 0) {
                extraire_infos_fallback(entry->d_name, &m);
            }

            // Persistence via la couche DAO [cite: 49, 154]
            AppError res = ajouter_musique(m);
            if (res == SUCCESS) {
                (*added)++;
                printf("[IMPORT] Ajouté : %s - %s (%ds)\n", m.artiste, m.titre, m.duree);
            } else if (res == ERR_DB_DUPLICATE) {
                (*duplicates)++; // Gestion des doublons 
            }
        }
    }
    closedir(dir);
}