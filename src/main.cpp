// Point d'entrée du jeu Rogue Bike : crée l'application et lance la boucle principale.
import std;
import ui.application;

// Lance le jeu. Renvoie 0 en cas de sortie normale, 1 en cas d'erreur fatale.
int main() {
    try {
        auto application = ui::Application{};
        application.Run();
    } catch (const std::exception& exception) {
        // Message générique : aucune information interne n'est exposée à l'utilisateur
        std::cerr << "Erreur fatale : " << exception.what() << '\n';
        return 1;
    }
    return 0;
}
