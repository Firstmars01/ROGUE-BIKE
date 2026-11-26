// Module ui.application (interface) : boucle de jeu, entrées clavier, possession des ressources.
// Implémentation : Application.cpp
module;
import std;

export module ui.application;

import core.grid;
import core.dungeonGenerator;
import core.entities;
import ui.window;
import ui.renderer;

export namespace ui {

    // Application : fenêtre, étage, joueur, ennemis, boss et renderer (libérés automatiquement, non copiable)
    class Application {
    public:
        // Ouvre la fenêtre, génère le quartier de démo et charge les sprites.
        // Lève std::runtime_error si la fenêtre ou un sprite ne peut pas être chargé.
        Application();

        // Boucle principale jusqu'à la fermeture de la fenêtre
        void Run();

    private:
        // Souris : case survolée, clic gauche = déplacement dans la zone ; R : nouveau quartier
        void HandleInput();

        // Recalcule la zone de déplacement du joueur (après chaque déplacement ou nouveau quartier)
        void UpdateReachableTiles();

        // Renvoie les cases occupées par les ennemis et le boss (obstacles pour le joueur)
        std::vector<core::GridPos> ObstacleTiles() const;

        // Dessine l'étage, le joueur et l'aide
        void Draw() const;

        // Renvoie un étage de la taille de core.config généré avec seed
        static core::Dungeon CreateDungeon(const std::uint32_t seed);

        // Crée un ennemi sur chaque case de départ prévue par l'étage
        static std::vector<std::unique_ptr<core::Enemy>> CreateEnemies(const core::Dungeon& dungeon);

        // Renvoie le dossier des sprites (copié à côté de l'exécutable par CMake)
        static std::filesystem::path SpritesDirectory();

        // Ordre important : la fenêtre est créée avant le renderer (textures) et détruite après
        Window window;
        std::uint32_t seed;
        core::Dungeon dungeon;
        core::Player player;
        std::vector<std::unique_ptr<core::Enemy>> enemies;   // Seul propriétaire des ennemis
        core::Boss boss;                                     // Boss devant la sortie
        Renderer renderer;
        std::vector<core::GridPos> reachableTiles;     // Zone de déplacement du joueur
        std::optional<core::GridPos> hoveredTile;      // Case praticable sous la souris
    };

}
