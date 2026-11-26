// Tests unitaires du module core.entities : création, dégâts, mort, polymorphisme.
#include <catch2/catch_all.hpp>
#include <memory>

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

TEST_CASE("SetPosition deplace l'entite", "[entities]") {
    auto player = Player{GridPos{0, 0}};
    player.SetPosition(GridPos{10, 3});
    REQUIRE(player.GetPosition() == GridPos{10, 3});
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

TEST_CASE("RestoreFullHealth rend tous les PV", "[entities]") {
    auto player = Player{GridPos{0, 0}};
    player.TakeDamage(25);
    player.RestoreFullHealth();
    REQUIRE(player.GetHealth() == Player::startHealth);
}

TEST_CASE("Le joueur s'utilise via un pointeur sur Entity (polymorphisme)", "[entities]") {
    const std::unique_ptr<Entity> entity = std::make_unique<Player>(GridPos{1, 2});

    // Appel virtuel : c'est la version de Player qui est exécutée
    REQUIRE(entity->GetName() == "Motard");
    REQUIRE(entity->GetPosition() == GridPos{1, 2});
}
