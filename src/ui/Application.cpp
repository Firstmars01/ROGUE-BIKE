// Module ui.application (implémentation).
module;
#include <raylib.h>
import std;

module ui.application;

import core.config;
import core.attacks;
import core.entities;
import ui.hud;

namespace ui {

    namespace {
        constexpr Color backgroundColor{24, 20, 28, 255};
        constexpr int helpFontSize{20};
        constexpr int helpMargin{20};
        // Durée pendant laquelle la zone d'une frappe ennemie est annoncée en rouge
        constexpr core::Seconds dangerWarning{1.0f};
        // Touches de choix des attaques (raylib nomme les touches selon leur position, pas leur symbole)
        constexpr std::array<int, 3> attackKeys{KEY_ONE, KEY_TWO, KEY_THREE};
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
        // Ce qui est sous la souris : un bouton d'attaque, sinon une case praticable
        const auto mouse = window.MousePosition();
        hoveredButton = AttackButtonAt(mouse);
        const auto cell = renderer.CellAt(mouse);
        hoveredTile = !hoveredButton && game.GetDungeon().grid.IsWalkable(cell) ? std::optional{cell} : std::nullopt;

        for (auto index = std::size_t{0}; index < attackKeys.size(); ++index) {
            if (IsKeyPressed(attackKeys[index])) {
                ToggleAttack(index);
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            selectedAttack.reset();
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (hoveredButton) {
                ToggleAttack(*hoveredButton);
            } else if (selectedAttack) {
                // Attaque validée par core::Game ; on repasse en déplacement si elle est lancée
                if (const auto target = AttackTarget(); target && game.TryAttack(*selectedAttack, *target)) {
                    selectedAttack.reset();
                }
            } else if (hoveredTile) {
                game.TryMove(*hoveredTile);
            }
        }
        if (IsKeyPressed(KEY_R)) {
            ++seed;
            game = core::Game{seed, core::mapWidth, core::mapHeight};
            selectedAttack.reset();
        }
    }

    void Application::ToggleAttack(const std::size_t index) {
        selectedAttack = selectedAttack == index ? std::nullopt : std::optional{index};
    }

    std::optional<core::GridPos> Application::AttackTarget() const {
        if (!selectedAttack) {
            return std::nullopt;
        }
        if (core::Player::attacks[*selectedAttack].range == 0) {
            return game.GetPlayer().GetPosition();
        }
        return hoveredTile;
    }

    void Application::Draw() const {
        ClearBackground(backgroundColor);

        // Surcouches : déplacement sans attaque choisie, sinon cases visables et zone visée
        const auto noTiles = std::vector<core::GridPos>{};
        const auto reachableTiles = selectedAttack ? noTiles : game.PlayerReachableTiles();
        const auto attackTargets = selectedAttack ? game.AttackTargets(*selectedAttack) : noTiles;
        const auto target = AttackTarget();
        const auto attackArea = target ? game.AttackArea(*selectedAttack, *target) : noTiles;
        const auto dangerTiles = game.ImminentStrikeTiles(dangerWarning);

        renderer.DrawScene(SceneView{game.GetDungeon(), game.GetPlayer(), game.GetEnemies(), game.GetBoss(),
                                     reachableTiles, attackTargets, attackArea, dangerTiles, hoveredTile});
        DrawAttackBar(game.GetPlayer(), selectedAttack);

        // Sans accents : la police par défaut de raylib ne les gère pas
        const auto help = std::format("Graine : {}   |   Clic gauche : se deplacer / attaquer   |   1-2-3 : choisir une attaque"
                                      "   |   Clic droit : annuler   |   R : nouveau donjon   |   Echap : quitter", seed);
        DrawText(help.c_str(), helpMargin, helpMargin, helpFontSize, RAYWHITE);
    }

    std::filesystem::path Application::SpritesDirectory() {
        // Chemin fixe, sans donnée utilisateur
        return std::filesystem::path{GetApplicationDirectory()} / "assets" / "sprites";
    }

}
