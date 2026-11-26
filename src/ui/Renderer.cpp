// Module ui.renderer (implémentation) : sprites en RAII et dessin avec raylib.
module;
#include <raylib.h>
import std;

module ui.renderer;

namespace ui {

    // Texture raylib en RAII : chargée à la construction, libérée à la destruction
    class TileSprite {
    public:
        // Charge file. Lève std::runtime_error (nom du fichier seulement) en cas d'échec.
        explicit TileSprite(const std::filesystem::path& file) : texture{LoadTexture(file.string().c_str())} {
            if (texture.id == 0) {
                throw std::runtime_error{std::format("Sprite introuvable : {}", file.filename().string())};
            }
        }

        // Libère la texture
        ~TileSprite() {
            UnloadTexture(texture);
        }

        TileSprite(const TileSprite&) = delete;
        TileSprite& operator=(const TileSprite&) = delete;

        // Dessine le sprite sur la case de centre center (bas du sprite aligné sur le bas du losange)
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

    // Toutes les textures du renderer
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

        // Renvoie la variante d'immeuble de la case pos (toujours la même pour une case)
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

    // Éléments internes à ce fichier
    namespace {

        // Repère provisoire du joueur : cercle bleu au-dessus du sol
        constexpr Color playerColor{60, 140, 230, 255};
        constexpr Color playerOutlineColor{20, 40, 80, 255};
        constexpr float playerRadius{6.0f};
        constexpr float playerElevation{8.0f};

        // Un immeuble (32 px) recouvre 2 rangées de cases derrière lui
        constexpr int buildingRowsCovered{2};

        // Dessine le repère du joueur sur la case de centre center
        void DrawPlayerMarker(const ScreenPos center) {
            const auto position = Vector2{center.x, center.y - playerElevation};
            DrawCircleV(position, playerRadius, playerColor);
            DrawCircleLines(static_cast<int>(position.x), static_cast<int>(position.y), playerRadius, playerOutlineColor);
        }

        // Indique si pos est un couloir (praticable et hors de toute salle)
        bool IsCorridorTile(const core::Dungeon& dungeon, const core::GridPos pos) {
            return dungeon.grid.IsWalkable(pos)
                   && std::ranges::none_of(dungeon.rooms, [pos](const core::Room& room) {
                          return room.Contains(pos);
                      });
        }

        // Indique si un immeuble en pos cacherait un couloir : cases (x - i, y - j) derrière lui
        // avec |i - j| <= 1 et i, j <= buildingRowsCovered
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

    Renderer::Renderer(const int mapWidth, const int mapHeight, const int screenWidth, const int screenHeight,
                       const std::filesystem::path& spritesDirectory)
        : origin{CenteredOrigin(mapWidth, mapHeight, screenWidth, screenHeight)},
          sprites{std::make_unique<Sprites>(spritesDirectory)} {}

    // Défini ici : Sprites n'est complet que dans ce fichier
    Renderer::~Renderer() = default;

    void Renderer::DrawDungeon(const core::Dungeon& dungeon, const core::Player& player) const {
        const auto& grid = dungeon.grid;
        const auto width = grid.GetWidth();
        const auto height = grid.GetHeight();
        // Parcours par diagonales (sum = x + y), en gardant y = sum - x dans la grille
        for (auto sum = 0; sum <= width + height - 2; ++sum) {
            const auto firstX = std::max(0, sum - (height - 1));
            const auto lastX = std::min(sum, width - 1);
            for (auto x = firstX; x <= lastX; ++x) {
                const auto pos = core::GridPos{x, sum - x};
                const auto center = GridToScreen(pos, origin);
                switch (grid.GetTile(pos).value_or(core::TileType::Empty)) {
                    case core::TileType::Floor:
                        if (pos == dungeon.playerStart) {
                            sprites->manhole.DrawOnTile(center);
                        } else {
                            (IsCorridorTile(dungeon, pos) ? sprites->asphalt : sprites->pavement).DrawOnTile(center);
                        }
                        break;
                    case core::TileType::Stairs:
                        sprites->metro.DrawOnTile(center);
                        break;
                    case core::TileType::Wall:
                        // Trottoir bas devant un couloir pour laisser voir la rue
                        (HidesCorridorTile(dungeon, pos) ? sprites->sidewalk : sprites->BuildingAt(pos))
                            .DrawOnTile(center);
                        break;
                    case core::TileType::Empty:
                        break;
                }
                // Joueur dessiné juste après sa case : les immeubles devant lui le recouvrent
                if (pos == player.GetPosition()) {
                    DrawPlayerMarker(center);
                }
            }
        }
    }

}
