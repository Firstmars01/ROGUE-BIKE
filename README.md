# Rogue Bike

Jeu de rôle tactique au tour par tour en vue isométrique (pixel art 2.5D), réalisé en C++23 moderne.
Le joueur explore des étages de donjon générés procéduralement, combat des ennemis et descend le plus profondément possible.

## Technologies

- C++23 avec modules (`.ixx`, `import std;`)
- LLVM/Clang 22 (LLVM-MinGW UCRT sous Windows), CMake ≥ 3.29, Ninja
- [raylib 5.5](https://www.raylib.com/) pour l'affichage (récupérée par `FetchContent`)
- [Catch2 v3](https://github.com/catchorg/Catch2) pour les tests unitaires (récupérée par `FetchContent`)
- IDE : JetBrains CLion

## Arborescence

```
.
├── CMakeLists.txt          # Cibles : rogueCore, rogueUi, RogueBike, tests
├── cmake/
│   ├── std.cmake           # Compilation de la bibliothèque standard en module (modèle du cours)
│   ├── compile_options.cmake
│   ├── raylib.cmake        # Téléchargement de raylib
│   └── catch2.cmake        # Téléchargement de Catch2
├── assets/sprites/         # Sprites pixel art isométriques (copiés à côté de l'exécutable au build)
├── tools/
│   └── GenerateSprites.py  # Script Python (Pillow) qui régénère les sprites
├── src/
│   ├── core/               # Logique métier, SANS dépendance graphique
│   │   ├── Config.ixx      # Module core.config : constantes du jeu
│   │   ├── Exceptions.ixx / Exceptions.cpp             # core.exceptions : InvalidDimensionException
│   │   ├── Grid.ixx / Grid.cpp                         # core.grid : GridPos, TileType, Grid (accès vérifiés)
│   │   └── DungeonGenerator.ixx / DungeonGenerator.cpp # core.dungeonGenerator : salles, couloirs en L, escalier
│   ├── ui/                 # Affichage (bibliothèque rogueUi)
│   │   ├── IsoMath.ixx     # ui.isoMath : projection isométrique constexpr (tuiles 32x16)
│   │   ├── Window.ixx / Window.cpp           # ui.window : fenêtre raylib en RAII
│   │   ├── Renderer.ixx / Renderer.cpp       # ui.renderer : sprites pixel art (RAII), ordre du peintre
│   │   └── Application.ixx / Application.cpp # ui.application : boucle de jeu, clavier
│   └── main.cpp            # Point d'entrée : crée l'application et la lance
└── tests/                  # Tests unitaires Catch2 (logique métier uniquement)
    ├── ConfigTests.cpp
    ├── GridTests.cpp
    ├── DungeonGeneratorTests.cpp
    └── IsoMathTests.cpp
```

Prévu : les modules `core.entities`, `core.game`, `core.pathfinding`.

## Installation

1. Installer la toolchain du TP0 : LLVM-MinGW 22.x (UCRT), CMake, Ninja, CLion.
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

## Exemples d'utilisation

- Lancer `RogueBike` : le donjon s'affiche en vue isométrique dans une fenêtre 1280x720.
- `R` : génère le donjon suivant (graine + 1, affichée en haut à gauche).
- `Échap` ou la croix : quitter.
- Thème urbain : places pavées (salles), rues en asphalte (couloirs), immeubles (murs, 3 variantes), trottoirs (murs placés devant une rue, abaissés pour la laisser visible), bouche de métro (sortie de l'étage).
- Les murs font toujours une seule case d'épaisseur.
- Le donjon de démo est toujours le même (carte 40x30, graine fixe `dungeonSeed` dans `core.config`).
- Lancer `tests` : Catch2 affiche le résultat des tests (`All tests passed`).

## Conventions de code

- Classes et fonctions en `CamelCase` (1re majuscule)
- Modules, espaces de noms, données, paramètres et variables en `camelCase`, sans préfixe
- Chaque module est séparé en interface `.ixx` (déclarations commentées) et implémentation `.cpp` (`module nom;`) ; seules les constantes et fonctions `constexpr` restent dans l'interface
- `struct` uniquement pour les données sans code (`GridPos`, `Dungeon`), `class` pour le reste
- `const` sur les paramètres, y compris dans les déclarations des `.ixx`
- Initialisation `{}`, `auto`, `const` dès que possible, aucun `new`/`delete`

## Sécurité

- Avertissements traités comme des erreurs (`-Werror`) sur tout le code du projet.
- Aucune allocation manuelle : gestion mémoire en RAII (`std::unique_ptr`, conteneurs STL) ; la fenêtre raylib est ouverte et fermée par `ui::Application` (RAII, non copiable).
- Les erreurs fatales sont attrapées dans `main` et affichées avec un message court, sans détail interne.
- Accès à la grille avec vérification des bornes (`std::optional`) pour éviter les dépassements mémoire.
- Dimensions invalides (grille, carte, salle) refusées par une exception dédiée `core::InvalidDimensionException`.
- Programmation par contrat : `assert` sur les préconditions internes (actif en Debug).
- Dépendances téléchargées depuis leurs dépôts officiels avec une version figée (`GIT_TAG`).
- Le jeu ne lit aucune donnée externe hormis ses sprites, chargés depuis un chemin fixe relatif à l'exécutable (aucune donnée utilisateur dans le chemin), et ne fait aucun accès réseau.
- Un sprite manquant provoque une erreur claire (nom du fichier uniquement, sans chemin complet) ; textures et fenêtre sont libérées en RAII dans le bon ordre.
