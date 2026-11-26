// Module core.pathfinding (implémentation).
module;
import std;

module core.pathfinding;

namespace core {

    std::vector<GridPos> ReachableTiles(const Grid& grid, const GridPos start, const int range,
                                        const std::vector<GridPos>& obstacles) {
        auto reachable = std::vector<GridPos>{};
        if (range <= 0 || !grid.IsWalkable(start)) {
            return reachable;
        }

        // Distance en pas depuis start pour chaque case (-1 = pas encore visitée).
        // Parenthèses et non {} : avec {}, le vector prendrait les valeurs comme éléments
        const auto width = grid.GetWidth();
        const auto indexOf = [width](const GridPos pos) {
            return static_cast<std::size_t>(pos.y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(pos.x);
        };
        auto distance = std::vector<int>(static_cast<std::size_t>(width) * static_cast<std::size_t>(grid.GetHeight()), -1);
        auto toVisit = std::queue<GridPos>{};
        distance[indexOf(start)] = 0;
        toVisit.push(start);

        constexpr auto directions = std::array{Direction::Up, Direction::Down, Direction::Left, Direction::Right};
        while (!toVisit.empty()) {
            const auto current = toVisit.front();
            toVisit.pop();
            const auto currentDistance = distance[indexOf(current)];
            // Portée atteinte : on n'explore pas plus loin depuis cette case
            if (currentDistance == range) {
                continue;
            }
            for (const auto direction : directions) {
                const auto next = Neighbour(current, direction);
                const auto blocked = std::ranges::find(obstacles, next) != obstacles.end();
                if (grid.IsWalkable(next) && !blocked && distance[indexOf(next)] < 0) {
                    distance[indexOf(next)] = currentDistance + 1;
                    reachable.push_back(next);
                    toVisit.push(next);
                }
            }
        }
        return reachable;
    }

}
