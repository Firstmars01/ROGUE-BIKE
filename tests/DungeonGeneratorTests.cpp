// Tests unitaires du module core.dungeonGenerator : dimensions, salles, départ, escalier,
// accessibilité et reproductibilité, vérifiés sur plusieurs graines.
#include <catch2/catch_all.hpp>
#include <array>
#include <queue>
#include <vector>

import core.grid;
import core.dungeonGenerator;

using namespace core;

// Indique si to est atteignable depuis from en se déplaçant sur les cases praticables
// (parcours en largeur, 4 directions).
// grid : carte ; from, to : cases de départ et d'arrivée.
static bool IsReachable(const Grid& grid, const GridPos from, const GridPos to) {
    if (!grid.IsWalkable(from) || !grid.IsWalkable(to)) {
        return false;
    }
    const auto width = grid.GetWidth();
    auto visited = std::vector<bool>(static_cast<std::size_t>(width * grid.GetHeight()), false);
    auto toVisit = std::queue<GridPos>{};
    toVisit.push(from);
    visited[static_cast<std::size_t>(from.y * width + from.x)] = true;

    constexpr auto directions = std::array<GridPos, 4>{{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};
    while (!toVisit.empty()) {
        const auto current = toVisit.front();
        toVisit.pop();
        if (current == to) {
            return true;
        }
        for (const auto& direction : directions) {
            const auto next = GridPos{current.x + direction.x, current.y + direction.y};
            if (grid.IsWalkable(next) && !visited[static_cast<std::size_t>(next.y * width + next.x)]) {
                visited[static_cast<std::size_t>(next.y * width + next.x)] = true;
                toVisit.push(next);
            }
        }
    }
    return false;
}

// Compte le nombre de tuiles d'un type donné dans la grille
static int CountTiles(const Grid& grid, const TileType type) {
    auto count = 0;
    for (const auto tile : grid.GetTiles()) {
        if (tile == type) {
            ++count;
        }
    }
    return count;
}

TEST_CASE("Le donjon genere a les dimensions demandees", "[dungeon]") {
    auto generator = DungeonGenerator{42};
    const auto dungeon = generator.Generate(40, 30);

    REQUIRE(dungeon.grid.GetWidth() == 40);
    REQUIRE(dungeon.grid.GetHeight() == 30);
}

TEST_CASE("Une carte trop petite est refusee", "[dungeon]") {
    auto generator = DungeonGenerator{42};
    REQUIRE_THROWS_AS(generator.Generate(5, 30), std::invalid_argument);
}

TEST_CASE("Les salles respectent les regles de placement", "[dungeon]") {
    const auto seed = GENERATE(range(0u, 50u));
    auto generator = DungeonGenerator{seed};
    const auto dungeon = generator.Generate(40, 30);
    const auto& rooms = dungeon.rooms;

    REQUIRE(rooms.size() >= 1);
    REQUIRE(rooms.size() <= static_cast<std::size_t>(DungeonGenerator::maxRoomCount));

    for (auto i = std::size_t{0}; i < rooms.size(); ++i) {
        // Taille autorisée et bordure de murs conservée
        REQUIRE(rooms[i].width >= DungeonGenerator::minRoomSize);
        REQUIRE(rooms[i].width <= DungeonGenerator::maxRoomSize);
        REQUIRE(rooms[i].height >= DungeonGenerator::minRoomSize);
        REQUIRE(rooms[i].height <= DungeonGenerator::maxRoomSize);
        REQUIRE(rooms[i].x >= 1);
        REQUIRE(rooms[i].y >= 1);
        REQUIRE(rooms[i].x + rooms[i].width <= 39);
        REQUIRE(rooms[i].y + rooms[i].height <= 29);

        // Aucun chevauchement entre salles
        for (auto j = i + 1; j < rooms.size(); ++j) {
            REQUIRE_FALSE(rooms[i].Intersects(rooms[j], DungeonGenerator::roomGap));
        }
    }
}

TEST_CASE("Le depart est sur du sol et l'escalier est unique et accessible", "[dungeon]") {
    const auto seed = GENERATE(range(0u, 50u));
    auto generator = DungeonGenerator{seed};
    const auto dungeon = generator.Generate(40, 30);

    REQUIRE(dungeon.grid.GetTile(dungeon.playerStart) == TileType::Floor);
    REQUIRE(dungeon.grid.GetTile(dungeon.stairs) == TileType::Stairs);
    REQUIRE(CountTiles(dungeon.grid, TileType::Stairs) == 1);
    REQUIRE_FALSE(dungeon.playerStart == dungeon.stairs);
    REQUIRE(IsReachable(dungeon.grid, dungeon.playerStart, dungeon.stairs));
}

TEST_CASE("Toutes les salles sont reliees au depart", "[dungeon]") {
    const auto seed = GENERATE(range(0u, 50u));
    auto generator = DungeonGenerator{seed};
    const auto dungeon = generator.Generate(40, 30);

    for (const auto& room : dungeon.rooms) {
        REQUIRE(IsReachable(dungeon.grid, dungeon.playerStart, room.Center()));
    }
}

TEST_CASE("Les bords de la carte ne sont jamais praticables", "[dungeon]") {
    const auto seed = GENERATE(range(0u, 50u));
    auto generator = DungeonGenerator{seed};
    const auto dungeon = generator.Generate(40, 30);
    const auto& grid = dungeon.grid;

    for (auto x = 0; x < grid.GetWidth(); ++x) {
        REQUIRE_FALSE(grid.IsWalkable(GridPos{x, 0}));
        REQUIRE_FALSE(grid.IsWalkable(GridPos{x, grid.GetHeight() - 1}));
    }
    for (auto y = 0; y < grid.GetHeight(); ++y) {
        REQUIRE_FALSE(grid.IsWalkable(GridPos{0, y}));
        REQUIRE_FALSE(grid.IsWalkable(GridPos{grid.GetWidth() - 1, y}));
    }
}

TEST_CASE("Une meme graine produit le meme donjon", "[dungeon]") {
    auto first = DungeonGenerator{1234};
    auto second = DungeonGenerator{1234};
    const auto dungeonA = first.Generate(40, 30);
    const auto dungeonB = second.Generate(40, 30);

    const auto tilesA = dungeonA.grid.GetTiles();
    const auto tilesB = dungeonB.grid.GetTiles();
    REQUIRE(tilesA.size() == tilesB.size());
    for (auto i = std::size_t{0}; i < tilesA.size(); ++i) {
        REQUIRE(tilesA[i] == tilesB[i]);
    }
    REQUIRE(dungeonA.playerStart == dungeonB.playerStart);
    REQUIRE(dungeonA.stairs == dungeonB.stairs);
}
