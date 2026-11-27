// Tests unitaires du module core.game : activation des adversaires, frappes, murs, attaques du joueur,
// retrait des morts, combat, immobilisation et fin de partie.
#include <catch2/catch_all.hpp>
#include <vector>

import core.grid;
import core.attacks;
import core.entities;
import core.dungeonGenerator;
import core.game;

using namespace core;

// Arène de test : 26x10 cases de sol et 3 salles
//   salle 0 (départ) : x 0..3,   y 0..3
//   salle 1          : x 8..13,  y 0..5
//   salle 2 (boss)   : x 16..23, y 0..7, sortie en (19,3), boss en (20,3) sur 2x2 cases
// Les colonnes 4..7 et 14..15 sont hors des salles (couloirs).
static Dungeon MakeArena(const GridPos playerStart, const std::vector<EnemySpawn>& enemies) {
    auto grid = Grid{26, 10};
    for (auto y = 0; y < 10; ++y) {
        for (auto x = 0; x < 26; ++x) {
            grid.SetTile(GridPos{x, y}, TileType::Floor);
        }
    }
    grid.SetTile(GridPos{19, 3}, TileType::Stairs);   // sortie, comme dans un vrai étage
    return Dungeon{std::move(grid), {Room{0, 0, 4, 4}, Room{8, 0, 6, 6}, Room{16, 0, 8, 8}},
                   playerStart, GridPos{19, 3}, GridPos{20, 3}, enemies};
}

TEST_CASE("Une partie generee place le joueur, les ennemis et le boss", "[game]") {
    const auto game = Game{42, 60, 45};
    REQUIRE(game.GetPlayer().GetPosition() == game.GetDungeon().playerStart);
    REQUIRE(game.GetEnemies().size() == game.GetDungeon().enemySpawns.size());
    REQUIRE(game.GetBoss().has_value());
    REQUIRE(game.GetState() == GameState::Playing);
}

TEST_CASE("Les adversaires ne s'activent que quand le joueur entre dans leur salle", "[game]") {
    auto game = Game{MakeArena(GridPos{1, 1}, {EnemySpawn{GridPos{11, 2}, EnemyKind::Fast}}), 1};

    game.Update(Seconds{10.0f});
    REQUIRE_FALSE(game.GetEnemies()[0]->IsActive());
    REQUIRE(game.GetPlayer().GetHealth() == 100);

    REQUIRE(game.TryMove(GridPos{5, 1}));   // couloir
    REQUIRE(game.TryMove(GridPos{9, 1}));   // entrée dans la salle 1
    game.Update(Seconds{0.0f});
    REQUIRE(game.GetEnemies()[0]->IsActive());
    REQUIRE_FALSE(game.GetBoss()->IsActive());
}

TEST_CASE("Un ennemi frappe sa zone a la fin de son compteur", "[game]") {
    // Couteau : carré de rayon 1 autour de (11,2), 8 dégâts au bout de 2 s
    SECTION("Joueur dans la zone : touché") {
        auto game = Game{MakeArena(GridPos{10, 1}, {EnemySpawn{GridPos{11, 2}, EnemyKind::Fast}}), 1};
        game.Update(Seconds{0.0f});   // activation
        game.Update(Seconds{1.5f});
        REQUIRE(game.GetPlayer().GetHealth() == 100);   // compteur pas encore fini
        game.Update(Seconds{0.5f});
        REQUIRE(game.GetPlayer().GetHealth() == 92);
    }
    SECTION("Joueur hors de la zone : épargné") {
        auto game = Game{MakeArena(GridPos{8, 4}, {EnemySpawn{GridPos{11, 2}, EnemyKind::Fast}}), 1};
        game.Update(Seconds{0.0f});
        game.Update(Seconds{2.0f});
        REQUIRE(game.GetPlayer().GetHealth() == 100);
    }
}

TEST_CASE("Un mur protege le joueur de la frappe d'un ennemi", "[game]") {
    // Batte : carré de rayon 2 autour de (11,2) ; mur en (10,2) entre l'ennemi et le joueur en (9,2)
    auto dungeon = MakeArena(GridPos{9, 2}, {EnemySpawn{GridPos{11, 2}, EnemyKind::Heavy}});
    dungeon.grid.SetTile(GridPos{10, 2}, TileType::Wall);
    auto game = Game{std::move(dungeon), 1};

    game.Update(Seconds{0.0f});
    game.Update(Seconds{5.0f});
    REQUIRE(game.GetPlayer().GetHealth() == 100);
}

TEST_CASE("Le joueur attaque, l'ennemi perd des PV puis disparait a sa mort", "[game]") {
    auto game = Game{MakeArena(GridPos{10, 1}, {EnemySpawn{GridPos{11, 2}, EnemyKind::Heavy}}), 1};

    REQUIRE(game.TryAttack(0, GridPos{11, 2}));          // coup de casque : 25 dégâts
    REQUIRE(game.GetEnemies()[0]->GetHealth() == 50);
    REQUIRE(game.GetEnemies()[0]->IsActive());           // touché : il s'active
    REQUIRE_FALSE(game.TryAttack(0, GridPos{11, 2}));    // cooldown de 0,5 s

    game.Update(Seconds{0.5f});
    REQUIRE(game.TryAttack(0, GridPos{11, 2}));
    game.Update(Seconds{0.5f});
    REQUIRE(game.TryAttack(0, GridPos{11, 2}));
    REQUIRE(game.GetEnemies().empty());                  // mort : retiré de la partie
    REQUIRE_FALSE(game.IsInCombat());
}

TEST_CASE("Une attaque hors de portee, hors de vue ou sur un mur est refusee", "[game]") {
    auto dungeon = MakeArena(GridPos{5, 2}, {EnemySpawn{GridPos{11, 2}, EnemyKind::Fast}});
    dungeon.grid.SetTile(GridPos{7, 4}, TileType::Wall);
    auto game = Game{std::move(dungeon), 1};

    REQUIRE_FALSE(game.TryAttack(0, GridPos{11, 2}));    // casque : portée 1
    REQUIRE_FALSE(game.TryAttack(2, GridPos{7, 4}));     // cocktail sur un mur
    REQUIRE_FALSE(game.TryAttack(1, GridPos{6, 2}));     // klaxon : portée 0, seulement sur soi
    REQUIRE_FALSE(game.TryAttack(9, GridPos{5, 2}));     // attaque inexistante
    REQUIRE_FALSE(game.GetPlayer().IsLocked());          // aucune attaque lancée
}

TEST_CASE("Le klaxon touche tous les ennemis autour du joueur", "[game]") {
    auto game = Game{MakeArena(GridPos{10, 2}, {EnemySpawn{GridPos{11, 2}, EnemyKind::Fast},
                                                 EnemySpawn{GridPos{12, 4}, EnemyKind::Fast},
                                                 EnemySpawn{GridPos{13, 2}, EnemyKind::Fast}}), 1};

    REQUIRE(game.TryAttack(1, GridPos{10, 2}));   // carré de rayon 2 : x 8..12, y 0..4
    REQUIRE(game.GetEnemies()[0]->GetHealth() == 55);
    REQUIRE(game.GetEnemies()[1]->GetHealth() == 55);
    REQUIRE(game.GetEnemies()[2]->GetHealth() == 75);   // trop loin
}

TEST_CASE("Un adversaire touche de loin s'active meme hors de la salle du joueur", "[game]") {
    auto game = Game{MakeArena(GridPos{6, 2}, {EnemySpawn{GridPos{11, 2}, EnemyKind::Fast}}), 1};

    REQUIRE(game.TryAttack(2, GridPos{11, 2}));   // cocktail, portée 5, depuis le couloir
    REQUIRE(game.GetEnemies()[0]->GetHealth() == 35);
    REQUIRE(game.GetEnemies()[0]->IsActive());
}

TEST_CASE("En combat, les deplacements sont espaces et bloques par l'immobilisation", "[game]") {
    auto game = Game{MakeArena(GridPos{9, 1}, {EnemySpawn{GridPos{12, 4}, EnemyKind::Fast}}), 1};
    REQUIRE(game.IsInCombat());

    REQUIRE(game.TryMove(GridPos{9, 2}));
    REQUIRE_FALSE(game.TryMove(GridPos{9, 3}));   // délai de 0,6 s
    game.Update(Player::moveDelay);
    REQUIRE(game.TryMove(GridPos{9, 3}));

    game.Update(Player::moveDelay);
    REQUIRE(game.TryAttack(1, GridPos{9, 3}));    // klaxon : immobilisé 1,5 s
    REQUIRE_FALSE(game.TryMove(GridPos{9, 2}));
    game.Update(hornBlast.lockDuration);
    REQUIRE(game.TryMove(GridPos{9, 2}));
}

TEST_CASE("Hors combat, les deplacements s'enchainent", "[game]") {
    auto game = Game{MakeArena(GridPos{1, 1}, {}), 1};
    REQUIRE_FALSE(game.IsInCombat());
    REQUIRE(game.TryMove(GridPos{2, 1}));
    REQUIRE(game.TryMove(GridPos{3, 1}));
}

TEST_CASE("Le boss ne prend les degats d'une attaque qu'une fois et frappe sa grande zone", "[game]") {
    auto game = Game{MakeArena(GridPos{18, 3}, {}), 1};
    game.Update(Seconds{0.0f});
    REQUIRE(game.GetBoss()->IsActive());

    REQUIRE(game.TryAttack(1, GridPos{18, 3}));   // klaxon : touche 2 cases du boss, 20 dégâts une seule fois
    REQUIRE(game.GetBoss()->GetHealth() == 980);

    game.Update(shockwave.cooldown);              // onde de choc : 40 dégâts
    REQUIRE(game.GetPlayer().GetHealth() == 60);
}

TEST_CASE("Le joueur sans PV met fin a la partie", "[game]") {
    auto game = Game{MakeArena(GridPos{18, 3}, {}), 1};
    game.Update(Seconds{0.0f});
    for (auto strike = 0; strike < 3; ++strike) {   // 3 ondes de choc : 120 dégâts
        game.Update(shockwave.cooldown);
    }
    REQUIRE(game.GetPlayer().GetHealth() == 0);
    REQUIRE(game.GetState() == GameState::GameOver);
    REQUIRE_FALSE(game.TryMove(GridPos{18, 4}));
    REQUIRE_FALSE(game.TryAttack(0, GridPos{19, 3}));
}

TEST_CASE("Les cases du boss et des ennemis sont des obstacles", "[game]") {
    auto game = Game{MakeArena(GridPos{18, 3}, {EnemySpawn{GridPos{18, 5}, EnemyKind::Fast}}), 1};
    const auto reachable = game.PlayerReachableTiles();
    REQUIRE(std::ranges::find(reachable, GridPos{20, 3}) == reachable.end());
    REQUIRE(std::ranges::find(reachable, GridPos{18, 5}) == reachable.end());
    REQUIRE_FALSE(game.TryMove(GridPos{21, 4}));
}

// ----- Saut des adversaires après une frappe -----

// Indique si pos est une case libre de sol de la salle room
static bool IsFreeFloorOf(const Game& game, const Room& room, const GridPos pos) {
    return room.Contains(pos) && game.GetDungeon().grid.GetTile(pos) == TileType::Floor
           && !(pos == game.GetPlayer().GetPosition());
}

TEST_CASE("Apres sa frappe, un ennemi saute sur une case libre proche dans sa salle", "[jump]") {
    const auto seed = GENERATE(range(0u, 50u));
    auto game = Game{MakeArena(GridPos{9, 1}, {EnemySpawn{GridPos{12, 4}, EnemyKind::Fast},
                                                EnemySpawn{GridPos{13, 0}, EnemyKind::Heavy}}), seed};
    const auto before = game.GetEnemies()[0]->GetPosition();
    game.Update(Seconds{0.0f});
    game.Update(knifeStab.cooldown);   // l'ennemi rapide frappe puis saute

    const auto after = game.GetEnemies()[0]->GetPosition();
    REQUIRE_FALSE(after == before);
    REQUIRE(ChebyshevDistance(after, before) <= Game::maxHostileJump);
    REQUIRE(IsFreeFloorOf(game, game.GetDungeon().rooms[1], after));
    REQUIRE_FALSE(after == game.GetEnemies()[1]->GetPosition());
}

TEST_CASE("Le rapide se colle au joueur, le costaud garde ses distances", "[jump]") {
    const auto seed = GENERATE(range(0u, 50u));
    auto game = Game{MakeArena(GridPos{9, 1}, {EnemySpawn{GridPos{12, 4}, EnemyKind::Fast},
                                                EnemySpawn{GridPos{13, 2}, EnemyKind::Heavy}}), seed};
    game.Update(Seconds{0.0f});
    game.Update(batSwing.cooldown);   // les deux ont frappé au moins une fois et sauté

    const auto player = game.GetPlayer().GetPosition();
    REQUIRE(ChebyshevDistance(game.GetEnemies()[0]->GetPosition(), player) == Enemy::fastPreferredDistance);
    REQUIRE(ChebyshevDistance(game.GetEnemies()[1]->GetPosition(), player) == Enemy::heavyPreferredDistance);
}

TEST_CASE("Le boss saute avec ses 4 cases dans sa salle, sans couvrir la sortie", "[jump]") {
    const auto seed = GENERATE(range(0u, 50u));
    auto game = Game{MakeArena(GridPos{16, 7}, {}), seed};
    const auto before = game.GetBoss()->GetPosition();
    game.Update(Seconds{0.0f});
    game.Update(shockwave.cooldown);

    REQUIRE(ChebyshevDistance(game.GetBoss()->GetPosition(), before) <= Game::maxHostileJump);
    for (const auto& tile : game.GetBoss()->OccupiedTiles()) {
        REQUIRE(IsFreeFloorOf(game, game.GetDungeon().rooms[2], tile));
        REQUIRE_FALSE(tile == game.GetDungeon().stairs);
    }
}

TEST_CASE("Les sauts sont reproductibles avec la meme graine", "[jump]") {
    auto first = Game{MakeArena(GridPos{9, 1}, {EnemySpawn{GridPos{12, 4}, EnemyKind::Fast}}), 7};
    auto second = Game{MakeArena(GridPos{9, 1}, {EnemySpawn{GridPos{12, 4}, EnemyKind::Fast}}), 7};
    for (auto step = 0; step < 5; ++step) {
        first.Update(knifeStab.cooldown);
        second.Update(knifeStab.cooldown);
        REQUIRE(first.GetEnemies()[0]->GetPosition() == second.GetEnemies()[0]->GetPosition());
    }
}

TEST_CASE("En pleine melee, les adversaires restent dans leur salle sans jamais se chevaucher", "[jump]") {
    const auto seed = GENERATE(range(0u, 50u));
    auto game = Game{MakeArena(GridPos{10, 2}, {EnemySpawn{GridPos{8, 0}, EnemyKind::Fast},
                                                 EnemySpawn{GridPos{13, 0}, EnemyKind::Heavy},
                                                 EnemySpawn{GridPos{8, 5}, EnemyKind::Fast},
                                                 EnemySpawn{GridPos{13, 5}, EnemyKind::Heavy}}), seed};
    const auto& room = game.GetDungeon().rooms[1];

    // 10 s par pas de 0,1 s : les 4 ennemis frappent et sautent plusieurs fois
    for (auto step = 0; step < 100; ++step) {
        game.Update(Seconds{0.1f});
        auto tiles = std::vector<GridPos>{game.GetPlayer().GetPosition()};
        for (const auto& enemy : game.GetEnemies()) {
            REQUIRE(room.Contains(enemy->GetPosition()));
            tiles.push_back(enemy->GetPosition());
        }
        for (auto i = std::size_t{0}; i < tiles.size(); ++i) {
            for (auto j = i + 1; j < tiles.size(); ++j) {
                REQUIRE_FALSE(tiles[i] == tiles[j]);
            }
        }
    }
}
