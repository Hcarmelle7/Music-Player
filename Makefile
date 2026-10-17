CC = gcc

# --- FUSION DES BIBLIOTHÈQUES ---
# On combine : gtk4 + sqlite3 + sndfile + gstreamer-1.0
# Ajout de gstreamer-pbutils-1.0 à la fin de la liste pkg-config
CFLAGS = -Wall -Wextra -std=c99 -Iinclude $(shell pkg-config --cflags gtk4 sqlite3 sndfile gstreamer-1.0 gstreamer-pbutils-1.0)

LDFLAGS = $(shell pkg-config --libs gtk4 sqlite3 sndfile gstreamer-1.0 gstreamer-pbutils-1.0) -lpthread -lm -ldl

# --- LISTE DES FICHIERS SOURCES ---
# J'ai ajouté src/error_handling.c à la liste ci-dessous
SRC = src/main.c \
      src/ihm.c \
      src/db_manager.c \
      src/dao_musique.c \
      src/importer.c \
      src/audio_engine.c \
      src/dao_playlist.c \
      src/error_handling.c \
      src/dao_paroles.c

# Conversion des .c en .o dans le dossier obj/
OBJ = $(SRC:src/%.c=obj/%.o)
EXEC = music_app

# --- RÈGLES DE COMPILATION ---

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf obj $(EXEC)

# --- COMMANDE SPÉCIALE "make run" ---
# Utile si vous êtes sur WSL pour le son
run: $(EXEC)
	@export PULSE_SERVER=unix:/mnt/wslg/PulseServer && \
	export GST_PLUGIN_FEATURE_RANK=pulsesink:MAX && \
	./$(EXEC)

# --- TESTS UNITAIRES ---
# On exclut main.c et ihm.c car les tests n'ont pas besoin d'interface graphique
OBJ_TEST = obj/db_manager.o obj/dao_musique.o obj/dao_playlist.o obj/error_handling.o obj/test_unitaire.o

# Compilation du fichier de test
obj/test_unitaire.o: src/test_unitaire.c
	$(CC) -o $@ -c $< $(CFLAGS)

# Création de l'exécutable de test
tests: $(OBJ_TEST)
	$(CC) -o run_tests $(OBJ_TEST) $(LDFLAGS)
	./run_tests