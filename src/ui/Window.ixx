// Module ui.window (interface) : fenêtre raylib gérée en RAII.
// Ouverte par le constructeur, fermée par le destructeur. Doit exister avant tout chargement
// de texture (le contexte graphique est créé avec la fenêtre).
// Implémentation : Window.cpp
module;
// Import rapide de la bibliothèque standard
import std;

// Nommage et exportation de ce module
export module ui.window;

export namespace ui {

    // Fenêtre unique de l'application : ni copiable ni déplaçable
    class Window {
    public:
        // Ouvre la fenêtre. width, height : taille en pixels ; title : titre ;
        // targetFps : images par seconde visées.
        // Lève std::runtime_error si la fenêtre ne peut pas être créée.
        Window(const int width, const int height, const std::string_view title, const int targetFps);

        // Ferme la fenêtre et libère le contexte graphique
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator=(Window&&) = delete;

        // Indique si l'utilisateur a demandé la fermeture (Échap ou croix)
        bool ShouldClose() const;
    };

}
