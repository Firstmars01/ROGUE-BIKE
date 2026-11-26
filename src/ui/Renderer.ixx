// Module ui.renderer (interface) : dessin isométrique de l'étage et du joueur avec des sprites pixel art.
// raylib reste cachée dans Renderer.cpp (idiome pimpl). Implémentation : Renderer.cpp
module;
import std;

export module ui.renderer;

import core.grid;
import core.dungeonGenerator;
import core.entities;
import ui.isoMath;

export namespace ui {

    // Dessine un étage centré dans la fenêtre
    class Renderer {
    public:
        // Charge les sprites de spritesDirectory (fenêtre déjà ouverte) et centre une carte
        // mapWidth x mapHeight dans un écran screenWidth x screenHeight.
        // Lève std::runtime_error si un sprite est introuvable.
        Renderer(const int mapWidth, const int mapHeight, const int screenWidth, const int screenHeight,
                 const std::filesystem::path& spritesDirectory);

        // Libère les textures
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        // Dessine l'étage et le joueur dans l'ordre du peintre (x + y croissant).
        // Pavés (salles), asphalte (couloirs), immeubles (murs) ou trottoirs (murs devant un couloir),
        // plaque d'égout (départ), métro (sortie). À appeler entre BeginDrawing() et EndDrawing().
        void DrawDungeon(const core::Dungeon& dungeon, const core::Player& player) const;

    private:
        // Textures chargées (définies dans Renderer.cpp)
        struct Sprites;

        ScreenPos origin{};
        std::unique_ptr<Sprites> sprites;
    };

}
