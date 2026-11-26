// Module ui.application (implémentation).
module;
#include <raylib.h>
import std;

module ui.application;

import core.config;

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
          renderer{core::mapWidth, core::mapHeight, core::windowWidth, core::windowHeight, SpritesDirectory()} {}

    void Application::Run() {
        while (!window.ShouldClose()) {
            HandleInput();
            BeginDrawing();
            Draw();
            EndDrawing();
        }
    }

    void Application::HandleInput() {
        if (IsKeyPressed(KEY_R)) {
            ++seed;
            dungeon = CreateDungeon(seed);
            player.SetPosition(dungeon.playerStart);
        }
    }

    void Application::Draw() const {
        ClearBackground(backgroundColor);
        renderer.DrawDungeon(dungeon, player);
        // Sans accents : la police par défaut de raylib ne les gère pas
        const auto help = std::format("Graine : {}   |   R : nouveau donjon   |   Echap : quitter", seed);
        DrawText(help.c_str(), helpMargin, helpMargin, helpFontSize, RAYWHITE);
    }

    core::Dungeon Application::CreateDungeon(const std::uint32_t seed) {
        auto generator = core::DungeonGenerator{seed};
        return generator.Generate(core::mapWidth, core::mapHeight);
    }

    std::filesystem::path Application::SpritesDirectory() {
        // Chemin fixe, sans donnée utilisateur
        return std::filesystem::path{GetApplicationDirectory()} / "assets" / "sprites";
    }

}
