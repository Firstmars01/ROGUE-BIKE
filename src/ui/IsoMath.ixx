// Module ui.isoMath : mathématiques de la projection isométrique (grille <-> écran).
// Fonctions constexpr pures, sans dépendance à raylib : testables avec Catch2.
// Pas de fichier .cpp : les fonctions constexpr doivent être définies dans l'interface du module.
module;
// Import rapide de la bibliothèque standard
import std;

// Nommage et exportation de ce module
export module ui.isoMath;

import core.grid;

export namespace ui {

    // Largeur d'une tuile isométrique (losange) en pixels
    inline constexpr float tileWidth{32.0f};

    // Hauteur d'une tuile isométrique en pixels (rapport 2:1 classique)
    inline constexpr float tileHeight{16.0f};

    // Demi-dimensions utilisées par les formules de projection
    inline constexpr float halfTileWidth{tileWidth / 2.0f};
    inline constexpr float halfTileHeight{tileHeight / 2.0f};

    // Position en pixels à l'écran (indépendante de raylib)
    struct ScreenPos {
        float x{};
        float y{};

        // Comparaison membre à membre générée par le compilateur
        bool operator==(const ScreenPos&) const = default;
    };

    // Arrondit value à l'entier inférieur (std::floor n'étant pas garanti constexpr partout).
    // Renvoie le plus grand entier inférieur ou égal à value.
    constexpr int FloorToInt(const float value) noexcept {
        const auto truncated = static_cast<int>(value);
        return value < static_cast<float>(truncated) ? truncated - 1 : truncated;
    }

    // Convertit une case de la grille en position écran du CENTRE de son losange.
    // pos : case logique ; origin : position écran du centre de la case (0, 0).
    // Renvoie la position écran correspondante.
    constexpr ScreenPos GridToScreen(const core::GridPos pos, const ScreenPos origin) noexcept {
        return ScreenPos{
            origin.x + static_cast<float>(pos.x - pos.y) * halfTileWidth,
            origin.y + static_cast<float>(pos.x + pos.y) * halfTileHeight
        };
    }

    // Convertit une position écran en case de la grille (inverse de GridToScreen).
    // screen : position en pixels (ex : souris) ; origin : même origine que GridToScreen.
    // Renvoie la case dont le losange contient le point (peut être hors de la carte).
    constexpr core::GridPos ScreenToGrid(const ScreenPos screen, const ScreenPos origin) noexcept {
        // Coordonnées exprimées en demi-tuiles par rapport à l'origine
        const auto u = (screen.x - origin.x) / halfTileWidth;
        const auto v = (screen.y - origin.y) / halfTileHeight;
        // Inversion du système : u = x - y et v = x + y, puis arrondi à la case la plus proche
        return core::GridPos{
            FloorToInt((v + u) / 2.0f + 0.5f),
            FloorToInt((v - u) / 2.0f + 0.5f)
        };
    }

    // Calcule l'origine (centre de la case (0, 0)) qui centre toute la carte dans l'écran.
    // mapWidth, mapHeight : dimensions de la carte en cases ;
    // screenWidth, screenHeight : dimensions de la fenêtre en pixels.
    // Renvoie l'origine à passer à GridToScreen / ScreenToGrid.
    constexpr ScreenPos CenteredOrigin(const int mapWidth, const int mapHeight,
                                       const int screenWidth, const int screenHeight) noexcept {
        // Boîte englobante de la carte relative à l'origine :
        // x de -mapHeight * halfTileWidth à mapWidth * halfTileWidth,
        // y de -halfTileHeight à (mapWidth + mapHeight - 1) * halfTileHeight
        return ScreenPos{
            static_cast<float>(screenWidth) / 2.0f
                - static_cast<float>(mapWidth - mapHeight) * halfTileWidth / 2.0f,
            static_cast<float>(screenHeight) / 2.0f
                - static_cast<float>(mapWidth + mapHeight - 2) * halfTileHeight / 2.0f
        };
    }

}
