// Module ui.renderer (implémentation) : sprites en RAII et dessin avec raylib.
module;
#include <raylib.h>
import std;

module ui.renderer;

import core.attacks;

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

        // Dessine le sprite debout, pieds posés sur ground ; flipped : retourné horizontalement
        // (les personnages regardent vers la droite par défaut) ; tint : teinte
        void DrawStanding(const ScreenPos ground, const bool flipped, const Color tint) const {
            const auto width = static_cast<float>(texture.width);
            const auto height = static_cast<float>(texture.height);
            // Largeur négative dans le rectangle source : raylib dessine l'image en miroir
            const auto source = Rectangle{0.0f, 0.0f, flipped ? -width : width, height};
            const auto destination = Rectangle{ground.x - width / 2.0f, ground.y - height, width, height};
            DrawTexturePro(texture, source, destination, Vector2{0.0f, 0.0f}, 0.0f, tint);
        }

        // Renvoie la hauteur du sprite en pixels
        float Height() const noexcept {
            return static_cast<float>(texture.height);
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
        TileSprite player;                      // Motard à pied
        TileSprite fastEnemy;                   // Voyou en trottinette
        TileSprite heavyEnemy;                  // Voyou en rollers
        TileSprite boss;                        // Cycliste

        // Charge chaque sprite depuis directory
        explicit Sprites(const std::filesystem::path& directory)
            : asphalt{directory / "asphalt.png"},
              pavement{directory / "pavement.png"},
              sidewalk{directory / "sidewalk.png"},
              manhole{directory / "manhole.png"},
              metro{directory / "metro.png"},
              buildings{TileSprite{directory / "building_brick.png"},
                        TileSprite{directory / "building_concrete.png"},
                        TileSprite{directory / "building_stucco.png"}},
              player{directory / "player.png"},
              fastEnemy{directory / "enemy_fast.png"},
              heavyEnemy{directory / "enemy_heavy.png"},
              boss{directory / "boss.png"} {}

        // Renvoie la variante d'immeuble de la case pos (toujours la même pour une case)
        const TileSprite& BuildingAt(const core::GridPos pos) const {
            return buildings[static_cast<std::size_t>((pos.x * 7 + pos.y * 13) % 3)];
        }
    };

    // Éléments internes à ce fichier
    namespace {

        // Barre de vie d'une entité : couleur et largeur
        struct HealthBarStyle {
            Color fill;
            float width;
        };

        constexpr HealthBarStyle playerBar{{70, 200, 90, 255}, 20.0f};
        constexpr HealthBarStyle enemyBar{{220, 50, 50, 255}, 20.0f};
        constexpr HealthBarStyle bossBar{{220, 50, 50, 255}, 40.0f};

        // Le joueur immobilisé après une attaque est grisé
        constexpr Color lockedTint{120, 120, 130, 255};
        // Décalage des pieds sous le centre de la case (le personnage paraît posé sur le sol)
        constexpr float footOffset{4.0f};

        constexpr Color healthBackgroundColor{30, 20, 20, 220};
        constexpr float healthBarHeight{3.0f};
        constexpr float healthBarGap{2.0f};       // Espace entre le sprite et la barre
        constexpr int countdownFontSize{10};
        constexpr float countdownGap{2.0f};       // Espace entre la barre de vie et le compteur

        // Surcouches des cases praticables (semi-transparentes)
        constexpr Color gridLineColor{255, 255, 255, 40};
        constexpr Color reachableColor{70, 150, 255, 90};
        constexpr Color hoveredReachableColor{150, 215, 255, 170};
        constexpr Color attackTargetColor{255, 160, 40, 80};
        constexpr Color dangerColor{220, 40, 40, 110};
        constexpr Color attackAreaColor{255, 70, 40, 190};
        constexpr Color hoveredColor{255, 255, 255, 60};

        // Surcouches présentes sur une case
        struct TileLayers {
            bool reachable;
            bool attackTarget;
            bool danger;
            bool attackArea;
            bool hovered;
        };

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

        // Renvoie la couleur de remplissage d'une case selon la surcouche la plus importante, ou std::nullopt
        std::optional<Color> OverlayColor(const TileLayers& layers) {
            if (layers.attackArea) {
                return attackAreaColor;
            }
            if (layers.danger) {
                return dangerColor;
            }
            if (layers.attackTarget) {
                return attackTargetColor;
            }
            if (layers.reachable) {
                return layers.hovered ? hoveredReachableColor : reachableColor;
            }
            if (layers.hovered) {
                return hoveredColor;
            }
            return std::nullopt;
        }

        // Dessine les surcouches d'une case praticable : quadrillage puis remplissage éventuel
        void DrawTileOverlays(const ScreenPos center, const TileLayers& layers) {
            const auto top = Vector2{center.x, center.y - halfTileHeight};
            const auto left = Vector2{center.x - halfTileWidth, center.y};
            const auto bottom = Vector2{center.x, center.y + halfTileHeight};
            const auto right = Vector2{center.x + halfTileWidth, center.y};
            DrawLineV(top, left, gridLineColor);
            DrawLineV(left, bottom, gridLineColor);
            DrawLineV(bottom, right, gridLineColor);
            DrawLineV(right, top, gridLineColor);
            if (const auto fill = OverlayColor(layers)) {
                FillTriangle(top, left, bottom, *fill);
                FillTriangle(top, bottom, right, *fill);
            }
        }

        // Dessine entity avec sprite posé sur ground (retourné si flipped, teinté par tint), sa barre de vie
        // au-dessus et, si countdown est fourni, le temps avant sa prochaine frappe au-dessus de la barre
        void DrawCharacter(const ScreenPos ground, const core::Entity& entity, const TileSprite& sprite,
                           const bool flipped, const Color tint, const HealthBarStyle& bar,
                           const std::optional<core::Seconds> countdown = std::nullopt) {
            const auto feet = ScreenPos{ground.x, ground.y + footOffset};
            sprite.DrawStanding(feet, flipped, tint);

            const auto ratio = static_cast<float>(entity.GetHealth()) / static_cast<float>(entity.GetMaxHealth());
            const auto left = feet.x - bar.width / 2.0f;
            const auto top = feet.y - sprite.Height() - healthBarGap - healthBarHeight;
            DrawRectangleRec(Rectangle{left - 1.0f, top - 1.0f, bar.width + 2.0f, healthBarHeight + 2.0f},
                             healthBackgroundColor);
            DrawRectangleRec(Rectangle{left, top, bar.width * ratio, healthBarHeight}, bar.fill);

            if (countdown) {
                const auto text = std::format("{:.1f}", countdown->count());
                const auto textWidth = static_cast<float>(MeasureText(text.c_str(), countdownFontSize));
                DrawText(text.c_str(), static_cast<int>(feet.x - textWidth / 2.0f),
                         static_cast<int>(top - countdownGap - static_cast<float>(countdownFontSize)), countdownFontSize,
                         RAYWHITE);
            }
        }

        // Renvoie le compteur à afficher au-dessus d'un adversaire (seulement s'il est actif)
        std::optional<core::Seconds> CountdownOf(const core::Hostile& hostile) {
            return hostile.IsActive() ? std::optional{hostile.TimeBeforeStrike()} : std::nullopt;
        }

        // Indique si tiles contient pos
        bool Contains(const std::vector<core::GridPos>& tiles, const core::GridPos pos) {
            return std::ranges::find(tiles, pos) != tiles.end();
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
            auto tiles = scene.boss ? scene.boss->OccupiedTiles() : std::vector<core::GridPos>{};
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
          screenCenter{static_cast<float>(screenWidth) / 2.0f, static_cast<float>(screenHeight) / 2.0f},
          sprites{std::make_unique<Sprites>(spritesDirectory)} {}

    // Défini ici : Sprites n'est complet que dans ce fichier
    Renderer::~Renderer() = default;

    void Renderer::ZoomIn() noexcept {
        zoom = std::min(zoom + 1, maxZoom);
    }

    void Renderer::ZoomOut() noexcept {
        zoom = std::max(zoom - 1, minZoom);
    }

    int Renderer::GetZoom() const noexcept {
        return zoom;
    }

    core::GridPos Renderer::CellAt(const ScreenPos screen, const core::GridPos focus) const noexcept {
        return ScreenToGrid(ScreenToWorld(screen, View(focus)), origin);
    }

    CameraView Renderer::View(const core::GridPos focus) const noexcept {
        if (zoom == minZoom) {
            // Vue fixe : le monde est affiché tel quel
            return CameraView{screenCenter, screenCenter, 1.0f};
        }
        return CameraView{GridToScreen(focus, origin), screenCenter, static_cast<float>(zoom)};
    }

    void Renderer::DrawScene(const SceneView& scene) const {
        // Tout le monde est dessiné à travers la caméra
        const auto view = View(scene.player.GetPosition());
        BeginMode2D(Camera2D{Vector2{view.offset.x, view.offset.y}, Vector2{view.target.x, view.target.y}, 0.0f, view.zoom});
        DrawWorld(scene);
        EndMode2D();
    }

    void Renderer::DrawWorld(const SceneView& scene) const {
        const auto& dungeon = scene.dungeon;
        const auto& grid = dungeon.grid;
        const auto importantTiles = ImportantTiles(scene);
        const auto playerScreenX = GridToScreen(scene.player.GetPosition(), origin).x;

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
                    DrawTileOverlays(center, TileLayers{
                        Contains(scene.reachableTiles, pos),
                        Contains(scene.attackTargets, pos),
                        Contains(scene.dangerTiles, pos),
                        Contains(scene.attackArea, pos),
                        scene.hoveredTile == pos
                    });
                }
                // Orientation : le joueur regarde vers la case visée, les adversaires vers le joueur
                if (pos == scene.player.GetPosition()) {
                    const auto facesLeft = scene.hoveredTile && GridToScreen(*scene.hoveredTile, origin).x < center.x;
                    DrawCharacter(center, scene.player, sprites->player, facesLeft,
                                  scene.player.IsLocked() ? lockedTint : WHITE, playerBar);
                }
                for (const auto& enemy : scene.enemies) {
                    if (enemy->GetPosition() == pos) {
                        const auto& sprite = enemy->GetKind() == core::EnemyKind::Fast ? sprites->fastEnemy : sprites->heavyEnemy;
                        DrawCharacter(center, *enemy, sprite, playerScreenX < center.x, WHITE, enemyBar, CountdownOf(*enemy));
                    }
                }
                // Boss après sa dernière case, centré sur son carré (milieu de la première et de la dernière case)
                if (scene.boss && pos == scene.boss->OccupiedTiles().back()) {
                    const auto first = GridToScreen(scene.boss->GetPosition(), origin);
                    const auto middle = ScreenPos{(first.x + center.x) / 2.0f, (first.y + center.y) / 2.0f};
                    DrawCharacter(middle, *scene.boss, sprites->boss, playerScreenX < middle.x, WHITE, bossBar,
                                  CountdownOf(*scene.boss));
                }
            }
        }
    }

}
