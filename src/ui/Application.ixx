// Module ui.application (interface) : application du jeu (fenêtre, boucle principale, entrées).
// La fenêtre est gérée en RAII : ouverte par le constructeur, fermée par le destructeur.
// L'interface ne dépend pas de raylib : seul Application.cpp l'utilise.
// Implémentation : Application.cpp
module;
// Import rapide de la bibliothèque standard
import std;

// Nommage et exportation de ce module
export module ui.application;

import core.dungeonGenerator;
import ui.renderer;

export namespace ui {

    // Application principale : possède la fenêtre, l'étage courant et le renderer
    class Application {
    public:
        // Ouvre la fenêtre et génère le donjon de démo (graine core::dungeonSeed).
        // Lève std::runtime_error si la fenêtre ne peut pas être créée.
        Application();

        // Ferme la fenêtre (libération RAII)
        ~Application();

        // Une seule fenêtre : ni copie ni déplacement
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
        Application(Application&&) = delete;
        Application& operator=(Application&&) = delete;

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

        std::uint32_t seed;
        core::Dungeon dungeon;
        Renderer renderer;
    };

}
