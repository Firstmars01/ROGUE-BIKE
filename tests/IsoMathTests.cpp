// Tests unitaires du module ui.isoMath : projection grille -> écran, conversion inverse et centrage.
#include <catch2/catch_all.hpp>

import core.grid;
import ui.isoMath;

using namespace core;
using namespace ui;

TEST_CASE("FloorToInt arrondit vers le bas, y compris les negatifs", "[isoMath]") {
    STATIC_REQUIRE(FloorToInt(2.7f) == 2);
    STATIC_REQUIRE(FloorToInt(2.0f) == 2);
    STATIC_REQUIRE(FloorToInt(-0.5f) == -1);
    STATIC_REQUIRE(FloorToInt(-2.0f) == -2);
}

TEST_CASE("GridToScreen applique la projection isometrique", "[isoMath]") {
    constexpr auto origin = ScreenPos{100.0f, 50.0f};

    // Calculs vérifiés à la compilation (fonctions constexpr)
    STATIC_REQUIRE(GridToScreen(GridPos{0, 0}, origin) == origin);
    // +1 en x : vers la droite et le bas
    STATIC_REQUIRE(GridToScreen(GridPos{1, 0}, origin) == ScreenPos{116.0f, 58.0f});
    // +1 en y : vers la gauche et le bas
    STATIC_REQUIRE(GridToScreen(GridPos{0, 1}, origin) == ScreenPos{84.0f, 58.0f});
    // Diagonale : même colonne écran, une tuile plus bas
    STATIC_REQUIRE(GridToScreen(GridPos{1, 1}, origin) == ScreenPos{100.0f, 66.0f});
}

TEST_CASE("ScreenToGrid est l'inverse de GridToScreen", "[isoMath]") {
    constexpr auto origin = ScreenPos{640.0f, 80.0f};

    for (auto y = 0; y < 30; ++y) {
        for (auto x = 0; x < 40; ++x) {
            const auto pos = GridPos{x, y};
            REQUIRE(ScreenToGrid(GridToScreen(pos, origin), origin) == pos);
        }
    }
}

TEST_CASE("ScreenToGrid trouve la case pour un point proche du bord du losange", "[isoMath]") {
    constexpr auto origin = ScreenPos{0.0f, 0.0f};
    const auto center = GridToScreen(GridPos{3, 2}, origin);

    // Points à l'intérieur du losange, près de chacun de ses 4 sommets
    REQUIRE(ScreenToGrid(ScreenPos{center.x, center.y - halfTileHeight + 1.0f}, origin) == GridPos{3, 2});
    REQUIRE(ScreenToGrid(ScreenPos{center.x, center.y + halfTileHeight - 1.0f}, origin) == GridPos{3, 2});
    REQUIRE(ScreenToGrid(ScreenPos{center.x - halfTileWidth + 2.0f, center.y}, origin) == GridPos{3, 2});
    REQUIRE(ScreenToGrid(ScreenPos{center.x + halfTileWidth - 2.0f, center.y}, origin) == GridPos{3, 2});

    // Juste au-delà du sommet droit : case voisine (x + 1, y - 1)
    REQUIRE(ScreenToGrid(ScreenPos{center.x + halfTileWidth + 2.0f, center.y}, origin) == GridPos{4, 1});
}

TEST_CASE("CenteredOrigin centre la carte dans la fenetre", "[isoMath]") {
    constexpr auto mapWidth = 40;
    constexpr auto mapHeight = 30;
    constexpr auto origin = CenteredOrigin(mapWidth, mapHeight, 1280, 720);

    // Bords extrêmes de la carte à l'écran (sommets des losanges des coins)
    const auto left = GridToScreen(GridPos{0, mapHeight - 1}, origin).x - halfTileWidth;
    const auto right = GridToScreen(GridPos{mapWidth - 1, 0}, origin).x + halfTileWidth;
    const auto top = GridToScreen(GridPos{0, 0}, origin).y - halfTileHeight;
    const auto bottom = GridToScreen(GridPos{mapWidth - 1, mapHeight - 1}, origin).y + halfTileHeight;

    // Le centre de la carte coïncide avec le centre de la fenêtre
    REQUIRE((left + right) / 2.0f == Catch::Approx(640.0f));
    REQUIRE((top + bottom) / 2.0f == Catch::Approx(360.0f));
    // La carte 40x30 tient entièrement dans la fenêtre 1280x720
    REQUIRE(left >= 0.0f);
    REQUIRE(right <= 1280.0f);
    REQUIRE(top >= 0.0f);
    REQUIRE(bottom <= 720.0f);
}
