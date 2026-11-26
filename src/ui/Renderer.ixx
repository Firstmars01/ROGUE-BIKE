// Module ui.renderer (interface) : dessin isométrique de l'étage avec des sprites pixel art
// (rues en asphalte, places pavées, immeubles, trottoirs, plaque d'égout au départ,
// bouche de métro à l'arrivée).
// L'interface ne dépend pas de raylib : les textures sont cachées dans Renderer.cpp (idiome pimpl).
// Implémentation : Renderer.cpp
module;
// Import rapide de la bibliothèque standard
import std;

// Nommage et exportation de ce module
export module ui.renderer;

import core.grid;
import core.dungeonGenerator;
import core.entities;
import ui.isoMath;

export namespace ui {

    // Dessine un étage en vue isométrique, centré dans la fenêtre
    class Renderer {
    public:
        // Charge les sprites depuis spritesDirectory et centre une carte mapWidth x mapHeight (en cases)
        // dans une fenêtre screenWidth x screenHeight (en pixels).
        // La fenêtre doit déjà être ouverte. Lève std::runtime_error si un sprite est introuvable.
        Renderer(const int mapWidth, const int mapHeight, const int screenWidth, const int screenHeight,
                 const std::filesystem::path& spritesDirectory);

        // Libère les textures (défini dans Renderer.cpp, où le type des sprites est complet)
        ~Renderer();

        // Les textures ne sont pas copiables
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        // Renvoie la position écran du centre de la case (0, 0)
        ScreenPos GetOrigin() const noexcept;

        // Dessine tout l'étage et le joueur dans l'ordre du peintre (x + y croissant).
        // Salles en pavés, couloirs en asphalte, murs en immeubles, plaque d'égout sur le départ,
        // bouche de métro sur l'escalier, joueur (repère provisoire) sur sa case ; un mur qui cacherait un couloir
        // est remplacé par un trottoir. À appeler entre BeginDrawing() et EndDrawing().
        void DrawDungeon(const core::Dungeon& dungeon, const core::Player& player) const;

    private:
        // Ensemble des textures chargées (défini dans Renderer.cpp)
        struct Sprites;

        ScreenPos origin{};
        std::unique_ptr<Sprites> sprites;
    };

}
