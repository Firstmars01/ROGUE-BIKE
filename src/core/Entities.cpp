// Module core.entities (implémentation) : code des classes Entity et Player.
// Interface : Entities.ixx
module;
// Import rapide de la bibliothèque standard
import std;

// Unité d'implémentation du module core.entities
module core.entities;

import core.exceptions;

namespace core {

    // ----- Entity -----

    // Vérifie les statistiques puis démarre avec tous ses points de vie
    Entity::Entity(const GridPos position, const int maxHealth, const int attack)
        : position{position}, health{maxHealth}, maxHealth{maxHealth}, attack{attack} {
        if (maxHealth <= 0 || attack <= 0) {
            throw InvalidStatException{"Les PV et l'attaque d'une entite doivent etre strictement positifs"};
        }
    }

    // Case occupée
    GridPos Entity::GetPosition() const noexcept {
        return position;
    }

    // Nouvelle case
    void Entity::SetPosition(const GridPos position) noexcept {
        this->position = position;
    }

    // PV actuels
    int Entity::GetHealth() const noexcept {
        return health;
    }

    // PV maximum
    int Entity::GetMaxHealth() const noexcept {
        return maxHealth;
    }

    // Attaque
    int Entity::GetAttack() const noexcept {
        return attack;
    }

    // Vivant tant qu'il reste des PV
    bool Entity::IsAlive() const noexcept {
        return health > 0;
    }

    // Dégâts bornés : jamais de soin par dégâts négatifs, jamais de PV négatifs
    void Entity::TakeDamage(const int amount) noexcept {
        health = std::max(0, health - std::max(0, amount));
    }

    // PV remis au maximum
    void Entity::RestoreFullHealth() noexcept {
        health = maxHealth;
    }

    // ----- Player -----

    // Joueur avec les statistiques de départ
    Player::Player(const GridPos start) : Entity{start, startHealth, startAttack} {}

    // Nom du joueur
    std::string Player::GetName() const {
        return "Motard";
    }

}
