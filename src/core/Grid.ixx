// Module core.grid (interface) : modèle de données de la carte (positions, types de tuiles, grille).
// Aucune dépendance graphique : utilisé par la génération, les entités, le pathfinding et les tests.
// Implémentation : Grid.cpp
module;
// Import rapide de la bibliothèque standard
import std;

// Nommage et exportation de ce module
export module core.grid;

export namespace core {

    // Position logique d'une case sur la grille (colonne x, ligne y)
    struct GridPos {
        int x{};
        int y{};

        // Comparaison membre à membre générée par le compilateur
        bool operator==(const GridPos&) const = default;
    };

    // Nature d'une case de la carte
    enum class TileType {
        Empty,  // Hors du donjon (rien à afficher)
        Floor,  // Sol praticable
        Wall,   // Mur infranchissable
        Stairs  // Escalier vers l'étage suivant (praticable)
    };

    // Grille 2D de tuiles stockée dans un tableau 1D (index = y * largeur + x).
    // Tous les accès sont vérifiés : aucune lecture ni écriture hors des bornes n'est possible.
    class Grid {
    public:
        // Crée une grille remplie de TileType::Empty.
        // width, height : dimensions en cases, strictement positives.
        // Lève std::invalid_argument si une dimension est nulle ou négative.
        Grid(int width, int height);

        // Renvoie la largeur de la grille en cases
        int GetWidth() const noexcept;

        // Renvoie la hauteur de la grille en cases
        int GetHeight() const noexcept;

        // Indique si pos est à l'intérieur de la grille
        bool IsInBounds(GridPos pos) const noexcept;

        // Renvoie le type de la tuile en pos, ou std::nullopt si pos est hors de la grille
        std::optional<TileType> GetTile(GridPos pos) const;

        // Remplace la tuile en pos par type.
        // Renvoie true si la modification a eu lieu, false si pos est hors de la grille.
        bool SetTile(GridPos pos, TileType type);

        // Remplit toute la grille avec le type donné
        void Fill(TileType type);

        // Indique si une entité peut se tenir en pos (sol ou escalier, dans les bornes)
        bool IsWalkable(GridPos pos) const;

        // Renvoie une vue en lecture seule sur toutes les tuiles (ordre ligne par ligne)
        std::span<const TileType> GetTiles() const noexcept;

    private:
        // Vérifie qu'une dimension est strictement positive et la renvoie.
        // Lève std::invalid_argument sinon.
        static int ValidateDimension(int value);

        // Convertit une position (déjà vérifiée dans les bornes) en index du tableau 1D
        std::size_t ToIndex(GridPos pos) const noexcept;

        int width{};
        int height{};
        std::vector<TileType> tiles{};
    };

}
