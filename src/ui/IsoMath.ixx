// Module ui.isoMath : projection isométrique grille <-> écran (fonctions constexpr, sans raylib).
// Pas de .cpp : les fonctions constexpr doivent être définies dans l'interface.
module;
import std;

export module ui.isoMath;

import core.grid;

export namespace ui {

    inline constexpr float tileWidth{32.0f};                    // Largeur d'une tuile (px)
    inline constexpr float tileHeight{16.0f};                   // Hauteur d'une tuile (px, rapport 2:1)
    inline constexpr float halfTileWidth{tileWidth / 2.0f};
    inline constexpr float halfTileHeight{tileHeight / 2.0f};

    // Position à l'écran en pixels
    struct ScreenPos {
        float x{};
        float y{};

        bool operator==(const ScreenPos&) const = default;
    };

    // Renvoie la position écran du centre de la case pos. origin : centre de la case (0, 0).
    constexpr ScreenPos GridToScreen(const core::GridPos pos, const ScreenPos origin) noexcept {
        return ScreenPos{
            origin.x + static_cast<float>(pos.x - pos.y) * halfTileWidth,
            origin.y + static_cast<float>(pos.x + pos.y) * halfTileHeight
        };
    }

    // Renvoie le plus grand entier <= value (std::floor n'est pas constexpr partout)
    constexpr int FloorToInt(const float value) noexcept {
        const auto truncated = static_cast<int>(value);
        return value < static_cast<float>(truncated) ? truncated - 1 : truncated;
    }

    // Renvoie la case dont le losange contient le point screen (inverse de GridToScreen).
    // Le résultat peut être hors de la carte.
    constexpr core::GridPos ScreenToGrid(const ScreenPos screen, const ScreenPos origin) noexcept {
        // Coordonnées en demi-tuiles : u = x - y et v = x + y, puis arrondi à la case la plus proche
        const auto u = (screen.x - origin.x) / halfTileWidth;
        const auto v = (screen.y - origin.y) / halfTileHeight;
        return core::GridPos{
            FloorToInt((v + u) / 2.0f + 0.5f),
            FloorToInt((v - u) / 2.0f + 0.5f)
        };
    }

    // Renvoie l'origine qui centre une carte mapWidth x mapHeight (cases)
    // dans un écran screenWidth x screenHeight (px).
    constexpr ScreenPos CenteredOrigin(const int mapWidth, const int mapHeight,
                                       const int screenWidth, const int screenHeight) noexcept {
        // Boîte de la carte relative à l'origine : x de -mapHeight à mapWidth demi-tuiles,
        // y de -1 à (mapWidth + mapHeight - 1) demi-tuiles
        return ScreenPos{
            static_cast<float>(screenWidth) / 2.0f
                - static_cast<float>(mapWidth - mapHeight) * halfTileWidth / 2.0f,
            static_cast<float>(screenHeight) / 2.0f
                - static_cast<float>(mapWidth + mapHeight - 2) * halfTileHeight / 2.0f
        };
    }

}
