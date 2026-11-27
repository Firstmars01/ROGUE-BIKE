// Module ui.application (implémentation).
module;
#include <raylib.h>
import std;

module ui.application;

import core.config;
import core.attacks;

namespace ui {

    namespace {
        constexpr Color backgroundColor{24, 20, 28, 255};
        constexpr int helpFontSize{20};
        constexpr int helpMargin{20};
    }

    Application::Application()
        : window{core::windowWidth, core::windowHeight, core::windowTitle, core::targetFps},
          seed{core::dungeonSeed},
          game{core::dungeonSeed, core::mapWidth, core::mapHeight},
          renderer{core::mapWidth, core::mapHeight, core::windowWidth, core::windowHeight, SpritesDirectory()} {}

    void Application::Run() {
        while (!window.ShouldClose()) {
            HandleInput();
            game.Update(core::Seconds{GetFrameTime()});
            window.BeginFrame();
            Draw();
            window.EndFrame();
        }
    }

    void Application::HandleInput() {
        // Case sous la souris (coordonnées du canevas), retenue seulement si elle est praticable
        const auto cell = renderer.CellAt(window.MousePosition());
        hoveredTile = game.GetDungeon().grid.IsWalkable(cell) ? std::optional{cell} : std::nullopt;

        // Clic gauche : déplacement (validé par core::Game)
        if (hoveredTile && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            game.TryMove(*hoveredTile);
        }
        if (IsKeyPressed(KEY_R)) {
            ++seed;
            game = core::Game{seed, core::mapWidth, core::mapHeight};
        }
    }

    void Application::Draw() const {
        ClearBackground(backgroundColor);
        const auto reachableTiles = game.PlayerReachableTiles();
        renderer.DrawScene(SceneView{game.GetDungeon(), game.GetPlayer(), game.GetEnemies(), game.GetBoss(),
                                     reachableTiles, hoveredTile});
        // Sans accents : la police par défaut de raylib ne les gère pas
        const auto help = std::format("Graine : {}   |   Clic gauche : se deplacer (zone bleue)   |   R : nouveau donjon   |   Echap : quitter", seed);
        DrawText(help.c_str(), helpMargin, helpMargin, helpFontSize, RAYWHITE);
    }

    std::filesystem::path Application::SpritesDirectory() {
        // Chemin fixe, sans donnée utilisateur
        return std::filesystem::path{GetApplicationDirectory()} / "assets" / "sprites";
    }

}
