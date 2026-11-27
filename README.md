# Rogue Bike

Jeu de rôle tactique en temps réel en vue isométrique (pixel art 2.5D), réalisé en C++23 moderne (TP noté C++ Moderne, esimed).
Le joueur, un motard, parcourt des quartiers générés procéduralement (places reliées par des rues), part d'une plaque d'égout, affronte des voyous et un chef de gang, puis rejoint la bouche de métro pour descendre à l'étage suivant.

## Technologies

- C++23 avec modules (`.ixx` / `.cpp`, `import std;`)
- LLVM/Clang 22 (LLVM-MinGW UCRT), CMake ≥ 3.29, Ninja
- [raylib 5.5](https://www.raylib.com/) pour l'affichage (récupérée par `FetchContent`)
- [Catch2 v3](https://github.com/catchorg/Catch2) pour les tests unitaires (récupérée par `FetchContent`)
- Python 3 + Pillow (optionnel) pour régénérer les sprites
- IDE : JetBrains CLion
- Plateforme testée : Windows uniquement

## Arborescence

```
.
├── CMakeLists.txt          # Cibles : rogueCore, rogueUi, RogueBike, tests, copyAssets
├── cmake/
│   ├── std.cmake           # Bibliothèque standard compilée en module (modèle du cours)
│   ├── compile_options.cmake
│   ├── raylib.cmake        # Téléchargement de raylib 5.5
│   └── catch2.cmake        # Téléchargement de Catch2 v3.7.0
├── assets/sprites/         # Sprites pixel art isométriques (copiés à côté de l'exécutable au build)
├── tools/
│   └── GenerateSprites.py  # Script Python (Pillow) qui régénère les sprites (décor et personnages)
├── src/
│   ├── core/               # Bibliothèque rogueCore : logique du jeu, SANS dépendance graphique
│   │   ├── Config.ixx                                  # core.config : fenêtre, taille de carte, graine
│   │   ├── Exceptions.ixx / Exceptions.cpp             # core.exceptions : GameException
│   │   ├── Grid.ixx / Grid.cpp                         # core.grid : GridPos, Direction, TileType, Grid
│   │   ├── Attacks.ixx / Attacks.cpp                   # core.attacks : Attack, zones d'effet, ligne de vue, Cooldown
│   │   ├── Pathfinding.ixx / Pathfinding.cpp           # core.pathfinding : ReachableTiles (parcours en largeur)
│   │   ├── Entities.ixx / Entities.cpp                 # core.entities : Entity -> Player ; Entity -> Hostile -> Enemy, Boss
│   │   ├── DungeonGenerator.ixx / DungeonGenerator.cpp # core.dungeonGenerator : Room, Dungeon, génération
│   │   └── Game.ixx / Game.cpp                         # core.game : règles d'une partie en temps réel
│   ├── ui/                 # Bibliothèque rogueUi : affichage et entrées (seule partie qui utilise raylib)
│   │   ├── IsoMath.ixx                       # ui.isoMath : projection grille <-> écran, caméra (constexpr)
│   │   ├── Window.ixx / Window.cpp           # ui.window : fenêtre RAII + canevas virtuel 1920x1080
│   │   ├── Renderer.ixx / Renderer.cpp       # ui.renderer : sprites (RAII), ordre du peintre, repères, surcouches
│   │   ├── Hud.ixx / Hud.cpp                 # ui.hud : barre des attaques (boutons, cooldowns)
│   │   └── Application.ixx / Application.cpp # ui.application : boucle, entrées, affichage d'une core::Game
│   └── main.cpp            # Point d'entrée : crée l'application et la lance
└── tests/                  # Tests unitaires Catch2 (logique du jeu + projection isométrique)
    ├── ConfigTests.cpp
    ├── GridTests.cpp
    ├── AttacksTests.cpp
    ├── PathfindingTests.cpp
    ├── EntitiesTests.cpp
    ├── DungeonGeneratorTests.cpp
    ├── IsoMathTests.cpp
    └── GameTests.cpp
```

## Installation

1. Installer la toolchain : LLVM-MinGW 22.x (UCRT), CMake, Ninja, CLion.
2. Cloner le dépôt :
   ```
   git clone https://github.com/Firstmars01/ROGUE-BIKE.git
   ```
3. Ouvrir le dossier dans CLion (toolchain LLVM). Le premier chargement de CMake télécharge raylib et Catch2.

Remarque : CLion affiche aussi `catch2-src` et `raylib-src` dans ses dépôts Git. Ce sont les sources téléchargées par `FetchContent` dans `cmake-build-debug/_deps/`, ignorées par le `.gitignore`.

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

## Utilisation

### Commandes

| Action | Effet |
|---|---|
| Survol de la souris | La case visée est éclaircie |
| Clic gauche sur une case bleue (aucune attaque choisie) | Le joueur s'y téléporte (un clic hors de la zone ne fait rien) |
| `1`, `2`, `3` ou clic sur un bouton en bas | Choisit le Coup de casque, le Klaxon ou le Cocktail (un second appui annule) |
| Clic gauche avec une attaque choisie | Attaque la case visée (le Klaxon frappe toujours autour du joueur) ; retour au déplacement si l'attaque part |
| Clic droit | Annule l'attaque choisie |
| Molette | Zoom par paliers x1 à x4 (x1 : vue fixe de toute la carte ; à partir de x2 : caméra centrée sur le joueur) |
| `R` | Génère le quartier suivant (graine + 1) et replace tout le monde |
| `Échap` ou la croix | Quitte le jeu |

### Affichage

- Plein écran sans bordure. Le jeu est dessiné sur un canevas fixe de 1920x1080, mis à l'échelle et centré si l'écran a une autre taille ; la souris est convertie de la même façon.
- Caméra : au lancement (zoom x1), vue fixe de toute la carte ; à partir de x2, la caméra se recentre instantanément sur le joueur. Paliers entiers pour garder le pixel art net. La barre d'attaques et l'aide ne sont pas zoomées.
- La graine, le zoom courant et l'aide sont affichés en haut à gauche.

### Entités

| Entité | Sprite | PV | Attaques | Règles |
|---|---|---|---|---|
| Joueur (motard) | Motard à pied, casque rouge ; barre verte | 100 | Casque, Klaxon, Cocktail | Part de la plaque d'égout ; se déplace d'au plus `speed` = 4 cases en marchant |
| Ennemi (voyou rapide ou costaud, au hasard) | Rapide : sweat vert en trottinette ; costaud : blouson bordeaux en rollers ; barre rouge | 75 | Couteau (rapide) ou Batte (costaud) | 2 à 5 par salle selon sa surface (5x5 : 2, 10x10 : 5) ; jamais dans la salle de départ ni dans celle du boss |
| Boss (chef de gang) | Cycliste en maillot jaune sur son vélo (sur ses 2x2 cases) ; barre rouge large | 1000 | Onde de choc | Occupe 2x2 cases, collé à la bouche de métro du côté d'où arrive le couloir ; seul dans sa salle (au moins 8x8) |

- La zone de déplacement du joueur s'affiche en bleu : cases atteignables en marchant (murs contournés) en au plus 4 pas.
- Les ennemis et le boss sont des obstacles : le joueur ne peut ni s'arrêter sur leurs cases ni passer à travers. La sortie reste toujours accessible en contournant le boss.
- Les adversaires ne marchent pas : ils frappent leur zone à intervalle régulier puis sautent sur une autre case de leur salle.

### Génération et rendu

- Carte de 60x45 cases, 7 à 9 salles de 5x5 à 10x10, reliées par des couloirs en L ; murs toujours d'une seule case d'épaisseur.
- Thème urbain :
  - places pavées (salles) et rues en asphalte (couloirs) ;
  - immeubles de 2 étages en 3 variantes (brique, béton, crépi) pour les murs ;
  - trottoirs gris à la place des immeubles qui cacheraient une rue ;
  - plaque d'égout sur la case de départ, bouche de métro sur la sortie ;
  - quadrillage fin sur toutes les cases praticables.
- Un immeuble qui cache le joueur, un ennemi, le boss ou la case visée devient à moitié transparent.
- Le quartier de démo est toujours le même (graine fixe `dungeonSeed` dans `core.config`).

### Attaques (modules `core.attacks`, `core.entities` et `core.game`)

Portée et zones carrées mesurées en cases, diagonales comprises (distance de Tchebychev). Les murs bloquent la ligne de vue et la zone d'effet. Temps en secondes (`std::chrono`).

| Qui | Attaque | Portée | Zone | Dégâts | Cooldown | Immobilisation |
|---|---|---|---|---|---|---|
| Joueur | Coup de casque | 1 | 1 case | 25 | 0,5 s | 1 s |
| Joueur | Klaxon | 0 (autour de lui) | carré de rayon 2 | 20 | 3 s | 1,5 s |
| Joueur | Cocktail | 5 | croix de rayon 1 | 40 | 5 s | 2 s |
| Ennemi rapide | Couteau | autour de lui | carré de rayon 1 | 8 | 2 s | - |
| Ennemi lourd | Batte | autour de lui | carré de rayon 2 | 25 | 5 s | - |
| Boss | Onde de choc | autour de lui | carré de rayon 3 | 40 | 4 s | - |

- Joueur : chaque attaque a son propre cooldown ; après une attaque, il est immobilisé (colonne Immobilisation). En combat, deux déplacements sont espacés d'au moins 0,6 s.
- Ennemis et boss (`Hostile`) : immobiles, leur compteur démarre quand le joueur entre dans leur salle (ou quand il les touche de loin) ; à 0, ils frappent leur zone puis le compteur repart.
- Après chaque frappe, l'adversaire saute sur une case de sol libre de sa salle, à 3 cases au plus (jamais sur le joueur, un autre adversaire ou la sortie), en visant une distance au joueur qui lui permet de le toucher : 1 case pour le rapide, 2 pour le costaud, 3 pour le boss (qui saute avec ses 4 cases). Parmi les meilleures cases, le choix est tiré au hasard (reproductible avec la même graine).
- `core::Game::Update(dt)` fait avancer la partie à chaque image (temps réel) : minuteries, activation, frappes, retrait des morts (`std::erase_if`), fin de partie (`GameState::GameOver`, sans écran pour l'instant).
- Affichage du combat :
  - barre d'attaques en bas : nom, touche, dégâts ou temps restant, voile gris qui descend avec le cooldown, attaque choisie encadrée en jaune ;
  - attaque choisie : cases visables en orange, zone touchée sous la souris en rouge vif (même règle que l'attaque réelle) ;
  - adversaires actifs : temps avant leur prochaine frappe au-dessus de leur barre de vie ; leur zone de frappe s'affiche en rouge pendant la dernière seconde ;
  - joueur immobilisé après une attaque : son sprite est grisé.
- Orientation : le joueur regarde vers la case sous la souris, les adversaires se tournent vers le joueur (sprite retourné horizontalement).
- Une attaque à distance ne peut jamais viser la case du joueur ; une attaque de portée 0 ne vise que lui.

### Tests

Lancer `tests` : Catch2 affiche le résultat (`All tests passed`). Les tests du générateur sont répétés sur 50 graines (`GENERATE`) : dimensions, placement des salles, accessibilité de la sortie et de toutes les salles, murs d'une case, placement des ennemis et du boss, reproductibilité.

## Conventions de code

- Classes et fonctions en `CamelCase` (1re majuscule)
- Modules, espaces de noms, données, paramètres et variables en `camelCase`, sans préfixe
- Chaque module est séparé en interface `.ixx` (déclarations commentées : rôle, paramètres, retour) et implémentation `.cpp` (`module nom;`, commentaires sur les étapes importantes uniquement) ; seules les constantes et fonctions `constexpr` restent dans l'interface
- Pas de code mort ni de code dupliqué
- `struct` uniquement pour les données sans code (`GridPos`, `Dungeon`, `SceneView`), `class` pour le reste
- `const` sur les paramètres, y compris dans les déclarations des `.ixx`
- Initialisation `{}`, `auto`, `const` dès que possible
- Gestion mémoire : `std::unique_ptr` pour la possession, références pour l'accès ; aucun `new`/`delete`, ni `std::shared_ptr`/`std::weak_ptr` ; pas de `std::span`
- raylib n'est incluse que dans les `.cpp` de `ui` : les interfaces des modules n'en dépendent pas

## Sécurité

- Avertissements traités comme des erreurs (`-Werror`) sur tout le code du projet.
- Aucune allocation manuelle : gestion mémoire en RAII (`std::unique_ptr`, conteneurs STL).
- Fenêtre raylib et canevas gérés par `ui::Window` (RAII, non copiable) : créés avant les textures, détruits après elles, canevas libéré avant la fermeture du contexte graphique.
- Les erreurs fatales sont attrapées dans `main` et affichées avec un message court, sans détail interne.
- Accès à la grille avec vérification des bornes (`std::optional`) pour éviter les dépassements mémoire.
- Valeurs invalides (dimensions de grille, de carte ou de salle, PV ou attaque ≤ 0, génération impossible) refusées par une exception dédiée `core::GameException`.
- Les PV ne deviennent jamais négatifs et des dégâts négatifs sont ignorés.
- Les déplacements sont validés dans `core` (portée, murs, obstacles), jamais seulement dans l'interface.
- Programmation par contrat : `assert` sur les préconditions internes (actif en Debug).
- Génération bornée : nombre d'essais de placement et de passes d'amincissement limité (pas de boucle infinie).
- Dépendances téléchargées depuis leurs dépôts officiels avec une version figée (`GIT_TAG`).
- Le jeu ne lit aucune donnée externe hormis ses sprites, chargés depuis un chemin fixe relatif à l'exécutable (aucune donnée utilisateur dans le chemin), et ne fait aucun accès réseau.
- Un sprite manquant provoque une erreur claire (nom du fichier uniquement, sans chemin complet).
