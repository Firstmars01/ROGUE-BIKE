# Rogue Bike

Jeu de rôle tactique au tour par tour en vue isométrique (pixel art 2.5D), réalisé en C++23 moderne en 7 jours (TP noté C++ Moderne, esimed).
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
├── CMakeLists.txt          # Cibles : rogueCore, RogueBike, tests
├── cmake/
│   ├── std.cmake           # Compilation de la bibliothèque standard en module (modèle du cours)
│   ├── compile_options.cmake
│   ├── raylib.cmake        # Téléchargement de raylib
│   └── catch2.cmake        # Téléchargement de Catch2
├── src/
│   ├── core/               # Logique métier, SANS dépendance graphique
│   │   ├── Config.ixx      # Module core.config : constantes du jeu
│   │   └── Grid.ixx        # Module core.grid : GridPos, TileType, Grid (accès vérifiés)
│   └── main.cpp            # Point d'entrée : fenêtre et boucle raylib
└── tests/                  # Tests unitaires Catch2 (logique métier uniquement)
    ├── ConfigTests.cpp
    └── GridTests.cpp
```

Prévu : `src/ui/` (rendu isométrique raylib), puis les modules `core.dungeonGenerator`, `core.entities`, `core.game`, `core.pathfinding`.

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
- Lancer `tests` : Catch2 affiche le résultat des tests (`All tests passed`).

## Conventions de code

- Classes et fonctions en `CamelCase` (1re majuscule)
- Modules, espaces de noms, données, paramètres et variables en `camelCase`, sans préfixe
- Initialisation `{}`, `auto`, `const` dès que possible, aucun `new`/`delete`

## Sécurité

- Avertissements traités comme des erreurs (`-Werror`) sur tout le code du projet.
- Aucune allocation manuelle : gestion mémoire en RAII (`std::unique_ptr`, conteneurs STL).
- Accès à la grille avec vérification des bornes (`std::optional`) pour éviter les dépassements mémoire.
- Dépendances téléchargées depuis leurs dépôts officiels avec une version figée (`GIT_TAG`).
- Le jeu ne lit aucune donnée externe hormis ses ressources graphiques et ne fait aucun accès réseau.
