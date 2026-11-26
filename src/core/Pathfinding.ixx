// Module core.pathfinding (interface) : recherche de cases accessibles en marchant (parcours en largeur).
// Implémentation : Pathfinding.cpp
module;
import std;

export module core.pathfinding;

import core.grid;

export namespace core {

    // Renvoie les cases praticables atteignables depuis start en au plus range pas
    // (4 directions, murs contournés), sans start lui-même. Vide si range <= 0 ou start non praticable.
    std::vector<GridPos> ReachableTiles(const Grid& grid, const GridPos start, const int range);

}
