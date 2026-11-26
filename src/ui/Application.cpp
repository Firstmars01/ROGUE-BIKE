// Module ui.application (implémentation).
module;
#include <raylib.h>
import std;

module ui.application;

import core.config;
import core.pathfinding;

namespace ui {

    namespace {
        constexpr Color backgroundColor{24, 20, 28, 255};
        constexpr int helpFontSize{20};
        constexpr int helpMargin{20};
    }

    Application::Application()
        : window{core::windowWidth, core::windowHeight, core::windowTitle, core::targetFps},
          seed{core::dungeonSeed},
          dungeon{CreateDungeon(core::dungeonSeed)},
          player{dungeon.playerStart},
          enemies{CreateEnemies(dungeon)},
          renderer{core::mapWidth, core::mapHeight, core::windowWidth, core::windowHeight, SpritesDirectory()} {
        UpdateReachableTiles();
    }

    void Application::Run() {
        while (!window.ShouldClose()) {
            HandleInput();
            window.BeginFrame();
            Draw();
            window.EndFrame();
        }
    }

    void Application::HandleInput() {
        // Case sous la souris (coordonnées du canevas), retenue seulement si elle est praticable
        const auto cell = renderer.CellAt(window.MousePosition());
        hoveredTile = dungeon.grid.IsWalkable(cell) ? std::optional{cell} : std::nullopt;

        // Clic gauche : téléportation si la case est dans la zone (validée par core)
        if (hoveredTile && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && player.TryMoveTo(dungeon.grid, *hoveredTile)) {
            UpdateReachableTiles();
        }
        if (IsKeyPressed(KEY_R)) {
            ++seed;
            dungeon = CreateDungeon(seed);
            player.SetPosition(dungeon.playerStart);
            enemies = CreateEnemies(dungeon);
            UpdateReachableTiles();
        }
    }

    void Application::UpdateReachableTiles() {
        reachableTiles = core::ReachableTiles(dungeon.grid, player.GetPosition(), player.GetSpeed());
    }

    void Application::Draw() const {
        ClearBackground(backgroundColor);
        renderer.DrawScene(SceneView{dungeon, player, enemies, reachableTiles, hoveredTile});
        // Sans accents : la police par défaut de raylib ne les gère pas
        const auto help = std::format("Graine : {}   |   Clic gauche : se deplacer (zone bleue)   |   R : nouveau donjon   |   Echap : quitter", seed);
        DrawText(help.c_str(), helpMargin, helpMargin, helpFontSize, RAYWHITE);
    }

    core::Dungeon Application::CreateDungeon(const std::uint32_t seed) {
        auto generator = core::DungeonGenerator{seed};
        return generator.Generate(core::mapWidth, core::mapHeight);
    }

    std::vector<std::unique_ptr<core::Enemy>> Application::CreateEnemies(const core::Dungeon& dungeon) {
        auto created = std::vector<std::unique_ptr<core::Enemy>>{};
        for (const auto& spawn : dungeon.enemySpawns) {
            created.push_back(std::make_unique<core::Enemy>(spawn));
        }
        return created;
    }

    std::filesystem::path Application::SpritesDirectory() {
        // Chemin fixe, sans donnée utilisateur
        return std::filesystem::path{GetApplicationDirectory()} / "assets" / "sprites";
    }

}
