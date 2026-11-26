// Module core.entities (interface) : entités du jeu (classe abstraite Entity et joueur).
// Aucune dépendance graphique. Les ennemis (Enemy) seront ajoutés au Jour 4.
// Implémentation : Entities.cpp
module;
// Import rapide de la bibliothèque standard
import std;

// Nommage et exportation de ce module
export module core.entities;

import core.grid;

export namespace core {

    // Entité présente sur la grille : position, points de vie et attaque.
    // Classe abstraite : seules les classes dérivées (Player, puis Enemy) sont instanciables.
    class Entity {
    public:
        // Destructeur virtuel : destruction correcte via un pointeur sur Entity
        virtual ~Entity() = default;

        // Renvoie la case occupée
        GridPos GetPosition() const noexcept;

        // Place l'entité sur la case position (aucune vérification de la grille ici)
        void SetPosition(const GridPos position) noexcept;

        // Renvoie les points de vie actuels (jamais négatifs)
        int GetHealth() const noexcept;

        // Renvoie les points de vie maximum
        int GetMaxHealth() const noexcept;

        // Renvoie la valeur d'attaque
        int GetAttack() const noexcept;

        // Indique si l'entité a encore des points de vie
        bool IsAlive() const noexcept;

        // Retire amount points de vie (une valeur négative est ignorée, les PV s'arrêtent à 0)
        void TakeDamage(const int amount) noexcept;

        // Rend tous ses points de vie à l'entité
        void RestoreFullHealth() noexcept;

        // Renvoie le nom affichable de l'entité (défini par chaque classe dérivée)
        virtual std::string GetName() const = 0;

    protected:
        // Crée une entité. position : case de départ ; maxHealth et attack : strictement positifs.
        // Lève core::InvalidStatException si une statistique est invalide.
        Entity(const GridPos position, const int maxHealth, const int attack);

        // Copie et déplacement réservés aux classes dérivées (évite le découpage d'objet)
        Entity(const Entity&) = default;
        Entity& operator=(const Entity&) = default;
        Entity(Entity&&) = default;
        Entity& operator=(Entity&&) = default;

    private:
        GridPos position{};
        int health{};
        int maxHealth{};
        int attack{};
    };

    // Joueur : le motard contrôlé par l'utilisateur
    class Player final : public Entity {
    public:
        // Points de vie et attaque de départ du joueur
        static constexpr int startHealth{30};
        static constexpr int startAttack{5};

        // Crée le joueur sur la case start, avec les statistiques de départ
        explicit Player(const GridPos start);

        // Renvoie "Motard"
        std::string GetName() const override;
    };

}
