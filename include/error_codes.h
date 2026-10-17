#ifndef ERROR_CODES_H
#define ERROR_CODES_H

typedef enum {
    SUCCESS = 0,
    
    // Erreurs Fichiers
    ERR_FILE_NOT_FOUND = -10,
    ERR_FILE_ACCESS_DENIED = -11,
    ERR_FILE_INVALID_FORMAT = -12,
    
    // Erreurs Audio
    ERR_AUDIO_INIT = -30,
    ERR_AUDIO_STREAM = -31,
    ERR_AUDIO_PLAYBACK = -32,
    ERR_AUDIO_INVALID_FORMAT = -33, // <--- AJOUT ICI
    
    // Erreurs BDD
    ERR_DB_OPEN = -20,
    ERR_DB_QUERY = -21,
    ERR_DB_DUPLICATE = -22
    
} AppError;

const char* get_error_message(AppError err);

#endif