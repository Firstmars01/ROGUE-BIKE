// Module core.config : constantes de configuration du jeu (fenêtre, cadence).
// Ne dépend d'aucune bibliothèque graphique : utilisable par le jeu ET par les tests.
module;
// Import rapide de la bibliothèque standard
import std;

// Nommage et exportation de ce module
export module core.config;

export namespace core {

    // Largeur de la fenêtre en pixels
    inline constexpr int windowWidth{1280};

    // Hauteur de la fenêtre en pixels
    inline constexpr int windowHeight{720};

    // Nombre d'images par seconde visé
    inline constexpr int targetFps{60};

    // Titre de la fenêtre (littéral : garanti terminé par '\0', donc compatible raylib via data())
    inline constexpr std::string_view windowTitle{"Rogue Bike"};

}
