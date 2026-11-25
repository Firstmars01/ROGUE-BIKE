// Module ui.window (implémentation) : appels raylib d'ouverture et de fermeture de la fenêtre.
// Interface : Window.ixx
module;
// raylib est incluse uniquement ici (fragment de module global)
#include <raylib.h>
// Import rapide de la bibliothèque standard
import std;

// Unité d'implémentation du module ui.window
module ui.window;

namespace ui {

    // Ouvre la fenêtre et fixe la cadence d'affichage
    Window::Window(const int width, const int height, const std::string_view title, const int targetFps) {
        // Copie en std::string pour garantir un titre terminé par '\0'
        const auto titleText = std::string{title};
        InitWindow(width, height, titleText.c_str());
        if (!IsWindowReady()) {
            throw std::runtime_error{"Impossible de creer la fenetre"};
        }
        SetTargetFPS(targetFps);
    }

    // Ferme la fenêtre ouverte par le constructeur
    Window::~Window() {
        CloseWindow();
    }

    // Demande de fermeture par l'utilisateur
    bool Window::ShouldClose() const {
        return WindowShouldClose();
    }

}
