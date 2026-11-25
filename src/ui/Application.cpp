// Module ui.application (implémentation) : boucle de jeu et entrées clavier.
// Interface : Application.ixx
module;
// raylib est incluse uniquement ici (fragment de module global)
#include <raylib.h>
// Import rapide de la bibliothèque standard
import std;

// Unité d'implémentation du module ui.application
module ui.application;

import core.config;

namespace ui {

    // Constantes internes à ce fichier
    namespace {
        constexpr Color backgroundColor{24, 20, 28, 255};
        constexpr int helpFontSize{20};
        constexpr int helpMargin{20};
    }

    // Fenêtre d'abord (membre déclaré en premier), puis donjon et renderer
    Application::Application()
        : window{core::windowWidth, core::windowHeight, core::windowTitle, core::targetFps},
          seed{core::dungeonSeed},
          dungeon{CreateDungeon(core::dungeonSeed)},
          renderer{core::mapWidth, core::mapHeight, core::windowWidth, core::windowHeight, SpritesDirectory()} {}

    // Boucle principale : entrées, puis dessin, à chaque image
    void Application::Run() {
        while (!window.ShouldClose()) {
            HandleInput();
            BeginDrawing();
            Draw();
            EndDrawing();
        }
    }

    // R : génère le donjon suivant (graine + 1)
    void Application::HandleInput() {
        if (IsKeyPressed(KEY_R)) {
            ++seed;
            dungeon = CreateDungeon(seed);
        }
    }

    // Efface l'écran, dessine la carte puis l'aide (sans accents : police raylib par défaut)
    void Application::Draw() const {
        ClearBackground(backgroundColor);
        renderer.DrawDungeon(dungeon);
        const auto help = std::format("Graine : {}   |   R : nouveau donjon   |   Echap : quitter", seed);
        DrawText(help.c_str(), helpMargin, helpMargin, helpFontSize, RAYWHITE);
    }

    // Génère un étage aux dimensions de core.config
    core::Dungeon Application::CreateDungeon(const std::uint32_t seed) {
        auto generator = core::DungeonGenerator{seed};
        return generator.Generate(core::mapWidth, core::mapHeight);
    }

    // Chemin fixe relatif à l'exécutable (aucune donnée utilisateur dans le chemin)
    std::filesystem::path Application::SpritesDirectory() {
        return std::filesystem::path{GetApplicationDirectory()} / "assets" / "sprites";
    }

}
