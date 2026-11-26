// Module core.grid (implémentation).
module;
#include <cassert>
import std;

module core.grid;

import core.exceptions;

namespace core {

    Grid::Grid(const int width, const int height)
        : width{ValidateDimension(width)}, height{ValidateDimension(height)} {
        // assign() et non {} : avec {}, le vector prendrait les valeurs comme éléments
        tiles.assign(static_cast<std::size_t>(this->width) * static_cast<std::size_t>(this->height),
                     TileType::Empty);
    }

    int Grid::GetWidth() const noexcept {
        return width;
    }

    int Grid::GetHeight() const noexcept {
        return height;
    }

    bool Grid::IsInBounds(const GridPos pos) const noexcept {
        return pos.x >= 0 && pos.x < width && pos.y >= 0 && pos.y < height;
    }

    std::optional<TileType> Grid::GetTile(const GridPos pos) const {
        if (!IsInBounds(pos)) {
            return std::nullopt;
        }
        return tiles[ToIndex(pos)];
    }

    bool Grid::SetTile(const GridPos pos, const TileType type) {
        if (!IsInBounds(pos)) {
            return false;
        }
        tiles[ToIndex(pos)] = type;
        return true;
    }

    bool Grid::IsWalkable(const GridPos pos) const {
        const auto tile = GetTile(pos);
        return tile == TileType::Floor || tile == TileType::Stairs;
    }

    const std::vector<TileType>& Grid::GetTiles() const noexcept {
        return tiles;
    }

    int Grid::ValidateDimension(const int value) {
        if (value <= 0) {
            throw GameException{"Les dimensions de la grille doivent etre strictement positives"};
        }
        return value;
    }

    std::size_t Grid::ToIndex(const GridPos pos) const noexcept {
        // Contrat : les appelants vérifient les bornes avant (actif en Debug)
        assert(IsInBounds(pos));
        return static_cast<std::size_t>(pos.y) * static_cast<std::size_t>(width)
               + static_cast<std::size_t>(pos.x);
    }

}
