// Module ui.renderer (interface) : dessin isométrique de la carte (sol, murs en relief, escalier).
// L'interface ne dépend pas de raylib : seul Renderer.cpp l'utilise.
// Implémentation : Renderer.cpp
module;
// Import rapide de la bibliothèque standard
import std;

// Nommage et exportation de ce module
export module ui.renderer;

import core.grid;
import core.dungeonGenerator;
import ui.isoMath;

export namespace ui {

    // Hauteur à l'écran d'un bloc de mur, en pixels
    inline constexpr float wallHeight{tileHeight};

    // Dessine une grille en vue isométrique, centrée dans la fenêtre
    class Renderer {
    public:
        // Crée un renderer qui centre une carte mapWidth x mapHeight (en cases)
        // dans une fenêtre screenWidth x screenHeight (en pixels).
        Renderer(const int mapWidth, const int mapHeight, const int screenWidth, const int screenHeight);

        // Renvoie la position écran du centre de la case (0, 0)
        ScreenPos GetOrigin() const noexcept;

        // Dessine tout l'étage dans l'ordre du peintre (x + y croissant),
        // pour que les cases proches de la caméra recouvrent celles du fond.
        // Les murs de premier plan qui cacheraient un couloir (case hors salle) sont dessinés bas.
        // À appeler entre BeginDrawing() et EndDrawing().
        void DrawDungeon(const core::Dungeon& dungeon) const;

        // Dessine une seule case selon son type (rien pour TileType::Empty), murs à pleine hauteur
        void DrawTile(const core::GridPos pos, const core::TileType type) const;

    private:
        ScreenPos origin{};
    };

}
