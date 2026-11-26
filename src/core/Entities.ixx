// Module core.entities (interface) : classe abstraite Entity et joueur.
// Implémentation : Entities.cpp
module;
import std;

export module core.entities;

import core.grid;

export namespace core {

    // Entité de la grille : position, points de vie, attaque (classe abstraite)
    class Entity {
    public:
        virtual ~Entity() = default;

        // Renvoie la case occupée
        GridPos GetPosition() const noexcept;

        // Place l'entité sur position (sans vérifier la grille)
        void SetPosition(const GridPos position) noexcept;

        // Renvoie les points de vie actuels
        int GetHealth() const noexcept;

        // Renvoie la valeur d'attaque
        int GetAttack() const noexcept;

        // Indique s'il reste des points de vie
        bool IsAlive() const noexcept;

        // Retire amount PV (valeur négative ignorée, PV bornés à 0)
        void TakeDamage(const int amount) noexcept;

        // Nom affichable, défini par chaque classe dérivée
        virtual std::string GetName() const = 0;

    protected:
        // health et attack doivent être > 0, sinon lève GameException
        Entity(const GridPos position, const int health, const int attack);

        // Copie et déplacement réservés aux classes dérivées (évite le découpage d'objet)
        Entity(const Entity&) = default;
        Entity& operator=(const Entity&) = default;
        Entity(Entity&&) = default;
        Entity& operator=(Entity&&) = default;

    private:
        GridPos position{};
        int health{};
        int attack{};
    };

    // Joueur : le motard
    class Player final : public Entity {
    public:
        static constexpr int startHealth{30};
        static constexpr int startAttack{5};

        // Crée le joueur sur la case start
        explicit Player(const GridPos start);

        // Renvoie "Motard"
        std::string GetName() const override;
    };

}
