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
        // plaque d'égout (départ), métro (sortie), quadrillage sur les cases praticables,
        // zone de déplacement reachableTiles en bleu et case hoveredTile (sous la souris) éclaircie.
        // À appeler entre Window::BeginFrame() et Window::EndFrame().
        void DrawDungeon(const core::Dungeon& dungeon, const core::Player& player,
                         const std::vector<core::GridPos>& reachableTiles,
                         const std::optional<core::GridPos> hoveredTile) const;

        // Renvoie la case située sous le point screen (peut être hors de la carte)
        core::GridPos CellAt(const ScreenPos screen) const noexcept;

    private:
        // Textures chargées (définies dans Renderer.cpp)
        struct Sprites;

        ScreenPos origin{};
        std::unique_ptr<Sprites> sprites;
    };

}
