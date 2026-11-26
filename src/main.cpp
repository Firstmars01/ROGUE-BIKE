// Point d'entrée de Rogue Bike : crée l'application et lance la boucle principale.
import std;
import ui.application;

// Renvoie 0 en sortie normale, 1 en cas d'erreur fatale
int main() {
    try {
        auto application = ui::Application{};
        application.Run();
    } catch (const std::exception& exception) {
        // Message court, sans détail interne
        std::cerr << "Erreur fatale : " << exception.what() << '\n';
        return 1;
    }
    return 0;
}
