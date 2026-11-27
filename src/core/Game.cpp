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
        : Game{DungeonGenerator{seed}.Generate(width, height), seed} {}

    Game::Game(Dungeon dungeon, const std::uint32_t seed)
        : dungeon{std::move(dungeon)},
          player{this->dungeon.playerStart},
          boss{Boss{this->dungeon.bossPosition}},
          random{seed} {
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
        // La zone est vide si l'index ou la cible n'est pas valide
        const auto area = AttackArea(attackIndex, target);
        if (state != GameState::Playing || area.empty() || !player.TryUseAttack(attackIndex)) {
            return false;
        }

        // Dégâts sur chaque adversaire touché (une seule fois, même s'il occupe plusieurs cases de la zone)
        const auto& attack = Player::attacks[attackIndex];
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

    std::vector<GridPos> Game::AttackTargets(const std::size_t attackIndex) const {
        auto targets = std::vector<GridPos>{};
        if (attackIndex >= Player::attacks.size()) {
            return targets;
        }
        const auto& attack = Player::attacks[attackIndex];
        const auto origin = player.GetPosition();
        for (auto dy = -attack.range; dy <= attack.range; ++dy) {
            for (auto dx = -attack.range; dx <= attack.range; ++dx) {
                const auto target = GridPos{origin.x + dx, origin.y + dy};
                if (IsInRange(dungeon.grid, attack, origin, target)) {
                    targets.push_back(target);
                }
            }
        }
        return targets;
    }

    std::vector<GridPos> Game::AttackArea(const std::size_t attackIndex, const GridPos target) const {
        if (attackIndex >= Player::attacks.size()
            || !IsInRange(dungeon.grid, Player::attacks[attackIndex], player.GetPosition(), target)) {
            return {};
        }
        return AffectedTiles(dungeon.grid, Player::attacks[attackIndex], target);
    }

    std::vector<GridPos> Game::ImminentStrikeTiles(const Seconds warning) const {
        auto tiles = std::vector<GridPos>{};
        const auto add = [this, &tiles, warning](const Hostile& hostile) {
            if (hostile.IsActive() && hostile.TimeBeforeStrike() <= warning) {
                for (const auto& tile : StrikeArea(hostile)) {
                    if (!Contains(tiles, tile)) {
                        tiles.push_back(tile);
                    }
                }
            }
        };
        for (const auto& enemy : enemies) {
            add(*enemy);
        }
        if (boss) {
            add(*boss);
        }
        return tiles;
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
        Jump(hostile);
    }

    void Game::Jump(Hostile& hostile) {
        const auto current = hostile.GetPosition();
        const auto room = RoomOf(current);
        if (!room) {
            return;
        }
        const auto& area = dungeon.rooms[*room];

        // Forme de l'adversaire relative à sa position (1 case, ou 2x2 pour le boss)
        auto footprint = std::vector<GridPos>{};
        for (const auto& tile : hostile.OccupiedTiles()) {
            footprint.push_back(GridPos{tile.x - current.x, tile.y - current.y});
        }
        const auto taken = TilesTakenByOthers(hostile);

        // Positions possibles, notées par l'écart à la distance préférée au joueur (0 = idéal)
        auto bestScore = std::numeric_limits<int>::max();
        auto best = std::vector<GridPos>{};
        for (auto y = area.GetY(); y < area.GetY() + area.GetHeight(); ++y) {
            for (auto x = area.GetX(); x < area.GetX() + area.GetWidth(); ++x) {
                const auto candidate = GridPos{x, y};
                if (candidate == current || ChebyshevDistance(candidate, current) > maxHostileJump) {
                    continue;
                }
                auto fits = true;
                auto distanceToPlayer = std::numeric_limits<int>::max();
                for (const auto& offset : footprint) {
                    const auto tile = GridPos{candidate.x + offset.x, candidate.y + offset.y};
                    // Sol libre de la salle (jamais la sortie, le joueur ou un autre adversaire)
                    fits = fits && area.Contains(tile) && dungeon.grid.GetTile(tile) == TileType::Floor
                           && !Contains(taken, tile);
                    distanceToPlayer = std::min(distanceToPlayer, ChebyshevDistance(tile, player.GetPosition()));
                }
                if (!fits) {
                    continue;
                }
                const auto score = std::abs(distanceToPlayer - hostile.GetPreferredDistance());
                if (score < bestScore) {
                    bestScore = score;
                    best.clear();
                }
                if (score == bestScore) {
                    best.push_back(candidate);
                }
            }
        }
        if (!best.empty()) {
            const auto index = std::uniform_int_distribution<std::size_t>{0, best.size() - 1}(random);
            hostile.SetPosition(best[index]);
        }
    }

    std::vector<GridPos> Game::TilesTakenByOthers(const Hostile& self) const {
        auto tiles = std::vector<GridPos>{player.GetPosition()};
        const auto add = [&tiles, &self](const Hostile& other) {
            if (&other != &self) {
                const auto occupied = other.OccupiedTiles();
                tiles.insert(tiles.end(), occupied.begin(), occupied.end());
            }
        };
        for (const auto& enemy : enemies) {
            add(*enemy);
        }
        if (boss) {
            add(*boss);
        }
        return tiles;
    }

    void Game::RemoveDead() {
        std::erase_if(enemies, [](const auto& enemy) { return !enemy->IsAlive(); });
        if (boss && !boss->IsAlive()) {
            boss.reset();
        }
    }

}
