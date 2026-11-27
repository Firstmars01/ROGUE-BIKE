// Tests unitaires du module core.entities : points de vie, déplacement, attaques et immobilisation
// du joueur, compteurs des ennemis et du boss, polymorphisme.
#include <catch2/catch_all.hpp>
#include <memory>
#include <vector>

import core.grid;
import core.attacks;
import core.entities;

using namespace core;

// Zone de test : 7x2 cases de sol, avec un mur en (3,1)
//   ligne 0 : . . . . . . .
//   ligne 1 : . . . # . . .
static Grid MakeTestArea() {
    auto grid = Grid{7, 2};
    for (auto y = 0; y < 2; ++y) {
        for (auto x = 0; x < 7; ++x) {
            grid.SetTile(GridPos{x, y}, TileType::Floor);
        }
    }
    grid.SetTile(GridPos{3, 1}, TileType::Wall);
    return grid;
}

// ----- Points de vie -----

TEST_CASE("Points de vie de depart : joueur 100, ennemi 75, boss 1000", "[entities]") {
    REQUIRE(Player{GridPos{0, 0}}.GetHealth() == 100);
    REQUIRE(Enemy{GridPos{0, 0}, EnemyKind::Fast}.GetHealth() == 75);
    REQUIRE(Boss{GridPos{0, 0}}.GetHealth() == 1000);
}

TEST_CASE("TakeDamage retire des PV sans descendre sous zero, degats negatifs ignores", "[entities]") {
    auto enemy = Enemy{GridPos{0, 0}, EnemyKind::Heavy};
    enemy.TakeDamage(30);
    enemy.TakeDamage(-20);
    REQUIRE(enemy.GetHealth() == 45);
    REQUIRE(enemy.GetMaxHealth() == 75);
    REQUIRE(enemy.IsAlive());

    enemy.TakeDamage(1000);
    REQUIRE(enemy.GetHealth() == 0);
    REQUIRE_FALSE(enemy.IsAlive());
}

// ----- Déplacement du joueur -----

TEST_CASE("TryMoveTo teleporte le joueur dans sa zone de portee", "[player]") {
    const auto grid = MakeTestArea();
    auto player = Player{GridPos{0, 0}};

    REQUIRE(player.TryMoveTo(grid, GridPos{4, 0}, {}, false));   // 4 pas exactement
    REQUIRE(player.GetPosition() == GridPos{4, 0});
}

TEST_CASE("TryMoveTo refuse une case trop loin, un mur, un obstacle ou la case actuelle", "[player]") {
    const auto grid = MakeTestArea();
    auto player = Player{GridPos{0, 0}};
    const auto obstacles = std::vector<GridPos>{GridPos{2, 0}};

    REQUIRE_FALSE(player.TryMoveTo(grid, GridPos{5, 0}, {}, false));          // 5 pas : trop loin
    REQUIRE_FALSE(player.TryMoveTo(grid, GridPos{3, 1}, {}, false));          // mur
    REQUIRE_FALSE(player.TryMoveTo(grid, GridPos{0, 0}, {}, false));          // case actuelle
    REQUIRE_FALSE(player.TryMoveTo(grid, GridPos{2, 0}, obstacles, false));   // case d'un ennemi
    REQUIRE(player.GetPosition() == GridPos{0, 0});
}

TEST_CASE("En combat, deux deplacements sont espaces d'au moins moveDelay", "[player]") {
    const auto grid = MakeTestArea();
    auto player = Player{GridPos{0, 0}};

    REQUIRE(player.TryMoveTo(grid, GridPos{1, 0}, {}, true));
    REQUIRE_FALSE(player.TryMoveTo(grid, GridPos{2, 0}, {}, true));   // délai non écoulé
    player.Update(Player::moveDelay);
    REQUIRE(player.TryMoveTo(grid, GridPos{2, 0}, {}, true));
}

TEST_CASE("Hors combat, les deplacements s'enchainent sans delai", "[player]") {
    const auto grid = MakeTestArea();
    auto player = Player{GridPos{0, 0}};

    REQUIRE(player.TryMoveTo(grid, GridPos{1, 0}, {}, false));
    REQUIRE(player.TryMoveTo(grid, GridPos{2, 0}, {}, false));
}

// ----- Attaques du joueur -----

TEST_CASE("Le joueur dispose de ses 3 attaques, toutes pretes au depart", "[player]") {
    const auto player = Player{GridPos{0, 0}};
    REQUIRE(Player::attacks[0].name == helmetStrike.name);
    REQUIRE(Player::attacks[1].name == hornBlast.name);
    REQUIRE(Player::attacks[2].name == molotov.name);
    for (auto index = std::size_t{0}; index < Player::attacks.size(); ++index) {
        REQUIRE(player.CanUseAttack(index));
    }
    REQUIRE_FALSE(player.CanUseAttack(3));   // index invalide
}

TEST_CASE("Une attaque relance son cooldown et immobilise le joueur", "[player]") {
    const auto grid = MakeTestArea();
    auto player = Player{GridPos{0, 0}};

    REQUIRE(player.TryUseAttack(2));   // cocktail : cooldown 5 s, immobilisation 2 s
    REQUIRE_FALSE(player.CanUseAttack(2));
    REQUIRE(player.CanUseAttack(0));   // les autres attaques restent disponibles
    REQUIRE(player.AttackCooldownRemaining(2).count() == Catch::Approx(5.0f));
    REQUIRE(player.IsLocked());
    REQUIRE_FALSE(player.TryMoveTo(grid, GridPos{1, 0}, {}, false));   // immobilisé

    player.Update(Seconds{2.0f});
    REQUIRE_FALSE(player.IsLocked());
    REQUIRE(player.TryMoveTo(grid, GridPos{1, 0}, {}, false));
    REQUIRE_FALSE(player.TryUseAttack(2));   // cooldown encore en cours (3 s)

    player.Update(Seconds{3.0f});
    REQUIRE(player.TryUseAttack(2));
}

TEST_CASE("Une attaque d'index invalide est refusee", "[player]") {
    auto player = Player{GridPos{0, 0}};
    REQUIRE_FALSE(player.TryUseAttack(7));
    REQUIRE_FALSE(player.IsLocked());
    REQUIRE(player.AttackCooldownRemaining(7).count() == Catch::Approx(0.0f));
}

// ----- Ennemis et boss -----

TEST_CASE("Le type d'un ennemi determine son attaque", "[enemies]") {
    const auto fast = Enemy{GridPos{0, 0}, EnemyKind::Fast};
    const auto heavy = Enemy{GridPos{0, 0}, EnemyKind::Heavy};

    REQUIRE(fast.GetKind() == EnemyKind::Fast);
    REQUIRE(fast.GetAttackType().name == knifeStab.name);
    REQUIRE(fast.GetName() == "Voyou rapide");
    REQUIRE(heavy.GetAttackType().name == batSwing.name);
    REQUIRE(heavy.GetName() == "Voyou costaud");
}

TEST_CASE("Le compteur d'un ennemi ne tourne qu'une fois active", "[enemies]") {
    auto enemy = Enemy{GridPos{0, 0}, EnemyKind::Fast};   // couteau : 2 s

    enemy.Update(Seconds{10.0f});
    REQUIRE_FALSE(enemy.IsActive());
    REQUIRE_FALSE(enemy.IsReadyToStrike());

    enemy.Activate();
    REQUIRE(enemy.TimeBeforeStrike().count() == Catch::Approx(2.0f));
    enemy.Update(Seconds{1.5f});
    REQUIRE_FALSE(enemy.IsReadyToStrike());
    enemy.Update(Seconds{0.5f});
    REQUIRE(enemy.IsReadyToStrike());

    // Réactiver ne relance pas le compteur ; Rearm le relance
    enemy.Activate();
    REQUIRE(enemy.IsReadyToStrike());
    enemy.Rearm();
    REQUIRE_FALSE(enemy.IsReadyToStrike());
    REQUIRE(enemy.TimeBeforeStrike().count() == Catch::Approx(2.0f));
}

TEST_CASE("Le boss occupe un carre de 2x2 cases et frappe avec l'onde de choc", "[boss]") {
    const auto boss = Boss{GridPos{4, 6}};

    REQUIRE(boss.GetName() == "Chef de gang");
    REQUIRE(boss.GetAttackType().name == shockwave.name);
    REQUIRE(boss.OccupiedTiles() == std::vector<GridPos>{GridPos{4, 6}, GridPos{5, 6}, GridPos{4, 7}, GridPos{5, 7}});
}

// ----- Polymorphisme -----

TEST_CASE("Joueur, ennemis et boss se manipulent ensemble via Entity", "[entities]") {
    auto entities = std::vector<std::unique_ptr<Entity>>{};
    entities.push_back(std::make_unique<Player>(GridPos{0, 0}));
    entities.push_back(std::make_unique<Enemy>(GridPos{1, 0}, EnemyKind::Heavy));
    entities.push_back(std::make_unique<Boss>(GridPos{2, 0}));

    // Appels virtuels : chaque classe dérivée répond avec son propre nom
    REQUIRE(entities[0]->GetName() == "Motard");
    REQUIRE(entities[1]->GetName() == "Voyou costaud");
    REQUIRE(entities[2]->GetName() == "Chef de gang");
}
