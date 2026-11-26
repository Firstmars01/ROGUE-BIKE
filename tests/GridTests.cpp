// Tests unitaires du module core.grid : dimensions, accès sécurisé, praticabilité.
#include <catch2/catch_all.hpp>

import core.grid;
import core.exceptions;

using namespace core;

// Compte le nombre de tuiles d'un type donné dans la grille.
// grid : grille à parcourir ; type : type recherché. Renvoie le nombre d'occurrences.
static int CountTiles(const Grid& grid, const TileType type) {
    auto count = 0;
    for (const auto tile : grid.GetTiles()) {
        if (tile == type) {
            ++count;
        }
    }
    return count;
}

TEST_CASE("Une nouvelle grille a les bonnes dimensions et est vide", "[grid]") {
    const auto grid = Grid{40, 30};

    REQUIRE(grid.GetWidth() == 40);
    REQUIRE(grid.GetHeight() == 30);
    REQUIRE(grid.GetTiles().size() == 40 * 30);
    REQUIRE(CountTiles(grid, TileType::Empty) == 40 * 30);
}

TEST_CASE("Des dimensions invalides sont refusees", "[grid]") {
    REQUIRE_THROWS_AS((Grid{0, 10}), GameException);
    REQUIRE_THROWS_AS((Grid{10, -1}), GameException);
}

TEST_CASE("IsInBounds detecte les positions hors de la grille", "[grid]") {
    const auto grid = Grid{5, 4};

    // Coins valides
    REQUIRE(grid.IsInBounds(GridPos{0, 0}));
    REQUIRE(grid.IsInBounds(GridPos{4, 3}));

    // Juste en dehors de chaque bord
    REQUIRE_FALSE(grid.IsInBounds(GridPos{-1, 0}));
    REQUIRE_FALSE(grid.IsInBounds(GridPos{0, -1}));
    REQUIRE_FALSE(grid.IsInBounds(GridPos{5, 0}));
    REQUIRE_FALSE(grid.IsInBounds(GridPos{0, 4}));
}

TEST_CASE("GetTile et SetTile sont securises", "[grid]") {
    auto grid = Grid{5, 4};

    SECTION("Lecture et ecriture dans les bornes") {
        REQUIRE(grid.SetTile(GridPos{2, 3}, TileType::Wall));
        REQUIRE(grid.GetTile(GridPos{2, 3}) == TileType::Wall);
        // Les cases voisines ne sont pas modifiées
        REQUIRE(grid.GetTile(GridPos{3, 2}) == TileType::Empty);
    }

    SECTION("Lecture hors des bornes : std::nullopt") {
        REQUIRE_FALSE(grid.GetTile(GridPos{-1, 0}).has_value());
        REQUIRE_FALSE(grid.GetTile(GridPos{5, 3}).has_value());
    }

    SECTION("Ecriture hors des bornes : refusee sans effet") {
        REQUIRE_FALSE(grid.SetTile(GridPos{5, 0}, TileType::Floor));
        REQUIRE(CountTiles(grid, TileType::Floor) == 0);
    }
}

TEST_CASE("IsWalkable accepte uniquement le sol et l'escalier", "[grid]") {
    auto grid = Grid{4, 1};
    grid.SetTile(GridPos{0, 0}, TileType::Floor);
    grid.SetTile(GridPos{1, 0}, TileType::Stairs);
    grid.SetTile(GridPos{2, 0}, TileType::Wall);
    // GridPos{3, 0} reste Empty

    REQUIRE(grid.IsWalkable(GridPos{0, 0}));
    REQUIRE(grid.IsWalkable(GridPos{1, 0}));
    REQUIRE_FALSE(grid.IsWalkable(GridPos{2, 0}));
    REQUIRE_FALSE(grid.IsWalkable(GridPos{3, 0}));
    REQUIRE_FALSE(grid.IsWalkable(GridPos{-1, 0}));
}
