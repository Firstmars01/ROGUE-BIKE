// Module ui.window (interface) : fenêtre raylib en RAII avec un canevas virtuel de taille fixe.
// Le jeu dessine sur le canevas, affiché mis à l'échelle et centré dans la fenêtre réelle :
// le rendu et la souris restent cohérents quelle que soit la taille de l'écran.
// Doit exister avant tout chargement de texture. Implémentation : Window.cpp
module;
import std;

export module ui.window;

import ui.isoMath;

export namespace ui {

    // Fenêtre unique : ni copiable ni déplaçable
    class Window {
    public:
        // Ouvre la fenêtre (plein écran sans bordure, redimensionnable) avec un canevas
        // width x height. Lève std::runtime_error en cas d'échec.
        Window(const int width, const int height, const std::string_view title, const int targetFps);

        // Libère le canevas puis ferme la fenêtre
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator=(Window&&) = delete;

        // Indique si l'utilisateur demande la fermeture (Échap ou croix)
        bool ShouldClose() const;

        // Commence une image : les dessins suivants vont sur le canevas
        void BeginFrame() const;

        // Termine l'image : affiche le canevas mis à l'échelle et centré dans la fenêtre
        void EndFrame() const;

        // Renvoie la position de la souris en coordonnées du canevas
        ScreenPos MousePosition() const;

    private:
        // Canevas et dimensions virtuelles (définis dans Window.cpp)
        struct Canvas;

        std::unique_ptr<Canvas> canvas;
    };

}
