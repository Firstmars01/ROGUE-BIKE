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

        // Dessine le sprite sur la case de centre center (bas du sprite aligné sur le bas du losange).
        // tint : teinte et transparence (blanc opaque par défaut)
        void DrawOnTile(const ScreenPos center, const Color tint = WHITE) const {
            const auto position = Vector2{
                center.x - halfTileWidth,
                center.y + halfTileHeight - static_cast<float>(texture.height)
            };
            DrawTextureV(texture, position, tint);
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

        // Repères provisoires des entités : cercles colorés au-dessus du sol
        constexpr Color playerColor{60, 140, 230, 255};
        constexpr Color playerOutlineColor{20, 40, 80, 255};
        constexpr Color enemyColor{220, 50, 50, 255};
        constexpr Color enemyOutlineColor{90, 15, 15, 255};
        constexpr float markerRadius{6.0f};
        constexpr float markerElevation{8.0f};

        // Surcouches des cases praticables (semi-transparentes)
        constexpr Color gridLineColor{255, 255, 255, 40};
        constexpr Color reachableColor{70, 150, 255, 90};
        constexpr Color hoveredReachableColor{150, 215, 255, 170};
        constexpr Color hoveredColor{255, 255, 255, 60};

        // Un immeuble (32 px) recouvre 2 rangées de cases derrière lui
        constexpr int buildingRowsCovered{2};

        // Immeuble à moitié transparent quand il cache une entité ou la case visée
        constexpr Color seeThroughTint{255, 255, 255, 90};

        // Les 4 sommets du losange d'une case
        struct Diamond {
            Vector2 top{};
            Vector2 left{};
            Vector2 bottom{};
            Vector2 right{};
        };

        // Renvoie le losange de la case de centre center
        Diamond MakeDiamond(const ScreenPos center) {
            return Diamond{
                Vector2{center.x, center.y - halfTileHeight},
                Vector2{center.x - halfTileWidth, center.y},
                Vector2{center.x, center.y + halfTileHeight},
                Vector2{center.x + halfTileWidth, center.y}
            };
        }

        // Dessine un triangle plein quel que soit l'ordre des sommets
        // (raylib n'affiche que les triangles donnés dans le sens inverse des aiguilles d'une montre)
        void FillTriangle(const Vector2 a, const Vector2 b, const Vector2 c, const Color color) {
            const auto cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
            if (cross < 0.0f) {
                DrawTriangle(a, b, c, color);
            } else if (cross > 0.0f) {
                DrawTriangle(a, c, b, color);
            }
        }

        // Remplit le losange de la case de centre center
        void FillTile(const ScreenPos center, const Color color) {
            const auto diamond = MakeDiamond(center);
            FillTriangle(diamond.top, diamond.left, diamond.bottom, color);
            FillTriangle(diamond.top, diamond.bottom, diamond.right, color);
        }

        // Trace le contour du losange de la case de centre center
        void OutlineTile(const ScreenPos center, const Color color) {
            const auto diamond = MakeDiamond(center);
            DrawLineV(diamond.top, diamond.left, color);
            DrawLineV(diamond.left, diamond.bottom, color);
            DrawLineV(diamond.bottom, diamond.right, color);
            DrawLineV(diamond.right, diamond.top, color);
        }

        // Dessine les surcouches d'une case praticable : quadrillage, zone de déplacement, survol
        void DrawTileOverlays(const ScreenPos center, const bool reachable, const bool hovered) {
            OutlineTile(center, gridLineColor);
            if (reachable) {
                FillTile(center, hovered ? hoveredReachableColor : reachableColor);
            } else if (hovered) {
                FillTile(center, hoveredColor);
            }
        }

        // Dessine un repère d'entité (cercle plein et contour) sur la case de centre center
        void DrawMarker(const ScreenPos center, const Color fill, const Color outline) {
            const auto position = Vector2{center.x, center.y - markerElevation};
            DrawCircleV(position, markerRadius, fill);
            DrawCircleLines(static_cast<int>(position.x), static_cast<int>(position.y), markerRadius, outline);
        }

        // Indique si pos est un couloir (praticable et hors de toute salle)
        bool IsCorridorTile(const core::Dungeon& dungeon, const core::GridPos pos) {
            return dungeon.grid.IsWalkable(pos)
                   && std::ranges::none_of(dungeon.rooms, [pos](const core::Room& room) {
                          return room.Contains(pos);
                      });
        }

        // Indique si un immeuble en building recouvre à l'écran la case target : cases (x - i, y - j)
        // derrière lui avec |i - j| <= 1 et i, j <= buildingRowsCovered
        bool Covers(const core::GridPos building, const core::GridPos target) {
            const auto i = building.x - target.x;
            const auto j = building.y - target.y;
            return i >= 0 && j >= 0 && i <= buildingRowsCovered && j <= buildingRowsCovered
                   && i + j > 0 && std::abs(i - j) <= 1;
        }

        // Indique si un immeuble en pos cacherait un couloir
        bool HidesCorridorTile(const core::Dungeon& dungeon, const core::GridPos pos) {
            for (auto i = 0; i <= buildingRowsCovered; ++i) {
                for (auto j = 0; j <= buildingRowsCovered; ++j) {
                    const auto behind = core::GridPos{pos.x - i, pos.y - j};
                    if (Covers(pos, behind) && IsCorridorTile(dungeon, behind)) {
                        return true;
                    }
                }
            }
            return false;
        }

        // Indique si un immeuble en pos cache le joueur, un ennemi ou la case sous la souris
        bool HidesImportantTile(const SceneView& scene, const core::GridPos pos) {
            if (Covers(pos, scene.player.GetPosition()) || (scene.hoveredTile && Covers(pos, *scene.hoveredTile))) {
                return true;
            }
            return std::ranges::any_of(scene.enemies, [pos](const auto& enemy) {
                return Covers(pos, enemy->GetPosition());
            });
        }

    }

    Renderer::Renderer(const int mapWidth, const int mapHeight, const int screenWidth, const int screenHeight,
                       const std::filesystem::path& spritesDirectory)
        : origin{CenteredOrigin(mapWidth, mapHeight, screenWidth, screenHeight)},
          sprites{std::make_unique<Sprites>(spritesDirectory)} {}

    // Défini ici : Sprites n'est complet que dans ce fichier
    Renderer::~Renderer() = default;

    core::GridPos Renderer::CellAt(const ScreenPos screen) const noexcept {
        return ScreenToGrid(screen, origin);
    }

    void Renderer::DrawScene(const SceneView& scene) const {
        const auto& dungeon = scene.dungeon;
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
                        if (HidesCorridorTile(dungeon, pos)) {
                            // Trottoir bas devant un couloir pour laisser voir la rue
                            sprites->sidewalk.DrawOnTile(center);
                        } else {
                            // Immeuble, transparent s'il cache une entité ou la case visée
                            sprites->BuildingAt(pos).DrawOnTile(center, HidesImportantTile(scene, pos) ? seeThroughTint : WHITE);
                        }
                        break;
                    case core::TileType::Empty:
                        break;
                }
                // Surcouches juste après la case : les immeubles placés devant les recouvrent
                if (grid.IsWalkable(pos)) {
                    const auto reachable = std::ranges::find(scene.reachableTiles, pos) != scene.reachableTiles.end();
                    DrawTileOverlays(center, reachable, scene.hoveredTile == pos);
                }
                // Entités dessinées juste après leur case : les immeubles devant elles les recouvrent
                if (pos == scene.player.GetPosition()) {
                    DrawMarker(center, playerColor, playerOutlineColor);
                }
                const auto hasEnemy = std::ranges::any_of(scene.enemies, [pos](const auto& enemy) {
                    return enemy->GetPosition() == pos;
                });
                if (hasEnemy) {
                    DrawMarker(center, enemyColor, enemyOutlineColor);
                }
            }
        }
    }

}
