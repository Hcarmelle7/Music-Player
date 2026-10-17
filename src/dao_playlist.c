#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "dao_playlist.h"
#include "db_manager.h" // Pour la variable 'db'

// Retourne l'ID de la playlist créée (ou -1 si erreur)
int creer_playlist_et_recuperer_id(const char *nom) {
    char *sql = "INSERT INTO playlists (nom) VALUES (?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return -1;
    
    sqlite3_bind_text(stmt, 1, nom, -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_DONE) {
        return (int)sqlite3_last_insert_rowid(db); // Retourne l'ID généré
    }
    return -1;
}

AppError supprimer_playlist(int id_playlist) {
    // 1. Supprimer les liaisons
    char *sql1 = "DELETE FROM composition WHERE id_playlist = ?;";
    sqlite3_stmt *stmt1;
    sqlite3_prepare_v2(db, sql1, -1, &stmt1, 0);
    sqlite3_bind_int(stmt1, 1, id_playlist);
    sqlite3_step(stmt1);
    sqlite3_finalize(stmt1);

    // 2. Supprimer la playlist
    char *sql2 = "DELETE FROM playlists WHERE id = ?;";
    sqlite3_stmt *stmt2;
    if (sqlite3_prepare_v2(db, sql2, -1, &stmt2, 0) != SQLITE_OK) return ERR_DB_QUERY;
    sqlite3_bind_int(stmt2, 1, id_playlist);
    int rc = sqlite3_step(stmt2);
    sqlite3_finalize(stmt2);
    
    return (rc == SQLITE_DONE) ? SUCCESS : ERR_DB_QUERY;
}

AppError renommer_playlist(int id_playlist, const char *nouveau_nom) {
    char *sql = "UPDATE playlists SET nom = ? WHERE id = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return ERR_DB_QUERY;
    
    sqlite3_bind_text(stmt, 1, nouveau_nom, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id_playlist);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? SUCCESS : ERR_DB_QUERY;
}

void recuperer_toutes_les_playlists(Playlist *tableau, int *nb_resultats) {
    char *sql = "SELECT id, nom FROM playlists ORDER BY nom ASC;";
    sqlite3_stmt *stmt;
    *nb_resultats = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tableau[*nb_resultats].id = sqlite3_column_int(stmt, 0);
        const unsigned char *txt = sqlite3_column_text(stmt, 1);
        snprintf(tableau[*nb_resultats].nom, 256, "%s", txt ? (char*)txt : "Sans nom");
        (*nb_resultats)++;
    }
    sqlite3_finalize(stmt);
}

AppError ajouter_musique_playlist(int id_playlist, int id_musique) {
    char *sql = "INSERT OR IGNORE INTO composition (id_playlist, id_musique) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return ERR_DB_QUERY;
    
    sqlite3_bind_int(stmt, 1, id_playlist);
    sqlite3_bind_int(stmt, 2, id_musique);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? SUCCESS : ERR_DB_QUERY;
}

AppError retirer_musique_playlist(int id_playlist, int id_musique) {
    char *sql = "DELETE FROM composition WHERE id_playlist = ? AND id_musique = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return ERR_DB_QUERY;
    
    sqlite3_bind_int(stmt, 1, id_playlist);
    sqlite3_bind_int(stmt, 2, id_musique);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? SUCCESS : ERR_DB_QUERY;
}

void recuperer_musiques_playlist(int id_playlist, Musique *tableau, int *nb_resultats) {
    char *sql = "SELECT m.id, m.titre, m.artiste, m.album, m.genre, m.duree, m.annee, m.numero_piste, m.chemin, m.favori "
                "FROM musiques m "
                "JOIN composition c ON m.id = c.id_musique "
                "WHERE c.id_playlist = ?;";
    sqlite3_stmt *stmt;
    *nb_resultats = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return;
    
    sqlite3_bind_int(stmt, 1, id_playlist);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Musique *m = &tableau[*nb_resultats];
        m->id = sqlite3_column_int(stmt, 0);
        const unsigned char *txt;
        txt = sqlite3_column_text(stmt, 1); snprintf(m->titre, 100, "%s", txt);
        txt = sqlite3_column_text(stmt, 2); snprintf(m->artiste, 100, "%s", txt);
        txt = sqlite3_column_text(stmt, 3); snprintf(m->album, 100, "%s", txt ? (char*)txt : "");
        txt = sqlite3_column_text(stmt, 4); snprintf(m->genre, 50, "%s", txt ? (char*)txt : "");
        m->duree = sqlite3_column_int(stmt, 5);
        m->annee = sqlite3_column_int(stmt, 6);
        m->numero_piste = sqlite3_column_int(stmt, 7);
        txt = sqlite3_column_text(stmt, 8); snprintf(m->chemin, 512, "%s", txt);
        m->est_favori = sqlite3_column_int(stmt, 9);
        (*nb_resultats)++;
    }
    sqlite3_finalize(stmt);
}