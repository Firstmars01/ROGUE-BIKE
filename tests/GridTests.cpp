// Tests unitaires du module core.grid : dimensions, accès sécurisé, praticabilité.
#include <catch2/catch_all.hpp>
#include <algorithm>

import core.grid;
import core.exceptions;

using namespace core;

TEST_CASE("Une nouvelle grille a les bonnes dimensions et est vide", "[grid]") {
    const auto grid = Grid{40, 30};

    REQUIRE(grid.GetWidth() == 40);
    REQUIRE(grid.GetHeight() == 30);
    REQUIRE(grid.GetTiles().size() == 40 * 30);
    REQUIRE(std::ranges::count(grid.GetTiles(), TileType::Empty) == 40 * 30);
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
        REQUIRE(std::ranges::count(grid.GetTiles(), TileType::Floor) == 0);
    }
}

TEST_CASE("Neighbour renvoie la case voisine dans chaque direction", "[grid]") {
    const auto pos = GridPos{5, 5};
    REQUIRE(Neighbour(pos, Direction::Up) == GridPos{5, 4});
    REQUIRE(Neighbour(pos, Direction::Down) == GridPos{5, 6});
    REQUIRE(Neighbour(pos, Direction::Left) == GridPos{4, 5});
    REQUIRE(Neighbour(pos, Direction::Right) == GridPos{6, 5});
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
