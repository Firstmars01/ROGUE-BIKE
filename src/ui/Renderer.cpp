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
        std::array<TileSprite, 3> buildings;   // Brique, béton, crépi

        // Charge chaque sprite depuis directory
        explicit Sprites(const std::filesystem::path& directory)
            : asphalt{directory / "asphalt.png"},
              pavement{directory / "pavement.png"},
              sidewalk{directory / "sidewalk.png"},
              manhole{directory / "manhole.png"},
              metro{directory / "metro.png"},
              buildings{TileSprite{directory / "building_brick.png"},
                        TileSprite{directory / "building_concrete.png"},
                        TileSprite{directory / "building_stucco.png"}} {}

        // Renvoie la variante d'immeuble de la case pos (toujours la même pour une case)
        const TileSprite& BuildingAt(const core::GridPos pos) const {
            return buildings[static_cast<std::size_t>((pos.x * 7 + pos.y * 13) % 3)];
        }
    };

    // Éléments internes à ce fichier
    namespace {

        // Apparence provisoire d'une entité : cercle coloré surélevé et barre de vie au-dessus
        struct MarkerStyle {
            Color fill;
            Color outline;
            Color health;
            float radius;
            float elevation;
            float healthBarWidth;
        };

        constexpr MarkerStyle playerStyle{{60, 140, 230, 255}, {20, 40, 80, 255}, {70, 200, 90, 255}, 6.0f, 8.0f, 20.0f};
        constexpr MarkerStyle enemyStyle{{220, 50, 50, 255}, {90, 15, 15, 255}, {220, 50, 50, 255}, 6.0f, 8.0f, 20.0f};
        constexpr MarkerStyle bossStyle{{110, 15, 20, 255}, {50, 5, 8, 255}, {220, 50, 50, 255}, 14.0f, 14.0f, 40.0f};

        constexpr Color healthBackgroundColor{30, 20, 20, 220};
        constexpr float healthBarHeight{3.0f};
        constexpr float healthBarGap{4.0f};       // Espace entre le cercle et la barre

        // Surcouches des cases praticables (semi-transparentes)
        constexpr Color gridLineColor{255, 255, 255, 40};
        constexpr Color reachableColor{70, 150, 255, 90};
        constexpr Color hoveredReachableColor{150, 215, 255, 170};
        constexpr Color hoveredColor{255, 255, 255, 60};

        // Un immeuble (32 px) recouvre 2 rangées de cases derrière lui
        constexpr int buildingRowsCovered{2};

        // Immeuble à moitié transparent quand il cache une entité ou la case visée
        constexpr Color seeThroughTint{255, 255, 255, 90};

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

        // Dessine les surcouches d'une case praticable : quadrillage, zone de déplacement, survol
        void DrawTileOverlays(const ScreenPos center, const bool reachable, const bool hovered) {
            const auto top = Vector2{center.x, center.y - halfTileHeight};
            const auto left = Vector2{center.x - halfTileWidth, center.y};
            const auto bottom = Vector2{center.x, center.y + halfTileHeight};
            const auto right = Vector2{center.x + halfTileWidth, center.y};
            DrawLineV(top, left, gridLineColor);
            DrawLineV(left, bottom, gridLineColor);
            DrawLineV(bottom, right, gridLineColor);
            DrawLineV(right, top, gridLineColor);
            if (reachable || hovered) {
                const auto fill = reachable ? (hovered ? hoveredReachableColor : reachableColor) : hoveredColor;
                FillTriangle(top, left, bottom, fill);
                FillTriangle(top, bottom, right, fill);
            }
        }

        // Dessine le repère de entity (style donné) posé sur ground, avec sa barre de vie
        void DrawMarker(const ScreenPos ground, const core::Entity& entity, const MarkerStyle& style) {
            const auto position = Vector2{ground.x, ground.y - style.elevation};
            DrawCircleV(position, style.radius, style.fill);
            DrawCircleLines(static_cast<int>(position.x), static_cast<int>(position.y), style.radius, style.outline);

            const auto ratio = static_cast<float>(entity.GetHealth()) / static_cast<float>(entity.GetMaxHealth());
            const auto left = position.x - style.healthBarWidth / 2.0f;
            const auto top = position.y - style.radius - healthBarGap - healthBarHeight;
            DrawRectangleRec(Rectangle{left - 1.0f, top - 1.0f, style.healthBarWidth + 2.0f, healthBarHeight + 2.0f},
                             healthBackgroundColor);
            DrawRectangleRec(Rectangle{left, top, style.healthBarWidth * ratio, healthBarHeight}, style.health);
        }

        // Indique si un immeuble en building recouvre à l'écran la case target : cases (x - i, y - j)
        // derrière lui avec |i - j| <= 1 et i, j <= buildingRowsCovered
        bool Covers(const core::GridPos building, const core::GridPos target) {
            const auto i = building.x - target.x;
            const auto j = building.y - target.y;
            return i >= 0 && j >= 0 && i <= buildingRowsCovered && j <= buildingRowsCovered
                   && i + j > 0 && std::abs(i - j) <= 1;
        }

        // Indique si un immeuble en pos cacherait un couloir (case praticable hors de toute salle)
        bool HidesCorridorTile(const core::Dungeon& dungeon, const core::GridPos pos) {
            for (auto i = 0; i <= buildingRowsCovered; ++i) {
                for (auto j = 0; j <= buildingRowsCovered; ++j) {
                    const auto behind = core::GridPos{pos.x - i, pos.y - j};
                    if (Covers(pos, behind) && dungeon.grid.IsWalkable(behind) && !core::IsInAnyRoom(dungeon.rooms, behind)) {
                        return true;
                    }
                }
            }
            return false;
        }

        // Renvoie les cases qu'aucun immeuble ne doit cacher : joueur, ennemis, boss, case sous la souris
        std::vector<core::GridPos> ImportantTiles(const SceneView& scene) {
            auto tiles = scene.boss.OccupiedTiles();
            tiles.push_back(scene.player.GetPosition());
            for (const auto& enemy : scene.enemies) {
                tiles.push_back(enemy->GetPosition());
            }
            if (scene.hoveredTile) {
                tiles.push_back(*scene.hoveredTile);
            }
            return tiles;
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
        const auto importantTiles = ImportantTiles(scene);
        const auto bossTopLeft = scene.boss.GetPosition();
        const auto bossLastTile = core::GridPos{bossTopLeft.x + core::Boss::size - 1, bossTopLeft.y + core::Boss::size - 1};

        // Parcours par diagonales (sum = x + y), en gardant y = sum - x dans la grille
        for (auto sum = 0; sum <= grid.GetWidth() + grid.GetHeight() - 2; ++sum) {
            const auto firstX = std::max(0, sum - (grid.GetHeight() - 1));
            const auto lastX = std::min(sum, grid.GetWidth() - 1);
            for (auto x = firstX; x <= lastX; ++x) {
                const auto pos = core::GridPos{x, sum - x};
                const auto center = GridToScreen(pos, origin);
                switch (grid.GetTile(pos).value_or(core::TileType::Empty)) {
                    case core::TileType::Floor:
                        if (pos == dungeon.playerStart) {
                            sprites->manhole.DrawOnTile(center);
                        } else {
                            (core::IsInAnyRoom(dungeon.rooms, pos) ? sprites->pavement : sprites->asphalt).DrawOnTile(center);
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
                            const auto hides = std::ranges::any_of(importantTiles, [pos](const core::GridPos tile) {
                                return Covers(pos, tile);
                            });
                            sprites->BuildingAt(pos).DrawOnTile(center, hides ? seeThroughTint : WHITE);
                        }
                        break;
                    case core::TileType::Empty:
                        break;
                }

                // Surcouches et entités juste après leur case : les immeubles placés devant les recouvrent
                if (grid.IsWalkable(pos)) {
                    const auto reachable = std::ranges::find(scene.reachableTiles, pos) != scene.reachableTiles.end();
                    DrawTileOverlays(center, reachable, scene.hoveredTile == pos);
                }
                if (pos == scene.player.GetPosition()) {
                    DrawMarker(center, scene.player, playerStyle);
                }
                for (const auto& enemy : scene.enemies) {
                    if (enemy->GetPosition() == pos) {
                        DrawMarker(center, *enemy, enemyStyle);
                    }
                }
                // Boss après sa dernière case, centré sur son carré (milieu de la première et de la dernière case)
                if (pos == bossLastTile) {
                    const auto first = GridToScreen(bossTopLeft, origin);
                    DrawMarker(ScreenPos{(first.x + center.x) / 2.0f, (first.y + center.y) / 2.0f}, scene.boss, bossStyle);
                }
            }
        }
    }

}
