// Module ui.hud (interface) : interface fixe à l'écran, barre des attaques du joueur en bas du canevas
// (nom, touche, cooldown, attaque choisie). raylib reste cachée dans Hud.cpp.
// Implémentation : Hud.cpp
module;
import std;

export module ui.hud;

import core.entities;
import ui.isoMath;

export namespace ui {

    // Dessine les boutons des attaques du joueur ; selected : attaque choisie (mise en évidence)
    void DrawAttackBar(const core::Player& player, const std::optional<std::size_t> selected);

    // Renvoie l'index du bouton d'attaque sous le point screen (coordonnées du canevas), ou std::nullopt
    std::optional<std::size_t> AttackButtonAt(const ScreenPos screen);

}
