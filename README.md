# Music App - catalogue musical

**Music App** est une application de bureau développée en C pour organiser et
écouter une bibliothèque musicale locale. Elle réunit dans une interface GTK 4
un catalogue persistant, l’import de fichiers audio, la gestion de playlists
et un lecteur intégré avec affichage de paroles.

Ce projet a été réalisé en collaboration par **HELLE Carmelle** et
**ATSA NANGOU**.

## Fonctionnalités

- **Bibliothèque musicale** : import récursif d’un dossier et de ses
  sous-dossiers, extraction des métadonnées disponibles (titre, artiste, album,
  genre, durée, année et numéro de piste), avec une interprétation du nom de
  fichier lorsque les tags sont absents.
- **Formats reconnus à l’import** : MP3, WAV, FLAC, OGG et M4A. La lecture
  dépend également des décodeurs GStreamer installés sur le système.
- **Recherche et organisation** : recherche dans le catalogue, tri des titres,
  gestion des favoris et modification des informations d’un morceau.
- **Playlists** : création, renommage et suppression de playlists, ajout et
  retrait de morceaux.
- **Écoute** : lecture, pause, reprise, arrêt et progression navigable dans le
  morceau.
- **Paroles** : import d’un fichier texte pour un morceau et consultation dans
  le panneau « Paroles / Karaoké ».
- **Persistance locale** : catalogue, playlists et paroles stockés dans une
  base SQLite.

## Technologies

- **C (C99)** — logique de l’application et modules de gestion des données
- **GTK 4** — interface graphique et commandes de lecture
- **SQLite 3** — stockage local du catalogue, des playlists et des paroles
- **GStreamer** — découverte des métadonnées et prise en charge de la lecture
  audio
- **libsndfile** — bibliothèque requise à la compilation par la configuration
  actuelle du projet
- **Make et GCC** — compilation et exécution des tests

## Prérequis

Le Makefile utilise `pkg-config` pour trouver les bibliothèques suivantes :
`gtk4`, `sqlite3`, `sndfile`, `gstreamer-1.0` et `gstreamer-pbutils-1.0`.
Un environnement Linux avec une session graphique est recommandé. Sous
Windows, le projet peut être compilé et exécuté depuis WSL avec la prise en
charge graphique WSLg.

Sur Debian ou Ubuntu, les paquets de développement peuvent être installés avec :

```bash
sudo apt update
sudo apt install build-essential pkg-config libgtk-4-dev libsqlite3-dev \
  libsndfile1-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
```

GStreamer doit aussi disposer des greffons de décodage correspondant aux
fichiers audio de la bibliothèque. La disponibilité des codecs dépend de la
configuration du système.

## Compilation et lancement

À la racine du projet :

```bash
make
./music_app
```

Pour lancer l’application sous WSLg, le Makefile fournit également :

```bash
make run
```

Au démarrage, l’application initialise la base `db/music_catalog.db` et crée
les tables nécessaires si elles n’existent pas encore. Dans l’interface, le
bouton d’ajout permet de sélectionner un dossier à analyser. Les fichiers déjà
importés sont détectés comme doublons.

## Tests

Le Makefile contient une cible de tests unitaires :

```bash
make tests
```

Cette cible compile et exécute les tests du catalogue et des playlists. Elle
utilise la base SQLite configurée par l’application (`db/music_catalog.db`) ;
les données créées lors des tests peuvent donc y rester.

## Organisation du projet

```text
.
├── include/       Interfaces des modules et structures partagées
├── src/           Application GTK, lecteur, import et accès aux données
├── db/            Base de données SQLite locale
├── diagramme/     Diagrammes de conception
├── Musique/       Fichiers audio d’exemple
├── Makefile       Compilation, lancement et tests
└── README.md      Présentation et instructions du projet
```

Les sources sont organisées par responsabilité : `ihm.c` gère l’interface,
`audio_engine.c` la lecture, `importer.c` le scan et les métadonnées,
`db_manager.c` l’initialisation SQLite, et les modules `dao_*.c` les opérations
sur les morceaux, playlists et paroles.