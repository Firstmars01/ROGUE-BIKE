// Module ui.application (interface) : boucle de jeu, entrées et affichage d'une partie core::Game.
// Implémentation : Application.cpp
module;
import std;

export module ui.application;

import core.grid;
import core.game;
import ui.window;
import ui.renderer;

export namespace ui {

    // Application : fenêtre, partie et renderer (libérés automatiquement, non copiable)
    class Application {
    public:
        // Ouvre la fenêtre, crée la partie de démo et charge les sprites.
        // Lève std::runtime_error si la fenêtre ou un sprite ne peut pas être chargé.
        Application();

        // Boucle principale jusqu'à la fermeture de la fenêtre
        void Run();

    private:
        // Souris : case survolée, clic gauche = déplacement ; R : nouvelle partie (graine + 1)
        void HandleInput();

        // Dessine la partie et l'aide
        void Draw() const;

        // Renvoie le dossier des sprites (copié à côté de l'exécutable par CMake)
        static std::filesystem::path SpritesDirectory();

        // Ordre important : la fenêtre est créée avant le renderer (textures) et détruite après
        Window window;
        std::uint32_t seed;
        core::Game game;
        Renderer renderer;
        std::optional<core::GridPos> hoveredTile;   // Case praticable sous la souris
    };

}
