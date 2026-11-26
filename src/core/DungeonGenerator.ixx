// Module core.dungeonGenerator (interface) : génération procédurale d'un étage
// (salles reliées par des couloirs en L, départ et sortie). Même graine = même étage.
// Implémentation : DungeonGenerator.cpp
module;
import std;

export module core.dungeonGenerator;

import core.grid;

export namespace core {

    // Salle rectangulaire : coin haut-gauche (x, y) et dimensions en cases
    class Room {
    public:
        // width et height doivent être > 0, sinon lève GameException
        Room(const int x, const int y, const int width, const int height);

        // Renvoie la colonne du coin haut-gauche
        int GetX() const noexcept;

        // Renvoie la ligne du coin haut-gauche
        int GetY() const noexcept;

        // Renvoie la largeur en cases
        int GetWidth() const noexcept;

        // Renvoie la hauteur en cases
        int GetHeight() const noexcept;

        // Renvoie la case centrale
        GridPos Center() const noexcept;

        // Indique si les deux salles se chevauchent ou sont à moins de gap cases
        bool Intersects(const Room& other, const int gap) const noexcept;

        // Indique si pos est dans la salle
        bool Contains(const GridPos pos) const noexcept;

    private:
        int x{};
        int y{};
        int width{};
        int height{};
    };

    // Étage généré
    struct Dungeon {
        Grid grid;                // Carte
        std::vector<Room> rooms;  // Salles, dans l'ordre où elles sont reliées
        GridPos playerStart;      // Départ (centre de la première salle)
        GridPos stairs;           // Sortie (centre de la dernière salle)
    };

    // Générateur procédural basé sur std::mt19937
    class DungeonGenerator {
    public:
        static constexpr int minRoomCount{7};              // Nombre de salles visé (min)
        static constexpr int maxRoomCount{9};              // Nombre de salles visé (max)
        static constexpr int minRoomSize{5};               // Côté minimal d'une salle
        static constexpr int maxRoomSize{10};              // Côté maximal d'une salle
        static constexpr int roomGap{1};                   // Écart minimal entre deux salles
        static constexpr int maxPlacementAttempts{500};    // Garde-fou du placement des salles
        static constexpr int maxThinningPasses{10};        // Garde-fou de l'amincissement des murs
        static constexpr int minMapSize{maxRoomSize + 2};  // Carte minimale (une salle + ses murs)

        // seed : graine du générateur aléatoire
        explicit DungeonGenerator(const std::uint32_t seed);

        // Génère un étage width x height. Lève GameException si la carte est trop petite
        // ou si moins de deux salles ont pu être placées.
        Dungeon Generate(const int width, const int height);

    private:
        // Renvoie un entier aléatoire dans [min, max]
        int RandomInt(const int min, const int max);

        // Place entre minRoomCount et maxRoomCount salles sans chevauchement
        std::vector<Room> PlaceRooms(const int width, const int height);

        // Renvoie une salle aléatoire dans la carte, en laissant une bordure pour les murs
        Room RandomRoom(const int width, const int height);

        // Transforme la salle en sol
        static void CarveRoom(Grid& grid, const Room& room);

        // Creuse un couloir en L entre from et to (ordre des segments aléatoire)
        void CarveCorridor(Grid& grid, const GridPos from, const GridPos to);

        // Creuse une ligne de sol horizontale de x1 à x2 sur la ligne y
        static void CarveHorizontal(Grid& grid, const int x1, const int x2, const int y);

        // Creuse une ligne de sol verticale de y1 à y2 sur la colonne x
        static void CarveVertical(Grid& grid, const int y1, const int y2, const int x);

        // Entoure de murs toutes les cases praticables
        static void AddWallsAroundFloors(Grid& grid);

        // Remplace un mur des motifs sol / mur / mur / sol par du sol (côté couloir).
        // Renvoie true si au moins un mur a été remplacé.
        static bool ThinDoubleWalls(Grid& grid, const std::vector<Room>& rooms);

        // Indique si pos appartient à une salle
        static bool IsInAnyRoom(const std::vector<Room>& rooms, const GridPos pos);

        // Indique si une des 8 cases voisines de pos est praticable
        static bool HasWalkableNeighbour(const Grid& grid, const GridPos pos);

        std::mt19937 random;
    };

}
