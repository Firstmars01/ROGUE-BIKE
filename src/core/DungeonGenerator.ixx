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
    class Room {
    public:
        // Crée une salle. x, y : coin haut-gauche ; width, height : dimensions (> 0).
        // Lève core::InvalidDimensionException si une dimension est nulle ou négative.
        Room(const int x, const int y, const int width, const int height);

        // Renvoie la colonne du coin haut-gauche
        int GetX() const noexcept;

        // Renvoie la ligne du coin haut-gauche
        int GetY() const noexcept;

        // Renvoie la largeur en cases
        int GetWidth() const noexcept;

        // Renvoie la hauteur en cases
        int GetHeight() const noexcept;

        // Renvoie la case centrale de la salle
        GridPos Center() const noexcept;

        // Indique si cette salle et other se chevauchent ou sont séparées de moins de gap cases.
        // gap : nombre minimal de cases libres exigé entre deux salles.
        bool Intersects(const Room& other, const int gap) const noexcept;

        // Indique si pos est à l'intérieur de la salle
        bool Contains(const GridPos pos) const noexcept;

    private:
        int x{};
        int y{};
        int width{};
        int height{};
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
        static constexpr int minRoomCount{7};
        static constexpr int maxRoomCount{9};
        // Taille d'une salle en cases (largeur et hauteur)
        static constexpr int minRoomSize{5};
        static constexpr int maxRoomSize{10};
        // Nombre de cases libres minimal entre deux salles
        static constexpr int roomGap{1};
        // Nombre maximal d'essais de placement (évite une boucle infinie si la carte est pleine)
        static constexpr int maxPlacementAttempts{500};
        // Dimension minimale de la carte pour contenir une salle entourée de murs
        static constexpr int minMapSize{maxRoomSize + 2};
        // Nombre maximal de passes d'amincissement des murs (garde-fou contre une boucle infinie)
        static constexpr int maxThinningPasses{10};

        // Crée un générateur. seed : graine du générateur aléatoire (même seed = même donjon)
        explicit DungeonGenerator(const std::uint32_t seed);

        // Génère un étage complet.
        // width, height : dimensions de la carte, au moins minMapSize.
        // Renvoie le donjon (grille, salles, départ du joueur, escalier).
        // Lève core::InvalidDimensionException si la carte est trop petite.
        Dungeon Generate(const int width, const int height);

    private:
        // Tire un entier uniforme dans [min, max] (bornes incluses)
        int RandomInt(const int min, const int max);

        // Place entre minRoomCount et maxRoomCount salles sans chevauchement.
        // Renvoie au moins une salle (la première trouve toujours sa place sur une carte valide).
        std::vector<Room> PlaceRooms(const int width, const int height);

        // Tire une salle aléatoire entièrement dans la carte, avec une bordure d'une case
        // réservée aux murs.
        Room RandomRoom(const int width, const int height);

        // Transforme toutes les cases de la salle en sol
        static void CarveRoom(Grid& grid, const Room& room);

        // Creuse un couloir en L entre from et to (horizontal puis vertical, ou l'inverse au hasard)
        void CarveCorridor(Grid& grid, const GridPos from, const GridPos to);

        // Creuse une ligne de sol horizontale de x1 à x2 (inclus) sur la ligne y
        static void CarveHorizontal(Grid& grid, const int x1, const int x2, const int y);

        // Creuse une ligne de sol verticale de y1 à y2 (inclus) sur la colonne x
        static void CarveVertical(Grid& grid, const int y1, const int y2, const int x);

        // Choisit la case de l'escalier : centre de la dernière salle.
        // S'il n'y a qu'une salle, coin opposé de cette salle (différent du départ du joueur).
        static GridPos ChooseStairs(const std::vector<Room>& rooms);

        // Transforme en mur chaque case vide voisine (8 directions) d'une case praticable
        static void AddWallsAroundFloors(Grid& grid);

        // Supprime les murs de 2 cases d'épaisseur : dans chaque motif sol / mur / mur / sol
        // (horizontal ou vertical), un des deux murs devient du sol, de préférence côté couloir.
        // Renvoie true si au moins un mur a été transformé.
        static bool ThinDoubleWalls(Grid& grid, const std::vector<Room>& rooms);

        // Indique si pos appartient à l'une des salles
        static bool IsInAnyRoom(const std::vector<Room>& rooms, const GridPos pos);

        // Indique si au moins une des 8 cases voisines de pos est praticable
        static bool HasWalkableNeighbour(const Grid& grid, const GridPos pos);

        std::mt19937 random;
    };

}
