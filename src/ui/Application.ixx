// Module ui.application (interface) : application du jeu (fenêtre, boucle principale, entrées).
// L'interface ne dépend pas de raylib : seul Application.cpp l'utilise.
// Implémentation : Application.cpp
module;
// Import rapide de la bibliothèque standard
import std;

// Nommage et exportation de ce module
export module ui.application;

import core.dungeonGenerator;
import ui.window;
import ui.renderer;

export namespace ui {

    // Application principale : possède la fenêtre, l'étage courant et le renderer.
    // Non copiable (la fenêtre ne l'est pas) ; ressources libérées automatiquement (RAII).
    class Application {
    public:
        // Ouvre la fenêtre, charge les sprites et génère le donjon de démo (graine core::dungeonSeed).
        // Lève std::runtime_error si la fenêtre ou un sprite ne peut pas être chargé.
        Application();

        // Exécute la boucle principale jusqu'à la fermeture de la fenêtre (Échap ou croix)
        void Run();

    private:
        // Traite les touches du clavier (R : nouveau donjon)
        void HandleInput();

        // Dessine l'image courante (carte et aide à l'écran)
        void Draw() const;

        // Génère un étage complet de la taille définie dans core.config.
        // seed : graine du générateur. Renvoie le donjon généré.
        static core::Dungeon CreateDungeon(const std::uint32_t seed);

        // Renvoie le dossier des sprites, copié à côté de l'exécutable par CMake
        static std::filesystem::path SpritesDirectory();

        // L'ordre compte : la fenêtre (contexte graphique) est créée avant le renderer (textures)
        // et détruite après lui
        Window window;
        std::uint32_t seed;
        core::Dungeon dungeon;
        Renderer renderer;
    };

}
