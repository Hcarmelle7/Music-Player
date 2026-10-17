/**
 * @file dao_paroles.c
 * @brief DAO pour la gestion des paroles (Karaoké).
 * Valide la 4ème table obligatoire et assure la persistance des textes.
 */

#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include "dao_paroles.h"
#include "db_manager.h"

/**
 * @brief Sauvegarde ou met à jour les paroles d'une musique (Logic UPSERT).
 * @param id_musique Identifiant de la musique liée.
 * @param texte Contenu du fichier .txt importé.
 * @return SUCCESS ou ERR_DB_QUERY.
 */
AppError sauvegarder_paroles(int id_musique, const char *texte) {
    // Utilisation de INSERT OR REPLACE pour gérer la mise à jour automatique
    const char *sql = "INSERT OR REPLACE INTO paroles (id_musique, texte) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        return ERR_DB_QUERY;
    }
    
    // Liaison sécurisée des paramètres (Protection contre injection SQL)
    sqlite3_bind_int(stmt, 1, id_musique);
    sqlite3_bind_text(stmt, 2, texte, -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt); // Libération impérative des ressources
    
    return (rc == SQLITE_DONE) ? SUCCESS : ERR_DB_QUERY;
}

/**
 * @brief Récupère les paroles associées à un ID de musique.
 * @param buffer_texte Buffer de sortie pour le texte.
 * @param taille_max Capacité maximale du buffer (Sécurité ANSSI).
 * @return 1 si trouvé, 0 sinon.
 */
int recuperer_paroles(int id_musique, char *buffer_texte, int taille_max) {
    const char *sql = "SELECT texte FROM paroles WHERE id_musique = ?;";
    sqlite3_stmt *stmt;
    int trouve = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        return 0;
    }
    
    sqlite3_bind_int(stmt, 1, id_musique);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *txt = sqlite3_column_text(stmt, 0);
        if (txt) {
            // Utilisation de snprintf pour garantir le respect de la taille du buffer
            snprintf(buffer_texte, taille_max, "%s", (char*)txt);
            trouve = 1;
        }
    }
    
    sqlite3_finalize(stmt);
    return trouve;
}