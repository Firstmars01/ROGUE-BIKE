// Module core.game (interface) : règles d'une partie en temps réel (déplacements, attaques du joueur,
// activation et frappes des adversaires, morts). Aucune dépendance graphique.
// Implémentation : Game.cpp
module;
import std;

export module core.game;

import core.grid;
import core.attacks;
import core.entities;
import core.dungeonGenerator;

export namespace core {

    // État de la partie
    enum class GameState {
        Playing,   // Partie en cours
        GameOver   // Le joueur n'a plus de PV : plus rien ne bouge
    };

    // Partie : possède l'étage, le joueur, les ennemis et le boss
    class Game {
    public:
        // Distance maximale (en cases, diagonales comprises) d'un saut d'adversaire après une frappe
        static constexpr int maxHostileJump{3};

        // Génère un étage width x height avec seed et place toutes les entités
        Game(const std::uint32_t seed, const int width, const int height);

        // Place les entités sur un étage déjà construit (utile pour les tests).
        // seed : graine du hasard des sauts des adversaires
        Game(Dungeon dungeon, const std::uint32_t seed);

        // Fait avancer la partie de elapsed : minuteries du joueur, activation des adversaires
        // de la salle du joueur, frappes des adversaires prêts, retrait des morts, fin de partie
        void Update(const Seconds elapsed);

        // Déplace le joueur sur target (portée, murs, obstacles ; délai entre deux déplacements en combat).
        // Renvoie true si le joueur s'est déplacé.
        bool TryMove(const GridPos target);

        // Lance l'attaque attackIndex du joueur sur target (pour une attaque de portée 0, target est la case
        // du joueur). Refusée si la cible n'est pas à portée ou en vue, ou si le cooldown n'est pas fini.
        // Les adversaires touchés perdent des PV et s'activent. Renvoie true si l'attaque est lancée.
        bool TryAttack(const std::size_t attackIndex, const GridPos target);

        // Renvoie l'étage
        const Dungeon& GetDungeon() const noexcept;

        // Renvoie le joueur
        const Player& GetPlayer() const noexcept;

        // Renvoie les ennemis encore en vie
        const std::vector<std::unique_ptr<Enemy>>& GetEnemies() const noexcept;

        // Renvoie le boss, ou std::nullopt s'il est mort
        const std::optional<Boss>& GetBoss() const noexcept;

        // Renvoie l'état de la partie
        GameState GetState() const noexcept;

        // Indique si le joueur est dans une salle où il reste des adversaires
        bool IsInCombat() const;

        // Renvoie les cases où le joueur peut se déplacer
        std::vector<GridPos> PlayerReachableTiles() const;

        // Renvoie les cases que hostile frappe (zone autour de chacune de ses cases, murs respectés)
        std::vector<GridPos> StrikeArea(const Hostile& hostile) const;

        // Renvoie les cases que le joueur peut viser avec l'attaque attackIndex (vide si index invalide)
        std::vector<GridPos> AttackTargets(const std::size_t attackIndex) const;

        // Renvoie les cases touchées par l'attaque attackIndex visant target,
        // ou une liste vide si la cible n'est pas valide (même règle que TryAttack)
        std::vector<GridPos> AttackArea(const std::size_t attackIndex, const GridPos target) const;

        // Renvoie les cases que des adversaires actifs frapperont dans moins de warning
        std::vector<GridPos> ImminentStrikeTiles(const Seconds warning) const;

    private:
        // Renvoie l'index de la salle qui contient pos, ou std::nullopt (couloir)
        std::optional<std::size_t> RoomOf(const GridPos pos) const;

        // Renvoie les cases occupées par les adversaires
        std::vector<GridPos> ObstacleTiles() const;

        // Active les adversaires de la salle où se trouve le joueur
        void ActivateHostilesInPlayerRoom();

        // Fait avancer le compteur de hostile ; s'il est prêt, frappe sa zone, le relance et le fait sauter
        void UpdateHostile(Hostile& hostile, const Seconds elapsed);

        // Fait sauter hostile sur une position libre de sa salle, à au plus maxHostileJump cases,
        // la plus proche de sa distance préférée au joueur (au hasard parmi les meilleures).
        // Reste sur place si aucune position ne convient.
        void Jump(Hostile& hostile);

        // Renvoie les cases occupées par le joueur et par les adversaires autres que self
        std::vector<GridPos> TilesTakenByOthers(const Hostile& self) const;

        // Retire les ennemis morts et le boss mort
        void RemoveDead();

        Dungeon dungeon;
        Player player;
        std::vector<std::unique_ptr<Enemy>> enemies;   // Seul propriétaire des ennemis
        std::optional<Boss> boss;
        GameState state{GameState::Playing};
        std::mt19937 random;
    };

}
