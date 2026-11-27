// Module ui.application (interface) : boucle de jeu, entrées et affichage d'une partie core::Game.
// Implémentation : Application.cpp
module;
import std;

export module ui.application;

import core.grid;
import core.game;
import ui.window;
import ui.renderer;

export namespace ui {

    // Application : fenêtre, partie et renderer (libérés automatiquement, non copiable)
    class Application {
    public:
        // Ouvre la fenêtre, crée la partie de démo et charge les sprites.
        // Lève std::runtime_error si la fenêtre ou un sprite ne peut pas être chargé.
        Application();

        // Boucle principale jusqu'à la fermeture de la fenêtre
        void Run();

    private:
        // Clavier et souris : zoom (molette), choix d'une attaque (1/2/3 ou boutons), visée et attaque
        // (clic gauche), annulation (clic droit), déplacement (clic gauche sans attaque choisie), nouvelle partie (R)
        void HandleInput();

        // Choisit l'attaque index, ou la désélectionne si elle l'était déjà
        void ToggleAttack(const std::size_t index);

        // Renvoie la case visée par l'attaque choisie : la case du joueur pour une attaque de portée 0,
        // sinon la case sous la souris (std::nullopt s'il n'y en a pas)
        std::optional<core::GridPos> AttackTarget() const;

        // Dessine la partie, la barre d'attaques et l'aide
        void Draw() const;

        // Renvoie le dossier des sprites (copié à côté de l'exécutable par CMake)
        static std::filesystem::path SpritesDirectory();

        // Ordre important : la fenêtre est créée avant le renderer (textures) et détruite après
        Window window;
        std::uint32_t seed;
        core::Game game;
        Renderer renderer;
        std::optional<core::GridPos> hoveredTile;        // Case praticable sous la souris
        std::optional<std::size_t> hoveredButton;        // Bouton d'attaque sous la souris
        std::optional<std::size_t> selectedAttack;       // Attaque en cours de visée
    };

}
