// Tests unitaires du module core.attacks : distances, ligne de vue, portée, zones d'effet, cooldowns.
#include <catch2/catch_all.hpp>
#include <algorithm>
#include <vector>

import core.grid;
import core.attacks;

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
    return std::ranges::find(tiles, pos) != tiles.end();
}

// Attaque de test : seules la forme et la taille comptent pour les zones
static Attack MakeArea(const AreaShape shape, const int radius) {
    return Attack{"Test", 1, 0, shape, radius, Seconds{1.0f}, Seconds{0.0f}};
}

TEST_CASE("ChebyshevDistance compte les diagonales comme un pas", "[attacks]") {
    REQUIRE(ChebyshevDistance(GridPos{0, 0}, GridPos{0, 0}) == 0);
    REQUIRE(ChebyshevDistance(GridPos{0, 0}, GridPos{1, 1}) == 1);
    REQUIRE(ChebyshevDistance(GridPos{2, 3}, GridPos{5, 1}) == 3);
}

TEST_CASE("La ligne de vue est bloquee par les murs", "[attacks]") {
    auto grid = MakeOpenGrid(7, 3);
    REQUIRE(HasLineOfSight(grid, GridPos{0, 1}, GridPos{6, 1}));
    REQUIRE(HasLineOfSight(grid, GridPos{0, 1}, GridPos{1, 1}));   // case voisine

    grid.SetTile(GridPos{3, 1}, TileType::Wall);
    REQUIRE_FALSE(HasLineOfSight(grid, GridPos{0, 1}, GridPos{6, 1}));  // mur au milieu
    REQUIRE_FALSE(HasLineOfSight(grid, GridPos{0, 1}, GridPos{3, 1}));  // cible sur le mur
    REQUIRE(HasLineOfSight(grid, GridPos{0, 0}, GridPos{6, 0}));        // ligne du dessus libre
}

TEST_CASE("Formes des zones d'effet sur un sol degage", "[attacks]") {
    const auto grid = MakeOpenGrid(11, 11);
    const auto center = GridPos{5, 5};

    REQUIRE(AffectedTiles(grid, MakeArea(AreaShape::Single, 0), center).size() == 1);
    REQUIRE(AffectedTiles(grid, MakeArea(AreaShape::Cross, 1), center).size() == 5);
    REQUIRE(AffectedTiles(grid, MakeArea(AreaShape::Cross, 2), center).size() == 9);
    REQUIRE(AffectedTiles(grid, MakeArea(AreaShape::Square, 1), center).size() == 9);
    REQUIRE(AffectedTiles(grid, MakeArea(AreaShape::Square, 2), center).size() == 25);

    const auto cross = AffectedTiles(grid, MakeArea(AreaShape::Cross, 1), center);
    REQUIRE(Contains(cross, GridPos{6, 5}));
    REQUIRE_FALSE(Contains(cross, GridPos{6, 6}));  // pas de diagonale dans une croix
}

TEST_CASE("La zone d'effet s'arrete aux bords et aux murs", "[attacks]") {
    auto grid = MakeOpenGrid(5, 5);

    // Coin de la carte : seulement 4 cases du carré de rayon 1
    REQUIRE(AffectedTiles(grid, MakeArea(AreaShape::Square, 1), GridPos{0, 0}).size() == 4);

    // Mur en (2,1) : ni la case du mur ni la case cachée derrière ne sont touchées
    grid.SetTile(GridPos{2, 1}, TileType::Wall);
    const auto tiles = AffectedTiles(grid, MakeArea(AreaShape::Square, 2), GridPos{2, 2});
    REQUIRE_FALSE(Contains(tiles, GridPos{2, 1}));
    REQUIRE_FALSE(Contains(tiles, GridPos{2, 0}));
    REQUIRE(Contains(tiles, GridPos{2, 4}));
}

TEST_CASE("IsInRange verifie la portee, le sol et la ligne de vue", "[attacks]") {
    auto grid = MakeOpenGrid(8, 3);
    const auto attacker = GridPos{0, 1};

    REQUIRE(IsInRange(grid, molotov, attacker, GridPos{5, 1}));         // portée 5
    REQUIRE_FALSE(IsInRange(grid, molotov, attacker, GridPos{6, 1}));   // trop loin
    REQUIRE(IsInRange(grid, helmetStrike, attacker, GridPos{1, 2}));    // voisin en diagonale
    REQUIRE(IsInRange(grid, hornBlast, attacker, attacker));            // portée 0 : sur soi
    REQUIRE_FALSE(IsInRange(grid, hornBlast, attacker, GridPos{1, 1}));
    REQUIRE_FALSE(IsInRange(grid, helmetStrike, attacker, attacker));   // attaque à distance : jamais sur soi

    grid.SetTile(GridPos{2, 1}, TileType::Wall);
    REQUIRE_FALSE(IsInRange(grid, molotov, attacker, GridPos{4, 1}));   // derrière le mur
    REQUIRE_FALSE(IsInRange(grid, molotov, attacker, GridPos{2, 1}));   // sur le mur
}

TEST_CASE("Le cooldown compte a rebours sans passer sous zero", "[attacks]") {
    auto cooldown = Cooldown{};
    REQUIRE(cooldown.IsReady());

    cooldown.Restart(Seconds{2.0f});
    REQUIRE_FALSE(cooldown.IsReady());
    REQUIRE(cooldown.Remaining().count() == Catch::Approx(2.0f));

    cooldown.Update(Seconds{1.5f});
    REQUIRE(cooldown.Remaining().count() == Catch::Approx(0.5f));

    cooldown.Update(Seconds{-3.0f});  // durée négative ignorée
    REQUIRE(cooldown.Remaining().count() == Catch::Approx(0.5f));

    cooldown.Update(Seconds{1.0f});
    REQUIRE(cooldown.IsReady());
    REQUIRE(cooldown.Remaining().count() == Catch::Approx(0.0f));

    cooldown.Restart(Seconds{-1.0f});  // durée négative : prêt tout de suite
    REQUIRE(cooldown.IsReady());
}

TEST_CASE("Caracteristiques des attaques prevues", "[attacks]") {
    REQUIRE(helmetStrike.damage == 25);
    REQUIRE(helmetStrike.range == 1);
    REQUIRE(hornBlast.shape == AreaShape::Square);
    REQUIRE(hornBlast.radius == 2);
    REQUIRE(molotov.range == 5);
    REQUIRE(molotov.cooldown.count() == Catch::Approx(5.0f));
    REQUIRE(knifeStab.cooldown.count() < batSwing.cooldown.count());   // rapide contre lourd
    REQUIRE(knifeStab.damage < batSwing.damage);
    REQUIRE(shockwave.radius == 3);
}
