// Module ui.window (interface) : fenêtre raylib en RAII (ouverte à la construction, fermée à la destruction).
// Doit exister avant tout chargement de texture. Implémentation : Window.cpp
module;
import std;

export module ui.window;

export namespace ui {

    // Fenêtre unique : ni copiable ni déplaçable
    class Window {
    public:
        // Ouvre la fenêtre. Lève std::runtime_error en cas d'échec.
        Window(const int width, const int height, const std::string_view title, const int targetFps);

        // Ferme la fenêtre
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator=(Window&&) = delete;

        // Indique si l'utilisateur demande la fermeture (Échap ou croix)
        bool ShouldClose() const;
    };

}
