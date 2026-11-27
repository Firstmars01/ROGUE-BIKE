// Module core.game (implémentation).
module;
import std;

module core.game;

import core.pathfinding;

namespace core {

    namespace {
        // Indique si tiles contient pos
        bool Contains(const std::vector<GridPos>& tiles, const GridPos pos) {
            return std::ranges::find(tiles, pos) != tiles.end();
        }

        // Indique si une des cases de hostile est dans area
        bool IsHit(const Hostile& hostile, const std::vector<GridPos>& area) {
            return std::ranges::any_of(hostile.OccupiedTiles(), [&area](const GridPos tile) {
                return Contains(area, tile);
            });
        }
    }

    Game::Game(const std::uint32_t seed, const int width, const int height)
        : Game{DungeonGenerator{seed}.Generate(width, height)} {}

    Game::Game(Dungeon dungeon)
        : dungeon{std::move(dungeon)},
          player{this->dungeon.playerStart},
          boss{Boss{this->dungeon.bossPosition}} {
        for (const auto& spawn : this->dungeon.enemySpawns) {
            enemies.push_back(std::make_unique<Enemy>(spawn.position, spawn.kind));
        }
    }

    void Game::Update(const Seconds elapsed) {
        if (state != GameState::Playing) {
            return;
        }
        player.Update(elapsed);
        ActivateHostilesInPlayerRoom();
        for (auto& enemy : enemies) {
            UpdateHostile(*enemy, elapsed);
        }
        if (boss) {
            UpdateHostile(*boss, elapsed);
        }
        RemoveDead();
        if (!player.IsAlive()) {
            state = GameState::GameOver;
        }
    }

    bool Game::TryMove(const GridPos target) {
        return state == GameState::Playing
               && player.TryMoveTo(dungeon.grid, target, ObstacleTiles(), IsInCombat());
    }

    bool Game::TryAttack(const std::size_t attackIndex, const GridPos target) {
        if (state != GameState::Playing || attackIndex >= Player::attacks.size()) {
            return false;
        }
        const auto& attack = Player::attacks[attackIndex];
        if (!IsInRange(dungeon.grid, attack, player.GetPosition(), target) || !player.TryUseAttack(attackIndex)) {
            return false;
        }

        // Dégâts sur chaque adversaire touché (une seule fois, même s'il occupe plusieurs cases de la zone)
        const auto area = AffectedTiles(dungeon.grid, attack, target);
        const auto strike = [&area, &attack](Hostile& hostile) {
            if (IsHit(hostile, area)) {
                hostile.TakeDamage(attack.damage);
                hostile.Activate();
            }
        };
        for (auto& enemy : enemies) {
            strike(*enemy);
        }
        if (boss) {
            strike(*boss);
        }
        RemoveDead();
        return true;
    }

    const Dungeon& Game::GetDungeon() const noexcept {
        return dungeon;
    }

    const Player& Game::GetPlayer() const noexcept {
        return player;
    }

    const std::vector<std::unique_ptr<Enemy>>& Game::GetEnemies() const noexcept {
        return enemies;
    }

    const std::optional<Boss>& Game::GetBoss() const noexcept {
        return boss;
    }

    GameState Game::GetState() const noexcept {
        return state;
    }

    bool Game::IsInCombat() const {
        const auto room = RoomOf(player.GetPosition());
        if (!room) {
            return false;
        }
        const auto inRoom = [this, room](const Hostile& hostile) { return RoomOf(hostile.GetPosition()) == room; };
        return std::ranges::any_of(enemies, [&inRoom](const auto& enemy) { return inRoom(*enemy); })
               || (boss && inRoom(*boss));
    }

    std::vector<GridPos> Game::PlayerReachableTiles() const {
        return ReachableTiles(dungeon.grid, player.GetPosition(), player.GetSpeed(), ObstacleTiles());
    }

    std::vector<GridPos> Game::StrikeArea(const Hostile& hostile) const {
        auto area = std::vector<GridPos>{};
        for (const auto& tile : hostile.OccupiedTiles()) {
            for (const auto& affected : AffectedTiles(dungeon.grid, hostile.GetAttackType(), tile)) {
                if (!Contains(area, affected)) {
                    area.push_back(affected);
                }
            }
        }
        return area;
    }

    std::optional<std::size_t> Game::RoomOf(const GridPos pos) const {
        for (auto index = std::size_t{0}; index < dungeon.rooms.size(); ++index) {
            if (dungeon.rooms[index].Contains(pos)) {
                return index;
            }
        }
        return std::nullopt;
    }

    std::vector<GridPos> Game::ObstacleTiles() const {
        auto tiles = boss ? boss->OccupiedTiles() : std::vector<GridPos>{};
        for (const auto& enemy : enemies) {
            tiles.push_back(enemy->GetPosition());
        }
        return tiles;
    }

    void Game::ActivateHostilesInPlayerRoom() {
        const auto room = RoomOf(player.GetPosition());
        if (!room) {
            return;
        }
        for (auto& enemy : enemies) {
            if (RoomOf(enemy->GetPosition()) == room) {
                enemy->Activate();
            }
        }
        if (boss && RoomOf(boss->GetPosition()) == room) {
            boss->Activate();
        }
    }

    void Game::UpdateHostile(Hostile& hostile, const Seconds elapsed) {
        hostile.Update(elapsed);
        if (!hostile.IsReadyToStrike()) {
            return;
        }
        if (Contains(StrikeArea(hostile), player.GetPosition())) {
            player.TakeDamage(hostile.GetAttackType().damage);
        }
        hostile.Rearm();
    }

    void Game::RemoveDead() {
        std::erase_if(enemies, [](const auto& enemy) { return !enemy->IsAlive(); });
        if (boss && !boss->IsAlive()) {
            boss.reset();
        }
    }

}
