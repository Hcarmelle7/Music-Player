#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h> // Pour unlink (suppression fichier)
#include "db_manager.h"
#include "dao_musique.h"
#include "dao_playlist.h"
#include "structures.h"
#include "error_codes.h"

// Macro pour afficher les succès en vert
#define TEST_PASS(msg) printf("\033[0;32m[PASS] %s\033[0m\n", msg)
#define TEST_FAIL(msg) printf("\033[0;31m[FAIL] %s\033[0m\n", msg)

void test_initialisation_bd() {
    // On supprime l'ancienne BDD de test pour partir propre
    unlink("db/test_catalog.db");
    
    // Attention: Il faut modifier temporairement le chemin dans db_manager.c 
    // ou accepter de tester sur la base réelle (ici on teste la connexion)
    AppError res = initialiser_bd();
    assert(res == SUCCESS);
    TEST_PASS("Initialisation BDD");
}

void test_ajout_musique() {
    Musique m;
    snprintf(m.titre, 256, "Test Song");
    snprintf(m.artiste, 256, "Test Artist");
    snprintf(m.album, 256, "Test Album");
    snprintf(m.genre, 100, "Rock");
    snprintf(m.chemin, 512, "/tmp/test.mp3"); // Fictif
    m.duree = 120;
    m.annee = 2024;
    m.numero_piste = 1;
    m.est_favori = 0;

    AppError res = ajouter_musique(m);
    assert(res == SUCCESS);
    TEST_PASS("Ajout Musique");

    // Test doublon
    res = ajouter_musique(m);
    assert(res == ERR_DB_DUPLICATE);
    TEST_PASS("Détection Doublon");
}

void test_playlist() {
    int id = creer_playlist_et_recuperer_id("Ma Playlist Test");
    assert(id > 0);
    TEST_PASS("Création Playlist");

    // Ajout d'une musique fictive (ID 1, car on vient d'en ajouter une au test précédent)
    AppError res = ajouter_musique_playlist(id, 1);
    assert(res == SUCCESS);
    TEST_PASS("Ajout Musique dans Playlist");
}

int main() {
    printf("=== DÉBUT DES TESTS UNITAIRES ===\n");
    
    test_initialisation_bd();
    test_ajout_musique();
    test_playlist();
    
    fermer_bd();
    
    printf("=== TOUS LES TESTS SONT PASSÉS ===\n");
    return 0;
}