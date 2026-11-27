// Module core.entities (implémentation).
module;
import std;

module core.entities;

import core.exceptions;
import core.pathfinding;

namespace core {

    // ----- Entity -----

    Entity::Entity(const GridPos position, const int health)
        : position{position}, health{health}, maxHealth{health} {
        if (health <= 0) {
            throw GameException{"Les PV d'une entite doivent etre strictement positifs"};
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

    int Entity::GetMaxHealth() const noexcept {
        return maxHealth;
    }

    bool Entity::IsAlive() const noexcept {
        return health > 0;
    }

    void Entity::TakeDamage(const int amount) noexcept {
        health = std::max(0, health - std::max(0, amount));
    }

    // ----- Player -----

    Player::Player(const GridPos start) : Entity{start, startHealth} {}

    std::string Player::GetName() const {
        return "Motard";
    }

    int Player::GetSpeed() const noexcept {
        return speed;
    }

    void Player::Update(const Seconds elapsed) noexcept {
        for (auto& cooldown : attackCooldowns) {
            cooldown.Update(elapsed);
        }
        lock.Update(elapsed);
        moveCooldown.Update(elapsed);
    }

    bool Player::IsLocked() const noexcept {
        return !lock.IsReady();
    }

    bool Player::CanUseAttack(const std::size_t index) const noexcept {
        return index < attacks.size() && attackCooldowns[index].IsReady();
    }

    bool Player::TryUseAttack(const std::size_t index) noexcept {
        if (!CanUseAttack(index)) {
            return false;
        }
        attackCooldowns[index].Restart(attacks[index].cooldown);
        lock.Restart(attacks[index].lockDuration);
        return true;
    }

    Seconds Player::AttackCooldownRemaining(const std::size_t index) const noexcept {
        return index < attackCooldowns.size() ? attackCooldowns[index].Remaining() : Seconds{0.0f};
    }

    bool Player::TryMoveTo(const Grid& grid, const GridPos target, const std::vector<GridPos>& obstacles,
                           const bool inCombat) {
        if (IsLocked() || (inCombat && !moveCooldown.IsReady())) {
            return false;
        }
        const auto reachable = ReachableTiles(grid, GetPosition(), speed, obstacles);
        if (std::ranges::find(reachable, target) == reachable.end()) {
            return false;
        }
        SetPosition(target);
        if (inCombat) {
            moveCooldown.Restart(moveDelay);
        }
        return true;
    }

    // ----- Hostile -----

    Hostile::Hostile(const GridPos position, const int health, const Attack& attack)
        : Entity{position, health}, attack{attack} {}

    const Attack& Hostile::GetAttackType() const noexcept {
        return attack;
    }

    bool Hostile::IsActive() const noexcept {
        return active;
    }

    void Hostile::Activate() noexcept {
        if (!active) {
            active = true;
            countdown.Restart(attack.cooldown);
        }
    }

    void Hostile::Update(const Seconds elapsed) noexcept {
        if (active) {
            countdown.Update(elapsed);
        }
    }

    bool Hostile::IsReadyToStrike() const noexcept {
        return active && countdown.IsReady();
    }

    void Hostile::Rearm() noexcept {
        countdown.Restart(attack.cooldown);
    }

    Seconds Hostile::TimeBeforeStrike() const noexcept {
        return countdown.Remaining();
    }

    // ----- Enemy -----

    Enemy::Enemy(const GridPos position, const EnemyKind kind)
        : Hostile{position, startHealth, kind == EnemyKind::Fast ? knifeStab : batSwing}, kind{kind} {}

    EnemyKind Enemy::GetKind() const noexcept {
        return kind;
    }

    std::string Enemy::GetName() const {
        return kind == EnemyKind::Fast ? "Voyou rapide" : "Voyou costaud";
    }

    // ----- Boss -----

    Boss::Boss(const GridPos topLeft) : Hostile{topLeft, startHealth, shockwave} {}

    std::string Boss::GetName() const {
        return "Chef de gang";
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

}
