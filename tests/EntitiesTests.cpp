// Tests unitaires du module core.entities : création, dégâts, mort, polymorphisme.
#include <catch2/catch_all.hpp>
#include <memory>
#include <vector>

import core.grid;
import core.entities;

using namespace core;

TEST_CASE("Le joueur demarre sur sa case avec ses statistiques de depart", "[entities]") {
    const auto player = Player{GridPos{4, 7}};

    REQUIRE(player.GetPosition() == GridPos{4, 7});
    REQUIRE(player.GetHealth() == Player::startHealth);
    REQUIRE(player.GetMaxHealth() == Player::startHealth);
    REQUIRE(player.GetAttack() == Player::startAttack);
    REQUIRE(player.IsAlive());
    REQUIRE(player.GetName() == "Motard");
}

TEST_CASE("Points de vie de depart : joueur 100, ennemi 75, boss 1000", "[entities]") {
    REQUIRE(Player{GridPos{0, 0}}.GetHealth() == 100);
    REQUIRE(Enemy{GridPos{0, 0}}.GetHealth() == 75);
    REQUIRE(Boss{GridPos{0, 0}}.GetHealth() == 1000);
}

TEST_CASE("Les PV maximum ne changent pas avec les degats", "[entities]") {
    auto enemy = Enemy{GridPos{0, 0}};
    enemy.TakeDamage(30);
    REQUIRE(enemy.GetHealth() == 45);
    REQUIRE(enemy.GetMaxHealth() == 75);
}

TEST_CASE("SetPosition deplace l'entite", "[entities]") {
    auto player = Player{GridPos{0, 0}};
    player.SetPosition(GridPos{10, 3});
    REQUIRE(player.GetPosition() == GridPos{10, 3});
}

// Couloir horizontal de test : 7 cases de sol, un mur en x = 3 sur une seconde ligne de sol
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

TEST_CASE("Le joueur demarre avec la vitesse de sa moto", "[entities]") {
    const auto player = Player{GridPos{0, 0}};
    REQUIRE(player.GetSpeed() == Player::startSpeed);
}

TEST_CASE("TryMoveTo teleporte le joueur sur une case de sa zone", "[entities]") {
    const auto grid = MakeTestArea();
    auto player = Player{GridPos{0, 0}};

    // 4 pas exactement (vitesse de départ)
    REQUIRE(player.TryMoveTo(grid, GridPos{4, 0}, {}));
    REQUIRE(player.GetPosition() == GridPos{4, 0});
}

TEST_CASE("TryMoveTo refuse une case trop loin, un mur ou la case actuelle", "[entities]") {
    const auto grid = MakeTestArea();
    auto player = Player{GridPos{0, 0}};

    REQUIRE_FALSE(player.TryMoveTo(grid, GridPos{5, 0}, {}));   // 5 pas : trop loin
    REQUIRE_FALSE(player.TryMoveTo(grid, GridPos{3, 1}, {}));   // mur
    REQUIRE_FALSE(player.TryMoveTo(grid, GridPos{0, 0}, {}));   // case actuelle
    REQUIRE_FALSE(player.TryMoveTo(grid, GridPos{-1, 0}, {}));  // hors de la carte
    REQUIRE(player.GetPosition() == GridPos{0, 0});
}

TEST_CASE("TryMoveTo refuse la case d'un ennemi et ne le traverse pas", "[entities]") {
    const auto grid = MakeTestArea();
    auto player = Player{GridPos{0, 0}};
    const auto enemies = std::vector<GridPos>{GridPos{2, 0}};

    REQUIRE_FALSE(player.TryMoveTo(grid, GridPos{2, 0}, enemies));  // case de l'ennemi
    REQUIRE(player.TryMoveTo(grid, GridPos{1, 0}, enemies));        // case libre avant lui
    REQUIRE(player.GetPosition() == GridPos{1, 0});
}

TEST_CASE("TakeDamage retire des PV sans descendre sous zero", "[entities]") {
    auto player = Player{GridPos{0, 0}};

    player.TakeDamage(12);
    REQUIRE(player.GetHealth() == Player::startHealth - 12);
    REQUIRE(player.IsAlive());

    // Dégâts supérieurs aux PV restants : PV à 0, entité morte
    player.TakeDamage(1000);
    REQUIRE(player.GetHealth() == 0);
    REQUIRE_FALSE(player.IsAlive());
}

TEST_CASE("Des degats negatifs sont ignores", "[entities]") {
    auto player = Player{GridPos{0, 0}};
    player.TakeDamage(5);
    player.TakeDamage(-20);
    REQUIRE(player.GetHealth() == Player::startHealth - 5);
}

TEST_CASE("Un ennemi demarre sur sa case avec ses statistiques", "[entities]") {
    const auto enemy = Enemy{GridPos{3, 9}};

    REQUIRE(enemy.GetPosition() == GridPos{3, 9});
    REQUIRE(enemy.GetHealth() == Enemy::startHealth);
    REQUIRE(enemy.GetAttack() == Enemy::startAttack);
    REQUIRE(enemy.GetName() == "Voyou");
}

TEST_CASE("Le boss occupe un carre de 2x2 cases", "[entities]") {
    const auto boss = Boss{GridPos{4, 6}};

    REQUIRE(boss.GetName() == "Chef de gang");
    REQUIRE(boss.GetHealth() == Boss::startHealth);
    REQUIRE(boss.GetAttack() == Boss::startAttack);
    REQUIRE(boss.OccupiedTiles() == std::vector<GridPos>{GridPos{4, 6}, GridPos{5, 6}, GridPos{4, 7}, GridPos{5, 7}});
}

TEST_CASE("Joueur et ennemis se manipulent ensemble via Entity (polymorphisme)", "[entities]") {
    auto entities = std::vector<std::unique_ptr<Entity>>{};
    entities.push_back(std::make_unique<Player>(GridPos{0, 0}));
    entities.push_back(std::make_unique<Enemy>(GridPos{1, 0}));
    entities.push_back(std::make_unique<Boss>(GridPos{2, 0}));

    // Appels virtuels : chaque classe dérivée répond avec son propre nom
    REQUIRE(entities[0]->GetName() == "Motard");
    REQUIRE(entities[1]->GetName() == "Voyou");
    REQUIRE(entities[2]->GetName() == "Chef de gang");
}

TEST_CASE("Le joueur s'utilise via un pointeur sur Entity (polymorphisme)", "[entities]") {
    const std::unique_ptr<Entity> entity = std::make_unique<Player>(GridPos{1, 2});

    // Appel virtuel : c'est la version de Player qui est exécutée
    REQUIRE(entity->GetName() == "Motard");
    REQUIRE(entity->GetPosition() == GridPos{1, 2});
}
