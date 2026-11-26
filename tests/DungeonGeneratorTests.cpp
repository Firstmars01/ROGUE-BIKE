// Tests unitaires du module core.dungeonGenerator : méthodes de Room, dimensions, salles, départ, escalier,
// accessibilité et reproductibilité, vérifiés sur plusieurs graines.
#include <catch2/catch_all.hpp>
#include <array>
#include <queue>
#include <vector>

import core.grid;
import core.dungeonGenerator;
import core.exceptions;

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

TEST_CASE("Room conserve sa position et ses dimensions", "[room]") {
    const auto room = Room{2, 3, 6, 4};
    REQUIRE(room.GetX() == 2);
    REQUIRE(room.GetY() == 3);
    REQUIRE(room.GetWidth() == 6);
    REQUIRE(room.GetHeight() == 4);
}

TEST_CASE("Room refuse des dimensions invalides", "[room]") {
    REQUIRE_THROWS_AS((Room{0, 0, 0, 4}), InvalidDimensionException);
    REQUIRE_THROWS_AS((Room{0, 0, 4, -2}), InvalidDimensionException);
}

TEST_CASE("Room::Center renvoie la case centrale", "[room]") {
    const auto room = Room{2, 3, 6, 4};
    REQUIRE(room.Center() == GridPos{5, 5});
}

TEST_CASE("Room::Contains detecte les cases de la salle", "[room]") {
    const auto room = Room{2, 3, 4, 4};

    // Coins intérieurs
    REQUIRE(room.Contains(GridPos{2, 3}));
    REQUIRE(room.Contains(GridPos{5, 6}));
    // Juste à l'extérieur
    REQUIRE_FALSE(room.Contains(GridPos{1, 3}));
    REQUIRE_FALSE(room.Contains(GridPos{6, 3}));
    REQUIRE_FALSE(room.Contains(GridPos{2, 7}));
}

TEST_CASE("Room::Intersects tient compte de l'ecart minimal", "[room]") {
    const auto room = Room{0, 0, 4, 4};

    // Chevauchement direct
    REQUIRE(room.Intersects(Room{2, 2, 4, 4}, 0));
    // Salles collées : séparées sans écart, trop proches avec un écart de 1
    REQUIRE_FALSE(room.Intersects(Room{4, 0, 4, 4}, 0));
    REQUIRE(room.Intersects(Room{4, 0, 4, 4}, 1));
    // Une case libre entre les deux : acceptée avec un écart de 1
    REQUIRE_FALSE(room.Intersects(Room{5, 0, 4, 4}, 1));
    // Le test est symétrique
    REQUIRE(Room{4, 0, 4, 4}.Intersects(room, 1));
}

TEST_CASE("Le donjon genere a les dimensions demandees", "[dungeon]") {
    auto generator = DungeonGenerator{42};
    const auto dungeon = generator.Generate(60, 45);

    REQUIRE(dungeon.grid.GetWidth() == 60);
    REQUIRE(dungeon.grid.GetHeight() == 45);
}

TEST_CASE("Une carte trop petite est refusee", "[dungeon]") {
    auto generator = DungeonGenerator{42};
    REQUIRE_THROWS_AS(generator.Generate(5, 30), InvalidDimensionException);
}

TEST_CASE("Les salles respectent les regles de placement", "[dungeon]") {
    const auto seed = GENERATE(range(0u, 50u));
    auto generator = DungeonGenerator{seed};
    const auto dungeon = generator.Generate(60, 45);
    const auto& rooms = dungeon.rooms;

    REQUIRE(rooms.size() >= static_cast<std::size_t>(DungeonGenerator::minRoomCount));
    REQUIRE(rooms.size() <= static_cast<std::size_t>(DungeonGenerator::maxRoomCount));

    for (auto i = std::size_t{0}; i < rooms.size(); ++i) {
        // Taille autorisée et bordure de murs conservée
        REQUIRE(rooms[i].GetWidth() >= DungeonGenerator::minRoomSize);
        REQUIRE(rooms[i].GetWidth() <= DungeonGenerator::maxRoomSize);
        REQUIRE(rooms[i].GetHeight() >= DungeonGenerator::minRoomSize);
        REQUIRE(rooms[i].GetHeight() <= DungeonGenerator::maxRoomSize);
        REQUIRE(rooms[i].GetX() >= 1);
        REQUIRE(rooms[i].GetY() >= 1);
        REQUIRE(rooms[i].GetX() + rooms[i].GetWidth() <= 59);
        REQUIRE(rooms[i].GetY() + rooms[i].GetHeight() <= 44);

        // Aucun chevauchement entre salles
        for (auto j = i + 1; j < rooms.size(); ++j) {
            REQUIRE_FALSE(rooms[i].Intersects(rooms[j], DungeonGenerator::roomGap));
        }
    }
}

TEST_CASE("Le depart est sur du sol et l'escalier est unique et accessible", "[dungeon]") {
    const auto seed = GENERATE(range(0u, 50u));
    auto generator = DungeonGenerator{seed};
    const auto dungeon = generator.Generate(60, 45);

    REQUIRE(dungeon.grid.GetTile(dungeon.playerStart) == TileType::Floor);
    REQUIRE(dungeon.grid.GetTile(dungeon.stairs) == TileType::Stairs);
    REQUIRE(CountTiles(dungeon.grid, TileType::Stairs) == 1);
    REQUIRE_FALSE(dungeon.playerStart == dungeon.stairs);
    REQUIRE(IsReachable(dungeon.grid, dungeon.playerStart, dungeon.stairs));
}

TEST_CASE("Toutes les salles sont reliees au depart", "[dungeon]") {
    const auto seed = GENERATE(range(0u, 50u));
    auto generator = DungeonGenerator{seed};
    const auto dungeon = generator.Generate(60, 45);

    for (const auto& room : dungeon.rooms) {
        REQUIRE(IsReachable(dungeon.grid, dungeon.playerStart, room.Center()));
    }
}

TEST_CASE("Les bords de la carte ne sont jamais praticables", "[dungeon]") {
    const auto seed = GENERATE(range(0u, 50u));
    auto generator = DungeonGenerator{seed};
    const auto dungeon = generator.Generate(60, 45);
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

TEST_CASE("Les murs entre deux zones praticables font une seule case d'epaisseur", "[dungeon]") {
    const auto seed = GENERATE(range(0u, 50u));
    auto generator = DungeonGenerator{seed};
    const auto dungeon = generator.Generate(60, 45);
    const auto& grid = dungeon.grid;

    // Aucun motif sol / mur / mur / sol, ni horizontal ni vertical
    for (auto y = 0; y < grid.GetHeight(); ++y) {
        for (auto x = 0; x < grid.GetWidth(); ++x) {
            const auto horizontal = grid.IsWalkable(GridPos{x, y})
                                    && grid.GetTile(GridPos{x + 1, y}) == TileType::Wall
                                    && grid.GetTile(GridPos{x + 2, y}) == TileType::Wall
                                    && grid.IsWalkable(GridPos{x + 3, y});
            const auto vertical = grid.IsWalkable(GridPos{x, y})
                                  && grid.GetTile(GridPos{x, y + 1}) == TileType::Wall
                                  && grid.GetTile(GridPos{x, y + 2}) == TileType::Wall
                                  && grid.IsWalkable(GridPos{x, y + 3});
            REQUIRE_FALSE(horizontal);
            REQUIRE_FALSE(vertical);
        }
    }
}

TEST_CASE("Une meme graine produit le meme donjon", "[dungeon]") {
    auto first = DungeonGenerator{1234};
    auto second = DungeonGenerator{1234};
    const auto dungeonA = first.Generate(60, 45);
    const auto dungeonB = second.Generate(60, 45);

    const auto tilesA = dungeonA.grid.GetTiles();
    const auto tilesB = dungeonB.grid.GetTiles();
    REQUIRE(tilesA.size() == tilesB.size());
    for (auto i = std::size_t{0}; i < tilesA.size(); ++i) {
        REQUIRE(tilesA[i] == tilesB[i]);
    }
    REQUIRE(dungeonA.playerStart == dungeonB.playerStart);
    REQUIRE(dungeonA.stairs == dungeonB.stairs);
}
