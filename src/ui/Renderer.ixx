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

    // Ce qu'il faut dessiner pour une image (références vers les données de l'application)
    struct SceneView {
        const core::Dungeon& dungeon;
        const core::Player& player;
        const std::vector<std::unique_ptr<core::Enemy>>& enemies;
        const core::Boss& boss;
        const std::vector<core::GridPos>& reachableTiles;   // Zone de déplacement du joueur
        std::optional<core::GridPos> hoveredTile;           // Case praticable sous la souris
    };

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

        // Dessine la scène dans l'ordre du peintre (x + y croissant) : pavés (salles), asphalte (couloirs),
        // immeubles (transparents s'ils cachent une entité ou la case visée) ou trottoirs (murs),
        // plaque d'égout (départ), métro (sortie), quadrillage,
        // zone de déplacement en bleu, case survolée éclaircie, joueur (point bleu), ennemis (points rouges),
        // boss (gros point rouge foncé centré sur ses 2x2 cases), barre de vie au-dessus de chaque entité.
        // À appeler entre Window::BeginFrame() et Window::EndFrame().
        void DrawScene(const SceneView& scene) const;

        // Renvoie la case située sous le point screen (peut être hors de la carte)
        core::GridPos CellAt(const ScreenPos screen) const noexcept;

    private:
        // Textures chargées (définies dans Renderer.cpp)
        struct Sprites;

        ScreenPos origin{};
        std::unique_ptr<Sprites> sprites;
    };

}
