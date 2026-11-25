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
├── src/
│   ├── core/               # Logique métier, SANS dépendance graphique
│   │   ├── Config.ixx      # Module core.config : constantes du jeu
│   │   ├── Exceptions.ixx / Exceptions.cpp             # core.exceptions : InvalidDimensionException
│   │   ├── Grid.ixx / Grid.cpp                         # core.grid : GridPos, TileType, Grid (accès vérifiés)
│   │   └── DungeonGenerator.ixx / DungeonGenerator.cpp # core.dungeonGenerator : salles, couloirs en L, escalier
│   ├── ui/                 # Affichage (bibliothèque rogueUi)
│   │   └── IsoMath.ixx     # ui.isoMath : projection isométrique constexpr (tuiles 32x16)
│   └── main.cpp            # Point d'entrée : fenêtre et boucle raylib
└── tests/                  # Tests unitaires Catch2 (logique métier uniquement)
    ├── ConfigTests.cpp
    ├── GridTests.cpp
    ├── DungeonGeneratorTests.cpp
    └── IsoMathTests.cpp
```

Prévu : `ui.renderer` (dessin isométrique raylib), puis les modules `core.entities`, `core.game`, `core.pathfinding`.

## Installation

1. Installer la toolchain du TP0 : LLVM-MinGW 22.x (UCRT), CMake, Ninja, CLion.
2. Cloner le dépôt :
   ```
   git clone https://github.com/Firstmars01/ROGUE-BIKE.git
   ```
3. Ouvrir le dossier dans CLion (toolchain LLVM). Le premier chargement de CMake télécharge raylib et Catch2.

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

- Lancer `RogueBike` : une fenêtre 1280x720 s'ouvre. `Échap` ou la croix pour quitter.
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
- Aucune allocation manuelle : gestion mémoire en RAII (`std::unique_ptr`, conteneurs STL).
- Accès à la grille avec vérification des bornes (`std::optional`) pour éviter les dépassements mémoire.
- Dimensions invalides (grille, carte, salle) refusées par une exception dédiée `core::InvalidDimensionException`.
- Programmation par contrat : `assert` sur les préconditions internes (actif en Debug).
- Dépendances téléchargées depuis leurs dépôts officiels avec une version figée (`GIT_TAG`).
- Le jeu ne lit aucune donnée externe hormis ses ressources graphiques et ne fait aucun accès réseau.
