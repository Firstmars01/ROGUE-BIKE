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

    bool Player::TryMoveTo(const Grid& grid, const GridPos target) {
        const auto reachable = ReachableTiles(grid, GetPosition(), speed);
        if (std::ranges::find(reachable, target) == reachable.end()) {
            return false;
        }
        SetPosition(target);
        return true;
    }

}
