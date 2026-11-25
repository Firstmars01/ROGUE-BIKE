// Module core.grid : modèle de données de la carte (positions, types de tuiles, grille).
// Aucune dépendance graphique : utilisé par la génération, les entités, le pathfinding et les tests.
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
        Grid(const int width, const int height)
            : width{ValidateDimension(width)}, height{ValidateDimension(height)} {
            // assign() plutôt qu'une initialisation {} : avec {}, std::vector
            // interpréterait les valeurs comme une liste d'éléments
            tiles.assign(static_cast<std::size_t>(this->width) * static_cast<std::size_t>(this->height),
                         TileType::Empty);
        }

        // Renvoie la largeur de la grille en cases
        int GetWidth() const noexcept { return width; }

        // Renvoie la hauteur de la grille en cases
        int GetHeight() const noexcept { return height; }

        // Indique si pos est à l'intérieur de la grille
        bool IsInBounds(const GridPos pos) const noexcept {
            return pos.x >= 0 && pos.x < width && pos.y >= 0 && pos.y < height;
        }

        // Renvoie le type de la tuile en pos, ou std::nullopt si pos est hors de la grille
        std::optional<TileType> GetTile(const GridPos pos) const {
            if (!IsInBounds(pos)) {
                return std::nullopt;
            }
            return tiles[ToIndex(pos)];
        }

        // Remplace la tuile en pos par type.
        // Renvoie true si la modification a eu lieu, false si pos est hors de la grille.
        bool SetTile(const GridPos pos, const TileType type) {
            if (!IsInBounds(pos)) {
                return false;
            }
            tiles[ToIndex(pos)] = type;
            return true;
        }

        // Remplit toute la grille avec le type donné
        void Fill(const TileType type) {
            std::ranges::fill(tiles, type);
        }

        // Indique si une entité peut se tenir en pos (sol ou escalier, dans les bornes)
        bool IsWalkable(const GridPos pos) const {
            const auto tile = GetTile(pos);
            return tile == TileType::Floor || tile == TileType::Stairs;
        }

        // Renvoie une vue en lecture seule sur toutes les tuiles (ordre ligne par ligne)
        std::span<const TileType> GetTiles() const noexcept { return tiles; }

    private:
        // Vérifie qu'une dimension est strictement positive et la renvoie.
        // Lève std::invalid_argument sinon.
        static int ValidateDimension(const int value) {
            if (value <= 0) {
                throw std::invalid_argument{"Les dimensions de la grille doivent etre strictement positives"};
            }
            return value;
        }

        // Convertit une position (déjà vérifiée dans les bornes) en index du tableau 1D
        std::size_t ToIndex(const GridPos pos) const noexcept {
            return static_cast<std::size_t>(pos.y) * static_cast<std::size_t>(width)
                   + static_cast<std::size_t>(pos.x);
        }

        int width{};
        int height{};
        std::vector<TileType> tiles{};
    };

}
