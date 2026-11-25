// Module core.grid (implémentation) : code des fonctions de la classe Grid.
// Interface : Grid.ixx
module;
// assert() pour la programmation par contrat (désactivé en Release par NDEBUG)
#include <cassert>
// Import rapide de la bibliothèque standard
import std;

// Unité d'implémentation du module core.grid
module core.grid;

import core.exceptions;

namespace core {

    // Crée une grille width x height remplie de TileType::Empty (dimensions vérifiées)
    Grid::Grid(const int width, const int height)
        : width{ValidateDimension(width)}, height{ValidateDimension(height)} {
        // assign() plutôt qu'une initialisation {} : avec {}, std::vector
        // interpréterait les valeurs comme une liste d'éléments
        tiles.assign(static_cast<std::size_t>(this->width) * static_cast<std::size_t>(this->height),
                     TileType::Empty);
    }

    // Renvoie la largeur en cases
    int Grid::GetWidth() const noexcept {
        return width;
    }

    // Renvoie la hauteur en cases
    int Grid::GetHeight() const noexcept {
        return height;
    }

    // Vrai si pos est dans la grille
    bool Grid::IsInBounds(const GridPos pos) const noexcept {
        return pos.x >= 0 && pos.x < width && pos.y >= 0 && pos.y < height;
    }

    // Lecture sécurisée : std::nullopt hors des bornes
    std::optional<TileType> Grid::GetTile(const GridPos pos) const {
        if (!IsInBounds(pos)) {
            return std::nullopt;
        }
        return tiles[ToIndex(pos)];
    }

    // Écriture sécurisée : false (et aucun effet) hors des bornes
    bool Grid::SetTile(const GridPos pos, const TileType type) {
        if (!IsInBounds(pos)) {
            return false;
        }
        tiles[ToIndex(pos)] = type;
        return true;
    }

    // Remplit toutes les cases avec type
    void Grid::Fill(const TileType type) {
        std::ranges::fill(tiles, type);
    }

    // Vrai si pos est du sol ou un escalier (une case hors bornes n'est jamais praticable)
    bool Grid::IsWalkable(const GridPos pos) const {
        const auto tile = GetTile(pos);
        return tile == TileType::Floor || tile == TileType::Stairs;
    }

    // Vue en lecture seule sur le tableau 1D des tuiles
    std::span<const TileType> Grid::GetTiles() const noexcept {
        return tiles;
    }

    // Lève InvalidDimensionException si value <= 0, sinon renvoie value
    int Grid::ValidateDimension(const int value) {
        if (value <= 0) {
            throw InvalidDimensionException{"Les dimensions de la grille doivent etre strictement positives"};
        }
        return value;
    }

    // Index 1D d'une position supposée valide : y * largeur + x
    std::size_t Grid::ToIndex(const GridPos pos) const noexcept {
        // Précondition : les appelants vérifient les bornes avant de convertir
        assert(IsInBounds(pos));
        return static_cast<std::size_t>(pos.y) * static_cast<std::size_t>(width)
               + static_cast<std::size_t>(pos.x);
    }

}
