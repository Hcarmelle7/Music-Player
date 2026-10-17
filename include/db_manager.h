#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <sqlite3.h>
#include "error_codes.h"

// Variable globale pour la connexion BDD
extern sqlite3 *db;

// Initialise la connexion et crée les tables si nécessaire
AppError initialiser_bd();

// Ferme la connexion proprement
void fermer_bd();

#endif