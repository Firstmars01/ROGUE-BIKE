// Point d'entrée du jeu Rogue Bike.
// Jour 0 : ouvre une fenêtre raylib vide pour valider la toolchain (Clang + CMake + raylib).
#include <raylib.h>

import core.config;

// Ouvre la fenêtre, exécute la boucle d'affichage jusqu'à la fermeture, puis libère la fenêtre.
// Renvoie 0 en cas de sortie normale.
int main() {
    // Création de la fenêtre avec les paramètres du module core.config
    InitWindow(core::windowWidth, core::windowHeight, core::windowTitle.data());
    SetTargetFPS(core::targetFps);

    // Boucle principale : tourne jusqu'à Échap ou clic sur la croix
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        // Pas d'accents : la police par défaut de raylib ne les gère pas
        DrawText("Rogue Bike", 20, 20, 20, RAYWHITE);
        EndDrawing();
    }

    // Libération des ressources graphiques
    CloseWindow();
    return 0;
}
