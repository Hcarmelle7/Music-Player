#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include "ihm.h"
#include "db_manager.h"

int main(int argc, char **argv) {
    // 1. Initialisation de la Base de Données
    if (initialiser_bd() != 0) {
        fprintf(stderr, "Erreur fatale : Impossible d'initialiser la base de données.\n");
        return -1;
    }

    // 2. Lancement de l'application GTK
    GtkApplication *app;
    int status;

    // Création de l'application
    app = gtk_application_new("org.music.app", G_APPLICATION_DEFAULT_FLAGS);
    
    // Connexion du signal d'activation (défini dans ihm.c)
    g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), NULL);
    
    // Exécution de la boucle principale
    status = g_application_run(G_APPLICATION(app), argc, argv);
    
    // Nettoyage GTK
    g_object_unref(app);

    // 3. Fermeture propre de la BDD
    fermer_bd();

    return status;
}