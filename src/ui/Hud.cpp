// Module ui.hud (implémentation).
module;
#include <raylib.h>
import std;

module ui.hud;

import core.config;
import core.attacks;

namespace ui {

    namespace {
        constexpr float buttonWidth{260.0f};
        constexpr float buttonHeight{70.0f};
        constexpr float buttonGap{20.0f};
        constexpr float bottomMargin{25.0f};
        constexpr int titleFontSize{20};
        constexpr int detailFontSize{16};
        constexpr Color buttonColor{40, 36, 48, 230};
        constexpr Color cooldownColor{10, 10, 14, 170};
        constexpr Color borderColor{110, 105, 120, 255};
        constexpr Color selectedBorderColor{250, 200, 60, 255};
        constexpr Color readyTextColor{120, 220, 120, 255};

        // Renvoie le rectangle du bouton index, centré en bas du canevas
        Rectangle ButtonRect(const std::size_t index) {
            const auto count = static_cast<float>(core::Player::attacks.size());
            const auto totalWidth = count * buttonWidth + (count - 1.0f) * buttonGap;
            const auto left = (static_cast<float>(core::windowWidth) - totalWidth) / 2.0f;
            return Rectangle{
                left + static_cast<float>(index) * (buttonWidth + buttonGap),
                static_cast<float>(core::windowHeight) - bottomMargin - buttonHeight,
                buttonWidth,
                buttonHeight
            };
        }
    }

    void DrawAttackBar(const core::Player& player, const std::optional<std::size_t> selected) {
        for (auto index = std::size_t{0}; index < core::Player::attacks.size(); ++index) {
            const auto& attack = core::Player::attacks[index];
            const auto rect = ButtonRect(index);
            DrawRectangleRec(rect, buttonColor);

            // Voile de cooldown : sa hauteur suit le temps restant
            const auto remaining = player.AttackCooldownRemaining(index);
            const auto ratio = remaining / attack.cooldown;
            DrawRectangleRec(Rectangle{rect.x, rect.y + rect.height * (1.0f - ratio), rect.width, rect.height * ratio},
                             cooldownColor);

            const auto isSelected = selected == index;
            DrawRectangleLinesEx(rect, isSelected ? 3.0f : 1.0f, isSelected ? selectedBorderColor : borderColor);

            // Texte sans accents : police raylib par défaut
            const auto title = std::format("[{}] {}", index + 1, attack.name);
            DrawText(title.c_str(), static_cast<int>(rect.x) + 12, static_cast<int>(rect.y) + 10, titleFontSize, RAYWHITE);
            const auto ready = player.CanUseAttack(index);
            const auto detail = ready ? std::format("Pret  -  {} degats", attack.damage)
                                      : std::format("{:.1f} s", remaining.count());
            DrawText(detail.c_str(), static_cast<int>(rect.x) + 12, static_cast<int>(rect.y) + 42, detailFontSize,
                     ready ? readyTextColor : LIGHTGRAY);
        }
    }

    std::optional<std::size_t> AttackButtonAt(const ScreenPos screen) {
        for (auto index = std::size_t{0}; index < core::Player::attacks.size(); ++index) {
            if (CheckCollisionPointRec(Vector2{screen.x, screen.y}, ButtonRect(index))) {
                return index;
            }
        }
        return std::nullopt;
    }

}
