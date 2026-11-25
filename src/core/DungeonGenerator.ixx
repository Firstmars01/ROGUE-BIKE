// Module core.dungeonGenerator (interface) : génération procédurale d'un étage de donjon
// (salles rectangulaires reliées par des couloirs en L, départ du joueur et escalier de sortie).
// Aucune dépendance graphique. Le résultat est reproductible : même seed = même donjon.
// Implémentation : DungeonGenerator.cpp
module;
// Import rapide de la bibliothèque standard
import std;

// Nommage et exportation de ce module
export module core.dungeonGenerator;

import core.grid;

export namespace core {

    // Salle rectangulaire : coin haut-gauche (x, y) et dimensions en cases
    struct Room {
        int x{};
        int y{};
        int width{};
        int height{};

        // Renvoie la case centrale de la salle
        GridPos Center() const noexcept;

        // Indique si cette salle et other se chevauchent ou sont séparées de moins de gap cases.
        // gap : nombre minimal de cases libres exigé entre deux salles.
        bool Intersects(const Room& other, int gap) const noexcept;

        // Indique si pos est à l'intérieur de la salle
        bool Contains(GridPos pos) const noexcept;
    };

    // Résultat de la génération d'un étage
    struct Dungeon {
        Grid grid;                // Carte de l'étage
        std::vector<Room> rooms;  // Salles, dans l'ordre où elles sont reliées
        GridPos playerStart;      // Case de départ du joueur (centre de la première salle)
        GridPos stairs;           // Case de l'escalier de sortie
    };

    // Générateur procédural de donjons basé sur std::mt19937
    class DungeonGenerator {
    public:
        // Nombre de salles visé (tiré au hasard entre ces deux bornes)
        static constexpr int minRoomCount{4};
        static constexpr int maxRoomCount{5};
        // Taille d'une salle en cases (largeur et hauteur)
        static constexpr int minRoomSize{4};
        static constexpr int maxRoomSize{8};
        // Nombre de cases libres minimal entre deux salles
        static constexpr int roomGap{1};
        // Nombre maximal d'essais de placement (évite une boucle infinie si la carte est pleine)
        static constexpr int maxPlacementAttempts{200};
        // Dimension minimale de la carte pour contenir une salle entourée de murs
        static constexpr int minMapSize{maxRoomSize + 2};

        // Crée un générateur. seed : graine du générateur aléatoire (même seed = même donjon)
        explicit DungeonGenerator(std::uint32_t seed);

        // Génère un étage complet.
        // width, height : dimensions de la carte, au moins minMapSize.
        // Renvoie le donjon (grille, salles, départ du joueur, escalier).
        // Lève std::invalid_argument si la carte est trop petite.
        Dungeon Generate(int width, int height);

    private:
        // Tire un entier uniforme dans [min, max] (bornes incluses)
        int RandomInt(int min, int max);

        // Place entre minRoomCount et maxRoomCount salles sans chevauchement.
        // Renvoie au moins une salle (la première trouve toujours sa place sur une carte valide).
        std::vector<Room> PlaceRooms(int width, int height);

        // Tire une salle aléatoire entièrement dans la carte, avec une bordure d'une case
        // réservée aux murs.
        Room RandomRoom(int width, int height);

        // Transforme toutes les cases de la salle en sol
        static void CarveRoom(Grid& grid, const Room& room);

        // Creuse un couloir en L entre from et to (horizontal puis vertical, ou l'inverse au hasard)
        void CarveCorridor(Grid& grid, GridPos from, GridPos to);

        // Creuse une ligne de sol horizontale de x1 à x2 (inclus) sur la ligne y
        static void CarveHorizontal(Grid& grid, int x1, int x2, int y);

        // Creuse une ligne de sol verticale de y1 à y2 (inclus) sur la colonne x
        static void CarveVertical(Grid& grid, int y1, int y2, int x);

        // Choisit la case de l'escalier : centre de la dernière salle.
        // S'il n'y a qu'une salle, coin opposé de cette salle (différent du départ du joueur).
        static GridPos ChooseStairs(const std::vector<Room>& rooms);

        // Transforme en mur chaque case vide voisine (8 directions) d'une case praticable
        static void AddWallsAroundFloors(Grid& grid);

        // Indique si au moins une des 8 cases voisines de pos est praticable
        static bool HasWalkableNeighbour(const Grid& grid, GridPos pos);

        std::mt19937 random;
    };

}
