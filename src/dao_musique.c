#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "dao_musique.h"
#include "db_manager.h"
#include "error_codes.h"

// Vérifie si une musique existe déjà (pour la détection des doublons) [cite: 72-75]
int musique_existe_deja(const char *titre, const char *artiste, int duree) {
    const char *sql = "SELECT id FROM musiques WHERE titre = ? AND artiste = ? AND duree = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return 0;
    
    sqlite3_bind_text(stmt, 1, titre, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, artiste, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, duree);
    
    int existe = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return existe;
}

// Ajoute une musique avec gestion d'erreur explicite [cite: 146-147]
AppError ajouter_musique(Musique m) {
    if (musique_existe_deja(m.titre, m.artiste, m.duree)) return ERR_DB_DUPLICATE;

    const char *sql = "INSERT INTO musiques (titre, artiste, album, genre, duree, annee, numero_piste, chemin, favori) VALUES (?, ?, ?, ?, ?, ?, ?, ?, 0);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return ERR_DB_QUERY;

    sqlite3_bind_text(stmt, 1, m.titre, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, m.artiste, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, m.album, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, m.genre, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, m.duree);
    sqlite3_bind_int(stmt, 6, m.annee);
    sqlite3_bind_int(stmt, 7, m.numero_piste);
    sqlite3_bind_text(stmt, 8, m.chemin, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? SUCCESS : ERR_DB_QUERY;
}

// Récupération triée pour l'écran catalogue [cite: 67-70, 130]
void recuperer_musiques_triees(int type_tri, Musique *tableau, int *nb_resultats) {
    const char *sql;
    switch(type_tri) {
        case TRI_ARTISTE: sql = "SELECT * FROM musiques ORDER BY artiste ASC, titre ASC;"; break;
        case TRI_TITRE:   sql = "SELECT * FROM musiques ORDER BY titre ASC;"; break;
        case TRI_ALBUM:   sql = "SELECT * FROM musiques ORDER BY album ASC, numero_piste ASC;"; break;
        case TRI_GENRE:   sql = "SELECT * FROM musiques ORDER BY genre ASC, artiste ASC;"; break;
        case TRI_DUREE:   sql = "SELECT * FROM musiques ORDER BY duree DESC;"; break;
        case TRI_RECENT:  sql = "SELECT * FROM musiques ORDER BY id DESC;"; break;
        default:          sql = "SELECT * FROM musiques ORDER BY titre ASC;"; break;
    }
    sqlite3_stmt *stmt;
    *nb_resultats = 0;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Musique *m = &tableau[*nb_resultats];
        m->id = sqlite3_column_int(stmt, 0);
        const unsigned char *txt;
        txt = sqlite3_column_text(stmt, 1); snprintf(m->titre, 256, "%s", txt ? (char*)txt : "");
        txt = sqlite3_column_text(stmt, 2); snprintf(m->artiste, 256, "%s", txt ? (char*)txt : "");
        txt = sqlite3_column_text(stmt, 3); snprintf(m->album, 256, "%s", txt ? (char*)txt : "");
        txt = sqlite3_column_text(stmt, 4); snprintf(m->genre, 100, "%s", txt ? (char*)txt : "");
        m->duree = sqlite3_column_int(stmt, 5);
        m->annee = sqlite3_column_int(stmt, 6);
        m->numero_piste = sqlite3_column_int(stmt, 7);
        txt = sqlite3_column_text(stmt, 8); snprintf(m->chemin, 512, "%s", txt ? (char*)txt : "");
        m->est_favori = sqlite3_column_int(stmt, 9);
        (*nb_resultats)++;
    }
    sqlite3_finalize(stmt);
}

// Recherche multicritères [cite: 70]
void rechercher_musiques(const char *mot_cle, Musique *tableau, int *nb_resultats) {
    const char *sql = "SELECT * FROM musiques WHERE titre LIKE ? OR artiste LIKE ?;";
    sqlite3_stmt *stmt;
    *nb_resultats = 0;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return;
    
    char pattern[300]; 
    snprintf(pattern, 300, "%%%s%%", mot_cle);
    
    sqlite3_bind_text(stmt, 1, pattern, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, pattern, -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Musique *m = &tableau[*nb_resultats];
        m->id = sqlite3_column_int(stmt, 0);
        const unsigned char *txt;
        txt = sqlite3_column_text(stmt, 1); snprintf(m->titre, 256, "%s", txt ? (char*)txt : "");
        txt = sqlite3_column_text(stmt, 2); snprintf(m->artiste, 256, "%s", txt ? (char*)txt : "");
        txt = sqlite3_column_text(stmt, 3); snprintf(m->album, 256, "%s", txt ? (char*)txt : "");
        txt = sqlite3_column_text(stmt, 4); snprintf(m->genre, 100, "%s", txt ? (char*)txt : "");
        m->duree = sqlite3_column_int(stmt, 5);
        m->annee = sqlite3_column_int(stmt, 6);
        m->numero_piste = sqlite3_column_int(stmt, 7);
        txt = sqlite3_column_text(stmt, 8); snprintf(m->chemin, 512, "%s", txt ? (char*)txt : "");
        m->est_favori = sqlite3_column_int(stmt, 9);
        (*nb_resultats)++;
    }
    sqlite3_finalize(stmt);
}

// Suppression et intégrité référentielle [cite: 66, 154]
void supprimer_musique(int id) {
    const char *sql1 = "DELETE FROM composition WHERE id_musique = ?;";
    sqlite3_stmt *stmt1;
    if (sqlite3_prepare_v2(db, sql1, -1, &stmt1, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt1, 1, id);
        sqlite3_step(stmt1);
        sqlite3_finalize(stmt1);
    }
    const char *sql2 = "DELETE FROM musiques WHERE id = ?;";
    sqlite3_stmt *stmt2;
    if (sqlite3_prepare_v2(db, sql2, -1, &stmt2, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt2, 1, id);
        sqlite3_step(stmt2);
        sqlite3_finalize(stmt2);
    }
}

// Mise à jour des favoris [cite: 156]
void basculer_favori(int id_musique, int etat) {
    const char *sql = "UPDATE musiques SET favori = ? WHERE id = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, etat);
        sqlite3_bind_int(stmt, 2, id_musique);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

// Récupération par artiste (écran Artistes) [cite: 131]
void recuperer_musiques_par_artiste(const char *artiste, Musique *tableau, int *nb_resultats) {
    const char *sql = "SELECT * FROM musiques WHERE artiste = ?;";
    sqlite3_stmt *stmt;
    *nb_resultats = 0;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, artiste, -1, SQLITE_STATIC);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Musique *m = &tableau[*nb_resultats];
        m->id = sqlite3_column_int(stmt, 0);
        const unsigned char *txt;
        txt = sqlite3_column_text(stmt, 1); snprintf(m->titre, 256, "%s", txt ? (char*)txt : "");
        txt = sqlite3_column_text(stmt, 2); snprintf(m->artiste, 256, "%s", txt ? (char*)txt : "");
        txt = sqlite3_column_text(stmt, 3); snprintf(m->album, 256, "%s", txt ? (char*)txt : "");
        txt = sqlite3_column_text(stmt, 4); snprintf(m->genre, 100, "%s", txt ? (char*)txt : "");
        m->duree = sqlite3_column_int(stmt, 5);
        m->annee = sqlite3_column_int(stmt, 6);
        m->numero_piste = sqlite3_column_int(stmt, 7);
        txt = sqlite3_column_text(stmt, 8); snprintf(m->chemin, 512, "%s", txt ? (char*)txt : "");
        m->est_favori = sqlite3_column_int(stmt, 9);
        (*nb_resultats)++;
    }
    sqlite3_finalize(stmt);
}

// Modification manuelle des métadonnées (La fonction qui manquait !) [cite: 66, 80, 82]
AppError modifier_infos_musique(int id, const char *nouveau_titre, const char *nouvel_artiste, const char *nouvel_album) {
    const char *sql = "UPDATE musiques SET titre = ?, artiste = ?, album = ? WHERE id = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return ERR_DB_QUERY;
    
    sqlite3_bind_text(stmt, 1, nouveau_titre, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, nouvel_artiste, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, nouvel_album, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? SUCCESS : ERR_DB_QUERY;
}