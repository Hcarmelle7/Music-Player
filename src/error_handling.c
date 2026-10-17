#include "error_codes.h"

const char* get_error_message(AppError err) {
    switch (err) {
        case SUCCESS: return "Succès.";
        
        case ERR_FILE_NOT_FOUND: return "Fichier introuvable.";
        case ERR_FILE_ACCESS_DENIED: return "Accès refusé.";
        case ERR_FILE_INVALID_FORMAT: return "Format fichier invalide.";
        
        case ERR_AUDIO_INIT: return "Erreur init audio.";
        case ERR_AUDIO_STREAM: return "Erreur flux audio.";
        case ERR_AUDIO_PLAYBACK: return "Erreur lecture.";
        case ERR_AUDIO_INVALID_FORMAT: return "Le format audio n'est pas supporté par le lecteur."; // <--- AJOUT ICI
        
        case ERR_DB_OPEN: return "Erreur ouverture BDD.";
        case ERR_DB_QUERY: return "Erreur requête SQL.";
        case ERR_DB_DUPLICATE: return "Ce fichier est déjà dans la bibliothèque.";
        
        default: return "Erreur inconnue.";
    }
}