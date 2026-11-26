// Module core.config : constantes du jeu (fenêtre, carte, graine).
module;
import std;

export module core.config;

export namespace core {

    inline constexpr int windowWidth{1920};                         // Largeur de la fenêtre (px)
    inline constexpr int windowHeight{1080};                        // Hauteur de la fenêtre (px)
    inline constexpr int targetFps{60};                             // Images par seconde visées
    inline constexpr std::string_view windowTitle{"Rogue Bike"};    // Titre de la fenêtre
    inline constexpr int mapWidth{60};                              // Largeur de la carte (cases)
    inline constexpr int mapHeight{45};                             // Hauteur de la carte (cases)
    inline constexpr std::uint32_t dungeonSeed{42};                 // Graine fixe de la démo

}
