// Module ui.renderer (implémentation) : chargement des sprites (RAII) et dessin des tuiles avec raylib.
// Interface : Renderer.ixx
module;
// raylib est incluse uniquement ici (fragment de module global) : elle reste invisible
// pour les modules qui importent ui.renderer
#include <raylib.h>
// Import rapide de la bibliothèque standard
import std;

// Unité d'implémentation du module ui.renderer
module ui.renderer;

namespace ui {

    // Texture raylib gérée en RAII : chargée par le constructeur, libérée par le destructeur
    class TileSprite {
    public:
        // Charge l'image file. Lève std::runtime_error si elle est introuvable ou invalide.
        explicit TileSprite(const std::filesystem::path& file) : texture{LoadTexture(file.string().c_str())} {
            if (texture.id == 0) {
                // Seul le nom du fichier est affiché, pas le chemin complet
                throw std::runtime_error{std::format("Sprite introuvable : {}", file.filename().string())};
            }
        }

        // Libère la texture de la mémoire graphique
        ~TileSprite() {
            UnloadTexture(texture);
        }

        TileSprite(const TileSprite&) = delete;
        TileSprite& operator=(const TileSprite&) = delete;

        // Dessine le sprite sur la case de centre center : le bas du sprite est aligné
        // sur le sommet bas du losange, ce qui permet des sprites plus hauts qu'une tuile
        void DrawOnTile(const ScreenPos center) const {
            const auto position = Vector2{
                center.x - halfTileWidth,
                center.y + halfTileHeight - static_cast<float>(texture.height)
            };
            DrawTextureV(texture, position, WHITE);
        }

    private:
        Texture2D texture;
    };

    // Toutes les textures utilisées par le renderer
    struct Renderer::Sprites {
        TileSprite asphalt;
        TileSprite pavement;
        TileSprite sidewalk;
        TileSprite manhole;
        TileSprite metro;
        TileSprite brickBuilding;
        TileSprite concreteBuilding;
        TileSprite stuccoBuilding;

        // Charge chaque sprite depuis directory
        explicit Sprites(const std::filesystem::path& directory)
            : asphalt{directory / "asphalt.png"},
              pavement{directory / "pavement.png"},
              sidewalk{directory / "sidewalk.png"},
              manhole{directory / "manhole.png"},
              metro{directory / "metro.png"},
              brickBuilding{directory / "building_brick.png"},
              concreteBuilding{directory / "building_concrete.png"},
              stuccoBuilding{directory / "building_stucco.png"} {}

        // Choisit une variante d'immeuble stable selon la position (même case = même immeuble)
        const TileSprite& BuildingAt(const core::GridPos pos) const {
            switch ((pos.x * 7 + pos.y * 13) % 3) {
                case 0:
                    return brickBuilding;
                case 1:
                    return concreteBuilding;
                default:
                    return stuccoBuilding;
            }
        }
    };

    // Fonctions internes à ce fichier (non exportées)
    namespace {

        // Nombre de rangées de cases qu'un immeuble (32 px de haut = 2 tuiles) recouvre à l'écran
        constexpr int buildingRowsCovered{2};

        // Indique si pos est une case de couloir : praticable et en dehors de toutes les salles
        bool IsCorridorTile(const core::Dungeon& dungeon, const core::GridPos pos) {
            return dungeon.grid.IsWalkable(pos)
                   && std::ranges::none_of(dungeon.rooms, [pos](const core::Room& room) {
                          return room.Contains(pos);
                      });
        }

        // Indique si un immeuble en pos cacherait une case de couloir.
        // Il recouvre à l'écran les cases (x - i, y - j) situées derrière lui avec
        // |i - j| <= 1 et i + j <= 2 * buildingRowsCovered.
        bool HidesCorridorTile(const core::Dungeon& dungeon, const core::GridPos pos) {
            for (auto i = 0; i <= buildingRowsCovered; ++i) {
                for (auto j = 0; j <= buildingRowsCovered; ++j) {
                    const auto behind = (i + j > 0) && std::abs(i - j) <= 1;
                    if (behind && IsCorridorTile(dungeon, core::GridPos{pos.x - i, pos.y - j})) {
                        return true;
                    }
                }
            }
            return false;
        }

    }

    // Charge les sprites et calcule une fois l'origine qui centre la carte
    Renderer::Renderer(const int mapWidth, const int mapHeight, const int screenWidth, const int screenHeight,
                       const std::filesystem::path& spritesDirectory)
        : origin{CenteredOrigin(mapWidth, mapHeight, screenWidth, screenHeight)},
          sprites{std::make_unique<Sprites>(spritesDirectory)} {}

    // Défini ici car Sprites n'est complet que dans ce fichier
    Renderer::~Renderer() = default;

    // Origine de la projection
    ScreenPos Renderer::GetOrigin() const noexcept {
        return origin;
    }

    // Parcours par diagonales : sum = x + y croissant (ordre du peintre)
    void Renderer::DrawDungeon(const core::Dungeon& dungeon) const {
        const auto& grid = dungeon.grid;
        const auto width = grid.GetWidth();
        const auto height = grid.GetHeight();
        for (auto sum = 0; sum <= width + height - 2; ++sum) {
            // Sur une diagonale, x varie de façon à garder y = sum - x dans la grille
            const auto firstX = std::max(0, sum - (height - 1));
            const auto lastX = std::min(sum, width - 1);
            for (auto x = firstX; x <= lastX; ++x) {
                const auto pos = core::GridPos{x, sum - x};
                const auto center = GridToScreen(pos, origin);
                switch (grid.GetTile(pos).value_or(core::TileType::Empty)) {
                    case core::TileType::Floor:
                        if (pos == dungeon.playerStart) {
                            // Départ : plaque d'égout
                            sprites->manhole.DrawOnTile(center);
                        } else {
                            (IsCorridorTile(dungeon, pos) ? sprites->asphalt : sprites->pavement).DrawOnTile(center);
                        }
                        break;
                    case core::TileType::Stairs:
                        // Arrivée : bouche de métro
                        sprites->metro.DrawOnTile(center);
                        break;
                    case core::TileType::Wall:
                        // Devant un couloir : trottoir bas pour laisser voir la rue
                        (HidesCorridorTile(dungeon, pos) ? sprites->sidewalk : sprites->BuildingAt(pos))
                            .DrawOnTile(center);
                        break;
                    case core::TileType::Empty:
                        // Hors du donjon : rien à dessiner
                        break;
                }
            }
        }
    }

}
