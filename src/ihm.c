#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "ihm.h"
#include "dao_musique.h"
#include "dao_playlist.h"
#include "dao_paroles.h"
#include "importer.h"
#include "structures.h"
#include "audio_engine.h"
#include "error_codes.h"

// --- PROTOTYPES ---
void basculer_favori(int id_musique, int etat);
int64_t obtenir_duree_totale_sec();
int64_t obtenir_temps_actuel_sec();
void definir_progression(double pourcentage);
gboolean est_en_lecture();
void reprendre_lecture();
void mettre_en_pause();
void arreter_musique();
float obtenir_progression();

void rafraichir_contenu_onglet();
void afficher_catalogue();
void afficher_lecteur();
void update_ui_play_state(gboolean playing);
void formater_temps(int64_t secondes, char *buffer);
void on_import_clicked(GtkButton *btn, gpointer user_data);
void on_musique_click(GtkListBox *box, GtkListBoxRow *row, gpointer user_data);
void afficher_erreur_ui(AppError err);
void on_coeur_clicked(GtkButton *btn, gpointer user_data);
void charger_liste_lecteur_queue(); 
// --- AJOUT DES PROTOTYPES (Pour éviter les erreurs "undeclared") ---
void on_scale_change_value(GtkRange *range, gpointer d);
void mettre_a_jour_affichage_karaoke();
int charger_paroles_pour_lecteur(int id_musique);


// --- VARIABLES GLOBALES ---
GtkWidget *window_principale;
GtkWidget *stack_principal; 
GtkWidget *stack_overlay;   
// --- AJOUT DES VARIABLES GLOBALES MANQUANTES ---
GtkWidget *btn_mini_prev;
GtkWidget *btn_mini_next;
GtkTextTag *tag_rose;


// Conteneurs
GtkWidget *box_catalogue_liste;
GtkWidget *flowbox_artistes;
GtkWidget *box_favoris_liste;
GtkWidget *box_playlists_liste; 
GtkWidget *box_playlist_content; 
GtkWidget *box_artiste_content; 
GtkWidget *stack_playlist; 
GtkWidget *stack_artiste; 
GtkWidget *barre_recherche;

// Navigation
static int playlist_ouverte_id = -1; 
static char artiste_ouvert_nom[100] = "";
static int mode_tri_actuel = 0; // Par défaut Titre

// Mini Player & Nav
GtkWidget *mini_player_box;
GtkWidget *nav_bar_box;
GtkWidget *label_mini_titre;
GtkWidget *label_mini_artiste;
GtkWidget *btn_mini_play;

// Grand Lecteur
GtkWidget *label_lecteur_titre;
GtkWidget *label_lecteur_artiste;
GtkWidget *scale_progression;
GtkWidget *label_temps_cur;
GtkWidget *label_temps_tot;
GtkWidget *btn_lecteur_play;
GtkWidget *btn_lecteur_coeur;
GtkWidget *btn_lecteur_shuffle;
GtkWidget *box_lecteur_queue;
GtkWidget *text_view_paroles; 
GtkWidget *stack_lecteur_droite; 

// États
static int onglet_actif = 0; 
static gboolean is_sliding = FALSE;
static gboolean mode_aleatoire = FALSE;

// --- CSS MODERNE & VIBRANT ---
void charger_css() {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css = 
        "window { background-color: #f0f2f5; color: #2c3e50; font-family: 'Segoe UI', sans-serif; }"
        ".nav-bar { background-color: #ffffff; padding: 12px; border-bottom: 1px solid rgba(0,0,0,0.05); box-shadow: 0 2px 15px rgba(0,0,0,0.03); }"
        "button.nav-btn { background: transparent; color: #78909c; font-weight: 700; padding: 8px 20px; border-radius: 25px; border: none; transition: all 0.3s ease; }"
        "button.nav-btn:hover { background-color: #fff0f5; color: #d81b60; transform: translateY(-1px); }"
        "button.nav-btn-active { background: linear-gradient(135deg, #ec407a 0%, #c2185b 100%); color: white; box-shadow: 0 4px 12px rgba(233, 30, 99, 0.35); }"
        ".mini-player { background-color: #ffffff; border-radius: 16px; margin: 15px; padding: 10px; border: 1px solid #f0f0f0; border-bottom: 3px solid #ec407a; box-shadow: 0 5px 20px rgba(0,0,0,0.08); }"
        ".mini-titre { font-weight: 800; font-size: 14px; color: #2c3e50; }"
        ".mini-artiste { color: #d81b60; font-size: 12px; font-weight: 600; }"
        ".card-artiste { background-color: #ffffff; border-radius: 14px; font-weight: 700; color: #455a64; padding: 15px; border: 1px solid transparent; transition: all 0.3s cubic-bezier(0.25, 0.8, 0.25, 1); box-shadow: 0 3px 10px rgba(0,0,0,0.04); }"
        ".card-artiste:hover { transform: translateY(-4px); box-shadow: 0 8px 25px rgba(236, 64, 122, 0.25); border-color: #f8bbd0; color: #c2185b; background: linear-gradient(to bottom right, #ffffff, #fff0f5); }"
        ".lecteur-bg { background: linear-gradient(180deg, #ffffff 0%, #fce4ec 60%, #f3e5f5 100%); }"
        ".title-big { font-size: 28px; font-weight: 900; color: #1a237e; margin-bottom: 5px; letter-spacing: -0.5px; }"
        ".artist-big { font-size: 18px; color: #d81b60; font-weight: 600; margin-bottom: 25px; opacity: 0.9; }"
        ".list-row { background-color: #ffffff; border-bottom: 1px solid #f5f5f5; padding: 10px; margin: 4px 10px; border-radius: 10px; transition: background-color 0.2s; box-shadow: 0 1px 3px rgba(0,0,0,0.02); }"
        ".list-row:hover { background-color: #fff5f8; box-shadow: 0 2px 8px rgba(233, 30, 99, 0.08); }"
        ".list-row:selected { background-color: #fce4ec; border-left: 4px solid #ec407a; }"
        ".titre-song { font-weight: 700; font-size: 15px; color: #37474f; }"
        ".artiste-song { font-size: 13px; color: #78909c; }"
        "button.btn-circle { background: linear-gradient(135deg, #ec407a 0%, #d81b60 100%); color: white; border-radius: 50%; padding: 0; border: none; box-shadow: 0 6px 15px rgba(216, 27, 96, 0.4); transition: all 0.2s; }"
        "button.btn-circle:hover { transform: scale(1.05); box-shadow: 0 8px 20px rgba(216, 27, 96, 0.5); }"
        "button.btn-circle:active { transform: scale(0.95); box-shadow: 0 3px 8px rgba(216, 27, 96, 0.6); }"
        "button.btn-icon { background: none; border: none; color: #546e7a; transition: color 0.2s; }" 
        "button.btn-icon:hover { color: #d81b60; background-color: rgba(236, 64, 122, 0.1); border-radius: 50%; }"
        ".favori-active { color: #e91e63; filter: drop-shadow(0 2px 4px rgba(233, 30, 99, 0.3)); }" 
        ".shuffle-active { color: #e91e63; font-weight: bold; background-color: #fff0f5; border-radius: 50%; box-shadow: inset 0 0 0 1px #ec407a; }"
        "entry { background-color: #ffffff; color: #37474f; border: 2px solid #eceff1; border-radius: 30px; padding: 8px 15px; box-shadow: 0 2px 5px rgba(0,0,0,0.03); transition: all 0.3s; }"
        "entry:focus { border-color: #ec407a; box-shadow: 0 4px 12px rgba(236, 64, 122, 0.15); }"
        "entry selection { background-color: #fce4ec; color: #d81b60; }"
        ".list-queue-row { background: rgba(255,255,255,0.6); margin: 2px 5px; border-radius: 6px; font-size: 13px; }"
        ".list-queue-row:selected { background: #ffffff; color: #d81b60; font-weight: 700; box-shadow: 0 2px 8px rgba(0,0,0,0.05); }"
        "textview { font-family: 'Monospace'; font-size: 14px; color: #333; background-color: rgba(255,255,255,0.5); }"
        ;
    gtk_css_provider_load_from_string(provider, css);
    gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

// --- UTILITAIRES ---
void afficher_erreur_ui(AppError err) {
    if (err == SUCCESS) return;
    const char *msg = get_error_message(err);
    GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(window_principale), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", msg);
    g_signal_connect(d, "response", G_CALLBACK(gtk_window_destroy), NULL);
    gtk_widget_set_visible(d, TRUE);
}

void formater_temps(int64_t secondes, char *buffer) {
    int m = secondes / 60; int s = secondes % 60;
    snprintf(buffer, 10, "%02d:%02d", m, s);
}

// --- CONSTRUCTEUR DE LIGNE (Commun) ---
GtkWidget* creer_ligne_musique_simple(Musique m) {
    GtkWidget *row = gtk_list_box_row_new();
    GtkWidget *lbl = gtk_label_new(NULL);
    char buf[300]; snprintf(buf, 300, "%s - %s", m.titre, m.artiste);
    gtk_label_set_text(GTK_LABEL(lbl), buf);
    gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    gtk_widget_set_margin_start(lbl, 10);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), lbl);
    
    g_object_set_data_full(G_OBJECT(row), "chemin", g_strdup(m.chemin), g_free);
    g_object_set_data_full(G_OBJECT(row), "titre", g_strdup(m.titre), g_free);
    g_object_set_data_full(G_OBJECT(row), "artiste", g_strdup(m.artiste), g_free);
    g_object_set_data(G_OBJECT(row), "id", GINT_TO_POINTER(m.id));
    g_object_set_data(G_OBJECT(row), "fav", GINT_TO_POINTER(m.est_favori));
    return row;
}

// --- GESTION LECTEUR & FILE D'ATTENTE ---
void charger_liste_lecteur_queue() {
    GtkWidget *child; 
    while ((child = gtk_widget_get_first_child(box_lecteur_queue))) 
        gtk_list_box_remove(GTK_LIST_BOX(box_lecteur_queue), child);

    Musique tab[200]; int nb = 0;

    if (onglet_actif == 0) recuperer_musiques_triees(mode_tri_actuel, tab, &nb); // UTILISATION DU TRI
    else if (onglet_actif == 1 && strlen(artiste_ouvert_nom) > 0) recuperer_musiques_par_artiste(artiste_ouvert_nom, tab, &nb);
    else if (onglet_actif == 2) {
        recuperer_musiques_triees(mode_tri_actuel, tab, &nb); // UTILISATION DU TRI
        int count = 0; Musique favs[200];
        for(int i=0; i<nb; i++) if(tab[i].est_favori) favs[count++] = tab[i];
        nb = count; memcpy(tab, favs, sizeof(Musique)*nb);
    }
    else if (onglet_actif == 3 && playlist_ouverte_id > 0) recuperer_musiques_playlist(playlist_ouverte_id, tab, &nb);

    for (int i=0; i<nb; i++) {
        GtkWidget *row = creer_ligne_musique_simple(tab[i]);
        gtk_widget_add_css_class(row, "list-queue-row");
        gtk_list_box_append(GTK_LIST_BOX(box_lecteur_queue), row);
    }
}

// GESTION DES PAROLES
int charger_paroles_pour_lecteur(int id_musique) {
    char buffer[4096] = "";
    int trouve = recuperer_paroles(id_musique, buffer, sizeof(buffer));
    GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_paroles));

    if (trouve && strlen(buffer) > 0) {
        gtk_text_buffer_set_text(tb, buffer, -1);
        return 1; // Succès
    } else {
        gtk_text_buffer_set_text(tb, "\n\n   Aucune parole disponible.", -1);
        return 0; // Rien trouvé
    }
}

void lancer_lecture_visuelle(const char* chemin, const char* titre, const char* artiste, int id, int est_fav) {
    // 1. Réinitialisation immédiate de l'interface pour le nouveau morceau
    if (scale_progression) {
        g_signal_handlers_block_by_func(scale_progression, on_scale_change_value, NULL);
        gtk_range_set_value(GTK_RANGE(scale_progression), 0.0);
        g_signal_handlers_unblock_by_func(scale_progression, on_scale_change_value, NULL);
    }
    if (label_temps_cur) gtk_label_set_text(GTK_LABEL(label_temps_cur), "00:00");
    if (label_temps_tot) gtk_label_set_text(GTK_LABEL(label_temps_tot), "00:00");

    // 2. Tentative de lecture audio
    AppError err = jouer_musique(chemin);
    if (err != SUCCESS) { 
        afficher_erreur_ui(err); 
        update_ui_play_state(FALSE); 
        return; 
    }

    // 3. Mise à jour des textes (Mini Player et Grand Lecteur)
    gtk_label_set_text(GTK_LABEL(label_mini_titre), titre);
    gtk_label_set_text(GTK_LABEL(label_mini_artiste), artiste);
    gtk_label_set_text(GTK_LABEL(label_lecteur_titre), titre);
    gtk_label_set_text(GTK_LABEL(label_lecteur_artiste), artiste);
    
    gtk_widget_set_visible(mini_player_box, TRUE);
    update_ui_play_state(TRUE);

    // 4. GESTION DU KARAOKÉ / PAROLES
    // On appelle la fonction de chargement (qui doit retourner 1 si trouvé, 0 sinon)
    int a_des_paroles = charger_paroles_pour_lecteur(id);
    
    if (stack_lecteur_droite) {
        if (a_des_paroles) {
            // Si on a des paroles, on affiche l'onglet "paroles" automatiquement
            gtk_stack_set_visible_child_name(GTK_STACK(stack_lecteur_droite), "paroles");
        } else {
            // Sinon on reste sur la file d'attente
            gtk_stack_set_visible_child_name(GTK_STACK(stack_lecteur_droite), "queue");
        }
    }
    
    // 5. Mise à jour du bouton favori (coeur)
    if (btn_lecteur_coeur) {
        g_object_set_data(G_OBJECT(btn_lecteur_coeur), "id", GINT_TO_POINTER(id));
        g_object_set_data(G_OBJECT(btn_lecteur_coeur), "etat", GINT_TO_POINTER(est_fav));
        
        if (est_fav) {
            gtk_widget_add_css_class(btn_lecteur_coeur, "favori-active");
            gtk_button_set_icon_name(GTK_BUTTON(btn_lecteur_coeur), "love");
        } else {
            gtk_widget_remove_css_class(btn_lecteur_coeur, "favori-active");
            gtk_button_set_icon_name(GTK_BUTTON(btn_lecteur_coeur), "non-starred-symbolic");
        }
    }
}

void on_musique_click(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    (void)user_data;
    if(box) gtk_list_box_select_row(box, row);
    const char *chemin = g_object_get_data(G_OBJECT(row), "chemin");
    const char *titre = g_object_get_data(G_OBJECT(row), "titre");
    const char *artiste = g_object_get_data(G_OBJECT(row), "artiste");
    int id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(row), "id"));
    int fav = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(row), "fav"));
    if (chemin) lancer_lecture_visuelle(chemin, titre, artiste, id, fav);
}

// --- IMPORTATION ---
static void on_folder_selected(GtkNativeDialog *native, int response, gpointer user_data) {
    (void)user_data;
    if (response == GTK_RESPONSE_ACCEPT) {
        GFile *folder = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(native));
        char *path = g_file_get_path(folder);
        int a = 0, d = 0;
        scanner_dossier(path, &a, &d);
        rafraichir_contenu_onglet();
        char msg[256]; snprintf(msg, 256, "Ajoutés: %d\nDoublons: %d", a, d);
        GtkWidget *diag = gtk_message_dialog_new(GTK_WINDOW(window_principale), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
        g_signal_connect(diag, "response", G_CALLBACK(gtk_window_destroy), NULL);
        gtk_widget_set_visible(diag, TRUE);
        g_free(path); g_object_unref(folder);
    }
    g_object_unref(native);
}
void on_import_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn; (void)user_data;
    GtkFileChooserNative *n = gtk_file_chooser_native_new("Choisir", GTK_WINDOW(window_principale), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "_Ouvrir", "_Annuler");
    g_signal_connect(n, "response", G_CALLBACK(on_folder_selected), NULL);
    gtk_native_dialog_show(GTK_NATIVE_DIALOG(n));
}

// --- LOGIQUE MODIFICATION METADONNEES ---
static void on_save_metadata_click(GtkButton *btn, gpointer user_data) {
    (void)btn;
    void **widgets = (void**)user_data;
    GtkDialog *dialog = GTK_DIALOG(widgets[0]);
    GtkEditable *e_titre = GTK_EDITABLE(widgets[1]);
    GtkEditable *e_artiste = GTK_EDITABLE(widgets[2]);
    GtkEditable *e_album = GTK_EDITABLE(widgets[3]);
    int id = GPOINTER_TO_INT(widgets[4]);

    const char *new_titre = gtk_editable_get_text(e_titre);
    const char *new_artiste = gtk_editable_get_text(e_artiste);
    const char *new_album = gtk_editable_get_text(e_album);

    AppError res = modifier_infos_musique(id, new_titre, new_artiste, new_album);
    
    if (res == SUCCESS) {
        rafraichir_contenu_onglet();
        gtk_window_destroy(GTK_WINDOW(dialog));
    } else {
        afficher_erreur_ui(res);
    }
    g_free(widgets);
}

static void action_modifier_dialog(GtkButton *btn, gpointer user_data) {
    (void)btn;
    int id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "id"));
    const char *old_titre = g_object_get_data(G_OBJECT(btn), "titre");
    const char *old_artiste = g_object_get_data(G_OBJECT(btn), "artiste");
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Modifier les infos", GTK_WINDOW(window_principale), GTK_DIALOG_MODAL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_widget_set_margin_start(content, 20); gtk_widget_set_margin_end(content, 20);
    gtk_widget_set_margin_top(content, 20); gtk_widget_set_margin_bottom(content, 20);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    
    GtkWidget *e_titre = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(e_titre), "Titre");
    gtk_editable_set_text(GTK_EDITABLE(e_titre), old_titre ? old_titre : "");
    gtk_box_append(GTK_BOX(vbox), gtk_label_new("Titre :"));
    gtk_box_append(GTK_BOX(vbox), e_titre);

    GtkWidget *e_artiste = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(e_artiste), "Artiste");
    gtk_editable_set_text(GTK_EDITABLE(e_artiste), old_artiste ? old_artiste : "");
    gtk_box_append(GTK_BOX(vbox), gtk_label_new("Artiste :"));
    gtk_box_append(GTK_BOX(vbox), e_artiste);

    GtkWidget *e_album = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(e_album), "Album");
    gtk_editable_set_text(GTK_EDITABLE(e_album), "Album inconnu"); 
    gtk_box_append(GTK_BOX(vbox), gtk_label_new("Album :"));
    gtk_box_append(GTK_BOX(vbox), e_album);

    GtkWidget *box_btns = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *btn_save = gtk_button_new_with_label("Enregistrer");
    gtk_widget_add_css_class(btn_save, "suggested-action"); 
    
    void **params = g_new(void*, 5);
    params[0] = dialog;
    params[1] = e_titre;
    params[2] = e_artiste;
    params[3] = e_album;
    params[4] = GINT_TO_POINTER(id);
    
    g_signal_connect(btn_save, "clicked", G_CALLBACK(on_save_metadata_click), params);
    
    gtk_box_append(GTK_BOX(box_btns), btn_save);
    gtk_box_append(GTK_BOX(vbox), box_btns);
    
    gtk_box_append(GTK_BOX(content), vbox);
    gtk_widget_set_visible(dialog, TRUE);
}

// --- LOGIQUE IMPORTATION PAROLES ---
static void on_paroles_file_selected(GtkNativeDialog *native, int response, gpointer user_data) {
    int id_musique = GPOINTER_TO_INT(user_data);
    if (response == GTK_RESPONSE_ACCEPT) {
        GFile *file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(native));
        char *path = g_file_get_path(file);
        FILE *f = fopen(path, "r");
        if (f) {
            fseek(f, 0, SEEK_END); long fsize = ftell(f); fseek(f, 0, SEEK_SET);
            char *string = malloc(fsize + 1);
            fread(string, 1, fsize, f); fclose(f); string[fsize] = 0;
            sauvegarder_paroles(id_musique, string);
            free(string);
            GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(window_principale), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Paroles importées avec succès !");
            g_signal_connect(d, "response", G_CALLBACK(gtk_window_destroy), NULL);
            gtk_widget_set_visible(d, TRUE);
        }
        g_free(path); g_object_unref(file);
    }
    g_object_unref(native);
}

static void action_importer_paroles(GtkButton *btn, gpointer user_data) {
    (void)btn;
    int id_musique = GPOINTER_TO_INT(user_data); 
    GtkFileChooserNative *n = gtk_file_chooser_native_new("Choisir un fichier texte", GTK_WINDOW(window_principale), GTK_FILE_CHOOSER_ACTION_OPEN, "_Importer", "_Annuler");
    g_signal_connect(n, "response", G_CALLBACK(on_paroles_file_selected), GINT_TO_POINTER(id_musique));
    gtk_native_dialog_show(GTK_NATIVE_DIALOG(n));
}

// --- CALLBACKS PLAYLISTS ---
static void on_new_playlist_confirm(GtkDialog *dialog, int response, gpointer user_data) {
    void **params = (void**)user_data;
    GtkWidget *entry = GTK_WIDGET(params[0]);
    int musique_id = GPOINTER_TO_INT(params[1]);

    if (response == GTK_RESPONSE_OK) {
        const char *txt = gtk_editable_get_text(GTK_EDITABLE(entry));
        if (strlen(txt) > 0) {
            int new_id = creer_playlist_et_recuperer_id(txt); 
            if (new_id > 0) {
                ajouter_musique_playlist(new_id, musique_id); 
                GtkWidget *succ = gtk_message_dialog_new(GTK_WINDOW(window_principale), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Playlist '%s' créée et musique ajoutée.", txt);
                g_signal_connect(succ, "response", G_CALLBACK(gtk_window_destroy), NULL);
                gtk_widget_set_visible(succ, TRUE);
                rafraichir_contenu_onglet();
            }
        }
    }
    g_free(params); 
    gtk_window_destroy(GTK_WINDOW(dialog));
}

// --- ACTIONS ---
static void action_supprimer_musique(GtkButton *btn, gpointer user_data) {
    (void)btn;
    int id_musique = GPOINTER_TO_INT(user_data);
    supprimer_musique(id_musique); 
    rafraichir_contenu_onglet();   
}

static void on_playlist_selection_click(GtkListBox *b, GtkListBoxRow *r, gpointer d_ptr) {
    (void)b;
    GtkWidget *dialog = GTK_WIDGET(d_ptr);
    int *data = (int*)g_object_get_data(G_OBJECT(dialog), "musique_id_ptr");
    int z_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(r), "pl_id"));
    
    if (z_id == 0) {
        gtk_window_destroy(GTK_WINDOW(dialog)); 
        GtkWidget *dial_new = gtk_dialog_new_with_buttons("Nom Playlist", GTK_WINDOW(window_principale), GTK_DIALOG_MODAL, "Créer", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
        GtkWidget *cont = gtk_dialog_get_content_area(GTK_DIALOG(dial_new));
        GtkWidget *ent = gtk_entry_new();
        gtk_box_append(GTK_BOX(cont), ent);
        gtk_widget_set_visible(ent, TRUE);
        
        void **params = g_new(void*, 2);
        params[0] = ent;
        params[1] = GINT_TO_POINTER(*data);
        
        g_signal_connect(dial_new, "response", G_CALLBACK(on_new_playlist_confirm), params);
        gtk_widget_set_visible(dial_new, TRUE);
    } else {
        ajouter_musique_playlist(z_id, *data);
        gtk_window_destroy(GTK_WINDOW(dialog));
    }
}

static void action_ajouter_playlist_dialog(GtkButton *btn, gpointer user_data) {
    (void)btn;
    int id_musique = GPOINTER_TO_INT(user_data);
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Ajouter à...", GTK_WINDOW(window_principale), GTK_DIALOG_MODAL, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_widget_set_margin_top(content, 10); gtk_widget_set_margin_bottom(content, 10);
    gtk_widget_set_margin_start(content, 10); gtk_widget_set_margin_end(content, 10);

    Playlist playlists[50]; int nb = 0;
    recuperer_toutes_les_playlists(playlists, &nb);

    GtkWidget *list = gtk_list_box_new();
    GtkWidget *row_new = gtk_list_box_row_new();
    GtkWidget *lbl_new = gtk_label_new("+ Nouvelle Playlist");
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row_new), lbl_new);
    g_object_set_data(G_OBJECT(row_new), "pl_id", GINT_TO_POINTER(0)); 
    gtk_list_box_append(GTK_LIST_BOX(list), row_new);

    for(int i=0; i<nb; i++) {
        GtkWidget *row = gtk_list_box_row_new();
        GtkWidget *lbl = gtk_label_new(playlists[i].nom);
        g_object_set_data(G_OBJECT(row), "pl_id", GINT_TO_POINTER(playlists[i].id));
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), lbl);
        gtk_list_box_append(GTK_LIST_BOX(list), row);
    }
    gtk_box_append(GTK_BOX(content), list);
    gtk_widget_set_visible(list, TRUE);

    g_signal_connect(list, "row-activated", G_CALLBACK(on_playlist_selection_click), dialog);

    int *id_ptr = g_new(int, 1); *id_ptr = id_musique;
    g_object_set_data_full(G_OBJECT(dialog), "musique_id_ptr", id_ptr, g_free);

    gtk_widget_set_visible(dialog, TRUE);
}

static void on_catalogue_select_for_playlist(GtkListBox *b, GtkListBoxRow *r, gpointer d) {
    (void)b; (void)d;
    if (playlist_ouverte_id > 0) {
        int id_zik = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(r), "id"));
        ajouter_musique_playlist(playlist_ouverte_id, id_zik);
    }
}

static void on_btn_ajouter_sons_playlist_click(GtkButton *b, gpointer d) {
    (void)b; (void)d;
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Choisir un titre", GTK_WINDOW(window_principale), GTK_DIALOG_MODAL, "Fermer", GTK_RESPONSE_CLOSE, NULL);
    GtkWidget *cont = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_widget_set_size_request(dialog, 300, 400);

    GtkWidget *scr = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scr, TRUE);
    GtkWidget *list = gtk_list_box_new();
    
    Musique tab[200]; int nb = 0;
    recuperer_musiques_triees(TRI_DEFAUT, tab, &nb);
    for(int i=0; i<nb; i++) {
        GtkWidget *row = creer_ligne_musique_simple(tab[i]);
        gtk_list_box_append(GTK_LIST_BOX(list), row);
    }
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scr), list);
    gtk_box_append(GTK_BOX(cont), scr);
    gtk_widget_set_visible(scr, TRUE);
    
    g_signal_connect(list, "row-activated", G_CALLBACK(on_catalogue_select_for_playlist), NULL);
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
    gtk_widget_set_visible(dialog, TRUE);
}

static void action_retirer_playlist(GtkButton *btn, gpointer user_data) {
    (void)btn;
    int id_musique = GPOINTER_TO_INT(user_data);
    if (playlist_ouverte_id > 0) {
        retirer_musique_playlist(playlist_ouverte_id, id_musique);
        rafraichir_contenu_onglet();
    }
}

void on_coeur_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    int id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "id"));
    int est_fav = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "etat"));
    basculer_favori(id, !est_fav);
    if (!est_fav) { gtk_widget_add_css_class(GTK_WIDGET(btn), "favori-active"); gtk_button_set_icon_name(btn, "love"); }
    else { gtk_widget_remove_css_class(GTK_WIDGET(btn), "favori-active"); gtk_button_set_icon_name(btn, "non-starred-symbolic"); }
    g_object_set_data(G_OBJECT(btn), "etat", GINT_TO_POINTER(!est_fav));
    if (onglet_actif == 2 && est_fav) rafraichir_contenu_onglet();
}

GtkWidget* creer_ligne_musique(Musique m) {
    GtkWidget *row = gtk_list_box_row_new();
    gtk_widget_add_css_class(row, "list-row"); 
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    GtkWidget *icon = gtk_image_new_from_icon_name("audio-x-generic-symbolic");
    gtk_widget_set_opacity(icon, 0.5);
    gtk_box_append(GTK_BOX(box), icon);
    
    GtkWidget *vbox_txt = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *lbl_titre = gtk_label_new(m.titre);
    gtk_widget_add_css_class(lbl_titre, "titre-song"); 
    gtk_widget_set_halign(lbl_titre, GTK_ALIGN_START);
    GtkWidget *lbl_art = gtk_label_new(m.artiste);
    gtk_widget_add_css_class(lbl_art, "artiste-song"); 
    gtk_widget_set_halign(lbl_art, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox_txt), lbl_titre);
    gtk_box_append(GTK_BOX(vbox_txt), lbl_art);
    gtk_widget_set_hexpand(vbox_txt, TRUE);
    gtk_box_append(GTK_BOX(box), vbox_txt);

    GtkWidget *btn_coeur = gtk_button_new_from_icon_name(m.est_favori ? "love" : "non-starred-symbolic"); 
    gtk_widget_add_css_class(btn_coeur, "btn-icon"); 
    if(m.est_favori) gtk_widget_add_css_class(btn_coeur, "favori-active");
    g_object_set_data(G_OBJECT(btn_coeur), "id", GINT_TO_POINTER(m.id));
    g_object_set_data(G_OBJECT(btn_coeur), "etat", GINT_TO_POINTER(m.est_favori));
    g_signal_connect(btn_coeur, "clicked", G_CALLBACK(on_coeur_clicked), NULL);
    gtk_box_append(GTK_BOX(box), btn_coeur);

    GtkWidget *btn_menu = gtk_menu_button_new();
    gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(btn_menu), "view-more-symbolic");
    gtk_widget_add_css_class(btn_menu, "btn-icon");

    GtkWidget *popover = gtk_popover_new();
    GtkWidget *box_menu = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    // IMPORT PAROLES
    GtkWidget *item_paroles = gtk_button_new_with_label("Importer des paroles");
    g_signal_connect(item_paroles, "clicked", G_CALLBACK(action_importer_paroles), GINT_TO_POINTER(m.id));
    gtk_box_append(GTK_BOX(box_menu), item_paroles);

    // MODIFIER
    GtkWidget *item_mod = gtk_button_new_with_label("Modifier les infos");
    g_object_set_data(G_OBJECT(item_mod), "id", GINT_TO_POINTER(m.id));
    g_object_set_data_full(G_OBJECT(item_mod), "titre", g_strdup(m.titre), g_free);
    g_object_set_data_full(G_OBJECT(item_mod), "artiste", g_strdup(m.artiste), g_free);
    g_signal_connect(item_mod, "clicked", G_CALLBACK(action_modifier_dialog), NULL);
    gtk_box_append(GTK_BOX(box_menu), item_mod);

    // AJOUTER PLAYLIST
    GtkWidget *item_add = gtk_button_new_with_label("Ajouter à une playlist");
    g_signal_connect(item_add, "clicked", G_CALLBACK(action_ajouter_playlist_dialog), GINT_TO_POINTER(m.id));
    gtk_box_append(GTK_BOX(box_menu), item_add);

    if (onglet_actif == 3 && playlist_ouverte_id > 0) {
        GtkWidget *item_rem = gtk_button_new_with_label("Retirer de la playlist");
        g_signal_connect(item_rem, "clicked", G_CALLBACK(action_retirer_playlist), GINT_TO_POINTER(m.id));
        gtk_box_append(GTK_BOX(box_menu), item_rem);
    }

    GtkWidget *item_del = gtk_button_new_with_label("Supprimer le fichier");
    gtk_widget_add_css_class(item_del, "error"); 
    g_signal_connect(item_del, "clicked", G_CALLBACK(action_supprimer_musique), GINT_TO_POINTER(m.id));
    gtk_box_append(GTK_BOX(box_menu), item_del);

    gtk_popover_set_child(GTK_POPOVER(popover), box_menu);
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(btn_menu), popover);
    
    gtk_box_append(GTK_BOX(box), btn_menu);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), box);

    g_object_set_data_full(G_OBJECT(row), "chemin", g_strdup(m.chemin), g_free);
    g_object_set_data_full(G_OBJECT(row), "titre", g_strdup(m.titre), g_free);
    g_object_set_data_full(G_OBJECT(row), "artiste", g_strdup(m.artiste), g_free);
    g_object_set_data(G_OBJECT(row), "id", GINT_TO_POINTER(m.id));
    g_object_set_data(G_OBJECT(row), "fav", GINT_TO_POINTER(m.est_favori));

    return row;
}

// --- GESTION PLAYLISTS ---
static void on_create_playlist_response(GtkDialog *dialog, int response, gpointer user_data) {
    GtkWidget *entry = GTK_WIDGET(user_data);
    if (response == GTK_RESPONSE_OK) {
        const char *txt = gtk_editable_get_text(GTK_EDITABLE(entry));
        if (strlen(txt) > 0) {
            creer_playlist_et_recuperer_id(txt);
            rafraichir_contenu_onglet();
        }
    }
    gtk_window_destroy(GTK_WINDOW(dialog));
}

void on_btn_creer_playlist_click(GtkButton *btn, gpointer d) {
    (void)btn; (void)d;
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Nouvelle Playlist", GTK_WINDOW(window_principale), GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Annuler", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Nom de la playlist");
    gtk_box_append(GTK_BOX(content), entry);
    gtk_widget_set_visible(entry, TRUE);
    g_signal_connect(dialog, "response", G_CALLBACK(on_create_playlist_response), entry);
    gtk_widget_set_visible(dialog, TRUE);
}

void on_ouvrir_playlist(GtkListBox *box, GtkListBoxRow *row, gpointer u) {
    (void)box; (void)u;
    int id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(row), "pl_id"));
    if (id > 0) {
        playlist_ouverte_id = id;
        rafraichir_contenu_onglet();
        gtk_stack_set_visible_child_name(GTK_STACK(stack_playlist), "contenu");
    }
}

void on_retour_playlist(GtkButton *b, gpointer d) {
    (void)b; (void)d;
    playlist_ouverte_id = -1;
    gtk_stack_set_visible_child_name(GTK_STACK(stack_playlist), "liste");
}

void on_supprimer_playlist_actuelle(GtkButton *b, gpointer d) {
    (void)b; (void)d;
    if (playlist_ouverte_id > 0) {
        supprimer_playlist(playlist_ouverte_id);
        on_retour_playlist(NULL, NULL);
        rafraichir_contenu_onglet();
    }
}

// --- GESTION ARTISTES (Ouverture) ---
static void on_btn_artiste_clicked(GtkButton *btn, gpointer u) {
    (void)u;
    const char *nom = gtk_button_get_label(btn);
    snprintf(artiste_ouvert_nom, 100, "%s", nom);
    rafraichir_contenu_onglet();
    gtk_stack_set_visible_child_name(GTK_STACK(stack_artiste), "contenu");
}

static void on_retour_artiste(GtkButton *b, gpointer d) {
    (void)b; (void)d;
    artiste_ouvert_nom[0] = '\0';
    gtk_stack_set_visible_child_name(GTK_STACK(stack_artiste), "liste");
}

// --- RAFRAICHISSEMENT ---
void rafraichir_contenu_onglet() {
    while (g_main_context_iteration(NULL, FALSE));
    const char *recherche = gtk_editable_get_text(GTK_EDITABLE(barre_recherche));
    Musique tab[200]; int nb = 0;
    
    // Catalogue
    if (onglet_actif == 0) {
        if (strlen(recherche)>0) rechercher_musiques(recherche, tab, &nb); else recuperer_musiques_triees(mode_tri_actuel, tab, &nb);
        GtkWidget *c; while ((c = gtk_widget_get_first_child(box_catalogue_liste))) gtk_list_box_remove(GTK_LIST_BOX(box_catalogue_liste), c);
        for(int i=0; i<nb; i++) gtk_list_box_append(GTK_LIST_BOX(box_catalogue_liste), creer_ligne_musique(tab[i]));
    }
    // Artistes (Avec Stack)
    if (onglet_actif == 1) {
        if (strlen(artiste_ouvert_nom) == 0) {
            // Liste Artistes
            if (strlen(recherche)>0) rechercher_musiques(recherche, tab, &nb); else recuperer_musiques_triees(TRI_DEFAUT, tab, &nb);
            GtkWidget *c; while ((c = gtk_widget_get_first_child(flowbox_artistes))) gtk_flow_box_remove(GTK_FLOW_BOX(flowbox_artistes), c);
            for(int i=0; i<nb; i++) {
                 if (i>0 && strcmp(tab[i].artiste, tab[i-1].artiste) == 0) continue;
                 GtkWidget *btn = gtk_button_new_with_label(tab[i].artiste);
                 gtk_widget_set_size_request(btn, 130, 80); gtk_widget_add_css_class(btn, "card-artiste");
                 g_signal_connect(btn, "clicked", G_CALLBACK(on_btn_artiste_clicked), NULL);
                 gtk_flow_box_append(GTK_FLOW_BOX(flowbox_artistes), btn);
            }
        } else {
            // Contenu Artiste
            recuperer_musiques_par_artiste(artiste_ouvert_nom, tab, &nb);
            GtkWidget *c; while ((c = gtk_widget_get_first_child(box_artiste_content))) gtk_list_box_remove(GTK_LIST_BOX(box_artiste_content), c);
            for(int i=0; i<nb; i++) gtk_list_box_append(GTK_LIST_BOX(box_artiste_content), creer_ligne_musique(tab[i]));
        }
    }
    // Favoris
    if (onglet_actif == 2) {
        recuperer_musiques_triees(mode_tri_actuel, tab, &nb);
        GtkWidget *c; while ((c = gtk_widget_get_first_child(box_favoris_liste))) gtk_list_box_remove(GTK_LIST_BOX(box_favoris_liste), c);
        for(int i=0; i<nb; i++) if (tab[i].est_favori) gtk_list_box_append(GTK_LIST_BOX(box_favoris_liste), creer_ligne_musique(tab[i]));
    }
    // Playlists
    if (onglet_actif == 3) {
        if (playlist_ouverte_id == -1) {
            GtkWidget *c; while ((c = gtk_widget_get_first_child(box_playlists_liste))) gtk_list_box_remove(GTK_LIST_BOX(box_playlists_liste), c);
            Playlist pls[50]; int nb_p = 0;
            recuperer_toutes_les_playlists(pls, &nb_p);
            for(int i=0; i<nb_p; i++) {
                GtkWidget *row = gtk_list_box_row_new();
                GtkWidget *lbl = gtk_label_new(pls[i].nom);
                gtk_widget_set_margin_top(lbl, 15); gtk_widget_set_margin_bottom(lbl, 15);
                gtk_widget_set_margin_start(lbl, 15); gtk_widget_set_margin_end(lbl, 15);
                gtk_widget_set_halign(lbl, GTK_ALIGN_START);
                g_object_set_data(G_OBJECT(row), "pl_id", GINT_TO_POINTER(pls[i].id));
                gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), lbl);
                gtk_list_box_append(GTK_LIST_BOX(box_playlists_liste), row);
            }
        } else {
            recuperer_musiques_playlist(playlist_ouverte_id, tab, &nb);
            GtkWidget *c; while ((c = gtk_widget_get_first_child(box_playlist_content))) gtk_list_box_remove(GTK_LIST_BOX(box_playlist_content), c);
            for(int i=0; i<nb; i++) gtk_list_box_append(GTK_LIST_BOX(box_playlist_content), creer_ligne_musique(tab[i]));
        }
    }
}

// ... FONCTIONS STANDARDS ...

void changer_onglet(GtkButton *btn, gpointer page_name) {
    const char *nom = (const char*)page_name;
    gtk_stack_set_visible_child_name(GTK_STACK(stack_principal), nom);
    if (strcmp(nom, "catalogue") == 0) onglet_actif = 0;
    else if (strcmp(nom, "artistes") == 0) onglet_actif = 1;
    else if (strcmp(nom, "favoris") == 0) onglet_actif = 2;
    else if (strcmp(nom, "playlists") == 0) onglet_actif = 3;
    
    GtkWidget *child = gtk_widget_get_first_child(nav_bar_box);
    while(child) {
        if (GTK_IS_BUTTON(child)) {
            gtk_widget_remove_css_class(child, "nav-btn-active");
            gtk_widget_add_css_class(child, "nav-btn");
        }
        child = gtk_widget_get_next_sibling(child);
    }
    gtk_widget_remove_css_class(GTK_WIDGET(btn), "nav-btn");
    gtk_widget_add_css_class(GTK_WIDGET(btn), "nav-btn-active");
    rafraichir_contenu_onglet();
}

void ouvrir_grand_lecteur() { 
    charger_liste_lecteur_queue(); 
    gtk_stack_set_visible_child_name(GTK_STACK(stack_overlay), "grand_lecteur"); 
    gtk_widget_set_visible(nav_bar_box, FALSE); 
}
void fermer_grand_lecteur() { gtk_stack_set_visible_child_name(GTK_STACK(stack_overlay), "principal"); gtk_widget_set_visible(nav_bar_box, TRUE); }
void on_scale_change_value(GtkRange *range, gpointer d) { (void)d; if (is_sliding) definir_progression(gtk_range_get_value(range)); }
void on_scale_press(GtkGestureClick *g, int n, double x, double y, gpointer d) { (void)g; (void)n; (void)x; (void)y; (void)d; is_sliding = TRUE; }
void on_scale_release(GtkGestureClick *g, int n, double x, double y, gpointer d) { (void)g; (void)n; (void)x; (void)y; (void)d; definir_progression(gtk_range_get_value(GTK_RANGE(scale_progression))); is_sliding = FALSE; }
void update_ui_play_state(gboolean playing) {
    const char *icon = playing ? "media-playback-pause-symbolic" : "media-playback-start-symbolic";
    if (btn_mini_play) gtk_button_set_icon_name(GTK_BUTTON(btn_mini_play), icon);
    if (btn_lecteur_play) gtk_button_set_icon_name(GTK_BUTTON(btn_lecteur_play), icon);
}
void on_play_pause_click(GtkButton *b, gpointer d) { (void)b; (void)d; if (est_en_lecture()) { mettre_en_pause(); update_ui_play_state(FALSE); } else { reprendre_lecture(); update_ui_play_state(TRUE); } }
void on_shuffle_clicked(GtkButton *btn, gpointer d) { (void)d; mode_aleatoire = !mode_aleatoire; if(mode_aleatoire) gtk_widget_add_css_class(GTK_WIDGET(btn), "shuffle-active"); else gtk_widget_remove_css_class(GTK_WIDGET(btn), "shuffle-active"); }
void on_recherche_changed(GtkWidget *w, gpointer d) { (void)w; (void)d; rafraichir_contenu_onglet(); }
void on_maximize_clicked(GtkButton *btn, gpointer d) { (void)btn; (void)d; if (gtk_window_is_maximized(GTK_WINDOW(window_principale))) gtk_window_unmaximize(GTK_WINDOW(window_principale)); else gtk_window_maximize(GTK_WINDOW(window_principale)); }

void on_next_clicked(GtkButton *b, gpointer d) {
    (void)b; (void)d;
    GtkListBox *liste = (onglet_actif==0) ? GTK_LIST_BOX(box_catalogue_liste) : (onglet_actif==2) ? GTK_LIST_BOX(box_favoris_liste) : NULL;
    if (onglet_actif == 3 && playlist_ouverte_id != -1) liste = GTK_LIST_BOX(box_playlist_content);
    if (onglet_actif == 1 && strlen(artiste_ouvert_nom)>0) liste = GTK_LIST_BOX(box_artiste_content);
    
    if(!liste) return;
    GtkListBoxRow *row = gtk_list_box_get_selected_row(liste);
    if(row) {
        GtkWidget *next = gtk_widget_get_next_sibling(GTK_WIDGET(row));
        if(next) on_musique_click(liste, GTK_LIST_BOX_ROW(next), NULL);
        else {
            GtkWidget *first = gtk_widget_get_first_child(GTK_WIDGET(liste));
            if(first) on_musique_click(liste, GTK_LIST_BOX_ROW(first), NULL);
        }
    }
}
void on_prev_clicked(GtkButton *b, gpointer d) {
    (void)b; (void)d;
    GtkListBox *liste = (onglet_actif==0) ? GTK_LIST_BOX(box_catalogue_liste) : (onglet_actif==2) ? GTK_LIST_BOX(box_favoris_liste) : NULL;
    if (onglet_actif == 3 && playlist_ouverte_id != -1) liste = GTK_LIST_BOX(box_playlist_content);
    if(!liste) return;
    GtkListBoxRow *row = gtk_list_box_get_selected_row(liste);
    if(row) {
        GtkWidget *prev = gtk_widget_get_prev_sibling(GTK_WIDGET(row));
        if(prev) on_musique_click(liste, GTK_LIST_BOX_ROW(prev), NULL);
    }
}

void mettre_a_jour_affichage_karaoke() {
    if (!est_en_lecture()) return;

    int64_t t_actuel = obtenir_temps_actuel_sec();
    int64_t t_total = obtenir_duree_totale_sec();
    if (t_total <= 5) return;

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_paroles));
    int nb_lignes = gtk_text_buffer_get_line_count(buffer);
    if (nb_lignes <= 1) return;

    // 1. Nettoyer le rose partout avant d'appliquer le nouveau
    GtkTextIter start_all, end_all;
    gtk_text_buffer_get_bounds(buffer, &start_all, &end_all);
    gtk_text_buffer_remove_tag(buffer, tag_rose, &start_all, &end_all);

    // 2. Calculer la ligne actuelle
    int ligne_cible = (int)((double)t_actuel / t_total * nb_lignes);

    // 3. Appliquer le rose sur la ligne actuelle
    GtkTextIter start_line, end_line;
    gtk_text_buffer_get_iter_at_line(buffer, &start_line, ligne_cible);
    end_line = start_line;
    gtk_text_iter_forward_to_line_end(&end_line);
    
    gtk_text_buffer_apply_tag(buffer, tag_rose, &start_line, &end_line);

    // 4. Faire défiler
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(text_view_paroles), &start_line, 0.0, TRUE, 0.5, 0.5);
}

static gboolean on_tick(gpointer data) {
    if (!est_en_lecture()) return G_SOURCE_CONTINUE;

    int64_t cur = obtenir_temps_actuel_sec();
    int64_t tot = obtenir_duree_totale_sec();
    char buf[16];

    // Mise à jour des labels de temps
    snprintf(buf, 16, "%02ld:%02ld", cur / 60, cur % 60);
    gtk_label_set_text(GTK_LABEL(label_temps_cur), buf);
    
    snprintf(buf, 16, "%02ld:%02ld", tot / 60, tot % 60);
    gtk_label_set_text(GTK_LABEL(label_temps_tot), buf);

    // Mise à jour de la barre bleue (Scale)
    if (tot > 0 && !is_sliding) {
        // Bloquer le signal pour éviter que la barre ne "saute" pendant qu'on la met à jour
        g_signal_handlers_block_by_func(scale_progression, on_scale_change_value, NULL);
        gtk_range_set_value(GTK_RANGE(scale_progression), (double)cur / tot);
        g_signal_handlers_unblock_by_func(scale_progression, on_scale_change_value, NULL);
    }
    // APPEL DE LA SYNCHRONISATION KARAOKÉ
    mettre_a_jour_affichage_karaoke();

    return G_SOURCE_CONTINUE;
}

// LOGIQUE POUR BASCULER ENTRE QUEUE ET PAROLES
void on_switch_view_queue(GtkButton *b, gpointer d) { (void)b; (void)d; gtk_stack_set_visible_child_name(GTK_STACK(stack_lecteur_droite), "queue"); }
void on_switch_view_paroles(GtkButton *b, gpointer d) { (void)b; (void)d; gtk_stack_set_visible_child_name(GTK_STACK(stack_lecteur_droite), "paroles"); }

// --- CALLBACK TRI (NOUVEAU) ---
static void on_sort_changed(GObject *object, GParamSpec *pspec, gpointer user_data) {
    (void)pspec; 
    (void)user_data;

    GtkDropDown *dropdown = GTK_DROP_DOWN(object);
    int selected = gtk_drop_down_get_selected(dropdown);
    
    // 1. Mise à jour du mode de tri
    switch(selected) {
        case 0: mode_tri_actuel = TRI_TITRE; break;
        case 1: mode_tri_actuel = TRI_ARTISTE; break;
        case 2: mode_tri_actuel = TRI_ALBUM; break;
        case 3: mode_tri_actuel = TRI_GENRE; break;
        case 4: mode_tri_actuel = TRI_DUREE; break;
        case 5: mode_tri_actuel = TRI_RECENT; break;
        default: mode_tri_actuel = TRI_DEFAUT;
    }
    
    // 2. CRUCIAL : Laisser le temps à l'interface de fermer le menu
    // Cette boucle traite les messages en attente (clic, fermeture du popup)
    while (g_main_context_iteration(NULL, FALSE));

    // 3. Lancer le rafraîchissement
    rafraichir_contenu_onglet();
    
    // 4. Libérer le focus pour que le clavier/souris revienne à la liste
    if (window_principale) {
        gtk_widget_grab_focus(GTK_WIDGET(window_principale));
    }
}

void on_app_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data; srand(time(NULL)); charger_css();
    window_principale = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(window_principale), 900, 650); 
    gtk_window_set_title(GTK_WINDOW(window_principale), "Music App");

    GtkWidget *header = gtk_header_bar_new();
    gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(header), TRUE);
    gtk_window_set_titlebar(GTK_WINDOW(window_principale), header);
    
    
    
    // --- DROPDOWN TRI ---
    const char *options[] = {"Titre", "Artiste", "Album", "Genre", "Durée", "Récent", NULL};
    GtkStringList *sl = gtk_string_list_new(options);
    
    // On garde le pointeur vers le dropdown si besoin (optionnel)
    GtkWidget *dropdown_sort = gtk_drop_down_new(G_LIST_MODEL(sl), NULL);
    
    // IMPORTANT : On définit la sélection par défaut sur 0 (Titre) au démarrage
    gtk_drop_down_set_selected(GTK_DROP_DOWN(dropdown_sort), 0);
    
    g_signal_connect(dropdown_sort, "notify::selected", G_CALLBACK(on_sort_changed), NULL);
    
    gtk_widget_set_valign(dropdown_sort, GTK_ALIGN_CENTER); // Centrer verticalement
    gtk_widget_set_margin_end(dropdown_sort, 10); // Espace avec la barre de recherche
    
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), dropdown_sort);
    // --------------------

    GtkWidget *btn_max = gtk_button_new_from_icon_name("view-fullscreen-symbolic");
    gtk_widget_add_css_class(btn_max, "btn-icon");
    g_signal_connect(btn_max, "clicked", G_CALLBACK(on_maximize_clicked), NULL);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), btn_max);

    GtkWidget *btn_import = gtk_button_new_from_icon_name("document-open-symbolic");
    gtk_widget_add_css_class(btn_import, "btn-icon");
    g_signal_connect(btn_import, "clicked", G_CALLBACK(on_import_clicked), NULL);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), btn_import);

    barre_recherche = gtk_search_entry_new();
    gtk_search_entry_set_placeholder_text(GTK_SEARCH_ENTRY(barre_recherche), "Rechercher...");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), barre_recherche);
    g_signal_connect(barre_recherche, "search-changed", G_CALLBACK(on_recherche_changed), NULL);

    stack_overlay = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack_overlay), GTK_STACK_TRANSITION_TYPE_SLIDE_UP);
    gtk_window_set_child(GTK_WINDOW(window_principale), stack_overlay);

    GtkWidget *vbox_main = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    stack_principal = gtk_stack_new();
    gtk_widget_set_vexpand(stack_principal, TRUE);
    
    GtkWidget *scr1 = gtk_scrolled_window_new();
    box_catalogue_liste = gtk_list_box_new();
    gtk_widget_set_margin_top(box_catalogue_liste, 10);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scr1), box_catalogue_liste);
    g_signal_connect(box_catalogue_liste, "row-activated", G_CALLBACK(on_musique_click), NULL);
    gtk_stack_add_named(GTK_STACK(stack_principal), scr1, "catalogue");

    // Artistes (Stack)
    stack_artiste = gtk_stack_new();
    GtkWidget *scr2 = gtk_scrolled_window_new();
    flowbox_artistes = gtk_flow_box_new();
    gtk_widget_set_margin_start(flowbox_artistes, 10); gtk_widget_set_margin_end(flowbox_artistes, 10);
    gtk_widget_set_margin_top(flowbox_artistes, 10); gtk_widget_set_margin_bottom(flowbox_artistes, 10);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(flowbox_artistes), 2);
    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(flowbox_artistes), GTK_SELECTION_NONE);
    // Pas de signal ici, on utilise les boutons
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scr2), flowbox_artistes);
    gtk_stack_add_named(GTK_STACK(stack_artiste), scr2, "liste");

    GtkWidget *vbox_art = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *btn_back_art = gtk_button_new_with_label("Retour");
    g_signal_connect(btn_back_art, "clicked", G_CALLBACK(on_retour_artiste), NULL);
    gtk_box_append(GTK_BOX(vbox_art), btn_back_art);
    GtkWidget *scr_art_c = gtk_scrolled_window_new();
    box_artiste_content = gtk_list_box_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scr_art_c), box_artiste_content);
    g_signal_connect(box_artiste_content, "row-activated", G_CALLBACK(on_musique_click), NULL);
    gtk_widget_set_vexpand(scr_art_c, TRUE);
    gtk_box_append(GTK_BOX(vbox_art), scr_art_c);
    gtk_stack_add_named(GTK_STACK(stack_artiste), vbox_art, "contenu");

    gtk_stack_add_named(GTK_STACK(stack_principal), stack_artiste, "artistes");

    // Favoris
    GtkWidget *scr3 = gtk_scrolled_window_new();
    box_favoris_liste = gtk_list_box_new();
    gtk_widget_set_margin_top(box_favoris_liste, 10);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scr3), box_favoris_liste);
    g_signal_connect(box_favoris_liste, "row-activated", G_CALLBACK(on_musique_click), NULL);
    gtk_stack_add_named(GTK_STACK(stack_principal), scr3, "favoris");

    // Playlists
    stack_playlist = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack_playlist), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    
    GtkWidget *vbox_pl_list = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *btn_add_pl = gtk_button_new_with_label("+ Créer une playlist");
    g_signal_connect(btn_add_pl, "clicked", G_CALLBACK(on_btn_creer_playlist_click), NULL);
    gtk_box_append(GTK_BOX(vbox_pl_list), btn_add_pl);
    
    GtkWidget *scr_pl = gtk_scrolled_window_new();
    box_playlists_liste = gtk_list_box_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scr_pl), box_playlists_liste);
    g_signal_connect(box_playlists_liste, "row-activated", G_CALLBACK(on_ouvrir_playlist), NULL);
    gtk_widget_set_vexpand(scr_pl, TRUE);
    gtk_box_append(GTK_BOX(vbox_pl_list), scr_pl);
    gtk_stack_add_named(GTK_STACK(stack_playlist), vbox_pl_list, "liste");

    GtkWidget *vbox_pl_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *header_pl = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *btn_back = gtk_button_new_from_icon_name("go-previous-symbolic");
    g_signal_connect(btn_back, "clicked", G_CALLBACK(on_retour_playlist), NULL);
    gtk_box_append(GTK_BOX(header_pl), btn_back);
    
    GtkWidget *btn_add_sons = gtk_button_new_with_label("+ Ajouter des sons");
    g_signal_connect(btn_add_sons, "clicked", G_CALLBACK(on_btn_ajouter_sons_playlist_click), NULL);
    gtk_box_append(GTK_BOX(header_pl), btn_add_sons);

    GtkWidget *btn_del_pl = gtk_button_new_from_icon_name("user-trash-symbolic");
    g_signal_connect(btn_del_pl, "clicked", G_CALLBACK(on_supprimer_playlist_actuelle), NULL);
    gtk_box_append(GTK_BOX(header_pl), btn_del_pl);
    
    gtk_box_append(GTK_BOX(vbox_pl_content), header_pl);
    GtkWidget *scr_pl_c = gtk_scrolled_window_new();
    box_playlist_content = gtk_list_box_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scr_pl_c), box_playlist_content);
    g_signal_connect(box_playlist_content, "row-activated", G_CALLBACK(on_musique_click), NULL);
    gtk_widget_set_vexpand(scr_pl_c, TRUE);
    gtk_box_append(GTK_BOX(vbox_pl_content), scr_pl_c);
    gtk_stack_add_named(GTK_STACK(stack_playlist), vbox_pl_content, "contenu");

    gtk_stack_add_named(GTK_STACK(stack_principal), stack_playlist, "playlists");
    gtk_box_append(GTK_BOX(vbox_main), stack_principal);

    mini_player_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_add_css_class(mini_player_box, "mini-player");
    gtk_widget_set_visible(mini_player_box, FALSE);
    GtkGesture *click = gtk_gesture_click_new();
    g_signal_connect(click, "pressed", G_CALLBACK(ouvrir_grand_lecteur), NULL);
    gtk_widget_add_controller(mini_player_box, GTK_EVENT_CONTROLLER(click));

    GtkWidget *mini_ico = gtk_image_new_from_icon_name("audio-x-generic-symbolic");
    gtk_widget_set_opacity(mini_ico, 0.5);
    gtk_box_append(GTK_BOX(mini_player_box), mini_ico);

    label_mini_titre = gtk_label_new("Titre");
    gtk_widget_add_css_class(label_mini_titre, "mini-titre");
    gtk_widget_set_hexpand(label_mini_titre, TRUE);
    gtk_widget_set_halign(label_mini_titre, GTK_ALIGN_START);
    
    label_mini_artiste = gtk_label_new("Artiste");
    gtk_widget_add_css_class(label_mini_artiste, "mini-artiste");
    gtk_widget_set_halign(label_mini_artiste, GTK_ALIGN_START);
    
    GtkWidget *vbox_mini_txt = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_valign(vbox_mini_txt, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(vbox_mini_txt), label_mini_titre);
    gtk_box_append(GTK_BOX(vbox_mini_txt), label_mini_artiste);
    gtk_box_append(GTK_BOX(mini_player_box), vbox_mini_txt);

    btn_mini_play = gtk_button_new_from_icon_name("media-playback-start-symbolic");
    gtk_widget_add_css_class(btn_mini_play, "btn-icon");
    g_signal_connect(btn_mini_play, "clicked", G_CALLBACK(on_play_pause_click), NULL);
    gtk_box_append(GTK_BOX(mini_player_box), btn_mini_play);

    gtk_box_append(GTK_BOX(vbox_main), mini_player_box);

    nav_bar_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class(nav_bar_box, "nav-bar");
    gtk_widget_set_halign(nav_bar_box, GTK_ALIGN_CENTER);
    const char *tabs[] = {"Catalogue", "Artistes", "Favoris", "Playlists"};
    const char *ids[] = {"catalogue", "artistes", "favoris", "playlists"};
    for(int i=0; i<4; i++) {
        GtkWidget *b = gtk_button_new_with_label(tabs[i]);
        gtk_widget_add_css_class(b, "nav-btn");
        if(i==0) gtk_widget_add_css_class(b, "nav-btn-active");
        g_signal_connect(b, "clicked", G_CALLBACK(changer_onglet), (gpointer)ids[i]);
        gtk_box_append(GTK_BOX(nav_bar_box), b);
    }
    gtk_box_append(GTK_BOX(vbox_main), nav_bar_box);
    gtk_stack_add_named(GTK_STACK(stack_overlay), vbox_main, "principal");

    // --- GRAND LECTEUR (Split View) ---
    GtkWidget *split_lecteur = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_add_css_class(split_lecteur, "lecteur-bg");
    
    // Partie Gauche (Controles)
    GtkWidget *vbox_lecteur = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_start(vbox_lecteur, 30); gtk_widget_set_margin_end(vbox_lecteur, 30);
    gtk_widget_set_margin_top(vbox_lecteur, 30); gtk_widget_set_margin_bottom(vbox_lecteur, 30);
    gtk_widget_set_valign(vbox_lecteur, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(vbox_lecteur, TRUE);

    GtkWidget *btn_close = gtk_button_new_from_icon_name("go-down-symbolic");
    gtk_widget_add_css_class(btn_close, "btn-icon");
    g_signal_connect(btn_close, "clicked", G_CALLBACK(fermer_grand_lecteur), NULL);
    gtk_widget_set_halign(btn_close, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox_lecteur), btn_close);

    GtkWidget *img_cover = gtk_image_new_from_icon_name("audio-x-generic-symbolic");
    gtk_widget_set_size_request(img_cover, 200, 200);
    gtk_widget_set_halign(img_cover, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(vbox_lecteur), img_cover);

    label_lecteur_titre = gtk_label_new("Titre");
    gtk_widget_add_css_class(label_lecteur_titre, "title-big");
    gtk_box_append(GTK_BOX(vbox_lecteur), label_lecteur_titre);

    label_lecteur_artiste = gtk_label_new("Artiste");
    gtk_widget_add_css_class(label_lecteur_artiste, "artist-big");
    gtk_box_append(GTK_BOX(vbox_lecteur), label_lecteur_artiste);
    
    scale_progression = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 1, 0.01);
    g_signal_connect(scale_progression, "value-changed", G_CALLBACK(on_scale_change_value), NULL);
    GtkGesture *g_scale = gtk_gesture_click_new();
    gtk_widget_add_controller(scale_progression, GTK_EVENT_CONTROLLER(g_scale));
    g_signal_connect(g_scale, "pressed", G_CALLBACK(on_scale_press), NULL);
    g_signal_connect(g_scale, "released", G_CALLBACK(on_scale_release), NULL);
    gtk_box_append(GTK_BOX(vbox_lecteur), scale_progression);
    
    GtkWidget *box_temps = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    label_temps_cur = gtk_label_new("00:00"); label_temps_tot = gtk_label_new("00:00");
    gtk_box_append(GTK_BOX(box_temps), label_temps_cur);
    GtkWidget *spacer = gtk_label_new(""); gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(box_temps), spacer);
    gtk_box_append(GTK_BOX(box_temps), label_temps_tot);
    gtk_box_append(GTK_BOX(vbox_lecteur), box_temps);

    GtkWidget *box_ctrl = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(box_ctrl, GTK_ALIGN_CENTER);
    
    btn_lecteur_shuffle = gtk_button_new_from_icon_name("media-playlist-shuffle-symbolic");
    gtk_widget_add_css_class(btn_lecteur_shuffle, "btn-icon");
    g_signal_connect(btn_lecteur_shuffle, "clicked", G_CALLBACK(on_shuffle_clicked), NULL);

    GtkWidget *btn_prev = gtk_button_new_from_icon_name("media-skip-backward-symbolic");
    gtk_widget_add_css_class(btn_prev, "btn-icon");
    g_signal_connect(btn_prev, "clicked", G_CALLBACK(on_prev_clicked), NULL);

    btn_lecteur_play = gtk_button_new_from_icon_name("media-playback-start-symbolic");
    gtk_widget_add_css_class(btn_lecteur_play, "btn-circle");
    gtk_widget_set_size_request(btn_lecteur_play, 60, 60);
    g_signal_connect(btn_lecteur_play, "clicked", G_CALLBACK(on_play_pause_click), NULL);

    GtkWidget *btn_next = gtk_button_new_from_icon_name("media-skip-forward-symbolic");
    gtk_widget_add_css_class(btn_next, "btn-icon");
    g_signal_connect(btn_next, "clicked", G_CALLBACK(on_next_clicked), NULL);

    btn_lecteur_coeur = gtk_button_new_from_icon_name("non-starred-symbolic");
    gtk_widget_add_css_class(btn_lecteur_coeur, "btn-icon");
    g_signal_connect(btn_lecteur_coeur, "clicked", G_CALLBACK(on_coeur_clicked), NULL);

    gtk_box_append(GTK_BOX(box_ctrl), btn_lecteur_shuffle);
    gtk_box_append(GTK_BOX(box_ctrl), btn_prev);
    gtk_box_append(GTK_BOX(box_ctrl), btn_lecteur_play);
    gtk_box_append(GTK_BOX(box_ctrl), btn_next);
    gtk_box_append(GTK_BOX(box_ctrl), btn_lecteur_coeur);
    gtk_box_append(GTK_BOX(vbox_lecteur), box_ctrl);
    
    // Partie Droite (Stack: Queue OU Paroles)
    GtkWidget *vbox_droite = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *box_switch = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *btn_q = gtk_button_new_with_label("File d'attente");
    g_signal_connect(btn_q, "clicked", G_CALLBACK(on_switch_view_queue), NULL);
    GtkWidget *btn_p = gtk_button_new_with_label("Paroles / Karaoké");
    g_signal_connect(btn_p, "clicked", G_CALLBACK(on_switch_view_paroles), NULL);
    gtk_box_append(GTK_BOX(box_switch), btn_q);
    gtk_box_append(GTK_BOX(box_switch), btn_p);
    gtk_box_append(GTK_BOX(vbox_droite), box_switch);

    stack_lecteur_droite = gtk_stack_new();
    
    // Vue Queue
    GtkWidget *scr_queue = gtk_scrolled_window_new();
    gtk_widget_set_size_request(scr_queue, 250, -1);
    box_lecteur_queue = gtk_list_box_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scr_queue), box_lecteur_queue);
    gtk_stack_add_named(GTK_STACK(stack_lecteur_droite), scr_queue, "queue");

    // Vue Paroles
    GtkWidget *scr_paroles = gtk_scrolled_window_new();
    text_view_paroles = gtk_text_view_new();
    // À mettre dans on_app_activate :
    GtkTextBuffer *buffer_p = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_paroles));
    tag_rose = gtk_text_buffer_create_tag(buffer_p, "highlight", 
                                        "foreground", "#ec407a", // Rose vibrant
                                        "weight", 700,           // Gras
                                        "scale", 1.2,            // Un peu plus grand
                                        NULL);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view_paroles), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view_paroles), GTK_WRAP_WORD);
    gtk_widget_set_margin_start(text_view_paroles, 10);
    gtk_widget_set_margin_end(text_view_paroles, 10);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scr_paroles), text_view_paroles);
    gtk_stack_add_named(GTK_STACK(stack_lecteur_droite), scr_paroles, "paroles");

    gtk_box_append(GTK_BOX(vbox_droite), stack_lecteur_droite);
    
    gtk_box_append(GTK_BOX(split_lecteur), vbox_lecteur);
    gtk_box_append(GTK_BOX(split_lecteur), gtk_separator_new(GTK_ORIENTATION_VERTICAL));
    gtk_box_append(GTK_BOX(split_lecteur), vbox_droite);

    gtk_stack_add_named(GTK_STACK(stack_overlay), split_lecteur, "grand_lecteur");

    g_timeout_add(1000, on_tick, NULL);
    rafraichir_contenu_onglet();
    gtk_window_present(GTK_WINDOW(window_principale));

    // Dans on_app_activate, ajouter :
    btn_mini_prev = gtk_button_new_from_icon_name("media-skip-backward-symbolic");
    g_signal_connect(btn_mini_prev, "clicked", G_CALLBACK(on_prev_clicked), NULL);

    btn_mini_next = gtk_button_new_from_icon_name("media-skip-forward-symbolic");
    g_signal_connect(btn_mini_next, "clicked", G_CALLBACK(on_next_clicked), NULL);

    // Et les ajouter à la box du mini player (vbox_mini ou autre)
    gtk_box_append(GTK_BOX(mini_player_box), btn_mini_prev);
    gtk_box_append(GTK_BOX(mini_player_box), btn_mini_play);
    gtk_box_append(GTK_BOX(mini_player_box), btn_mini_next);
}