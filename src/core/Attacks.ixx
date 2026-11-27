// Module core.attacks (interface) : description des attaques (portée, zone, dégâts, temps),
// zones d'effet, ligne de vue et cooldowns. Aucune dépendance graphique.
// Implémentation : Attacks.cpp
module;
import std;

export module core.attacks;

import core.grid;

export namespace core {

    // Durée en secondes (temps réel)
    using Seconds = std::chrono::duration<float>;

    // Forme de la zone d'effet autour de la case visée
    enum class AreaShape {
        Single,  // La case visée seulement
        Cross,   // Croix : la case visée et radius cases dans les 4 directions
        Square   // Carré : toutes les cases à au plus radius cases (diagonales comprises)
    };

    // Caractéristiques d'une attaque (données seulement)
    struct Attack {
        std::string_view name;   // Nom affiché (sans accents : police raylib)
        int damage;              // Dégâts infligés à chaque cible de la zone
        int range;               // Distance maximale de la case visée (0 = centrée sur l'attaquant)
        AreaShape shape;         // Forme de la zone d'effet
        int radius;              // Taille de la zone d'effet
        Seconds cooldown;        // Délai avant de pouvoir réutiliser l'attaque
        Seconds lockDuration;    // Immobilisation de l'attaquant après l'attaque
    };

    // Attaques du joueur
    inline constexpr Attack helmetStrike{"Coup de casque", 25, 1, AreaShape::Single, 0, Seconds{0.5f}, Seconds{1.0f}};
    inline constexpr Attack hornBlast{"Klaxon", 20, 0, AreaShape::Square, 2, Seconds{3.0f}, Seconds{1.5f}};
    inline constexpr Attack molotov{"Cocktail", 40, 5, AreaShape::Cross, 1, Seconds{5.0f}, Seconds{2.0f}};

    // Attaques des ennemis (toujours centrées sur eux) et du boss
    inline constexpr Attack knifeStab{"Couteau", 8, 0, AreaShape::Square, 1, Seconds{2.0f}, Seconds{0.0f}};
    inline constexpr Attack batSwing{"Batte", 25, 0, AreaShape::Square, 2, Seconds{5.0f}, Seconds{0.0f}};
    inline constexpr Attack shockwave{"Onde de choc", 40, 0, AreaShape::Square, 3, Seconds{4.0f}, Seconds{0.0f}};

    // Renvoie la distance entre a et b en cases, diagonales comprises (distance de Tchebychev)
    int ChebyshevDistance(const GridPos a, const GridPos b) noexcept;

    // Indique si une ligne droite de from à to ne traverse que des cases praticables
    // (to compris, from exclu). Les murs et le vide bloquent la vue.
    bool HasLineOfSight(const Grid& grid, const GridPos from, const GridPos to);

    // Indique si attacker peut viser target avec attack : case praticable, à portée, en ligne de vue
    bool IsInRange(const Grid& grid, const Attack& attack, const GridPos attacker, const GridPos target);

    // Renvoie les cases touchées par attack visant center : cases de la forme, praticables
    // et en ligne de vue depuis center (la zone ne traverse pas les murs)
    std::vector<GridPos> AffectedTiles(const Grid& grid, const Attack& attack, const GridPos center);

    // Compte à rebours en secondes (prêt quand il atteint 0)
    class Cooldown {
    public:
        // Fait avancer le temps de elapsed (une durée négative est ignorée, le temps restant s'arrête à 0)
        void Update(const Seconds elapsed) noexcept;

        // Indique si le compte à rebours est terminé
        bool IsReady() const noexcept;

        // Relance le compte à rebours pour duration (une durée négative le rend prêt)
        void Restart(const Seconds duration) noexcept;

        // Renvoie le temps restant (0 si prêt)
        Seconds Remaining() const noexcept;

    private:
        Seconds remaining{0.0f};
    };

}
