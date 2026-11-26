// Tests unitaires du module core.pathfinding : zone atteignable en marchant (portée, murs, détours).
#include <catch2/catch_all.hpp>
#include <algorithm>
#include <vector>

import core.grid;
import core.pathfinding;

using namespace core;

// Grille width x height entièrement en sol
static Grid MakeOpenGrid(const int width, const int height) {
    auto grid = Grid{width, height};
    for (auto y = 0; y < height; ++y) {
        for (auto x = 0; x < width; ++x) {
            grid.SetTile(GridPos{x, y}, TileType::Floor);
        }
    }
    return grid;
}

// Indique si tiles contient pos
static bool Contains(const std::vector<GridPos>& tiles, const GridPos pos) {
    return std::find(tiles.begin(), tiles.end(), pos) != tiles.end();
}

TEST_CASE("Sur un sol degage, la zone forme un losange autour du depart", "[pathfinding]") {
    const auto grid = MakeOpenGrid(11, 11);
    const auto center = GridPos{5, 5};

    // Portée 1 : les 4 voisins ; portée 2 : 4 + 8 cases ; le départ n'est jamais inclus
    REQUIRE(ReachableTiles(grid, center, 1).size() == 4);
    REQUIRE(ReachableTiles(grid, center, 2).size() == 12);
    REQUIRE_FALSE(Contains(ReachableTiles(grid, center, 2), center));
    REQUIRE_FALSE(Contains(ReachableTiles(grid, center, 2), GridPos{7, 7}));  // diagonale : 4 pas
}

TEST_CASE("La zone contourne les murs (distance en marchant)", "[pathfinding]") {
    // Mur vertical en x = 1 sur les lignes 0 et 1 : pour aller de (0,0) à (2,0), il faut passer par la ligne 2
    auto grid = MakeOpenGrid(3, 3);
    grid.SetTile(GridPos{1, 0}, TileType::Wall);
    grid.SetTile(GridPos{1, 1}, TileType::Wall);

    REQUIRE_FALSE(Contains(ReachableTiles(grid, GridPos{0, 0}, 3), GridPos{2, 0}));  // 2 cases à vol d'oiseau
    REQUIRE(Contains(ReachableTiles(grid, GridPos{0, 0}, 6), GridPos{2, 0}));        // 6 pas en marchant
    REQUIRE_FALSE(Contains(ReachableTiles(grid, GridPos{0, 0}, 6), GridPos{1, 0}));  // jamais sur un mur
}

TEST_CASE("Portee nulle ou depart non praticable : zone vide", "[pathfinding]") {
    auto grid = MakeOpenGrid(5, 5);
    grid.SetTile(GridPos{2, 2}, TileType::Wall);

    REQUIRE(ReachableTiles(grid, GridPos{0, 0}, 0).empty());
    REQUIRE(ReachableTiles(grid, GridPos{0, 0}, -3).empty());
    REQUIRE(ReachableTiles(grid, GridPos{2, 2}, 4).empty());
    REQUIRE(ReachableTiles(grid, GridPos{-1, 0}, 4).empty());
}

TEST_CASE("La zone ne sort jamais de la carte", "[pathfinding]") {
    const auto grid = MakeOpenGrid(3, 3);
    const auto tiles = ReachableTiles(grid, GridPos{0, 0}, 10);

    REQUIRE(tiles.size() == 8);  // les 9 cases moins le départ
    for (const auto& tile : tiles) {
        REQUIRE(grid.IsInBounds(tile));
    }
}
