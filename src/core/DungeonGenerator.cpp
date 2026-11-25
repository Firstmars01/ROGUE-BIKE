// Module core.dungeonGenerator (implémentation) : code de Room et DungeonGenerator.
// Interface : DungeonGenerator.ixx
module;
// Import rapide de la bibliothèque standard
import std;

// Unité d'implémentation du module core.dungeonGenerator
module core.dungeonGenerator;

import core.grid;

namespace core {

    // ----- Room -----

    // Case centrale de la salle
    GridPos Room::Center() const noexcept {
        return GridPos{x + width / 2, y + height / 2};
    }

    // Test de chevauchement de deux rectangles agrandis de gap cases
    bool Room::Intersects(const Room& other, const int gap) const noexcept {
        return x < other.x + other.width + gap && other.x < x + width + gap
               && y < other.y + other.height + gap && other.y < y + height + gap;
    }

    // Vrai si pos est dans la salle
    bool Room::Contains(const GridPos pos) const noexcept {
        return pos.x >= x && pos.x < x + width && pos.y >= y && pos.y < y + height;
    }

    // ----- DungeonGenerator -----

    // Initialise le générateur aléatoire avec la graine
    DungeonGenerator::DungeonGenerator(const std::uint32_t seed) : random{seed} {}

    // Génère un étage : salles, couloirs, escalier, puis murs
    Dungeon DungeonGenerator::Generate(const int width, const int height) {
        if (width < minMapSize || height < minMapSize) {
            throw std::invalid_argument{"Carte trop petite pour generer un donjon"};
        }

        auto grid = Grid{width, height};
        const auto rooms = PlaceRooms(width, height);

        // Creusement des salles, puis des couloirs entre salles consécutives
        for (const auto& room : rooms) {
            CarveRoom(grid, room);
        }
        for (auto index = std::size_t{1}; index < rooms.size(); ++index) {
            CarveCorridor(grid, rooms[index - 1].Center(), rooms[index].Center());
        }

        const auto playerStart = rooms.front().Center();
        const auto stairs = ChooseStairs(rooms);
        grid.SetTile(stairs, TileType::Stairs);

        // Les murs sont ajoutés en dernier, uniquement autour des zones praticables
        AddWallsAroundFloors(grid);

        return Dungeon{std::move(grid), rooms, playerStart, stairs};
    }

    // Entier uniforme dans [min, max]
    int DungeonGenerator::RandomInt(const int min, const int max) {
        return std::uniform_int_distribution<int>{min, max}(random);
    }

    // Essaie de placer le nombre de salles visé, en rejetant les chevauchements
    std::vector<Room> DungeonGenerator::PlaceRooms(const int width, const int height) {
        const auto targetCount = RandomInt(minRoomCount, maxRoomCount);
        auto rooms = std::vector<Room>{};

        for (auto attempt = 0; attempt < maxPlacementAttempts
                               && std::cmp_less(rooms.size(), targetCount); ++attempt) {
            const auto candidate = RandomRoom(width, height);
            const auto overlaps = std::ranges::any_of(rooms, [&candidate](const Room& room) {
                return candidate.Intersects(room, roomGap);
            });
            if (!overlaps) {
                rooms.push_back(candidate);
            }
        }
        return rooms;
    }

    // Salle de taille aléatoire, placée en laissant une case de bordure pour les murs
    Room DungeonGenerator::RandomRoom(const int width, const int height) {
        const auto roomWidth = RandomInt(minRoomSize, maxRoomSize);
        const auto roomHeight = RandomInt(minRoomSize, maxRoomSize);
        return Room{
            RandomInt(1, width - roomWidth - 1),
            RandomInt(1, height - roomHeight - 1),
            roomWidth,
            roomHeight
        };
    }

    // Toutes les cases de la salle deviennent du sol
    void DungeonGenerator::CarveRoom(Grid& grid, const Room& room) {
        for (auto y = room.y; y < room.y + room.height; ++y) {
            for (auto x = room.x; x < room.x + room.width; ++x) {
                grid.SetTile(GridPos{x, y}, TileType::Floor);
            }
        }
    }

    // Couloir en L : l'ordre des deux segments est tiré au hasard
    void DungeonGenerator::CarveCorridor(Grid& grid, const GridPos from, const GridPos to) {
        if (RandomInt(0, 1) == 0) {
            CarveHorizontal(grid, from.x, to.x, from.y);
            CarveVertical(grid, from.y, to.y, to.x);
        } else {
            CarveVertical(grid, from.y, to.y, from.x);
            CarveHorizontal(grid, from.x, to.x, to.y);
        }
    }

    // Segment horizontal de sol
    void DungeonGenerator::CarveHorizontal(Grid& grid, const int x1, const int x2, const int y) {
        for (auto x = std::min(x1, x2); x <= std::max(x1, x2); ++x) {
            grid.SetTile(GridPos{x, y}, TileType::Floor);
        }
    }

    // Segment vertical de sol
    void DungeonGenerator::CarveVertical(Grid& grid, const int y1, const int y2, const int x) {
        for (auto y = std::min(y1, y2); y <= std::max(y1, y2); ++y) {
            grid.SetTile(GridPos{x, y}, TileType::Floor);
        }
    }

    // Escalier au centre de la dernière salle (ou au coin opposé s'il n'y a qu'une salle)
    GridPos DungeonGenerator::ChooseStairs(const std::vector<Room>& rooms) {
        if (rooms.size() == 1) {
            const auto& room = rooms.front();
            return GridPos{room.x + room.width - 1, room.y + room.height - 1};
        }
        return rooms.back().Center();
    }

    // Les cases vides qui touchent une case praticable deviennent des murs
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

    // Parcourt les 8 voisins de pos à la recherche d'une case praticable
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
