// Module core.entities (interface) : classe abstraite Entity, joueur, ennemis et boss.
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
        static constexpr int startSpeed{4};    // Vitesse de la moto = portée de déplacement (cases)

        // Crée le joueur sur la case start
        explicit Player(const GridPos start);

        // Renvoie "Motard"
        std::string GetName() const override;

        // Renvoie la vitesse de la moto (nombre de cases parcourables en un déplacement)
        int GetSpeed() const noexcept;

        // Se téléporte sur target si elle est atteignable en marchant en au plus GetSpeed() pas,
        // sans passer ni s'arrêter sur une case de obstacles (ennemis).
        // Renvoie true si le joueur s'est déplacé, false sinon (trop loin, mur, ennemi, case actuelle).
        bool TryMoveTo(const Grid& grid, const GridPos target, const std::vector<GridPos>& obstacles);

    private:
        int speed{startSpeed};
    };

    // Ennemi : un voyou du quartier (statistiques utilisées à partir des combats)
    class Enemy final : public Entity {
    public:
        static constexpr int startHealth{10};
        static constexpr int startAttack{3};

        // Crée un ennemi sur la case position
        explicit Enemy(const GridPos position);

        // Renvoie "Voyou"
        std::string GetName() const override;
    };

    // Boss : garde la bouche de métro, occupe un carré de size x size cases.
    // Sa position (GetPosition) est la case en haut à gauche du carré.
    class Boss final : public Entity {
    public:
        static constexpr int size{2};
        static constexpr int startHealth{50};
        static constexpr int startAttack{8};

        // Crée le boss avec topLeft comme case en haut à gauche
        explicit Boss(const GridPos topLeft);

        // Renvoie "Chef de gang"
        std::string GetName() const override;

        // Indique si pos fait partie du carré occupé par le boss
        bool Occupies(const GridPos pos) const noexcept;

        // Renvoie les size x size cases occupées
        std::vector<GridPos> OccupiedTiles() const;
    };

}
