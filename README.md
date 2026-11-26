# Rogue Bike

Jeu de rôle tactique au tour par tour en vue isométrique (pixel art 2.5D), réalisé en C++23 moderne.
Le joueur parcourt des quartiers générés procéduralement (places reliées par des rues), part d'une plaque d'égout, affronte des ennemis et rejoint la bouche de métro pour descendre à l'étage suivant.

## Technologies

- C++23 avec modules (`.ixx` / `.cpp`, `import std;`)
- LLVM/Clang 22 (LLVM-MinGW UCRT sous Windows), CMake ≥ 3.29, Ninja
- [raylib 5.5](https://www.raylib.com/) pour l'affichage (récupérée par `FetchContent`)
- [Catch2 v3](https://github.com/catchorg/Catch2) pour les tests unitaires (récupérée par `FetchContent`)
- Python 3 + Pillow (optionnel) pour régénérer les sprites
- IDE : JetBrains CLion

## Arborescence

```
.
├── CMakeLists.txt          # Cibles : rogueCore, rogueUi, RogueBike, tests, copyAssets
├── cmake/
│   ├── std.cmake           # Compilation de la bibliothèque standard en module (modèle du cours)
│   ├── compile_options.cmake
│   ├── raylib.cmake        # Téléchargement de raylib
│   └── catch2.cmake        # Téléchargement de Catch2
├── assets/sprites/         # Sprites pixel art isométriques (copiés à côté de l'exécutable au build)
├── tools/
│   └── GenerateSprites.py  # Script Python (Pillow) qui régénère les sprites
├── src/
│   ├── core/               # Bibliothèque rogueCore : logique métier, SANS dépendance graphique
│   │   ├── Config.ixx                                  # core.config : fenêtre, taille de carte, graine
│   │   ├── Exceptions.ixx / Exceptions.cpp             # core.exceptions : GameException
│   │   ├── Grid.ixx / Grid.cpp                         # core.grid : GridPos, TileType, Grid (accès vérifiés)
│   │   ├── DungeonGenerator.ixx / DungeonGenerator.cpp # core.dungeonGenerator : salles, rues, départ, sortie
│   │   └── Entities.ixx / Entities.cpp                 # core.entities : Entity (abstraite), Player
│   ├── ui/                 # Bibliothèque rogueUi : affichage (seule partie qui utilise raylib)
│   │   ├── IsoMath.ixx                       # ui.isoMath : projection grille -> écran constexpr (tuiles 32x16)
│   │   ├── Window.ixx / Window.cpp           # ui.window : fenêtre raylib en RAII
│   │   ├── Renderer.ixx / Renderer.cpp       # ui.renderer : sprites pixel art (RAII), ordre du peintre
│   │   └── Application.ixx / Application.cpp # ui.application : boucle de jeu, clavier
│   └── main.cpp            # Point d'entrée : crée l'application et la lance
└── tests/                  # Tests unitaires Catch2 (logique métier + mathématiques de projection)
    ├── ConfigTests.cpp
    ├── GridTests.cpp
    ├── DungeonGeneratorTests.cpp
    ├── EntitiesTests.cpp
    └── IsoMathTests.cpp
```

## Installation

1. Installer la toolchain : LLVM-MinGW 22.x (UCRT), CMake, Ninja, CLion.
2. Cloner le dépôt :
   ```
   git clone https://github.com/Firstmars01/ROGUE-BIKE.git
   ```
3. Ouvrir le dossier dans CLion (toolchain LLVM). Le premier chargement de CMake télécharge raylib et Catch2.

### Sprites (optionnel)

Les PNG de `assets/sprites/` sont versionnés. Pour les modifier puis les régénérer :
```
pip install pillow
python tools/GenerateSprites.py
```

## Lancement

- Dans CLion : configuration **RogueBike** pour le jeu, **tests** pour les tests unitaires.
- En ligne de commande :
  ```
  cmake -B build -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug
  cmake --build build
  .\build\RogueBike.exe
  .\build\tests.exe
  ```
  Les sprites sont copiés automatiquement dans `build/assets/` (cible `copyAssets`).

## Exemples d'utilisation

- Lancer `RogueBike` : le quartier s'affiche en vue isométrique dans une fenêtre 1920x1080, caméra fixe montrant toute la carte.
- Le joueur (repère provisoire : cercle bleu) apparaît sur la plaque d'égout. Il ne se déplace pas encore.
- `R` : génère le quartier suivant (graine + 1, affichée en haut à gauche) et replace le joueur au départ.
- `Échap` ou la croix : quitter.
- Lancer `tests` : Catch2 affiche le résultat des tests (`All tests passed`).

### Génération et rendu

- Carte de 60x45 cases, 7 à 9 salles de 5x5 à 10x10, reliées par des couloirs en L.
- Thème urbain :
  - places pavées (salles) et rues en asphalte (couloirs) ;
  - immeubles de 2 étages en 3 variantes (brique, béton, crépi) pour les murs ;
  - trottoirs gris à la place des immeubles qui cacheraient une rue ;
  - plaque d'égout sur la case de départ, bouche de métro sur la sortie.
- Les murs font toujours une seule case d'épaisseur.
- Le quartier de démo est toujours le même (graine fixe `dungeonSeed` dans `core.config`).

## Conventions de code

- Classes et fonctions en `CamelCase` (1re majuscule)
- Modules, espaces de noms, données, paramètres et variables en `camelCase`, sans préfixe
- Chaque module est séparé en interface `.ixx` (déclarations commentées : rôle, paramètres, retour) et implémentation `.cpp` (`module nom;`, commentaires sur les étapes importantes uniquement) ; seules les constantes et fonctions `constexpr` restent dans l'interface
- Pas de code mort : une fonction n'est ajoutée que lorsqu'elle sert
- `struct` uniquement pour les données sans code (`GridPos`, `Dungeon`), `class` pour le reste
- `const` sur les paramètres, y compris dans les déclarations des `.ixx`
- Initialisation `{}`, `auto`, `const` dès que possible
- Gestion mémoire : `std::unique_ptr` pour la possession, références pour l'accès ; aucun `new`/`delete`, ni `std::shared_ptr`/`std::weak_ptr` ; pas de `std::span`
- raylib n'est incluse que dans les `.cpp` de `ui` : les interfaces des modules n'en dépendent pas

## Sécurité

- Avertissements traités comme des erreurs (`-Werror`) sur tout le code du projet.
- Aucune allocation manuelle : gestion mémoire en RAII (`std::unique_ptr`, conteneurs STL).
- Fenêtre raylib ouverte et fermée par `ui::Window` (RAII, non copiable), créée avant les textures et détruite après elles.
- Les erreurs fatales sont attrapées dans `main` et affichées avec un message court, sans détail interne.
- Accès à la grille avec vérification des bornes (`std::optional`) pour éviter les dépassements mémoire.
- Valeurs invalides (dimensions de grille, de carte ou de salle, PV ou attaque ≤ 0, génération impossible) refusées par une exception dédiée `core::GameException`.
- Les PV ne deviennent jamais négatifs et des dégâts négatifs sont ignorés.
- Programmation par contrat : `assert` sur les préconditions internes (actif en Debug).
- Génération bornée : nombre d'essais de placement et de passes d'amincissement limité (pas de boucle infinie).
- Dépendances téléchargées depuis leurs dépôts officiels avec une version figée (`GIT_TAG`).
- Le jeu ne lit aucune donnée externe hormis ses sprites, chargés depuis un chemin fixe relatif à l'exécutable (aucune donnée utilisateur dans le chemin), et ne fait aucun accès réseau.
- Un sprite manquant provoque une erreur claire (nom du fichier uniquement, sans chemin complet).
