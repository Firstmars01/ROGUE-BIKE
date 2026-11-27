// Module core.attacks (implémentation).
module;
import std;

module core.attacks;

namespace core {

    int ChebyshevDistance(const GridPos a, const GridPos b) noexcept {
        return std::max(std::abs(a.x - b.x), std::abs(a.y - b.y));
    }

    bool HasLineOfSight(const Grid& grid, const GridPos from, const GridPos to) {
        // Tracé de Bresenham : on avance case par case de from vers to
        const auto deltaX = std::abs(to.x - from.x);
        const auto deltaY = -std::abs(to.y - from.y);
        const auto stepX = from.x < to.x ? 1 : -1;
        const auto stepY = from.y < to.y ? 1 : -1;
        auto error = deltaX + deltaY;
        auto current = from;
        while (current != to) {
            const auto doubledError = 2 * error;
            if (doubledError >= deltaY) {
                error += deltaY;
                current.x += stepX;
            }
            if (doubledError <= deltaX) {
                error += deltaX;
                current.y += stepY;
            }
            if (!grid.IsWalkable(current)) {
                return false;
            }
        }
        return true;
    }

    bool IsInRange(const Grid& grid, const Attack& attack, const GridPos attacker, const GridPos target) {
        return grid.IsWalkable(target)
               && ChebyshevDistance(attacker, target) <= attack.range
               && HasLineOfSight(grid, attacker, target);
    }

    std::vector<GridPos> AffectedTiles(const Grid& grid, const Attack& attack, const GridPos center) {
        // Cases candidates selon la forme
        auto candidates = std::vector<GridPos>{center};
        if (attack.shape == AreaShape::Cross) {
            for (auto step = 1; step <= attack.radius; ++step) {
                candidates.push_back(GridPos{center.x + step, center.y});
                candidates.push_back(GridPos{center.x - step, center.y});
                candidates.push_back(GridPos{center.x, center.y + step});
                candidates.push_back(GridPos{center.x, center.y - step});
            }
        } else if (attack.shape == AreaShape::Square) {
            candidates.clear();
            for (auto dy = -attack.radius; dy <= attack.radius; ++dy) {
                for (auto dx = -attack.radius; dx <= attack.radius; ++dx) {
                    candidates.push_back(GridPos{center.x + dx, center.y + dy});
                }
            }
        }

        // On ne garde que les cases praticables visibles depuis le centre
        auto tiles = std::vector<GridPos>{};
        for (const auto& tile : candidates) {
            if (grid.IsWalkable(tile) && HasLineOfSight(grid, center, tile)) {
                tiles.push_back(tile);
            }
        }
        return tiles;
    }

    void Cooldown::Update(const Seconds elapsed) noexcept {
        remaining = std::max(Seconds{0.0f}, remaining - std::max(Seconds{0.0f}, elapsed));
    }

    bool Cooldown::IsReady() const noexcept {
        return remaining <= Seconds{0.0f};
    }

    void Cooldown::Restart(const Seconds duration) noexcept {
        remaining = std::max(Seconds{0.0f}, duration);
    }

    Seconds Cooldown::Remaining() const noexcept {
        return remaining;
    }

}
