// Module core.entities (implémentation).
module;
import std;

module core.entities;

import core.exceptions;
import core.pathfinding;

namespace core {

    Entity::Entity(const GridPos position, const int health, const int attack)
        : position{position}, health{health}, attack{attack} {
        if (health <= 0 || attack <= 0) {
            throw GameException{"Les PV et l'attaque d'une entite doivent etre strictement positifs"};
        }
    }

    GridPos Entity::GetPosition() const noexcept {
        return position;
    }

    void Entity::SetPosition(const GridPos position) noexcept {
        this->position = position;
    }

    int Entity::GetHealth() const noexcept {
        return health;
    }

    int Entity::GetAttack() const noexcept {
        return attack;
    }

    bool Entity::IsAlive() const noexcept {
        return health > 0;
    }

    void Entity::TakeDamage(const int amount) noexcept {
        health = std::max(0, health - std::max(0, amount));
    }

    Player::Player(const GridPos start) : Entity{start, startHealth, startAttack} {}

    std::string Player::GetName() const {
        return "Motard";
    }

    int Player::GetSpeed() const noexcept {
        return speed;
    }

    Enemy::Enemy(const GridPos position) : Entity{position, startHealth, startAttack} {}

    std::string Enemy::GetName() const {
        return "Voyou";
    }

    Boss::Boss(const GridPos topLeft) : Entity{topLeft, startHealth, startAttack} {}

    std::string Boss::GetName() const {
        return "Chef de gang";
    }

    bool Boss::Occupies(const GridPos pos) const noexcept {
        const auto topLeft = GetPosition();
        return pos.x >= topLeft.x && pos.x < topLeft.x + size && pos.y >= topLeft.y && pos.y < topLeft.y + size;
    }

    std::vector<GridPos> Boss::OccupiedTiles() const {
        const auto topLeft = GetPosition();
        auto tiles = std::vector<GridPos>{};
        for (auto dy = 0; dy < size; ++dy) {
            for (auto dx = 0; dx < size; ++dx) {
                tiles.push_back(GridPos{topLeft.x + dx, topLeft.y + dy});
            }
        }
        return tiles;
    }

    bool Player::TryMoveTo(const Grid& grid, const GridPos target, const std::vector<GridPos>& obstacles) {
        const auto reachable = ReachableTiles(grid, GetPosition(), speed, obstacles);
        if (std::ranges::find(reachable, target) == reachable.end()) {
            return false;
        }
        SetPosition(target);
        return true;
    }

}
