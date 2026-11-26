// Module core.grid (interface) : positions, types de tuiles et grille de la carte.
// Implémentation : Grid.cpp
module;
import std;

export module core.grid;

export namespace core {

    // Position d'une case (colonne x, ligne y)
    struct GridPos {
        int x{};
        int y{};

        bool operator==(const GridPos&) const = default;
    };

    // Nature d'une case
    enum class TileType {
        Empty,  // Hors du donjon
        Floor,  // Sol praticable
        Wall,   // Mur
        Stairs  // Sortie de l'étage (praticable)
    };

    // Grille 2D stockée dans un tableau 1D (index = y * largeur + x), accès toujours vérifiés
    class Grid {
    public:
        // Grille width x height remplie de TileType::Empty. Lève GameException si une dimension est <= 0.
        Grid(const int width, const int height);

        // Renvoie la largeur en cases
        int GetWidth() const noexcept;

        // Renvoie la hauteur en cases
        int GetHeight() const noexcept;

        // Indique si pos est dans la grille
        bool IsInBounds(const GridPos pos) const noexcept;

        // Renvoie la tuile en pos, ou std::nullopt hors de la grille
        std::optional<TileType> GetTile(const GridPos pos) const;

        // Modifie la tuile en pos. Renvoie false (sans effet) hors de la grille.
        bool SetTile(const GridPos pos, const TileType type);

        // Indique si pos est praticable (sol ou sortie)
        bool IsWalkable(const GridPos pos) const;

        // Renvoie toutes les tuiles en lecture seule (ligne par ligne)
        const std::vector<TileType>& GetTiles() const noexcept;

    private:
        // Renvoie value si elle est > 0, sinon lève GameException
        static int ValidateDimension(const int value);

        // Index 1D de pos (précondition : pos dans la grille)
        std::size_t ToIndex(const GridPos pos) const noexcept;

        int width{};
        int height{};
        std::vector<TileType> tiles{};
    };

}
