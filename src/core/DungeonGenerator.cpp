// Module core.dungeonGenerator (implémentation).
module;
import std;

module core.dungeonGenerator;

import core.exceptions;

namespace core {

    // ----- Room -----

    Room::Room(const int x, const int y, const int width, const int height)
        : x{x}, y{y}, width{width}, height{height} {
        if (width <= 0 || height <= 0) {
            throw GameException{"Les dimensions d'une salle doivent etre strictement positives"};
        }
    }

    int Room::GetX() const noexcept {
        return x;
    }

    int Room::GetY() const noexcept {
        return y;
    }

    int Room::GetWidth() const noexcept {
        return width;
    }

    int Room::GetHeight() const noexcept {
        return height;
    }

    GridPos Room::Center() const noexcept {
        return GridPos{x + width / 2, y + height / 2};
    }

    bool Room::Intersects(const Room& other, const int gap) const noexcept {
        return x < other.x + other.width + gap && other.x < x + width + gap
               && y < other.y + other.height + gap && other.y < y + height + gap;
    }

    bool Room::Contains(const GridPos pos) const noexcept {
        return pos.x >= x && pos.x < x + width && pos.y >= y && pos.y < y + height;
    }

    bool IsInAnyRoom(const std::vector<Room>& rooms, const GridPos pos) {
        return std::ranges::any_of(rooms, [pos](const Room& room) { return room.Contains(pos); });
    }

    // ----- DungeonGenerator -----

    DungeonGenerator::DungeonGenerator(const std::uint32_t seed) : random{seed} {}

    Dungeon DungeonGenerator::Generate(const int width, const int height) {
        if (width < minMapSize || height < minMapSize) {
            throw GameException{"Carte trop petite pour generer un donjon"};
        }

        auto grid = Grid{width, height};
        const auto rooms = PlaceRooms(width, height);
        if (rooms.size() < 2) {
            throw GameException{"Impossible de placer assez de salles"};
        }

        // Salles, puis couloirs entre salles consécutives
        for (const auto& room : rooms) {
            CarveRoom(grid, room);
        }
        for (auto index = std::size_t{1}; index < rooms.size(); ++index) {
            CarveCorridor(grid, rooms[index - 1].Center(), rooms[index].Center());
        }

        const auto playerStart = rooms.front().Center();
        const auto stairs = rooms.back().Center();
        grid.SetTile(stairs, TileType::Stairs);

        // Murs autour du sol, puis amincis à une case (chaque correction peut en créer une autre)
        AddWallsAroundFloors(grid);
        for (auto pass = 0; pass < maxThinningPasses && ThinDoubleWalls(grid, rooms); ++pass) {
            AddWallsAroundFloors(grid);
        }

        // Boss devant la sortie, du côté de la salle précédente
        const auto bossPosition = BossPositionFor(stairs, rooms[rooms.size() - 2].Center());

        // Ennemis placés en dernier : la forme de l'étage ne dépend pas de leur tirage
        auto enemySpawns = PlaceEnemies(rooms);
        return Dungeon{std::move(grid), rooms, playerStart, stairs, bossPosition, std::move(enemySpawns)};
    }

    GridPos DungeonGenerator::BossPositionFor(const GridPos stairs, const GridPos comingFrom) noexcept {
        const auto dx = comingFrom.x - stairs.x;
        const auto dy = comingFrom.y - stairs.y;
        // Côté dominant : le carré du boss est collé à la sortie sur ce côté
        if (std::abs(dx) >= std::abs(dy)) {
            return dx >= 0 ? GridPos{stairs.x + 1, stairs.y} : GridPos{stairs.x - Boss::size, stairs.y};
        }
        return dy >= 0 ? GridPos{stairs.x, stairs.y + 1} : GridPos{stairs.x, stairs.y - Boss::size};
    }

    int DungeonGenerator::EnemyCountFor(const Room& room) noexcept {
        constexpr auto minArea = minRoomSize * minRoomSize;
        constexpr auto maxArea = maxRoomSize * maxRoomSize;
        const auto area = std::clamp(room.GetWidth() * room.GetHeight(), minArea, maxArea);
        return minEnemiesPerRoom + (area - minArea) * (maxEnemiesPerRoom - minEnemiesPerRoom) / (maxArea - minArea);
    }

    std::vector<GridPos> DungeonGenerator::PlaceEnemies(const std::vector<Room>& rooms) {
        auto spawns = std::vector<GridPos>{};
        // Ni la salle de départ (première) ni la salle du boss (dernière)
        for (auto index = std::size_t{1}; index + 1 < rooms.size(); ++index) {
            const auto& room = rooms[index];
            // Toutes les cases libres de la salle, mélangées, puis on garde les premières
            auto candidates = std::vector<GridPos>{};
            for (auto y = room.GetY(); y < room.GetY() + room.GetHeight(); ++y) {
                for (auto x = room.GetX(); x < room.GetX() + room.GetWidth(); ++x) {
                    candidates.push_back(GridPos{x, y});
                }
            }
            std::ranges::shuffle(candidates, random);
            const auto count = std::min(static_cast<std::size_t>(EnemyCountFor(room)), candidates.size());
            spawns.insert(spawns.end(), candidates.begin(), candidates.begin() + static_cast<std::ptrdiff_t>(count));
        }
        return spawns;
    }

    int DungeonGenerator::RandomInt(const int min, const int max) {
        return std::uniform_int_distribution<int>{min, max}(random);
    }

    std::vector<Room> DungeonGenerator::PlaceRooms(const int width, const int height) {
        const auto targetCount = RandomInt(minRoomCount, maxRoomCount);
        // La salle du boss est placée en premier, sur la carte vide : elle trouve toujours sa place
        auto rooms = std::vector<Room>{RandomRoom(width, height, bossRoomMinSize)};

        for (auto attempt = 0; attempt < maxPlacementAttempts
                               && std::cmp_less(rooms.size(), targetCount); ++attempt) {
            const auto candidate = RandomRoom(width, height, minRoomSize);
            const auto overlaps = std::ranges::any_of(rooms, [&candidate](const Room& room) {
                return candidate.Intersects(room, roomGap);
            });
            if (!overlaps) {
                rooms.push_back(candidate);
            }
        }
        // La salle du boss passe en dernier : c'est la salle de la sortie
        std::ranges::rotate(rooms, rooms.begin() + 1);
        return rooms;
    }

    Room DungeonGenerator::RandomRoom(const int width, const int height, const int minSize) {
        const auto roomWidth = RandomInt(minSize, maxRoomSize);
        const auto roomHeight = RandomInt(minSize, maxRoomSize);
        return Room{
            RandomInt(1, width - roomWidth - 1),
            RandomInt(1, height - roomHeight - 1),
            roomWidth,
            roomHeight
        };
    }

    void DungeonGenerator::CarveRoom(Grid& grid, const Room& room) {
        for (auto y = room.GetY(); y < room.GetY() + room.GetHeight(); ++y) {
            for (auto x = room.GetX(); x < room.GetX() + room.GetWidth(); ++x) {
                grid.SetTile(GridPos{x, y}, TileType::Floor);
            }
        }
    }

    void DungeonGenerator::CarveCorridor(Grid& grid, const GridPos from, const GridPos to) {
        // Le coude du L est tiré au hasard : (to.x, from.y) ou (from.x, to.y)
        const auto corner = RandomInt(0, 1) == 0 ? GridPos{to.x, from.y} : GridPos{from.x, to.y};
        CarveLine(grid, from, corner);
        CarveLine(grid, corner, to);
    }

    void DungeonGenerator::CarveLine(Grid& grid, const GridPos from, const GridPos to) {
        for (auto y = std::min(from.y, to.y); y <= std::max(from.y, to.y); ++y) {
            for (auto x = std::min(from.x, to.x); x <= std::max(from.x, to.x); ++x) {
                grid.SetTile(GridPos{x, y}, TileType::Floor);
            }
        }
    }

    void DungeonGenerator::AddWallsAroundFloors(Grid& grid) {
        for (auto y = 0; y < grid.GetHeight(); ++y) {
            for (auto x = 0; x < grid.GetWidth(); ++x) {
                const auto pos = GridPos{x, y};
                if (grid.GetTile(pos) == TileType::Empty && HasWalkableNeighbour(grid, pos)) {
                    grid.SetTile(pos, TileType::Wall);
                }
            }
        }
    }

    bool DungeonGenerator::ThinDoubleWalls(Grid& grid, const std::vector<Room>& rooms) {
        auto changed = false;
        constexpr auto directions = std::array<GridPos, 2>{{{1, 0}, {0, 1}}};
        for (auto y = 0; y < grid.GetHeight(); ++y) {
            for (auto x = 0; x < grid.GetWidth(); ++x) {
                for (const auto& direction : directions) {
                    const auto first = GridPos{x, y};
                    const auto wall1 = GridPos{x + direction.x, y + direction.y};
                    const auto wall2 = GridPos{x + 2 * direction.x, y + 2 * direction.y};
                    const auto last = GridPos{x + 3 * direction.x, y + 3 * direction.y};
                    if (grid.IsWalkable(first) && grid.GetTile(wall1) == TileType::Wall
                        && grid.GetTile(wall2) == TileType::Wall && grid.IsWalkable(last)) {
                        // On élargit le couloir plutôt que la salle
                        const auto widenLastSide = IsInAnyRoom(rooms, first) && !IsInAnyRoom(rooms, last);
                        grid.SetTile(widenLastSide ? wall2 : wall1, TileType::Floor);
                        changed = true;
                    }
                }
            }
        }
        return changed;
    }

    bool DungeonGenerator::HasWalkableNeighbour(const Grid& grid, const GridPos pos) {
        for (auto dy = -1; dy <= 1; ++dy) {
            for (auto dx = -1; dx <= 1; ++dx) {
                if ((dx != 0 || dy != 0) && grid.IsWalkable(GridPos{pos.x + dx, pos.y + dy})) {
                    return true;
                }
            }
        }
        return false;
    }

}
