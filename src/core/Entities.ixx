// Module core.entities (interface) : entités du jeu.
// Entity (abstraite) -> Player ; Entity -> Hostile (abstraite, attaque à compteur) -> Enemy, Boss.
// Implémentation : Entities.cpp
module;
import std;

export module core.entities;

import core.grid;
import core.attacks;

export namespace core {

    // Entité de la grille : position et points de vie (classe abstraite)
    class Entity {
    public:
        virtual ~Entity() = default;

        // Renvoie la case occupée
        GridPos GetPosition() const noexcept;

        // Place l'entité sur position (sans vérifier la grille)
        void SetPosition(const GridPos position) noexcept;

        // Renvoie les points de vie actuels
        int GetHealth() const noexcept;

        // Renvoie les points de vie maximum (ceux du départ)
        int GetMaxHealth() const noexcept;

        // Indique s'il reste des points de vie
        bool IsAlive() const noexcept;

        // Retire amount PV (valeur négative ignorée, PV bornés à 0)
        void TakeDamage(const int amount) noexcept;

        // Nom affichable, défini par chaque classe dérivée
        virtual std::string GetName() const = 0;

    protected:
        // health doit être > 0, sinon lève GameException
        Entity(const GridPos position, const int health);

        // Copie et déplacement réservés aux classes dérivées (évite le découpage d'objet)
        Entity(const Entity&) = default;
        Entity& operator=(const Entity&) = default;
        Entity(Entity&&) = default;
        Entity& operator=(Entity&&) = default;

    private:
        GridPos position{};
        int health{};
        int maxHealth{};
    };

    // Joueur : le motard, avec 3 attaques à cooldown, une immobilisation après chaque attaque
    // et un délai entre deux déplacements dans les salles occupées
    class Player final : public Entity {
    public:
        static constexpr int startHealth{100};
        static constexpr int startSpeed{4};                       // Portée de déplacement (cases)
        static constexpr Seconds moveDelay{0.6f};                 // Délai entre deux déplacements en combat
        static constexpr std::array<Attack, 3> attacks{helmetStrike, hornBlast, molotov};

        // Crée le joueur sur la case start
        explicit Player(const GridPos start);

        // Renvoie "Motard"
        std::string GetName() const override;

        // Renvoie la vitesse de la moto (nombre de cases parcourables en un déplacement)
        int GetSpeed() const noexcept;

        // Fait avancer les cooldowns, l'immobilisation et le délai de déplacement de elapsed
        void Update(const Seconds elapsed) noexcept;

        // Indique si le joueur est immobilisé (juste après une attaque)
        bool IsLocked() const noexcept;

        // Indique si l'attaque index existe et si son cooldown est terminé
        bool CanUseAttack(const std::size_t index) const noexcept;

        // Lance l'attaque index si possible : relance son cooldown et immobilise le joueur.
        // Renvoie true si l'attaque est lancée (l'application des dégâts est faite par le jeu).
        bool TryUseAttack(const std::size_t index) noexcept;

        // Renvoie le temps restant avant de pouvoir réutiliser l'attaque index (0 si index invalide)
        Seconds AttackCooldownRemaining(const std::size_t index) const noexcept;

        // Se téléporte sur target si elle est atteignable en marchant en au plus GetSpeed() pas,
        // sans passer ni s'arrêter sur une case de obstacles. Refusé si le joueur est immobilisé,
        // ou si inCombat et que le délai de déplacement n'est pas écoulé (il est alors relancé).
        // Renvoie true si le joueur s'est déplacé.
        bool TryMoveTo(const Grid& grid, const GridPos target, const std::vector<GridPos>& obstacles,
                       const bool inCombat);

    private:
        int speed{startSpeed};
        std::array<Cooldown, 3> attackCooldowns{};
        Cooldown lock{};
        Cooldown moveCooldown{};
    };

    // Adversaire immobile qui frappe sa zone à intervalle régulier une fois activé (classe abstraite)
    class Hostile : public Entity {
    public:
        // Renvoie l'attaque utilisée
        const Attack& GetAttackType() const noexcept;

        // Indique si le compteur tourne (le joueur est entré dans sa salle)
        bool IsActive() const noexcept;

        // Active l'adversaire et lance son compteur (sans effet s'il est déjà actif)
        void Activate() noexcept;

        // Fait avancer le compteur de elapsed s'il est actif
        void Update(const Seconds elapsed) noexcept;

        // Indique si l'adversaire est actif et que son compteur est arrivé à 0
        bool IsReadyToStrike() const noexcept;

        // Relance le compteur après une frappe
        void Rearm() noexcept;

        // Renvoie le temps restant avant la prochaine frappe
        Seconds TimeBeforeStrike() const noexcept;

    protected:
        // attack : attaque utilisée à chaque fin de compteur
        Hostile(const GridPos position, const int health, const Attack& attack);

    private:
        Attack attack;
        Cooldown countdown{};
        bool active{false};
    };

    // Type d'ennemi, tiré au hasard à la génération
    enum class EnemyKind {
        Fast,   // Couteau : petite zone, peu de dégâts, cooldown court
        Heavy   // Batte : grande zone, beaucoup de dégâts, cooldown long
    };

    // Ennemi : un voyou du quartier
    class Enemy final : public Hostile {
    public:
        static constexpr int startHealth{75};

        // Crée un ennemi de type kind sur la case position
        Enemy(const GridPos position, const EnemyKind kind);

        // Renvoie le type de l'ennemi
        EnemyKind GetKind() const noexcept;

        // Renvoie "Voyou rapide" ou "Voyou costaud"
        std::string GetName() const override;

    private:
        EnemyKind kind;
    };

    // Boss : garde la bouche de métro, occupe un carré de size x size cases.
    // Sa position (GetPosition) est la case en haut à gauche du carré.
    class Boss final : public Hostile {
    public:
        static constexpr int size{2};
        static constexpr int startHealth{1000};

        // Crée le boss avec topLeft comme case en haut à gauche
        explicit Boss(const GridPos topLeft);

        // Renvoie "Chef de gang"
        std::string GetName() const override;

        // Renvoie les size x size cases occupées
        std::vector<GridPos> OccupiedTiles() const;
    };

}
