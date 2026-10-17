#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <sys/stat.h> // Pour mkdir
#include "db_manager.h"
#include "error_codes.h"

sqlite3 *db = NULL;

AppError initialiser_bd() {
    // Création du dossier db s'il n'existe pas
    #ifdef _WIN32
        _mkdir("db");
    #else
        mkdir("db", 0777);
    #endif

    int rc = sqlite3_open("db/music_catalog.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Impossible d'ouvrir la base de données: %s\n", sqlite3_errmsg(db));
        return ERR_DB_OPEN;
    }

    char *err_msg = 0;
    const char *sql;

    // 1. Table Musiques
    sql = "CREATE TABLE IF NOT EXISTS musiques ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
          "titre TEXT,"
          "artiste TEXT,"
          "album TEXT,"
          "genre TEXT,"
          "duree INTEGER,"
          "annee INTEGER,"
          "numero_piste INTEGER,"
          "chemin TEXT UNIQUE,"
          "favori INTEGER DEFAULT 0);";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erreur SQL (Musiques): %s\n", err_msg);
        sqlite3_free(err_msg);
        return ERR_DB_QUERY;
    }

    // 2. Table Playlists
    sql = "CREATE TABLE IF NOT EXISTS playlists ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
          "nom TEXT UNIQUE);";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erreur SQL (Playlists): %s\n", err_msg);
        sqlite3_free(err_msg);
        return ERR_DB_QUERY;
    }

    // 3. Table Composition (Liaison Many-to-Many Playlists <-> Musiques)
    sql = "CREATE TABLE IF NOT EXISTS composition ("
          "id_playlist INTEGER,"
          "id_musique INTEGER,"
          "PRIMARY KEY (id_playlist, id_musique),"
          "FOREIGN KEY(id_playlist) REFERENCES playlists(id),"
          "FOREIGN KEY(id_musique) REFERENCES musiques(id));";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erreur SQL (Composition): %s\n", err_msg);
        sqlite3_free(err_msg);
        return ERR_DB_QUERY;
    }

    // 4. Table Paroles (Karaoké) - Le Défi
    sql = "CREATE TABLE IF NOT EXISTS paroles ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
          "id_musique INTEGER UNIQUE,"
          "texte TEXT,"
          "FOREIGN KEY(id_musique) REFERENCES musiques(id));";
    
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erreur SQL (Paroles): %s\n", err_msg);
        sqlite3_free(err_msg);
        return ERR_DB_QUERY;
    }

    printf("Base de données initialisée avec succès (4 tables).\n");
    return SUCCESS;
}

void fermer_bd() {
    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
}