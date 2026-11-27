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
        const std::optional<core::Boss>& boss;              // Vide si le boss est mort
        const std::vector<core::GridPos>& reachableTiles;   // Zone de déplacement du joueur (bleu)
        const std::vector<core::GridPos>& attackTargets;    // Cases visables avec l'attaque choisie (orange)
        const std::vector<core::GridPos>& attackArea;       // Zone touchée par l'attaque visée (rouge vif)
        const std::vector<core::GridPos>& dangerTiles;      // Frappes imminentes des adversaires (rouge)
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

        // Dessine la scène à travers la caméra (zoom, centrée sur le joueur à partir de x2),
        // dans l'ordre du peintre (x + y croissant) : pavés (salles), asphalte (couloirs),
        // immeubles (transparents s'ils cachent une entité ou la case visée) ou trottoirs (murs),
        // plaque d'égout (départ), métro (sortie), quadrillage, surcouches de la scène (déplacement,
        // visée, zones de frappe, survol), personnages (motard, grisé s'il est immobilisé ; voyous en trottinette
        // ou en rollers ; boss cycliste sur ses 2x2 cases), barres de vie et compteurs de frappe.
        // Le joueur regarde vers la case visée, les adversaires vers le joueur.
        // À appeler entre Window::BeginFrame() et Window::EndFrame().
        void DrawScene(const SceneView& scene) const;

        // Paliers de zoom : x1 = vue fixe de toute la carte ; à partir de x2, caméra centrée sur le joueur
        static constexpr int minZoom{1};
        static constexpr int maxZoom{4};

        // Augmente le zoom d'un palier (sans dépasser maxZoom)
        void ZoomIn() noexcept;

        // Diminue le zoom d'un palier (sans descendre sous minZoom)
        void ZoomOut() noexcept;

        // Renvoie le palier de zoom actuel
        int GetZoom() const noexcept;

        // Renvoie la case située sous le point screen (coordonnées du canevas), en tenant compte du zoom ;
        // focus : case suivie par la caméra (le joueur). Le résultat peut être hors de la carte.
        core::GridPos CellAt(const ScreenPos screen, const core::GridPos focus) const noexcept;

    private:
        // Textures chargées (définies dans Renderer.cpp)
        struct Sprites;

        // Renvoie la vue de la caméra : fixe à x1, centrée sur la case focus à partir de x2
        CameraView View(const core::GridPos focus) const noexcept;

        // Dessine la scène en coordonnées du monde (appelée entre BeginMode2D et EndMode2D)
        void DrawWorld(const SceneView& scene) const;

        ScreenPos origin{};
        ScreenPos screenCenter{};
        int zoom{minZoom};
        std::unique_ptr<Sprites> sprites;
    };

}
