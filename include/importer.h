#ifndef IMPORTER_H
#define IMPORTER_H

// Scanner un dossier et ajouter les musiques à la BDD
// chemin_dossier : le chemin à scanner
// added : pointeur vers un entier pour compter les ajouts (succès)
// duplicates : pointeur vers un entier pour compter les doublons (fichiers déjà présents)
void scanner_dossier(const char *chemin_dossier, int *added, int *duplicates);

#endif