// Module ui.window (implémentation).
module;
#include <raylib.h>
import std;

module ui.window;

namespace ui {

    Window::Window(const int width, const int height, const std::string_view title, const int targetFps) {
        // std::string garantit un titre terminé par '\0'
        const auto titleText = std::string{title};
        InitWindow(width, height, titleText.c_str());
        if (!IsWindowReady()) {
            throw std::runtime_error{"Impossible de creer la fenetre"};
        }
        SetTargetFPS(targetFps);
    }

    Window::~Window() {
        CloseWindow();
    }

    bool Window::ShouldClose() const {
        return WindowShouldClose();
    }

}
